#!/usr/bin/env python3
"""
Karl Wiegers Requirements Smell Detector & Ambiguity Linter
Scans requirements documents (Markdown, text) against Karl Wiegers' Table 11-2
ambiguity smell dictionary and ISO/IEC/IEEE 29148 rules.
"""

import sys
import re
import argparse
from dataclasses import dataclass
from typing import List, Pattern

@dataclass
class SmellRule:
    rule_id: str
    category: str
    pattern: Pattern
    description: str
    suggestion: str
    severity: str  # "ERROR", "WARNING", "INFO"

SMELL_RULES: List[SmellRule] = [
    # 1. Subjective Usability Smells
    SmellRule(
        "SMELL-01", "主观可用性 (Subjective Usability)",
        re.compile(r'\b(user[- ]friendly|simple|easy|straightforward|intuitive)\b|用户友好|简单易用|直观|界面简洁', re.I),
        "使用主观性修饰词，无法被客观验证或编写自动化断言。",
        "替换为可量化的可用性指标（如任务完成时间、独立操作所需培训时长、SUS 得分）。",
        "ERROR"
    ),
    # 2. Vague Performance Smells
    SmellRule(
        "SMELL-02", "模糊性能 (Vague Performance)",
        re.compile(r'\b(fast|quick|rapid|efficient|high[- ]performance|timely)\b|快速|高效|及时|高性能|迅速', re.I),
        "模糊的速度或性能描述，不同干系人标准迥异。",
        "使用 Planguage 或给出明确度量（如 P99 响应延迟 ≤ 500ms，并发吞吐量 ≥ 1000 TPS）。",
        "ERROR"
    ),
    # 3. Open-Ended Boundaries
    SmellRule(
        "SMELL-03", "开放式边界与穷举遗漏 (Open-Ended List)",
        re.compile(r'\b(etc\.?|and so on|and the like|such as|including but not limited to)\b|等等|及其他|包括但不限于|诸如此类', re.I),
        "开放式边界导致测试范围无法封闭，容易引发范围争议与实现遗漏。",
        "穷举全部合法枚举值，或显式引用受控配置列表/标准清单。",
        "ERROR"
    ),
    # 4. Vague Verbs
    SmellRule(
        "SMELL-04", "空洞动词 (Vague Action Verbs)",
        re.compile(r'\b(support|enable|handle|manage|process)\b(?:\s+the)?|支持|允许|处理|搞定', re.I),
        "动词语义过于宽泛，未说明具体系统动作（创建、校验、存储、传输还是展示？）。",
        "拆解为具体功能动词（如解析、校验、持久化存储、广播事件通知）。",
        "WARNING"
    ),
    # 5. Non-Enforceable Modal Verbs
    SmellRule(
        "SMELL-05", "含糊情态动词 (Ambiguous Modal Verbs)",
        re.compile(r'\b(should|would|might|could|ought to)\b|应该|最好|尽量|需', re.I),
        "使用 'should' / '应该' 带有协商与非强制色彩，导致交付边界产生纠纷。",
        "强制功能统一使用 '系统应 (shall)'；可选合法权利使用 '用户可 (may)'。",
        "WARNING"
    ),
    # 6. Ambiguous Connectors
    SmellRule(
        "SMELL-06", "歧义连词与斜杠 (Ambiguous Conjunctions)",
        re.compile(r'\b(and/or)\b|和/或|/ (?![\w/])', re.I),
        "'and/or' 或斜杠让读者自行猜测是并集、交集还是三者皆可。",
        "拆解为独立的明晰陈述，明确是单独 A、单独 B 还是 A 与 B 联合输入。",
        "WARNING"
    ),
    # 7. Unspecified Exceptions
    SmellRule(
        "SMELL-07", "未定义的常规与异常 (Unspecified Conditions)",
        re.compile(r'\b(normally|ideally|in most cases|generally|almost always)\b|通常|一般而言|正常情况下|原则上', re.I),
        "暗示存在异常分支，但未定义非正常情况下的系统行为。",
        "补全对称的异常分支处理逻辑（若处于非正常条件，系统应执行何种操作）。",
        "ERROR"
    ),
    # 8. Subjective Judgments
    SmellRule(
        "SMELL-08", "主观裁量权 (Subjective Judgments)",
        re.compile(r'\b(reasonable|appropriate|as practicable|if possible|where applicable)\b|合理|适时|酌情|视情况|可行时', re.I),
        "制造了判定真空，未明确判定主体是谁、依据何种算法或阈值判定。",
        "给出具体的客观判定准则与数值上下限。",
        "ERROR"
    ),
    # 9. Vague Quantifiers
    SmellRule(
        "SMELL-09", "模糊数量词 (Vague Quantifiers)",
        re.compile(r'\b(several|multiple|many|few|numerous)\b|若干|多个|许多|少量', re.I),
        "数量范围模糊，无法指导数据库容量规划与压测边界设计。",
        "明确数值范围区间（如 1 到 50 个，最多不超过 100 个）。",
        "WARNING"
    ),
    # 10. Unresolved Placeholders
    SmellRule(
        "SMELL-10", "未解占位符与遗留项 (Unresolved Placeholders)",
        re.compile(r'\b(TBD|TODO|FIXME|WIP)\b|待定|待商榷|待确认', re.I),
        "需求基线中残留待定占位符，严重阻塞下游设计与编码。",
        "在基线冻结前召开评审彻底定稿，或拆分移入后续版本。",
        "ERROR"
    ),
]

def scan_file(file_path: str):
    issues = []
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for line_idx, line in enumerate(lines, 1):
        line_clean = line.strip()
        # Skip pure code block markers or empty lines
        if not line_clean or line_clean.startswith("```") or line_clean.startswith("#"):
            continue

        for rule in SMELL_RULES:
            match = rule.pattern.search(line)
            if match:
                matched_str = match.group(0)
                issues.append({
                    "line": line_idx,
                    "matched": matched_str,
                    "rule_id": rule.rule_id,
                    "category": rule.category,
                    "desc": rule.description,
                    "suggestion": rule.suggestion,
                    "severity": rule.severity,
                    "snippet": line_clean[:120]
                })
    return issues

def print_report(issues, file_path):
    print(f"\n========================================================")
    print(f" 需求歧义与坏味道静态检测报告 (Karl Wiegers Requirements Linter)")
    print(f" 检查目标文件: {file_path}")
    print(f" 发现潜在问题数: {len(issues)}")
    print(f"========================================================\n")

    if not issues:
        print("恭喜！未检测到 Table 11-2 经典需求歧义词汇与格式缺陷。\n")
        return

    errors = [i for i in issues if i["severity"] == "ERROR"]
    warnings = [i for i in issues if i["severity"] == "WARNING"]

    print(f"统计概览: {len(errors)} 个严重阻断错误 (ERROR), {len(warnings)} 个警告项 (WARNING)\n")

    for issue in issues:
        tag = f"[{issue['severity']}] {issue['rule_id']} - {issue['category']}"
        print(f"Line {issue['line']}: {tag}")
        print(f"  命中坏味道: \"{issue['matched']}\"")
        print(f"  代码上下文: {issue['snippet']}")
        print(f"  原因诊断: {issue['desc']}")
        print(f"  重构建议: {issue['suggestion']}")
        print("-" * 56)

def main():
    parser = argparse.ArgumentParser(description="Karl Wiegers Requirements Smell Detector")
    parser.add_argument("file", help="Path to requirements document (.md, .txt)")
    args = parser.parse_args()

    issues = scan_file(args.file)
    print_report(issues, args.file)
    sys.exit(1 if any(i["severity"] == "ERROR" for i in issues) else 0)

if __name__ == "__main__":
    main()
