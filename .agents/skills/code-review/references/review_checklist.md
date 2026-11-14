# Code Review 快速核查清单 (Checklist)

在执行代码审查时，按以下维度进行逐项快速比对：

---

## 1. 包与组件依赖架构层面 (Package & Component Level)
- [ ] **无环依赖 (ADP)**：模块/包之间是否存在循环依赖（`A -> B -> C -> A`）？若有，是否通过 DIP 接口倒置或抽取公共包解开？
- [ ] **共同封闭 (CCP)**：新增业务变动时，修改点是否集中在单一包内？是否存在跨多个包的霰弹式修改？
- [ ] **共同重用 (CRP)**：包内是否存在互不相关的工具类被强行打包，迫使调用方为不需要的依赖买单？
- [ ] **惯例优于配置 (CoC)**：项目是否使用了符合行业公认的目录规范与约定，消除了不必要的胶水配置文件？

---

## 2. 接口契约与面向对象设计层面 (Interface & Contract Level)
- [ ] **开闭原则 (OCP)**：面对新增业务类型时，是否通过扩展策略/多态实现？是否违规直接修改已稳定的核心调度类？
- [ ] **接口隔离 (ISP)**：接口是否过于臃肿（胖接口）？实现类是否存在大量留空的无意义方法或抛出“不支持操作”的哑方法？
- [ ] **契约式设计 (DbC)**：
  - **前置条件**：入参边界是否在入口处明确校验并快速失败（Fail-Fast）？
  - **后置承诺**：返回数据是否具有明确可信的契约（例如约定永不返回 null 而是空集合）？
- [ ] **发散式变化 (Divergent Change)**：类是否因不同方向的业务需求而频繁修改？
- [ ] **依恋情结 (Feature Envy)**：方法是否过度访问其他类的数据而不是自己类？
- [ ] **过度耦合消息链 (Message Chains)**：是否有 `a.b().c().d()` 违背迪米特法则？
- [ ] **被拒绝的遗赠 (Refused Bequest)**：子类是否只复用父类的少量方法，甚至重写抛出异常？（应改继承为委托）
- [ ] **夸夸其谈未来性 (Speculative Generality)**：是否存在无实际调用、空想未来的抽象层和参数？

---

## 3. 函数与日常编码逻辑层面 (Method & Logic Level)
- [ ] **短小与单一抽象层 (SLAP)**：单函数是否在 20~30 行以内？内部语句是否处于同一抽象级别？
- [ ] **重复代码 (Duplicated Code)**：是否有同构或高度相似的代码块？
- [ ] **过长参数列 (Long Parameter List)**：参数是否超过 3 个？是否需要引入参数对象 (Parameter Object)？
- [ ] **深层嵌套分支**：是否可以使用卫语句 (Guard Clauses) 提前退出，消除多层缩进？
- [ ] **基本类型偏执 (Primitive Obsession)**：是否滥用 string/int 承载强类型业务概念（如金额、手机号）？
- [ ] **I/O 分流与缄默原则**：`stdout` 是否只承载机器数据流？日志与诊断信息是否严格输出到 `stderr`？
- [ ] **注释除臭 (Comments)**：注释是否在解释一段糟糕代码的逻辑？（应提取自解释函数，注释仅记录“为什么”）。

---

## 4. 全局系统与并发安全回归层面 (Global Systems & Concurrency Level)
- [ ] **数据竞争与同步归属 (Data Race Defense)**：
  - 所有跨线程共享的可变状态（含标志位、计数器、缓存容器）是否均受同一把互斥锁或 `std::atomic` 保护？是否消除了跨线程裸 `bool`/普通变量读写？
  - 涉及并发逻辑的变更是否通过 `ThreadSanitizer (-fsanitize=thread)` 验证无数据竞争？
- [ ] **锁边界与 Check-Then-Act 防御 (Lock Scope & TOCTOU)**：
  - 是否存在“锁内取指针/引用/迭代器，锁外解引用”的逃逸问题？（容器扩容或析构将直接引发 UAF）
  - 是否存在“加锁检查 $\rightarrow$ 解锁 $\rightarrow$ 再加锁修改”的 Check-Then-Act 竞态？
- [ ] **条件变量与线程生命周期 (Condition & Thread Lifetime)**：
  - `condition_variable::wait` 是否强制配合谓词（`cv.wait(lk, pred)`）防止虚假唤醒与信号丢失？
  - 后台线程/回调是否可能访问已析构的宿主对象（`this` 悬空）？线程退出前是否保证 `join`（或使用 `std::jthread`）？
- [ ] **不可重入锁与自死锁防御 (Non-recursive Mutex Defense)**：
  - 是否消除了递归锁？公有加锁函数内部是否违规调用了其它加锁公有接口？
  - 内部方法是否规范沉降为无锁纯虚/私有实现（如 `_Unlocked` 系列）？
- [ ] **全局跨模块锁时序 (Lock Hierarchy)**：
  - 涉及多把锁时，全局加锁顺序是否单向保序（如 `L1 -> L2 -> L3`），是否存在逆向加锁导致的 AB-BA 死锁？
- [ ] **成熟三方库全局推广与旧轮子淘汰 (Library Standardization)**：
  - 引入优秀三方库后，全局是否彻底清除同类手写轮子与反模式（如多线程裸 `fork` / `popen` / `system` 统一收敛至 `reproc++`）？
- [ ] **环境寻址确定性 (Deterministic Resolution)**：
  - 辅助进程或关键配置文件寻址是否仅基于当前可执行文件同级目录（`/proc/self/exe`）或显式传参？是否杜绝了脆弱的相对工作目录猜测（CWD Hijacking）？
- [ ] **全量回归铁律 (Full Regression Gate)**：
  - 是否对所有测试套件执行了全量自动化回归？
  - 测试命令是否按任务量级配置了弹性超时熔断（非死板静态时间）？
  - 是否确认全量测试 100% PASS（除非用户明确指定跳过全量测试）？

---

## 5. 国际化与硬编码字符串层面 (i18n & Hardcoded String Level)
- [ ] **UI 文案零硬编码 (Zero Hardcoded UI Strings)**：
  - 是否消除了表现层中裸写的字符串常量（如 `QMessageBox::critical(..., "中文标题", "中文文案")`）？
  - 面向用户的窗口标题、对话框、提示信息、按钮文本是否全部收敛至国际化翻译中心（如 `soeasy::gui::Tr("key")`）？
- [ ] **双语字典对称性 (Dictionary Symmetry)**：
  - 新增翻译词条是否在中英文字典中均成对补充？是否存在英文模式下缺失回退或乱码问题？
- [ ] **参数化占位符规范 (Parameterized Messages)**：
  - 动态文本是否规范使用了 `%1`, `%2` 标准占位符（`.arg(...)`），杜绝因语言语法差异导致字符串裸拼接语序错乱？

---

## 6. 边界条件与故障路径层面 (Boundary Conditions & Failure Paths)
- [ ] **空值与零规模边界 (Null & Zero-Size)**：
  - 是否处理了 `nullptr`、空字符串 `""`、空容器（`empty()`）及 `size == 0`？
  - 是否有 `size - 1`、`end - 1` 类无符号减法在 `size == 0` 时下溢为 `SIZE_MAX` 的风险？
- [ ] **Off-by-One 与区间开闭 (Interval & Index Bounds)**：
  - 循环与下标判断 `<` / `<=` 是否准确？字符串是否预留了 `\0` 终结符？半开区间 `[begin, end)` 是否存在尾元素越界？
- [ ] **极值与算术边界 (Numeric Extremes & Overflow)**：
  - 是否验证了 `0`、`-1`、`INT_MIN`（取负溢出）、`INT_MAX` / `SIZE_MAX` 算术溢出与截断？
  - 是否消除了有符号 `int` 与无符号 `size_t` 的直接混用比较？除数与模数是否为零已前置校验？
- [ ] **I/O、超时与异常早退边界 (I/O, Timeout & Error Paths)**：
  - `read`/`write`/`recv`/`send` 是否处理了部分读写（short read/write）与 `EINTR` 中断？
  - 超时参数为 `0`、负值或极大值时行为是否明确？错误/异常早退分支是否正确释放了已申请资源并保持对象状态一致？
