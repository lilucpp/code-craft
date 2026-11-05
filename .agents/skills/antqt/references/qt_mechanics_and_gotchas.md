# Qt Technical Mechanics & Gotchas Guide

This document resolves technical quirks, QSS limitations, and rendering bugs when applying Ant Design to Qt desktop applications.

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
Use `QGraphicsDropShadowEffect` for elevated cards, dialogs, and popovers:
```python
from PyQt6.QtWidgets import QGraphicsDropShadowEffect
from PyQt6.QtGui import QColor

def apply_ant_shadow(widget, blur_radius=16, y_offset=4, alpha=30):
    shadow = QGraphicsDropShadowEffect(widget)
    shadow.setBlurRadius(blur_radius)
    shadow.setOffset(0, y_offset)
    shadow.setColor(QColor(0, 0, 0, alpha))
    widget.setGraphicsEffect(shadow)
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
        # Force Ant standard row height (48px default or 39px compact)
        return QSize(size.width(), 48)
```

---

## 4. High-DPI Scaling Guardrails (高分屏文字模糊问题)

Ensure clear rendering on 2K/4K high-DPI displays:
```python
# Must be executed before QApplication instantiation in Qt 5
# Qt 6 enables High-DPI by default
import os
os.environ["QT_ENABLE_HIGHDPI_SCALING"] = "1"
os.environ["QT_AUTO_SCREEN_SCALE_FACTOR"] = "1"
```

---

## 5. QSS Specificity & Cascading Traps

1. **Root QWidget styling**: Never write `QWidget { background-color: #FFFFFF; }` as it recursively forces every child widget (labels, layouts, buttons) to have opaque white backgrounds. Style specific containers (`QMainWindow`, `QFrame[type="card"]`) instead.
2. **Sub-control selectors**: When styling `QComboBox::drop-down`, always declare `subcontrol-origin` and `subcontrol-position` explicitly, otherwise the drop-down arrow may misalign or disappear.
3. **No negative margins**: QSS does not reliably support negative margins. Use layout alignment and stretch factors instead.
