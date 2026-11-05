# 60+ Components Dictionary (Ant Design to Qt Mapping)

This dictionary defines how the 60+ core Ant Design components map into desktop Qt Widgets and Qt Quick (QML), along with their layout metrics and behavioral rules.

---

## 1. General Components (通用组件)

### Button
- **Ant Design Archetype**: 5 variants (Primary, Default, Dashed, Text, Link) + Danger state.
- **QtWidgets**: `QPushButton` with dynamic property `type="primary|danger|dashed|text"`.
- **Qt Quick**: `AntButton.qml` with `buttonType` property.
- **Metrics**: Height: 32px; Radius: 6px; Horizontal padding: 15px.
- **Rule**: A page or card section must feature at most **one** Primary button. Secondary actions use Default or Text.

### Icon
- **QtWidgets**: `QIcon` loaded from SVG assets.
- **Qt Quick**: `Image` with SVG source or `Text` with icon font glyphs.
- **Metrics**: Standard size: 16px / 14px. Gap to adjoining text: 8px.

### Typography
- **QtWidgets**: `QLabel` with dynamic properties `type="title-lg|title-md|secondary|disabled"`.
- **Qt Quick**: `Text` with `AntTheme` font metrics.
- **Rule**: Never use pure black (`#000000`); use Ant primary `#1F1F1F` or secondary `#595959`.

---

## 2. Layout & Containers (布局与容器)

### Layout (Sider + Header + Content)
- **QtWidgets**: `QMainWindow` with `QDockWidget` or `QSplitter` for Sider navigation (width 208px or folded 48px), top `QFrame` Header (height 48px/56px), central `QWidget` Content.
- **Qt Quick**: `AntProLayout.qml`.
- **Rule**: Sider background is white `#FFFFFF` or dark `#001529`; Content canvas is always `#F5F5F5`.

### Card
- **QtWidgets**: `QFrame[type="card"]` or `QGroupBox`.
- **Qt Quick**: `AntCard.qml`.
- **Metrics**: Background: `#FFFFFF`; Radius: 8px; Border: 1px solid `#F0F0F0`; Padding: 24px (compact: 16px).

### Divider
- **QtWidgets**: `QFrame` with `HLine` or `VLine`, styled with `background-color: #F0F0F0; max-height: 1px`.
- **Rule**: Eliminate etched/grooved 3D borders. Use flat 1px lines.

---

## 3. Navigation (导航组件)

### Menu / Sider
- **QtWidgets**: `QTreeWidget` or custom `QListView`.
- **Selected State**: Light blue background `#E6F4FF`, text color `#1677FF`, 3px vertical accent bar on the left/right edge.

### Tabs
- **QtWidgets**: `QTabWidget`.
- **Rule**: Flat line style. Active tab features a 2px solid `#1677FF` bottom border. Tab pane background is pure `#FFFFFF`.

### Breadcrumb
- **QtWidgets**: `QHBoxLayout` combining `QLabel` links and `/` separator labels in `#BFBFBF`.

### Steps
- **QtWidgets / QML**: Horizontal indicator with circle numbers (24px diameter). Completed steps show a green `#52C41A` checkmark, active step shows blue `#1677FF`, pending steps show gray `#D9D9D9`.

---

## 4. Data Entry (数据录入组件)

### Input & Textarea
- **QtWidgets**: `QLineEdit` (height 32px), `QTextEdit` / `QPlainTextEdit`.
- **Metrics**: Height: 32px; Radius: 6px; Padding: `4px 11px`.
- **Focus**: Border turns `#1677FF` with outline glow. Error state: border turns `#FF4D4F`.

### Select / ComboBox
- **QtWidgets**: `QComboBox`.
- **Metrics**: Height: 32px; Dropdown popup has 4px padding, 6px radius, item hover `#E6F4FF`.

### Checkbox & Radio
- **QtWidgets**: `QCheckBox`, `QRadioButton`.
- **Metrics**: Indicator size: 16px; Checked fill: `#1677FF`; Radius: 4px for Checkbox, 8px for Radio.

### Form Layout
- **QtWidgets**: `QFormLayout` with `spacing: 16px`. Labels right-aligned or top-aligned. Mandatory fields marked with red star `*`.

---

## 5. Data Display (数据展示组件)

### Table (High-Impact Enterprise Core)
- **QtWidgets**: `QTableView` or `QTableWidget` + `QHeaderView`.
- **Rules**:
  1. **Header**: Background `#FAFAFA`, Font weight 600, Height 48px, bottom border 1px solid `#F0F0F0`.
  2. **Gridlines**: Remove vertical lines. Keep only horizontal 1px solid `#F0F0F0` row dividers.
  3. **Hover**: Highlight entire row with `#E6F4FF`.
  4. **No Zebra**: Never use alternating gray zebra stripes.
  5. **Numbers**: Align numeric values to the right with `tabular-nums`.

### Tag & Badge
- **QtWidgets**: Custom `QLabel`.
- **Metrics**: Tag height: 22px, radius 4px, font 12px. Badge is 9999px pill or 6px dot.

### Tooltip & Popover
- **QtWidgets**: Styled `QToolTip` or custom borderless dialog with drop shadow.
- **Metrics**: Background `#1F1F1F` with white text (Tooltip) or `#FFFFFF` with drop shadow (Popover).

---

## 6. Feedback (反馈组件)

### Modal / Dialog
- **QtWidgets**: `QDialog`.
- **Metrics**: Border radius: 8px; Padding: `20px 24px`; Buttons placed in the bottom right corner (Cancel first, then Primary Confirm).

### Drawer
- **QtWidgets**: Child `QFrame` sliding from the right edge with `QPropertyAnimation` (duration 200ms).
- **Rule**: Width: 380px (standard) or 520px (large). Includes close button, title, and bottom action bar.

### Message & Notification
- **QtWidgets**: Frameless top-center floating banner with 0.2s fade-in and auto-dismiss after 3s.
