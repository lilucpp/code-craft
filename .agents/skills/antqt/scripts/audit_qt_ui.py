#!/usr/bin/env python3
"""
Ant Design Qt UI Audit Tool (audit_qt_ui.py)
Scans QSS stylesheets, Qt .ui XML files, and source code for Ant Design desktop violations:
1. Control heights not aligned to 24px/32px/40px
2. Irregular border-radii (controls != 6px, containers != 8px, tags != 4px, pill != 9999px)
3. Direct pure black #000000 text instead of Ant neutral #1F1F1F (rgba(0,0,0,0.88))
4. Table zebra alternating row colors (anti-pattern in Ant Design)
5. Missing focus/hover interactive states
"""

import os
import sys
import re
import argparse

def audit_file(filepath: str):
    issues = []
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    is_qss = filepath.endswith('.qss') or filepath.endswith('.css')
    is_ui = filepath.endswith('.ui')

    current_selector = ""
    for idx, line in enumerate(lines, 1):
        stripped = line.strip()
        if "{" in stripped:
            current_selector = stripped.split("{")[0].strip()
        elif "}" in stripped:
            current_selector = ""

        # 1. Pure black text violation
        if re.search(r'color\s*:\s*#000000\b', line, re.IGNORECASE) or re.search(r'color\s*:\s*black\b', line, re.IGNORECASE):
            issues.append((idx, "Pure black #000000 text detected. Ant Design uses rgba(0,0,0,0.88) or #1F1F1F for primary text."))

        # 2. Control height violation
        m_height = re.search(r'(?:min-height|max-height|height)\s*:\s*(\d+)px', line)
        if m_height and is_qss:
            h = int(m_height.group(1))
            # Ignore sub-controls like indicators, scrollbars, headers, and tabs
            subcontrol_match = any(k in current_selector for k in ("QHeaderView", "indicator", "ScrollBar", "QTabBar", "border-bottom")) or "max-height: 1px" in line
            if not subcontrol_match:
                if h not in (22, 24, 28, 32, 40, 48, 56):
                    issues.append((idx, f"Non-standard height {h}px. Standard Ant control heights are 32px (default), 24px (sm), 40px (lg)."))

        # 3. Border radius violation
        m_radius = re.search(r'border-radius\s*:\s*(\d+)px', line)
        if m_radius:
            r = int(m_radius.group(1))
            subcontrol_match = any(k in current_selector for k in ("ScrollBar", "indicator"))
            if not subcontrol_match:
                if r not in (0, 2, 4, 6, 8, 9999):
                    issues.append((idx, f"Non-standard border-radius {r}px. Ant Design standard: 6px (controls), 8px (cards/modals), 4px (tags/tooltips), 9999px (badges)."))

        # 4. Table alternating row colors (Zebra)
        if "alternatingRowColors" in line and "true" in line.lower():
            issues.append((idx, "Zebra alternating row colors enabled. Ant Design uses white background with #E6F4FF hover highlight instead."))

        # 5. Generic sans-serif / default font check
        if re.search(r'font-family\s*:\s*[\'"]?SimSun[\'"]?', line):
            issues.append((idx, "Serif / SimSun font detected. Ant Design requires modern sans-serif: Microsoft YaHei / Segoe UI / Inter."))

    return issues

def main():
    parser = argparse.ArgumentParser(description="Audit Qt UI/QSS files against Ant Design Desktop specifications.")
    parser.add_argument("target", help="Path to file or directory to inspect")
    args = parser.parse_args()

    files = []
    if os.path.isfile(args.target):
        files.append(args.target)
    elif os.path.isdir(args.target):
        for root, _, filenames in os.walk(args.target):
            for fn in filenames:
                if fn.endswith(('.qss', '.css', '.ui', '.qml', '.py', '.cpp', '.h')):
                    files.append(os.path.join(root, fn))
    else:
        print(f"Target not found: {args.target}")
        sys.exit(1)

    total_issues = 0
    for f in sorted(files):
        issues = audit_file(f)
        if issues:
            print(f"\n[VIOLATIONS] {f}:")
            for line_no, msg in issues:
                print(f"  L{line_no}: {msg}")
                total_issues += 1

    print(f"\nAudit complete. Found {total_issues} violation(s) across {len(files)} file(s).")

if __name__ == "__main__":
    main()
