# 代码坏味道与重构手法速查映射表 (Smell to Refactoring Map)

| 坏味道 (Bad Smell) | 推荐优先手法 (First Choice) | 次选与深度手法 (Alternative) |
| :--- | :--- | :--- |
| **3.1 重复代码 (Duplicated Code)** | 提炼函数 (Extract Method 6.1) | 函数上移 (Pull Up Method 11.2)、塑造模板函数 (Form Template Method 11.10) |
| **3.2 过长函数 (Long Method)** | 提炼函数 (Extract Method 6.1) | 以查询取代临时变量 (Replace Temp with Query 6.4)、以函数对象取代函数 (Replace Method with Method Object 6.8) |
| **3.3 过大的类 (Large Class)** | 提炼类 (Extract Class 7.3) | 提炼子类 (Extract Subclass 11.6)、提炼接口 (Extract Interface 11.8) |
| **3.4 过长参数列 (Long Parameter List)** | 引入参数对象 (Introduce Parameter Object 10.9) | 保持对象完整 (Preserve Whole Object 10.7)、以函数取代参数 (Replace Parameter with Methods 10.8) |
| **3.5 发散式变化 (Divergent Change)** | 提炼类 (Extract Class 7.3) | 搬移函数 (Move Method 7.1) |
| **3.6 霰弹式修改 (Shotgun Surgery)** | 搬移函数 (Move Method 7.1) / 搬移字段 (7.2) | 将类内联化 (Inline Class 7.4) |
| **3.7 依恋情结 (Feature Envy)** | 搬移函数 (Move Method 7.1) | 提炼函数 (Extract Method 6.1) 并搬移 |
| **3.8 数据泥团 (Data Clumps)** | 提炼类 (Extract Class 7.3) | 引入参数对象 (Introduce Parameter Object 10.9) |
| **3.9 基本类型偏执 (Primitive Obsession)** | 以对象取代数据值 (Replace Data Value with Object 8.2) | 以类取代类型码 (8.13)、以State/Strategy取代类型码 (8.15) |
| **3.10 条件分支惊悚 (Switch Statements)** | 以多态取代条件表达式 (Replace Conditional with Polymorphism 9.6) | 以显式函数取代参数 (10.6)、引入Null对象 (9.7) |
| **3.11 平行继承体系 (Parallel Inheritance)** | 搬移函数 (Move Method 7.1) / 搬移字段 (7.2) | 折叠继承体系 (Collapse Hierarchy 11.9) |
| **3.12 冗赘类 (Lazy Class)** | 将类内联化 (Inline Class 7.4) | 折叠继承体系 (Collapse Hierarchy 11.9) |
| **3.13 夸夸其谈未来性 (Speculative Generality)** | 折叠继承体系 (Collapse Hierarchy 11.9) | 将类内联化 (7.4)、移除参数 (10.3) |
| **3.14 临时字段 (Temporary Field)** | 提炼类 (Extract Class 7.3) | 引入Null对象 (Introduce Null Object 9.7) |
| **3.15 过度耦合消息链 (Message Chains)** | 隐藏委托关系 (Hide Delegate 7.5) | 提炼函数 (6.1) 并搬移 (7.1) |
| **3.16 中间人 (Middle Man)** | 移除中间人 (Remove Middle Man 7.6) | 内联函数 (Inline Method 6.2) |
| **3.17 狎昵关系 (Inappropriate Intimacy)** | 搬移函数/字段 (Move Method/Field 7.1/7.2) | 将双向关联改为单向 (8.8)、隐藏委托 (7.5) |
| **3.18 异曲同工的类 (Alternative Classes)** | 函数改名 (Rename Method 10.1) | 搬移函数 (7.1)、提炼超类 (11.7) |
| **3.19 不完美库类 (Incomplete Library Class)** | 引入外加函数 (Introduce Foreign Method 7.7) | 引入本地扩展 (Introduce Local Extension 7.8) |
| **3.20 纯稚数据类 (Data Class)** | 搬移函数 (Move Method 7.1) | 封装字段 (Encapsulate Field 8.10)、封装集合 (8.11) |
| **3.21 被拒绝的遗赠 (Refused Bequest)** | 以委托取代继承 (Replace Inheritance with Delegation 11.11) | 下移方法/字段 (Push Down Method/Field 11.4/11.5) |
| **3.22 过多的注释 (Comments)** | 提炼函数 (Extract Method 6.1) | 引入断言 (Introduce Assertion 9.8)、函数改名 (10.1) |
