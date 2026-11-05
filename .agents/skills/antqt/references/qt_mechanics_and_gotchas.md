# Qt Technical Mechanics & Gotchas Guide (实战避坑与机制全指南)

This document resolves technical quirks, QSS limitations, and rendering bugs when applying Ant Design to Qt desktop applications. 汇总了在大型企业级 Qt 桌面端落地 Ant Design 规范时的底层机制、深坑排查与开箱即用解决方案。

---

## 1. QSS Dynamic Property Refresh (属性选择器失效问题)

When setting dynamic properties like `widget.setProperty("type", "primary")` at runtime, Qt does **not** automatically re-evaluate QSS selectors.

### The Fix
Force style re-polish immediately after modifying custom properties:
```python
# Python (PyQt6 / PySide6)
btn.setProperty("type", "primary")
btn.style().unpolish(btn)
btn.style().polish(btn)
```
```cpp
// C++
btn->setProperty("type", "primary");
btn->style()->unpolish(btn);
btn->style()->polish(btn);
```

---

## 2. Drop Shadows on Desktop Windows (QSS 缺失 box-shadow)

QSS does not support the CSS3 `box-shadow` property.

### The Fix
Use `QGraphicsDropShadowEffect` for elevated cards, dialogs, drawers, and popovers:
```python
from PyQt6.QtWidgets import QGraphicsDropShadowEffect
from PyQt6.QtGui import QColor

def apply_ant_shadow(widget, blur_radius=14, y_offset=2, alpha=20):
    shadow = QGraphicsDropShadowEffect(widget)
    shadow.setBlurRadius(blur_radius)
    shadow.setOffset(0, y_offset)
    shadow.setColor(QColor(0, 0, 0, alpha))
    widget.setGraphicsEffect(shadow)
```
```cpp
// C++
void applyAntShadow(QWidget* w, int blurRadius = 14, int yOffset = 2, int alpha = 20) {
    auto* shadow = new QGraphicsDropShadowEffect(w);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, yOffset);
    shadow->setColor(QColor(0, 0, 0, alpha));
    w->setGraphicsEffect(shadow);
}
```

---

## 3. High-Performance Modern Table Delegate (QStyledItemDelegate)

By default, `QTableView` renders rows with native OS styles, ignoring Ant Design's row hover highlight `#E6F4FF` or custom padding.

### Clean Delegate Pattern
```python
from PyQt6.QtWidgets import QStyledItemDelegate
from PyQt6.QtCore import QSize

class AntTableDelegate(QStyledItemDelegate):
    def sizeHint(self, option, index):
        size = super().sizeHint(option, index)
        # Force Ant standard row height (48px default or 44px compact)
        return QSize(size.width(), 44)
```

---

## 4. High-DPI Scaling Guardrails (高分屏文字与边框模糊问题)

Ensure clear rendering on 2K/4K high-DPI displays:
> [!IMPORTANT]
> 必须在 `QApplication app(argc, argv)` 实例化**之前**设置，否则在 Qt 5 下无效！
```cpp
// C++
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
QApplication app(argc, argv);

// 配置抗锯齿平滑现代字体栈
QFont font = app.font();
font.setFamily("-apple-system, BlinkMacSystemFont, 'Segoe UI', 'Microsoft YaHei', sans-serif");
font.setStyleStrategy(QFont::PreferAntialias);
font.setPointSize(10);
app.setFont(font);
```

---

## 5. QSS Specificity & Cascading Traps (特异度与层叠继承陷阱)

1. **Root QWidget styling**：严禁写 `QWidget { background-color: #FFFFFF; }`，否则所有子控件（Label、Layout、按钮）都会被强制递归变成死白背景。应只给特定容器命名或使用属性选择器（如 `QMainWindow`, `QFrame[type="card"]`）。
2. **Sub-control selectors**：当样式化 `QComboBox::drop-down` 或 `QSpinBox::up-button` 时，必须显式声明 `subcontrol-origin` 与 `subcontrol-position`，否则下拉或步进箭头会错位甚至消失。
3. **No negative margins**：QSS 对负 margin 支持极不稳定，应采用布局对齐（Alignment）与伸缩因子（Stretch）。

---

## 6. 父容器内联 CSS 继承导致按钮文字隐形陷阱 (White-on-White Button Gotcha)

### 现象
在弹出编辑对话框（`QDialog`）或自定义卡片中，确认按钮变成了**“白底蓝框，文字彻底隐形（空白）”**。

### 根因
开发者在 C++ 中写了 `dlg->setStyleSheet("background-color: #FFFFFF;");`。
QSS 的继承规则会导致子控件 `QPushButton` 继承了父级的白色背景；而 Ant Design 的 Primary 按钮文字原本是纯白色（`color: #FFFFFF`），两者相撞导致 **白色背景 + 白色文字 = 字体隐形**！

### 铁律法则
- **严禁**在 C++ 控件上调用硬编码颜色的内联 `setStyleSheet`；
- 所有对话框与卡片背景统一由全局 QSS 中的 `QDialog { background-color: #FFFFFF; }`（暗黑 `#1F1F1F`）接管；
- 按钮在创建时统一设置 `btn->setProperty("type", "primary")` 并执行 `unpolish/polish`。

---

## 7. QSS 无法使用 CSS border 绘制三角形陷阱 (SpinBox 压扁黑横线问题)

### 现象
微调器（`QSpinBox` / `QDoubleSpinBox`）的上下箭头显示为一条黑色的死板横线，而不是尖角三角。

### 根因
Qt 的 QSS 盒模型引擎**不支持** Web CSS 中利用 `width: 0; height: 0; border-left: ...` 拼接几何三角形的技巧。Qt 会将其绘制成无高度的矩形边框（即压扁的一条黑横线）。

### 黄金解法
必须使用 Qt 资源系统（`resources.qrc`）配合 `image: url(":/qss/icons/spin_up.png")` 引入标准矢量或高清位图：
```css
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
    image: url(":/qss/icons/spin_up.png");
    width: 9px;
    height: 7px;
}
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
    image: url(":/qss/icons/spin_down.png");
    width: 9px;
    height: 7px;
}
```

---

## 8. 图标高分屏发虚模糊与 4x 超采样 Lanczos 重采样方案

### 现象
微调器箭头、树节点图标在普通屏幕尚可，但在 2K/4K 视网膜屏上边缘发虚、泛白模糊。

### 根因
低分辨率点阵图标在 High-DPI 模式下被 Qt 默认的线性插值拉伸算法放大插值，导致边缘发虚。

### 黄金解法
采用计算机图形学高保真流程：
1. 从 Ant Design 官方库（`@ant-design/icons`）提取纯矢量 SVG 路径；
2. 采用 **4x 超采样抗锯齿 (SSAA) + Lanczos 降采样重采样算法** 生成视网膜级 2x/3x 高清资产；
3. 配合 `QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps)`，边缘绝对深沉纯净、锐利无毛边。

---

## 9. QComboBox Ant 官方 `DownOutlined` 优雅 V 箭头规范

### 规范标准
Ant Design 5.0 的 Select 下拉框官方图标并非粗笨实心三角，而是轻盈精致的 **`DownOutlined`（折角 V 细线折叠箭头）**。

### QSS 实现
```css
QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 26px;
    border: none;
}
QComboBox::down-arrow {
    image: url(":/qss/icons/select_down.png");
    width: 10px;
    height: 10px;
    margin-right: 8px;
}
```

---

## 10. QTreeWidget 分支展开/收起指示器规范

### 避坑指南
严禁在美化 QTreeWidget 时盲目声明 `QTreeWidget::branch { image: none; }`，否则会导致树节点的展开/折叠三角彻底丢失！

### 黄金规范
必须为 `closed` 与 `open` 状态分别绑定 Ant 官方标准的旋转指示小三角：
- **收起时（Closed）**：指向右方的微缩指示符 `▶`（`:/qss/icons/tree_closed.png`）；
- **展开时（Open）**：顺时针旋转 90 度指向下方的指示符 `▼`（`:/qss/icons/tree_open.png`）。

```css
QTreeWidget::branch:has-children:!has-siblings:closed,
QTreeWidget::branch:closed:has-children:has-siblings {
    image: url(":/qss/icons/tree_closed.png");
}
QTreeWidget::branch:open:has-children:!has-siblings,
QTreeWidget::branch:open:has-children:has-siblings {
    image: url(":/qss/icons/tree_open.png");
}
```
