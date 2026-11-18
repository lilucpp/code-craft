#!/usr/bin/env python3
"""C++ module boundary & include DAG checker (ADP + encapsulation enforcer).

Enforces three architectural invariants without requiring a full compiler AST:
1. ADP (No Cycles): Detects module-level cyclic #include dependencies.
2. Private Encapsulation: Blocks cross-module #include into `detail/` or `internal/`
   and blocks `../` parent traversal across module boundaries.
3. Layer Rules (Optional .arch-boundaries.json): Enforces allowed/forbidden
   directed dependencies between layers/modules (e.g., domain cannot include infra).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, List, Optional, Set, Tuple

CPP_EXTS = (".h", ".hpp", ".hh", ".hxx", ".c", ".cc", ".cpp", ".cxx")
DEFAULT_EXCLUDES = {".git", ".agents", "build", "cmake-build", "_deps", "third_party", "vendor", "external"}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')
RAW_STR_START_RE = re.compile(r'R"([^\s()\\]{0,16})\(')
PRIVATE_SEGMENTS = {"detail", "internal"}


@dataclass(frozen=True)
class IncludeEdge:
  src_file: str
  line_no: int
  src_mod: str
  dst_file: str
  dst_mod: str
  raw_include: str


@dataclass(frozen=True)
class BoundaryViolation:
  file_path: str
  line_no: int
  rule_id: str
  message: str


def strip_cpp_comments(source: str) -> str:
  """Replace // and /* */ comments with spaces, preserving line numbers."""
  out: List[str] = []
  n = len(source)
  i = 0
  in_line = False
  in_block = False
  in_str: Optional[str] = None

  while i < n:
    ch = source[i]
    nxt = source[i + 1] if i + 1 < n else ""

    if in_line:
      if ch == "\n":
        in_line = False
        out.append("\n")
      else:
        out.append(" ")
      i += 1
      continue

    if in_block:
      if ch == "*" and nxt == "/":
        in_block = False
        out.extend([" ", " "])
        i += 2
        continue
      out.append("\n" if ch == "\n" else " ")
      i += 1
      continue

    if in_str is not None:
      out.append(ch)
      if ch == "\\" and i + 1 < n:
        out.append(source[i + 1])
        i += 2
        continue
      if ch == in_str or ch == "\n":
        in_str = None
      i += 1
      continue

    if ch == "/" and nxt == "/":
      in_line = True
      out.extend([" ", " "])
      i += 2
      continue
    if ch == "/" and nxt == "*":
      in_block = True
      out.extend([" ", " "])
      i += 2
      continue
    if ch == "R" and nxt == '"':
      raw_m = RAW_STR_START_RE.match(source, i)
      if raw_m:
        delim = raw_m.group(1)
        end_marker = f"){delim}\""
        end_idx = source.find(end_marker, raw_m.end())
        end_pos = (end_idx + len(end_marker)) if end_idx != -1 else n
        for rc in source[i:end_pos]:
          out.append("\n" if rc == "\n" else " ")
        i = end_pos
        continue
    if ch in ('"', "'"):
      in_str = ch
      out.append(ch)
      i += 1
      continue

    out.append(ch)
    i += 1

  return "".join(out)


def is_excluded(rel_path: str, extra_excludes: Set[str]) -> bool:
  parts = rel_path.replace("\\", "/").split("/")
  for part in parts[:-1]:
    if part in DEFAULT_EXCLUDES or part in extra_excludes or part.startswith("build-") or part.startswith("cmake-build"):
      return True
  return False


def collect_files(root: str, extra_excludes: Set[str]) -> List[str]:
  files: List[str] = []
  for dirpath, dirnames, filenames in os.walk(root):
    dirnames[:] = [
        d
        for d in sorted(dirnames)
        if d not in DEFAULT_EXCLUDES
        and d not in extra_excludes
        and not d.startswith("build-")
        and not d.startswith("cmake-build")
    ]
    for fname in sorted(filenames):
      if fname.endswith(CPP_EXTS):
        rel = os.path.relpath(os.path.join(dirpath, fname), root).replace("\\", "/")
        if not is_excluded(rel, extra_excludes):
          files.append(rel)
  return files


def infer_module(rel_path: str, module_roots: List[str]) -> str:
  """Infer module name from path patterns like include/<mod>/..., src/<mod>/..., or modules/<mod>/..."""
  norm = rel_path.replace("\\", "/")
  parts = norm.split("/")

  # Pattern 1: modules/<mod>/... or libs/<mod>/... or packages/<mod>/...
  for prefix in ("modules", "libs", "packages", "components"):
    if len(parts) >= 3 and parts[0] == prefix:
      return parts[1]

  # Pattern 2: include/<mod>/... or src/<mod>/...
  for prefix in module_roots:
    prefix_norm = prefix.strip("/")
    if not prefix_norm:
      continue
    prefix_parts = prefix_norm.split("/")
    if parts[: len(prefix_parts)] == prefix_parts and len(parts) > len(prefix_parts) + 1:
      return parts[len(prefix_parts)]

  # Fallback: top-level directory or root
  return parts[0] if len(parts) > 1 else "_root"


def discover_include_roots(root: str, all_files: List[str], configured_roots: List[str]) -> List[str]:
  roots: List[str] = []
  seen: Set[str] = set()

  def add_root(r: str) -> None:
    norm = r.strip("/")
    if norm not in seen:
      seen.add(norm)
      roots.append(norm)

  for r in configured_roots:
    add_root(r)
  for candidate in ("include", "src"):
    if os.path.isdir(os.path.join(root, candidate)):
      add_root(candidate)

  # Also discover modules/<mod>/include and modules/<mod>/src
  for rel in all_files:
    parts = rel.split("/")
    for idx, seg in enumerate(parts[:-1]):
      if seg in ("include", "src") and idx > 0:
        add_root("/".join(parts[: idx + 1]))

  add_root("")  # Repo root as last fallback
  return roots


def resolve_include(
    src_rel: str,
    raw_inc: str,
    quote_kind: str,
    include_roots: List[str],
    file_set: Set[str],
) -> Optional[str]:
  """Resolve an #include target to a project-relative file path, or None if external."""
  inc_norm = os.path.normpath(raw_inc).replace("\\", "/")

  # 1. Quoted include checks current file's directory first
  if quote_kind == '"':
    src_dir = os.path.dirname(src_rel)
    local_candidate = os.path.normpath(os.path.join(src_dir, inc_norm)).replace("\\", "/")
    if local_candidate in file_set:
      return local_candidate

  # 2. Check configured/discovered include roots
  for inc_root in include_roots:
    candidate = os.path.normpath(os.path.join(inc_root, inc_norm) if inc_root else inc_norm).replace("\\", "/")
    if candidate in file_set:
      return candidate

  return None


def find_cycles(adj: Dict[str, Set[str]]) -> List[List[str]]:
  """Find elementary cycles in module dependency graph (deterministic sorted order)."""
  state: Dict[str, int] = {}  # 0/missing=unvisited, 1=visiting, 2=done
  stack: List[str] = []
  cycles: List[List[str]] = []
  seen_cycle_keys: Set[Tuple[str, ...]] = set()

  def canonical_cycle(cyc: List[str]) -> Tuple[str, ...]:
    core = cyc[:-1]
    min_idx = min(range(len(core)), key=lambda idx: core[idx])
    rotated = core[min_idx:] + core[:min_idx]
    return tuple(rotated)

  def dfs(node: str) -> None:
    state[node] = 1
    stack.append(node)
    for nxt in sorted(adj.get(node, ())):
      nxt_state = state.get(nxt, 0)
      if nxt_state == 1:
        idx = stack.index(nxt)
        cyc = stack[idx:] + [nxt]
        key = canonical_cycle(cyc)
        if key not in seen_cycle_keys:
          seen_cycle_keys.add(key)
          cycles.append(cyc)
      elif nxt_state == 0:
        dfs(nxt)
    stack.pop()
    state[node] = 2

  for node in sorted(adj.keys()):
    if state.get(node, 0) == 0:
      dfs(node)

  return cycles


def load_config(config_path: Optional[str]) -> dict:
  if not config_path or not os.path.isfile(config_path):
    return {}
  with open(config_path, "r", encoding="utf-8") as fh:
    return json.load(fh)


def main(argv: List[str]) -> int:
  parser = argparse.ArgumentParser(description="Check C++ #include DAG and module encapsulation boundaries.")
  parser.add_argument("--root", default=".", help="Project root directory (default: .)")
  parser.add_argument("--config", default=None, help="Optional .arch-boundaries.json config path")
  parser.add_argument("--warn-only", action="store_true", help="Report violations without failing (exit 0)")
  parser.add_argument("--dump-graph", action="store_true", help="Print discovered module dependency graph")
  args = parser.parse_args(argv)

  root = os.path.abspath(args.root)
  default_cfg = os.path.join(root, ".arch-boundaries.json")
  cfg_path = args.config if args.config else (default_cfg if os.path.isfile(default_cfg) else None)
  cfg = load_config(cfg_path)

  extra_excludes = set(cfg.get("exclude_dirs", []))
  module_roots = cfg.get("include_roots", ["include", "src"])
  module_rules = cfg.get("modules", {})
  forbid_parent_rel = cfg.get("forbid_relative_parent_includes", True)

  all_files = collect_files(root, extra_excludes)
  if not all_files:
    print("✅ 未发现需要检查的 C++ 文件。")
    return 0

  file_set = set(all_files)
  include_roots = discover_include_roots(root, all_files, module_roots)

  edges: List[IncludeEdge] = []
  adj: Dict[str, Set[str]] = defaultdict(set)
  edge_lookup: Dict[Tuple[str, str], List[IncludeEdge]] = defaultdict(list)
  violations: List[BoundaryViolation] = []

  for rel_path in all_files:
    src_mod = infer_module(rel_path, module_roots)
    adj.setdefault(src_mod, set())
    abs_path = os.path.join(root, rel_path)
    with open(abs_path, "r", encoding="utf-8", errors="replace") as fh:
      cleaned = strip_cpp_comments(fh.read())

    for line_no, line in enumerate(cleaned.splitlines(), start=1):
      m = INCLUDE_RE.match(line)
      if not m:
        continue
      quote_kind, raw_inc = m.group(1), m.group(2)
      dst_rel = resolve_include(rel_path, raw_inc, quote_kind, include_roots, file_set)
      if dst_rel is None:
        continue
      dst_mod = infer_module(dst_rel, module_roots)

      if src_mod == dst_mod:
        continue

      edge = IncludeEdge(
          src_file=rel_path,
          line_no=line_no,
          src_mod=src_mod,
          dst_file=dst_rel,
          dst_mod=dst_mod,
          raw_include=raw_inc,
      )
      edges.append(edge)
      adj[src_mod].add(dst_mod)
      adj.setdefault(dst_mod, set())
      edge_lookup[(src_mod, dst_mod)].append(edge)

      # Rule B1: Cross-module parent relative path traversal
      if forbid_parent_rel and ".." in raw_inc.replace("\\", "/").split("/"):
        violations.append(
            BoundaryViolation(
                file_path=rel_path,
                line_no=line_no,
                rule_id="arch-relative-parent-include",
                message=f"禁止跨模块使用相对父路径引用 '{raw_inc}' ({src_mod} -> {dst_mod})，请通过模块公开头文件路径引用",
            )
        )

      # Rule B2: Cross-module penetration into detail/ or internal/
      dst_parts = dst_rel.split("/")
      if any(seg in PRIVATE_SEGMENTS for seg in dst_parts):
        violations.append(
            BoundaryViolation(
                file_path=rel_path,
                line_no=line_no,
                rule_id="arch-private-header-leak",
                message=f"禁止跨模块穿透引用私有头文件 '{dst_rel}' ({src_mod} -> {dst_mod})",
            )
        )

      # Rule B3: Explicit module dependency rules in .arch-boundaries.json
      if src_mod in module_rules:
        mod_cfg = module_rules[src_mod]
        allowed = mod_cfg.get("allow")
        forbidden = mod_cfg.get("forbid", [])
        if allowed is not None and dst_mod not in allowed:
          violations.append(
              BoundaryViolation(
                  file_path=rel_path,
                  line_no=line_no,
                  rule_id="arch-layer-violation",
                  message=f"违反分层边界: 模块 '{src_mod}' 禁止依赖 '{dst_mod}' (仅允许: {allowed})，违规引用 '{raw_inc}'",
              )
          )
        elif dst_mod in forbidden:
          violations.append(
              BoundaryViolation(
                  file_path=rel_path,
                  line_no=line_no,
                  rule_id="arch-layer-violation",
                  message=f"违反分层边界: 模块 '{src_mod}' 显式禁止依赖 '{dst_mod}'，违规引用 '{raw_inc}'",
              )
          )

  # Rule B4: ADP Cycle detection
  cycles = find_cycles(adj)
  for cyc in cycles:
    cyc_str = " -> ".join(cyc)
    samples = edge_lookup.get((cyc[0], cyc[1]), [])
    sample_loc = f"{samples[0].src_file}:{samples[0].line_no}" if samples else root
    sample_line = samples[0].line_no if samples else 1
    sample_file = samples[0].src_file if samples else root
    violations.append(
        BoundaryViolation(
            file_path=sample_file,
            line_no=sample_line,
            rule_id="arch-cyclic-dependency",
            message=f"违反 ADP 无环依赖原则，检测到模块环路 [{cyc_str}] (示例边: {sample_loc})",
        )
    )

  if args.dump_graph:
    print("📊 模块依赖拓扑 (Mermaid):")
    print("```mermaid")
    print("flowchart LR")
    for src in sorted(adj.keys()):
      for dst in sorted(adj[src]):
        print(f"    {src} --> {dst}")
    print("```")

  if not violations:
    print(f"✅ 架构边界与 Include DAG 检查通过 ({len(all_files)} 文件, {len(adj)} 模块, 0 环路/越权)。")
    return 0

  for v in violations:
    print(f"{v.file_path}:{v.line_no}: error: {v.message} [{v.rule_id}]", file=sys.stderr)

  print(f"❌ 架构边界检查检出 {len(violations)} 项违规。")
  return 0 if args.warn_only else 1


if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
