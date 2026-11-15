# 并发减法与生命期管理模式

提炼自《Linux 多线程服务端编程》第 1~5 章、第 8 章及 `muduo` 核心源码，重点解决多线程对象析构竞态、锁粒度控制与异步回调安全性。

---

## 1. 线程封闭与 `One Loop Per Thread`

- **核心原则**：能在一个线程内串行化的状态，绝不跨线程共享加锁。
- **架构约束**：
  - 每个 IO 线程至多运行一个 `EventLoop`（通过 `thread_local EventLoop*` 校验）；
  - 每个连接固定归属一个 IO 线程，所有读写回调均在该线程触发，内部状态（如输入/输出 `Buffer`）无需加锁；
  - 构造时记录所属 `threadId_`，关键写路径入口调用 `assertInLoopThread()`，把并发越界尽早暴露在开发/测试期。
- **跨线程任务转移（`runInLoop` / `queueInLoop`）**：
  - 同线程调用直接同步执行；跨线程调用则加锁入队 `pendingFunctors_`，并通过 `eventfd` 唤醒阻塞在 `epoll_wait` 的目标线程。
  - **关键细节**：`doPendingFunctors()` 先在短锁内把 `pendingFunctors_` `swap` 到局部 `std::vector`，**解锁后再逐个执行**。既避免持锁执行未知回调导致死锁，也避免阻塞其他线程入队。若在执行回调期间又有新任务入队，同样需要唤醒以免下一轮 `poll` 阻塞。

---

## 2. 极简同步原语与 Copy-on-Write

- **为什么禁用递归互斥锁**：同一线程重入加锁往往掩盖“外层持锁遍历/修改容器，内层回调又修改同一容器”导致的迭代器失效 Bug；非递归锁会直接死锁，便于在 `gdb` 下通过线程调用栈精准定位。若内部函数需在“已持锁”与“未持锁”两种上下文复用，应拆分为 `func()`（加锁后调内部实现）与 `funcInLock()`（断言/注解要求已持锁）。
- **为什么用 `std::shared_ptr` CoW 替代读写锁**：
  - 读写锁在读临界区极小时维护读者计数的开销可能高于普通 `mutex`，且存在写饥饿与读锁升级死锁隐患。
  - **CoW 模式**：
    - **读端**：短锁内拷贝一份局部 `std::shared_ptr<const T>`（引用计数 +1），立即解锁并在锁外无锁读取，多个读线程互不阻塞；
    - **写端**：全程加锁，检查 `ptr.use_count() == 1`（C++17 已废弃 `unique()`；**注意：仅当所有读写者均受同一锁保护、且外部不存在指向该对象的 `weak_ptr` 时才可据此原地修改**，否则可能被 `weak_ptr::lock()` 并发提升计数，应一律构造新副本替换），若仍有读线程持有旧版本，则拷贝生成新对象修改后 `swap` 替换全局指针；旧对象由最后一个离开作用域的读线程自动释放，不阻塞写端。

---

## 3. 双缓冲（Double Buffering）异步日志模式

适用于高频日志或监控数据上报，核心目标是**前端业务线程不阻塞在磁盘/网络 IO 上，且避免逐条传递带来的堆分配**：

1. **结构**：前端持有 `currentBuffer_`（固定大小栈/预分配块，如 4 MiB）与预备 `nextBuffer_`，以及待写队列 `buffers_`；后端专用线程自备 `newBuffer1`、`newBuffer2` 与 `buffersToWrite`。
2. **前端写入**：持短锁检查 `currentBuffer_` 剩余空间；足够则直接 `memcpy` 追加；写满则 `std::move` 进 `buffers_`，将 `nextBuffer_` 切换为当前缓冲并唤醒后端。
3. **后端落盘**：等待超时（如 3 秒）或被唤醒后，持锁将 `currentBuffer_` 与 `buffers_` 全部 `swap` 进 `buffersToWrite`，并用 `newBuffer1`/`newBuffer2` 补位前端缓冲；**出锁后**批量写盘并回收前两块缓冲复用，多余突发缓冲释放；队列堆积超过阈值时主动丢弃防内存耗尽。

---

## 4. 对象生命期与异步回调守卫

### 4.1 跨线程销毁延迟模式（`TcpConnection`）
当连接断开、从管理表 `connections_` 移除时，当前可能正处于 `Channel::handleEvent()` 调用栈深处。若直接销毁对象会导致正在执行的成员函数访问已释放内存：
- 移除映射后，将持有 `std::shared_ptr<TcpConnection>` 的销毁任务投递回该连接所属 `EventLoop` 的队列（`queueInLoop`），利用任务闭包的值捕获把连接寿命延长到当前事件处理完成之后再安全析构。

### 4.2 弱引用守卫（`Channel::tie` / `WeakCallback`）
对于“不拥有宿主、仅在宿主存活时才执行”的异步回调（如定时器触发、外部事件回调）：

```cpp
// C++17 弱回调（WeakCallback）惯用写法
template <typename Class, typename... Args>
auto makeWeakCallback(const std::shared_ptr<Class>& obj,
                      void (Class::*func)(Args...)) {
  return [wk = std::weak_ptr<Class>(obj), func](Args... args) {
    if (auto sp = wk.lock()) {
      (sp.get()->*func)(std::forward<Args>(args)...);
    }
  };
}
```

- **原则**：绝不把 `shared_from_this()` 直接强绑定进长期存在的定时器或注册表回调（会意外延长对象生命期甚至造成内存泄漏），统一转为 `std::weak_ptr` 后在调用瞬间 `lock()` 续命。
- **对象池清理**：若工厂/对象池需在对象销毁时自动从 `std::unordered_map` 抹除键，可给 `std::shared_ptr` 绑定定制删除器，删除器内部持有工厂的 `std::weak_ptr`，`lock()` 成功时再加锁擦除对应条目。
