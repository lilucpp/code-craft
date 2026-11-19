#!/usr/bin/env python3
"""Static redline linter for C++17 codebases (Frozen Lexical/API Scope).

Catches high-risk lexical patterns that clang-tidy/cppcheck often miss in C++ mode:
1. R1 (redline-banned-c-api): Banned/unsafe C APIs (strcpy, sprintf, system, fork, rand, atoi, etc.)
2. R2 (redline-swallowed-catch): Empty catch blocks silently swallowing exceptions
3. R3 (redline-fragile-cwd-path): Fragile CWD-relative build paths ("./build/", "../build/")
4. R4 (redline-ui-hardcoded-text, opt-in via --check-ui-i18n): Unwrapped CJK literals in Qt/GUI calls
5. R6 (redline-ignored-fs-error-code): Unchecked std::error_code after std::filesystem::exists(path, ec)

[COMPLEXITY FREEZE & 4-STEP ADMISSION PROTOCOL - ENFORCED BY scripts/self_test.sh]
- Budget Cap: Max 5 distinct `redline-*` rule IDs, Max 380 total lines of code.
- Before adding ANY rule here, follow the 4-step routing gate:
  Step 1 (AST/Compiler): Check `clang-tidy --list-checks` and compiler `-W...` flags first.
  Step 2 (Module DAG): Put `#include` / directory layering rules in `check_include_dag.py`.
  Step 3 (Lexical Redline): Only pure API/literal blacklists (zero variable-name guessing or arithmetic
         dataflow heuristics) may enter this file, within the 5-rule / 380-line cap.
  Step 4 (Semantic/Protocol): Put protocol checksums, arithmetic bounds (`len > size - offset`), and
         lock topology into `coding-standards.md` upfront heuristics + `cpp-verification-playbook` Gate 3.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple


@dataclass(frozen=True)
class BannedApiSpec:
  cert_tag: str
  alternative: str


BANNED_APIS: Dict[str, BannedApiSpec] = {
    "gets": BannedApiSpec("CWE-242/MSC24-C", "std::getline(std::cin, str)"),
    "strcpy": BannedApiSpec("CWE-120/STR31-C", "std::string 赋值或 std::string_view"),
    "wcscpy": BannedApiSpec("CWE-120/STR31-C", "std::wstring 赋值或 std::wstring_view"),
    "strcat": BannedApiSpec("CWE-120/STR31-C", "std::string::append() / +="),
    "wcscat": BannedApiSpec("CWE-120/STR31-C", "std::wstring::append() / +="),
    "strncpy": BannedApiSpec("STR32-C", "std::string / std::string_view::substr()"),
    "strncat": BannedApiSpec("STR32-C", "std::string::append()"),
    "sprintf": BannedApiSpec("CWE-120", "snprintf(dst, sizeof(dst), ...) 并校验返回值"),
    "vsprintf": BannedApiSpec("CWE-120", "vsnprintf(dst, sizeof(dst), ...) 并校验返回值"),
    "mktemp": BannedApiSpec("CWE-377/FIO21-C", "mkstemp()"),
    "tmpnam": BannedApiSpec("CWE-377/FIO21-C", "mkstemp()"),
    "tempnam": BannedApiSpec("CWE-377/FIO21-C", "mkstemp()"),
    "system": BannedApiSpec("CWE-78/ENV33-C", "posix_spawn() / execve() / reproc++"),
    "popen": BannedApiSpec("CWE-78/ENV33-C", "posix_spawn() + pipe / reproc++"),
    "fork": BannedApiSpec("MT-Safety", "posix_spawn() / reproc++（避免多线程 fork 死锁）"),
    "vfork": BannedApiSpec("MT-Safety", "posix_spawn() / reproc++"),
    "rand": BannedApiSpec("CWE-338/MSC30-C", "std::mt19937 + std::random_device 或 OS CSPRNG"),
    "srand": BannedApiSpec("CWE-338/MSC30-C", "std::mt19937 + std::random_device"),
    "alloca": BannedApiSpec("CWE-770", "std::vector<T> 或固定上限 std::array"),
    "_alloca": BannedApiSpec("CWE-770", "std::vector<T> 或固定上限 std::array"),
    "atoi": BannedApiSpec("MSC24-C/ERR34-C", "std::from_chars (C++17) 或 strtol + errno 校验"),
    "atol": BannedApiSpec("MSC24-C/ERR34-C", "std::from_chars (C++17) 或 strtol + errno 校验"),
    "atof": BannedApiSpec("MSC24-C/ERR34-C", "std::from_chars / strtod + errno 校验"),
    "strtok": BannedApiSpec("CON33-C", "strtok_r() 或 std::string_view::find()"),
    "asctime": BannedApiSpec("CON33-C", "strftime() / <chrono>"),
    "ctime": BannedApiSpec("CON33-C", "localtime_r() + strftime()"),
    "gmtime": BannedApiSpec("CON33-C", "gmtime_r()"),
    "localtime": BannedApiSpec("CON33-C", "localtime_r()"),
}

BANNED_CALL_RE = re.compile(
    r"(?<![.\w>])(?:std\s*::\s*|::\s*)?\b("
    + "|".join(re.escape(name) for name in BANNED_APIS)
    + r")\s*\("
)
CALL_PRECEDING_KEYWORDS = {"return", "co_return", "co_yield", "else", "throw", "case", "do"}
PRECEDING_WORD_RE = re.compile(r"([A-Za-z_]\w*|[*&>])\s*$")
RAW_STR_START_RE = re.compile(r'R"([^\s()\\]{0,16})\(')

EMPTY_CATCH_RE = re.compile(r"\bcatch\s*\([^)]*\)\s*\{\s*\}", re.MULTILINE)
FRAGILE_CWD_RE = re.compile(r"(?:\.\.?/)+(?:cmake-)?build(?:-[^/\"'\s]+)?/")
FS_EXISTS_EC_RE = re.compile(r"\b(?:filesystem::|fs::)exists\s*\([^;,\n]+,\s*([A-Za-z_]\w*)\s*\)")
UI_HARDCODED_CJK_RE = re.compile(
    r"\b(?:QMessageBox::\w+|setText|setWindowTitle|setToolTip|setPlaceholderText|showMessage)\s*\([^;\n]*[\u4e00-\u9fff]"
)
UI_TR_WRAPPED_RE = re.compile(r"(?:[\w:]+::)?\b[Tt]r\s*\([^)]*\)")
NOLINT_RE = re.compile(r"NOLINT(NEXTLINE)?\(\s*redline\s*\)\s*:\s*\S+")


@dataclass
class Violation:
  file_path: str
  line: int
  rule_id: str
  message: str


def strip_comments_and_strings(source: str) -> Tuple[str, str, Dict[int, bool]]:
  """Return (code_only, code_with_strings, exempt_lines).

  - code_only replaces comments and string/char literal bodies with spaces, preserving newlines.
  - code_with_strings replaces only comments with spaces, preserving string literals and newlines.
  - exempt_lines maps 1-based line numbers exempted via `NOLINT(redline): <reason>`
    or `NOLINTNEXTLINE(redline): <reason>`.
  """
  code_only: List[str] = []
  code_with_strings: List[str] = []
  exempt_lines: Dict[int, bool] = {}

  n = len(source)
  i = 0
  line = 1
  in_line_comment = False
  in_block_comment = False
  in_string: Optional[str] = None
  comment_buf: List[str] = []
  comment_start_line = 1

  def record_comment_exemption(text: str, comment_line: int) -> None:
    match = NOLINT_RE.search(text)
    if not match:
      return
    target_line = comment_line + 1 if match.group(1) == "NEXTLINE" else comment_line
    exempt_lines[target_line] = True

  while i < n:
    ch = source[i]
    nxt = source[i + 1] if i + 1 < n else ""

    if in_line_comment:
      if ch == "\n":
        record_comment_exemption("".join(comment_buf), comment_start_line)
        comment_buf.clear()
        in_line_comment = False
        code_only.append("\n")
        code_with_strings.append("\n")
        line += 1
      else:
        comment_buf.append(ch)
        code_only.append(" ")
        code_with_strings.append(" ")
      i += 1
      continue

    if in_block_comment:
      if ch == "*" and nxt == "/":
        record_comment_exemption("".join(comment_buf), line)
        comment_buf.clear()
        in_block_comment = False
        code_only.extend([" ", " "])
        code_with_strings.extend([" ", " "])
        i += 2
        continue
      if ch == "\n":
        code_only.append("\n")
        code_with_strings.append("\n")
        line += 1
      else:
        comment_buf.append(ch)
        code_only.append(" ")
        code_with_strings.append(" ")
      i += 1
      continue

    if in_string is not None:
      code_with_strings.append(ch)
      if ch == "\\":
        code_only.append(" ")
        if i + 1 < n:
          nxt_ch = source[i + 1]
          code_with_strings.append(nxt_ch)
          if nxt_ch == "\n":
            code_only.append("\n")
            line += 1
          else:
            code_only.append(" ")
          i += 2
          continue
        i += 1
        continue
      if ch == in_string:
        in_string = None
        code_only.append(ch)
        i += 1
        continue
      if ch == "\n":
        in_string = None
        code_only.append("\n")
        line += 1
      else:
        code_only.append(" ")
      i += 1
      continue

    # Not in comment or string
    if ch == "/" and nxt == "/":
      in_line_comment = True
      comment_start_line = line
      comment_buf.clear()
      code_only.extend([" ", " "])
      code_with_strings.extend([" ", " "])
      i += 2
      continue

    if ch == "/" and nxt == "*":
      in_block_comment = True
      comment_start_line = line
      comment_buf.clear()
      code_only.extend([" ", " "])
      code_with_strings.extend([" ", " "])
      i += 2
      continue

    if ch == "R" and nxt == '"':
      raw_m = RAW_STR_START_RE.match(source, i)
      if raw_m:
        delim = raw_m.group(1)
        end_marker = f"){delim}\""
        body_start = raw_m.end()
        end_idx = source.find(end_marker, body_start)
        end_pos = (end_idx + len(end_marker)) if end_idx != -1 else n
        raw_chunk = source[i:end_pos]
        code_with_strings.append(raw_chunk)
        for idx_c, rc in enumerate(raw_chunk):
          if rc == "\n":
            code_only.append("\n")
            line += 1
          elif idx_c == 0 or idx_c == 1 or (end_idx != -1 and i + idx_c == end_pos - 1):
            code_only.append(rc)
          else:
            code_only.append(" ")
        i = end_pos
        continue

    if ch in ('"', "'"):
      in_string = ch
      code_only.append(ch)
      code_with_strings.append(ch)
      i += 1
      continue

    code_only.append(ch)
    code_with_strings.append(ch)
    if ch == "\n":
      line += 1
    i += 1

  if in_line_comment and comment_buf:
    record_comment_exemption("".join(comment_buf), comment_start_line)

  return "".join(code_only), "".join(code_with_strings), exempt_lines


def offset_to_line(text: str, offset: int) -> int:
  return text.count("\n", 0, offset) + 1


def scan_file(file_path: str, check_ui_i18n: bool) -> List[Violation]:
  with open(file_path, "r", encoding="utf-8", errors="replace") as fh:
    source = fh.read()

  code_only, code_with_strings, exempt_lines = strip_comments_and_strings(source)
  violations: List[Violation] = []

  # R1: Banned C API calls
  for line_no, raw_line in enumerate(code_only.splitlines(), start=1):
    if exempt_lines.get(line_no):
      continue
    stripped = raw_line.lstrip()
    if stripped.startswith("#"):
      continue
    for match in BANNED_CALL_RE.finditer(raw_line):
      # Ignore member access (obj->fork()) and function declarations (int rand() const)
      start = match.start()
      prefix = raw_line[:start]
      if prefix.rstrip().endswith("->"):
        continue
      prev_word = PRECEDING_WORD_RE.search(prefix)
      if prev_word:
        token = prev_word.group(1)
        if token in ("*", "&", ">"):
          if "=" not in prefix and "(" not in prefix and "<<" not in prefix and ";" not in prefix and "return" not in prefix.split():
            continue
        elif token not in CALL_PRECEDING_KEYWORDS:
          continue
      api = match.group(1)
      spec = BANNED_APIS[api]
      violations.append(Violation(
          file_path, line_no, rule_id="redline-banned-c-api",
          message=f"禁止调用高危/非线程安全函数 '{api}()' [{spec.cert_tag}]，请改用: {spec.alternative}",
      ))

  # R2: Empty catch blocks (single or multi-line)
  for match in EMPTY_CATCH_RE.finditer(code_only):
    line_no = offset_to_line(code_only, match.start())
    if not exempt_lines.get(line_no):
      violations.append(Violation(
          file_path, line_no, rule_id="redline-swallowed-catch",
          message="禁止使用空 catch 块静默吞掉异常；请记录错误上下文并向上传播或 Fail-Fast",
      ))

  # R3: Fragile CWD-relative build paths inside string literals / code
  for line_no, raw_line in enumerate(code_with_strings.splitlines(), start=1):
    if not exempt_lines.get(line_no) and FRAGILE_CWD_RE.search(raw_line):
      violations.append(Violation(
          file_path, line_no, rule_id="redline-fragile-cwd-path",
          message="禁止硬编码依赖 CWD 的脆弱相对构建路径 (./build/, ../build/)；请基于可执行文件同级目录 (/proc/self/exe) 或显式参数解析",
      ))

  # R4: Optional GUI hardcoded CJK strings (ignore text properly wrapped in Tr(...) / tr(...))
  if check_ui_i18n:
    for line_no, raw_line in enumerate(code_with_strings.splitlines(), start=1):
      if not exempt_lines.get(line_no) and UI_HARDCODED_CJK_RE.search(UI_TR_WRAPPED_RE.sub("", raw_line)):
        violations.append(Violation(
            file_path, line_no, rule_id="redline-ui-hardcoded-text",
            message="表现层控件/弹窗禁止硬编码中文字符串；请统一接入国际化字典 Tr(...)",
        ))

  # R5: Unchecked std::error_code after std::filesystem::exists(path, ec)
  code_lines = code_only.splitlines()
  for idx, raw_line in enumerate(code_lines):
    line_no = idx + 1
    if exempt_lines.get(line_no):
      continue
    for match in FS_EXISTS_EC_RE.finditer(raw_line):
      ec_name = match.group(1)
      lookahead = raw_line[match.end() :] + "\n" + "\n".join(code_lines[idx + 1 : idx + 7])
      ec_esc = re.escape(ec_name)
      ec_checked_re = re.compile(
          rf"(?:if\s*\([^;\n]*\b{ec_esc}\b|!\s*{ec_esc}\b|\b{ec_esc}\s*(?:&&|\|\||[!=]=|\?|\.value\()|return\s+[^;\n]*\b{ec_esc}\b|(?:assert|EXPECT_|REQUIRE)[^;\n]*\b{ec_esc}\b)"
      )
      if not ec_checked_re.search(lookahead):
        violations.append(Violation(
            file_path, line_no, rule_id="redline-ignored-fs-error-code",
            message=f"调用 filesystem::exists(..., {ec_name}) 后未检查错误码 '{ec_name}'；需显式区分不存在与 EACCES/ENOTDIR",
        ))

  return violations


def parse_args(argv: List[str]) -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Static redline linter for C++ files.")
  parser.add_argument("files", nargs="+", help="C++ source/header files to scan")
  parser.add_argument("--warn-only", action="store_true", help="Report violations without failing (exit 0)")
  parser.add_argument("--check-ui-i18n", action="store_true", help="Check hardcoded CJK strings in UI calls")
  return parser.parse_args(argv)


def main(argv: List[str]) -> int:
  args = parse_args(argv)
  all_violations: List[Violation] = []

  for path in args.files:
    if not os.path.isfile(path):
      continue
    all_violations.extend(scan_file(path, check_ui_i18n=args.check_ui_i18n))

  if not all_violations:
    print("   ✅ 红线静态扫描通过 (0 违规)。")
    return 0

  for v in all_violations:
    print(f"{v.file_path}:{v.line}: error: {v.message} [{v.rule_id}]", file=sys.stderr)

  print(f"   ❌ 红线静态扫描检出 {len(all_violations)} 项违规（豁免需标注 // NOLINT(redline): <原因>）。")
  return 0 if args.warn_only else 1


if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
