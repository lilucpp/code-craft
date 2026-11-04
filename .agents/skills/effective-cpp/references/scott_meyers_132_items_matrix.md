# Scott Meyers 132 条款全景演进映射表 (经典三部曲)

> 涵盖 Scott Meyers C++ 著作三部曲全集：
> 1. 《Effective C++ (第3版)》(55 条)
> 2. 《More Effective C++》(35 条)
> 3. 《Effective Modern C++》(42 条)
> 共计 132 条经典法则索引与现代 C++ (C++11/14/17/20) 演进升级对照表。

---

## 1. 第一部：《Effective C++ (第3版)》55 条速查矩阵

| 章节 | 条款编号与核心观点 | 现代 C++ 演进建议 |
| :--- | :--- | :--- |
| **一、习惯 C++** | **1.** 视 C++ 为语言联邦<br/>**2.** 用 `const`, `enum`, `inline` 取代 `#define`<br/>**3.** 只要可能就用 `const`<br/>**4.** 确保对象在使用前被初始化 | 优先使用 `constexpr` 与 `consteval`；使用花括号初始化 `{}` 彻底杜绝未初始化；`const` 具有多线程读安全契约。 |
| **二、构造/析构/赋值** | **5.** 了解 C++ 默默编写和调用的函数<br/>**6.** 若不想使用编译器生成函数，就明确拒绝<br/>**7.** 多态基类声明 `virtual` 析构函数<br/>**8.** 别让异常逃离析构函数<br/>**9.** 绝不在构造和析构期间调用虚函数<br/>**10.** `operator=` 返回 `*this` 引用<br/>**11.** 在 `operator=` 中处理自我赋值<br/>**12.** 复制对象时勿忘其每一个成分 | 条款 5/6 升级为 `= default` 与 `= delete`；引入 Rule of 0/3/5；析构函数默认隐式 `noexcept`。 |
| **三、资源管理** | **13.** 以对象管理资源 (RAII)<br/>**14.** 在资源管理类中小心 copying 行为<br/>**15.** 在资源管理类中提供对原始资源的访问<br/>**16.** 成对使用相同形式的 `new` 和 `delete`<br/>**17.** 以独立语句将 new 出来的对象置入智能指针 | 彻底淘汰 `std::auto_ptr`；全面采用 `std::unique_ptr` 与 `std::shared_ptr`；条款 17 升级为 `make_unique` / `make_shared`。 |
| **四、设计与声明** | **18.** 让接口容易被正确使用，不易被误用<br/>**19.** 视 class 设计为 type 设计<br/>**20.** 宁以 pass-by-reference-to-const 替换 pass-by-value<br/>**21.** 必须返回对象时，别妄想返回引用<br/>**22.** 将成员变量声明为 `private`<br/>**23.** 宁以 non-member、non-friend 替换 member 函数<br/>**24.** 若所有参数皆需类型转换，请采用 non-member 函数<br/>**25.** 考虑写出一个不抛异常的 swap 函数 | 移动成本极低且必拷贝时可按值传递（EMC++ 条款 41）；强类型枚举 `enum class` 强化接口；`swap` 必须标记 `noexcept`。 |
| **五、实现** | **26.** 尽可能延后变量定义式的出现时间<br/>**27.** 尽量少做转型动作<br/>**28.** 避免返回 handles 指向对象内部成分<br/>**29.** 为“异常安全”努力是值得的<br/>**30.** 透彻了解 inlining 的里里外外<br/>**31.** 将文件间的编译依存关系降至最低 | 采用 C++ 风格显式转型；`noexcept` 配合异常安全；Pimpl 惯用法结合 `std::unique_ptr` 降低编译依赖。 |
| **六、继承与面向对象** | **32.** 确定 public 继承塑模出 is-a 关系<br/>**33.** 避免遮掩继承而来的名称<br/>**34.** 区分接口继承和实现继承<br/>**35.** 考虑 virtual 函数以外的其他选择<br/>**36.** 绝不重新定义继承而来的 non-virtual 函数<br/>**37.** 绝不重新定义继承而来的缺省参数值<br/>**38.** 通过复合塑模出 has-a 或根据某物实现出<br/>**39.** 审慎使用 private 继承<br/>**40.** 审慎使用多重继承 | 条款 33 升级为现代 `using Base::func;` 与显式 `override` 关键字；策略模式可由现代 Lambda / `std::function` 优雅替代。 |
| **七、模板与泛型** | **41.** 了解隐式接口和编译期多态<br/>**42.** 了解 typename 的双重意义<br/>**43.** 学习处理模板化基类内的名称<br/>**44.** 将与参数无关的代码抽离 template<br/>**45.** 运用成员函数模板接受所有兼容类型<br/>**46.** 需要类型转换时请为模板定义非成员函数<br/>**47.** 请使用 traits classes 表现类型信息<br/>**48.** 认识 template metaprogramming (TMP) | 现代 C++ 引入别名模板、折叠表达式、`std::type_traits` 与 C++20 Concepts 大幅简化 TMP 门槛与提升编译器诊断。 |
| **八、定制 new/delete** | **49.** 了解 new-handler 的行为<br/>**50.** 了解何时置换 new 和 delete 合理<br/>**51.** 编写 new 和 delete 时需固守常规<br/>**52.** 写了 placement new 也要写 placement delete | 现代工程推荐使用标准库自定义 Allocator（如 PMR 多态内存资源）或 Arena 分配器，减少全局介入。 |
| **九、杂项** | **53.** 不要轻易忽视编译器警告<br/>**54.** 让自己熟悉包括 TR1 在内的标准库<br/>**55.** 让自己熟悉 Boost | TR1 与 Boost 大量优秀组件（智能指针、tuple、chrono、thread）已全量合入现代 C++ 标准库。 |

---

## 2. 第二部：《More Effective C++》35 条速查矩阵

| 章节 | 条款编号与核心观点 | 关联物理机制与现代演进 |
| :--- | :--- | :--- |
| **一、基础议题** | **1.** 仔细区别 pointers 和 references<br/>**2.** 最好使用 C++ 转型操作符<br/>**3.** 绝对不要多态化处理数组 (Never treat arrays polymorphically)<br/>**4.** 避免无意义的 default constructors | **条款 3 绝杀**：数组下标寻址依据基类步长 `ptr + i*sizeof(Base)`，派生类数组会导致严重指针错位与内存践踏；改用 `std::span`。 |
| **二、运算符** | **5.** 谨慎处理自定义转换函数<br/>**6.** 区分自增自减的前置与后置形式<br/>**7.** 千万不要重载 `&&`、`||` 或 `,`<br/>**8.** 了解各种不同意义的 new 和 delete | **条款 7 绝杀**：重载 `&&`/`||` 丧失语言级短路求值与顺序保证；条款 5 现代以 `explicit` 构造函数和 `explicit operator bool()` 防御。 |
| **三、异常** | **9.** 利用析构函数避免泄漏 (RAII)<br/>**10.** 在 constructors 内阻止资源泄漏<br/>**11.** 禁止异常流出 destructors 之外<br/>**12.** 理解 throw 异常与传参/调虚函数的差异<br/>**13.** 以 by reference 方式捕获异常<br/>**14.** 谨慎使用 exception specifications<br/>**15.** 了解异常处理的成本与代价值 | **条款 10/13 绝杀**：构造函数抛异常析构不被执行，成员必须用智能指针保护；捕获异常强制用引用 `catch (const std::exception& e)` 防对象切片；旧式规格说明升级为 `noexcept`。 |
| **四、效率** | **16.** 谨记 80-20 法则 (Profiler 驱动优化)<br/>**17.** 考虑使用 lazy evaluation (缓式求值)<br/>**18.** 分摊预期计算成本 (Amortize computation)<br/>**19.** 理解临时对象的来源<br/>**20.** 协助返回值优化 (RVO)<br/>**21.** 通过重载避免隐式类型转换<br/>**22.** 考虑使用 `op=` 代替独立操作符 `op`<br/>**23.** 考虑使用其他替代程序库<br/>**24.** 理解虚函数、多重继承、虚基类与 RTTI 的物理代价 | **条款 22**：成员 `+=` 零临时对象，非成员 `+` 基于 `+=` 实现；条款 20 在 C++17 升级为强制保证复制消除 (Guaranteed Copy Elision)。 |
| **五、经典技巧** | **25.** 将构造函数与非成员函数虚拟化 (Virtual Constructor)<br/>**26.** 限制某个 class 的对象产生数量<br/>**27.** 要求或禁止对象产生于 heap 之中<br/>**28.** 智能指针 (Smart Pointers 演进雏形)<br/>**29.** 引用计数 (Reference Counting)<br/>**30.** 替身类 (Proxy Classes)<br/>**31.** 让函数根据多个对象类型实现虚化 (Double Dispatch) | **条款 25**：虚拟拷贝构造演化为现代 `std::unique_ptr<Base> clone() const`；条款 28/29 演变为标准库 `unique_ptr`/`shared_ptr`；条款 31 对应现代访问者模式与 `std::visit`。 |
| **六、杂项** | **32.** 未来时态下编程 (Program in the future tense)<br/>**33.** 将非尾端类 (non-leaf) 设计为抽象类<br/>**34.** 理解如何在同一个程序中结合 C++ 与 C<br/>**35.** 让自己熟悉语言标准规范 | 条款 34 掌握 `extern "C"` 防名字修饰（Name Mangling）；条款 33 防公有派生类之间的对象切片赋值。 |

---

## 3. 第三部：《Effective Modern C++》42 条速查矩阵

| 章节 | 条款编号与核心观点 | 关联核心原理与禁忌 |
| :--- | :--- | :--- |
| **一、类型推导** | **1.** 理解模板类型推导<br/>**2.** 理解 auto 类型推导<br/>**3.** 理解 decltype<br/>**4.** 学会查看类型推导结果 | 指针/通用引用/传值三种推导规则；警惕 `auto x = {27}` 推导为 `initializer_list`；掌握 `decltype(auto)`。 |
| **二、auto** | **5.** 优先考虑 auto 而非显式类型声明<br/>**6.** auto 推导若非己愿，使用显式类型初始化惯用法 | 避免未初始化、避免复杂的迭代器声明；警惕 `vector<bool>::reference` 代理类，需使用 `static_cast<bool>`。 |
| **三、移步现代 C++** | **7.** 区别使用 () 和 {} 创建对象<br/>**8.** 优先考虑 nullptr 而非 0 和 NULL<br/>**9.** 优先考虑别名声明 (using) 而非 typedefs<br/>**10.** 优先考虑限域 enum (enum class)<br/>**11.** 优先考虑 deleted 函数而非未定义的私有声明<br/>**12.** 使用 override 声明重写函数<br/>**13.** 优先考虑 const_iterator 而非 iterator<br/>**14.** 如果函数不抛出异常请使用 noexcept<br/>**15.** 尽可能使用 constexpr<br/>**16.** 让 const 成员函数线程安全<br/>**17.** 理解特殊成员函数的生成 | `{}` 防窄化但贪婪匹配 `initializer_list`；`nullptr` 解决重载决议；`using` 支持模板；`noexcept` 解锁 STL 移动性能；五大件互斥生成矩阵。 |
| **四、智能指针** | **18.** 对于独占资源使用 std::unique_ptr<br/>**19.** 对于共享资源使用 std::shared_ptr<br/>**20.** 当 std::shared_ptr 可能悬空时使用 std::weak_ptr<br/>**21.** 优先考虑使用 std::make_unique 和 std::make_shared<br/>**22.** 使用 Pimpl 惯用法时，在实现文件中定义特殊成员函数 | `unique_ptr` 零开销；`make_shared` 合并控制块分配；`weak_ptr` 破除环状引用；Pimpl 析构必须在 cpp 实现防不完整类型报错。 |
| **五、右值与移动** | **23.** 理解 std::move 和 std::forward<br/>**24.** 区分通用引用与右值引用<br/>**25.** 对右值引用用 move，对通用引用用 forward<br/>**26.** 避免在通用引用上重载<br/>**27.** 熟悉通用引用重载的替代方法<br/>**28.** 理解引用折叠<br/>**29.** 假定移动操作不存在、成本高、未被使用<br/>**30.** 熟悉完美转发失败的情况 | move/forward 运行时不产生指令；通用引用需要推导；避免贪婪重载劫持拷贝；按值返回局部对象严禁 std::move 破坏 RVO。 |
| **六、Lambda 表达式** | **31.** 避免使用默认捕获模式 ([&] / [=])<br/>**32.** 使用初始化捕获移动对象到闭包中<br/>**33.** 对 auto&& 形参使用 decltype 以 std::forward 它们<br/>**34.** 考虑 lambda 而非 std::bind | 防局部引用悬空；防隐式捕获 this 指针引发野指针；C++14 广义捕获解决 move-only 类型捕获；全面弃用 `std::bind`。 |
| **七、并发 API** | **35.** 优先考虑基于任务的编程 (std::async)<br/>**36.** 若有异步必要请指定 std::launch::async<br/>**37.** 使 std::thread 在所有路径最后都不可结合<br/>**38.** 关注不同线程句柄的析构行为<br/>**39.** 对一次性事件通信考虑使用 void 的 futures<br/>**40.** 并发使用 std::atomic，特殊内存使用 volatile | `async` 自动传递异常与返回值；joinable 线程析构触发 terminate 终止进程；`volatile` 与多线程并发完全无关。 |
| **八、微调** | **41.** 移动成本低且总是被拷贝的形参，考虑按值传递<br/>**42.** 考虑使用置入 (emplace) 代替插入 (insert) | 单一按值传参代替 const& 和 && 两次重载；`emplace_back` 直接就地构造避免额外移动/拷贝。 |
