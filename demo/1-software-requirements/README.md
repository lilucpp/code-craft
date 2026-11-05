# Software Requirements 工业级需求工程验证范例 (FileGate)

本目录展示了运用 **`software-requirements` 技能**（基于需求工程权威 Karl Wiegers《软件需求》第3版与 ISO/IEC/IEEE 29148:2018 工业级标准）输出的标准工程需求规格范例。

---

## 目录文件概览

| 文件 | 说明 |
| :--- | :--- |
| **[`filegate_requirements_spec.md`](./filegate_requirements_spec.md)** | 《企业级文件安全外发与防泄漏审计平台 (FileGate)》完整需求规格说明书 (Final v2.0) |
| **[`requirements_engineering_process_and_refinement.md`](./requirements_engineering_process_and_refinement.md)** | **需求全生命周期过程复盘报告**：意图下钻生成推导、初稿坏味道微调过程与双重验证 |
| **[`run_lint_check.sh`](./run_lint_check.sh)** | 自动化需求歧义机检脚本（基于 Karl Wiegers Table 11-2 坏味道词典） |

---

## 核心设计标准与规范落地

1. **业务痛点下钻 (SIBV)**：
   - 杜绝直接陷入 UI 或技术选型，严格按照 `Situation -> Problem -> Implication -> Benefit -> Vision` 剖析商业价值；
   - 明确划定 **3 条排除范围 (Out-of-Scope)**，从源头阻断范围蔓延 (Scope Creep)。
2. **需求三层级与用户代言机制**：
   - 严谨分离业务需求 (BR)、用户需求 (UR/用例)、系统功能需求 (FR)；
   - 确立产品代言人 (Product Champion) 与唯一死锁打破人 (Deadlock Breaker)。
3. **用例与敏捷 BDD 验收准则**：
   - 用例包含主成功流、分支扩展流（敏感阻断、时效越界截断、主管驳回）与后置条件；
   - 敏捷故事提供 3 组严谨的 `Given - When - Then` (Gherkin) 验收场景，测试人员可直接转化为自动化测试用例。
4. **功能需求 100% 主动强约束语态**：
   - 彻底铲除“应该 (should)”、“用户可以”等模糊句式，全部采用“**系统应 (The system shall [动词] [对象])**”。
5. **业务规则独立目录化 (Rules Catalog)**：
   - 将密级矩阵 (事实)、有效期上限 (约束)、审批流转 (动作触发)、动态水印 (计算) 从主流程剥离，独立编号与解耦。
6. **非功能需求 Planguage 形式化量化**：
   - 彻底消灭“响应迅速、高可靠性”等空洞口号，采用 Tom Gilb's Planguage 语法（`SCALE / METER / MUST / PLAN / FAIL`）对下载时延与审计零丢失率进行工程化度量。

---

## 本地一键静态机检验证

在终端中执行：
```bash
cd demo/1-software-requirements
./run_lint_check.sh
```
机检将自动扫描 Table 11-2 中的 8 种歧义坏味道，验证达成 **0 ERROR 违规**。
