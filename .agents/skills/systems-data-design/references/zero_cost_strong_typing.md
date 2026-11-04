# 零成本强类型与编译期类型状态机 (Typestate)

> **理论源泉**：现代 C++ 零成本抽象标准、Rust 类型系统架构、《C++ Core Guidelines》

---

## 1. 强标量类型（Strong Types / Newtype Pattern）

### 1.1 痛点：原始类型偏执与参数混淆
在系统编程中，大量底层概念（文件描述符、端口号、会话 ID、内存偏移、时间量）往往被简单定义为裸类型（`int`, `uint64_t`）。
```cpp
// ❌ 脆弱接口：调用者把 offset 和 length 传反，或者把 user_id 和 session_id 传反，编译期完全静默！
void read_block(int fd, uint64_t offset, uint64_t length, uint64_t user_id);
```

### 1.2 零成本强类型包装器
使用空的幽灵标签（Tag Struct）配合轻量模板，实现**类型安全隔离**。
在启用编译器优化（-O2/-O3）后，该结构体在汇编层面会完全退化为裸整数，**零额外 CPU 指令、零额外内存开销**！

```cpp
template <typename Tag, typename T>
class StrongType {
public:
    explicit constexpr StrongType(T val) : value_(val) {}
    constexpr T get() const noexcept { return value_; }

    // 默认支持同类型比较
    constexpr bool operator==(const StrongType& rhs) const noexcept { return value_ == rhs.value_; }
    constexpr bool operator!=(const StrongType& rhs) const noexcept { return value_ != rhs.value_; }
private:
    T value_;
};

// 业务强类型声明（每个类型拥有独特的 Tag）
using UserId = StrongType<struct UserIdTag, uint64_t>;
using SessionId = StrongType<struct SessionIdTag, uint64_t>;
using ByteOffset = StrongType<struct ByteOffsetTag, uint64_t>;
using ByteLength = StrongType<struct ByteLengthTag, uint64_t>;

//  类型强安全签名：任何参数顺序颠倒均会引发编译报错！
void read_block(int fd, ByteOffset offset, ByteLength length, UserId user);
```

---

## 2. 编译期类型状态机 (Typestate Pattern)

### 2.1 传统状态机的缺陷
传统代码通常在对象内部维护一个运行时枚举状态 `state_`：
```cpp
class Socket {
public:
    void send(const void* buf, size_t len) {
        if (state_ != State::Connected) {
            throw std::runtime_error("Socket not connected!"); // 运行时异常！
        }
        // ...
    }
private:
    State state_{State::Disconnected};
};
```
* **代价**：每个方法内部都充斥着运行时的状态检查分支；一旦调用者遗漏检查，错误只能在生产环境运行时爆发。

### 2.2 优雅实践：将状态编码进类型系统中
通过所有权转移（Move Semantics）与类型转换，**让不合法的调用在编译期无法通过语法检查**！

```cpp
// 状态标记
struct DisconnectedState {};
struct ConnectedState {};

template <typename State>
class NetworkChannel;

// 1. 未连接状态下的 Channel：只允许调用 connect()
template <>
class NetworkChannel<DisconnectedState> {
public:
    explicit NetworkChannel(std::string remote_addr) : addr_(std::move(remote_addr)) {}

    // connect 成功后消耗自身所有权，返回 ConnectedState 的新 Channel
    NetworkChannel<ConnectedState> connect() && {
        // 执行底层握手逻辑...
        return NetworkChannel<ConnectedState>(std::move(addr_));
    }
private:
    std::string addr_;
};

// 2. 已连接状态下的 Channel：才拥有 send() 与 close() 方法！
template <>
class NetworkChannel<ConnectedState> {
public:
    void send(std::span<const uint8_t> data) {
        // 无需判断是否连接，类型系统本身就是最高保证！
    }

    NetworkChannel<DisconnectedState> close() && {
        // 执行挥手逻辑...
        return NetworkChannel<DisconnectedState>(std::move(addr_));
    }
private:
    friend class NetworkChannel<DisconnectedState>;
    explicit NetworkChannel(std::string addr) : addr_(std::move(addr)) {}
    std::string addr_;
};
```

#### 编译期守护效果：
```cpp
NetworkChannel<DisconnectedState> channel("127.0.0.1:8080");
// channel.send(data); // ❌ 编译直接报错！DisconnectedState 根本没有 send 成员函数！

auto connected = std::move(channel).connect();
connected.send(data); //  合法安全调用！
```
