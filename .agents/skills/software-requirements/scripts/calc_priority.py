#!/usr/bin/env python3
"""
Karl Wiegers Analytical Requirements Prioritization Calculator
Based on "Software Requirements, 3rd Edition" (Karl Wiegers & Joy Beatty), Chapter 16.

Formula:
  Total Value = (Benefit * W_benefit) + (Penalty * W_penalty)
  Value % = Total Value / sum(Total Value) * 100
  Cost % = (Cost * W_cost) / sum(Cost * W_cost) * 100
  Risk % = (Risk * W_risk) / sum(Risk * W_risk) * 100
  Priority = Value % / (Cost % + Risk %)
"""

import sys
import json
import csv
import argparse

def calculate_priorities(items, w_benefit=2.0, w_penalty=1.0, w_cost=1.0, w_risk=0.5):
    # Calculate Total Value for each item
    for item in items:
        b = float(item['benefit'])
        p = float(item['penalty'])
        item['total_val'] = (b * w_benefit) + (p * w_penalty)
        item['weighted_cost'] = float(item['cost']) * w_cost
        item['weighted_risk'] = float(item['risk']) * w_risk

    sum_total_val = sum(item['total_val'] for item in items)
    sum_cost = sum(item['weighted_cost'] for item in items)
    sum_risk = sum(item['weighted_risk'] for item in items)

    if sum_total_val == 0 or (sum_cost + sum_risk) == 0:
        raise ValueError("Total value or total cost+risk sum is zero, check inputs.")

    for item in items:
        item['val_pct'] = (item['total_val'] / sum_total_val) * 100.0
        item['cost_pct'] = (item['weighted_cost'] / sum_cost) * 100.0 if sum_cost > 0 else 0.0
        item['risk_pct'] = (item['weighted_risk'] / sum_risk) * 100.0 if sum_risk > 0 else 0.0
        denom = item['cost_pct'] + item['risk_pct']
        item['priority'] = (item['val_pct'] / denom) if denom > 0 else 0.0

    # Sort descending by priority
    items.sort(key=lambda x: x['priority'], reverse=True)
    for idx, item in enumerate(items, 1):
        item['rank'] = idx

    return items, sum_total_val, sum_cost, sum_risk

def render_markdown(items, w_benefit, w_penalty, w_cost, w_risk):
    lines = []
    lines.append(f"### 需求分析型优先级排序表 (Karl Wiegers Analytical Prioritization)")
    lines.append(f"> **权重配置**: 收益(W_b)={w_benefit:.1f}, 损失(W_p)={w_penalty:.1f}, 成本(W_c)={w_cost:.1f}, 风险(W_r)={w_risk:.1f}\n")
    lines.append("| 排名 | 需求标识与描述 | 收益(1-9) | 损失(1-9) | 总价值 | 价值% | 成本(1-9) | 成本% | 风险(1-9) | 风险% | **优先级评分** |")
    lines.append("| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |")
    for item in items:
        lines.append(
            f"| **{item['rank']}** | {item['name']} | "
            f"{item['benefit']} | {item['penalty']} | "
            f"{item['total_val']:.1f} | {item['val_pct']:.1f}% | "
            f"{item['cost']} | {item['cost_pct']:.1f}% | "
            f"{item['risk']} | {item['risk_pct']:.1f}% | "
            f"**{item['priority']:.2f}** |"
        )
    return "\n".join(lines)

def main():
    parser = argparse.ArgumentParser(description="Karl Wiegers Analytical Requirements Prioritization Calculator")
    parser.add_argument("input_file", nargs="?", help="Path to JSON or CSV input file")
    parser.add_argument("--wb", type=float, default=2.0, help="Weight for benefit (default: 2.0)")
    parser.add_argument("--wp", type=float, default=1.0, help="Weight for penalty (default: 1.0)")
    parser.add_argument("--wc", type=float, default=1.0, help="Weight for cost (default: 1.0)")
    parser.add_argument("--wr", type=float, default=0.5, help="Weight for risk (default: 0.5)")
    parser.add_argument("--demo", action="store_true", help="Run with classic Chemical Tracking System demo data")

    args = parser.parse_args()

    demo_data = [
        {"name": "F-01: 核心化学品库存出入库管理", "benefit": 9, "penalty": 9, "cost": 3, "risk": 2},
        {"name": "F-02: 供应商电子画册与价格检索", "benefit": 8, "penalty": 4, "cost": 4, "risk": 2},
        {"name": "F-03: 危险品资质过期合规警告", "benefit": 5, "penalty": 8, "cost": 3, "risk": 3},
        {"name": "F-04: 生成实验室合规审计月报", "benefit": 6, "penalty": 3, "cost": 3, "risk": 2},
        {"name": "F-05: 3D 分子结构导入与图纸解析", "benefit": 7, "penalty": 2, "cost": 9, "risk": 8},
        {"name": "F-06: 历史领用单据批量 Excel 导出", "benefit": 4, "penalty": 3, "cost": 2, "risk": 1},
    ]

    if args.demo or not args.input_file:
        items = demo_data
    else:
        path = args.input_file
        if path.endswith(".json"):
            with open(path, "r", encoding="utf-8") as f:
                items = json.load(f)
        elif path.endswith(".csv"):
            items = []
            with open(path, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for row in reader:
                    items.append({
                        "name": row.get("name") or row.get("title") or row.get("id"),
                        "benefit": float(row["benefit"]),
                        "penalty": float(row["penalty"]),
                        "cost": float(row["cost"]),
                        "risk": float(row["risk"])
                    })
        else:
            sys.exit(f"Unsupported file format: {path}. Use .json or .csv")

    ranked_items, _, _, _ = calculate_priorities(items, args.wb, args.wp, args.wc, args.wr)
    md_output = render_markdown(ranked_items, args.wb, args.wp, args.wc, args.wr)
    print(md_output)

if __name__ == "__main__":
    main()
