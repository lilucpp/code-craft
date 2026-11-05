# Ant Design Desktop Tokens & Metrics Guide

This guide establishes the foundational visual tokens, metrics, and mathematical ratios for enterprise PC desktop applications built with Qt (C++, PySide6, PyQt, and QML).

---

## 1. Color System (色彩体系)

Ant Design utilizes an HSB-based algorithmic color generation system with **#1677FF** (Daybreak Blue) as the primary brand seed.

### 1.1 Primary & Functional Palettes
| Token Name | Hex Code | Purpose & Usage in Desktop UI |
| :--- | :--- | :--- |
| `primary` (Step 6) | `#1677FF` | Primary button background, active menu items, focus borders, primary tabs |
| `primary-hover` (Step 5) | `#4096FF` | Button hover, link hover, input border hover |
| `primary-active` (Step 7) | `#0958D9` | Button pressed, active click feedback |
| `primary-bg` (Step 1) | `#E6F4FF` | Table row hover, selected list item background, light tag background |
| `success` | `#52C41A` | Operational success, online status badge, positive trend indicators |
| `warning` | `#FAAD14` | Warning alerts, caution badges, pending operations |
| `error` | `#FF4D4F` | Destructive buttons, validation error borders, failed tasks |
| `info` | `#1677FF` | Informational callouts and tooltips |

### 1.2 Three-Layer Surface Topology (三层表面拓扑)
Desktop interfaces must follow strict spatial hierarchy:
1. **Background Layer (`bg-layout`)**: `#F5F5F5`
   - Window root canvas, outer workspace background.
2. **Container Layer (`bg-container`)**: `#FFFFFF`
   - Content cards (`QFrame[type="card"]`), data tables, side navigation bars.
3. **Elevated Layer (`bg-elevated`)**: `#FFFFFF` with drop shadow
   - Modals (`QDialog`), dropdown menus (`QComboBox QAbstractItemView`), tooltips, right drawers.

### 1.3 Neutral Typography Hierarchy (基于白底的中性文字)
- **Primary Text (`text-primary`)**: `rgba(0,0,0,0.88)` / `#1F1F1F` (Headings, data cells, main button text).
- **Secondary Text (`text-secondary`)**: `rgba(0,0,0,0.65)` / `#595959` (Subtitles, table headers, form labels).
- **Disabled Text (`text-disabled`)**: `rgba(0,0,0,0.25)` / `#BFBFBF` (Disabled buttons, placeholder text).
- **Border (`border`)**: `rgba(0,0,0,0.15)` / `#D9D9D9` (Input borders, default button borders).
- **Split Line (`border-secondary`)**: `rgba(0,0,0,0.06)` / `#F0F0F0` (Card borders, table row separators).

---

## 2. Typography System (排版与文字规范)

Desktop applications are viewed from a distance of approximately 50 cm. All metrics are optimized for clarity and eye comfort.

### 2.1 Font Families
- **Windows**: `"Segoe UI", "Microsoft YaHei", sans-serif`
- **Linux**: `"Inter", "Noto Sans CJK SC", "WenQuanYi Micro Hei", sans-serif`
- **macOS**: `"-apple-system", "PingFang SC", sans-serif`

### 2.2 Desktop Font Scales
- **12px** (Line height: 20px): Status tags, form validation error hints, auxiliary timestamps.
- **14px** (Line height: 22px): **Base Body**. Standard buttons, inputs, table body, tree nodes.
- **16px** (Line height: 24px): Card titles, dialog titles, tab labels.
- **20px** (Line height: 28px): Page section titles, drawer headers.
- **24px** (Line height: 32px): Main window title, dashboard top KPI numbers.

### 2.3 Strict Rules
- **Weights**: Strictly **400** (Regular) and **600** (Semi-bold / Bold). Never use 300 (thin) or italic fonts in desktop UI chrome.
- **Numbers**: Enable `tabular-nums` for tables and financial figures so digits 0–9 align with equal width vertically.

---

## 3. Spacing & Grid System (栅格与尺寸标尺)

### 3.1 4px / 8px Spacing Scale
- `xs: 4px` — Control internal padding, icon-to-text gap.
- `sm: 8px` — Spacing between adjacent buttons, tight form field margins.
- `md: 16px` — Standard card inner padding (compact), layout gutter.
- `lg: 24px` — Standard page margin, generous card padding.
- `xl: 32px` — Section vertical separation.

### 3.2 Standard Control Heights
All interactive desktop controls must strictly align to fixed heights:
- **Small (sm)**: `24px` (Compact table action buttons).
- **Default (middle)**: `32px` (**Mandatory baseline** for Buttons, LineEdits, ComboBoxes).
- **Large (lg)**: `40px` (Prominent search bars, landing actions).

---

## 4. Shapes & Border Radiuses (几何圆角规范)

- **Interactive Controls (Buttons, Inputs, Selects)**: `6px`
- **Containers & Surfaces (Cards, Modals, Drawers)**: `8px`
- **Tags & Tooltips**: `4px`
- **Pill (9999px / 50%)**: **Exclusively reserved for numeric badges and avatar circles**. Never use pill shape on desktop action buttons.

---

## 5. Elevation & Motion (阴影与微动效)

### 5.1 Elevation Depth
- **Low (Card Hover)**: Offset (0, 2px), Blur 8px, `rgba(0, 0, 0, 0.08)`.
- **Medium (Dropdowns / Popovers)**: Offset (0, 4px), Blur 16px, `rgba(0, 0, 0, 0.12)`.
- **High (Dialogs / Modals)**: Offset (0, 6px), Blur 24px, `rgba(0, 0, 0, 0.16)`.

### 5.2 Motion Durations
- **0.1s (100ms)**: Hover, active, focus state transitions.
- **0.2s (200ms)**: Drawer slide-in, accordion expansion, tooltip fade.
- **0.3s (300ms)**: Full modal popup, page-level switching.
