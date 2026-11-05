# Ant Design Pro Desktop Archetypes Guide

This document specifies the four canonical enterprise desktop layouts ported from Ant Design Pro.

---

## Archetype 1: Workbench & Monitoring Dashboard (工作台与监控大盘)

### Spatial Structure
1. **Top Header**: Global search, notification badge, user profile.
2. **Top Metric Tier (4 KPI Cards)**:
   - 4 equal-width `QFrame[type="card"]` placed horizontally in a `QHBoxLayout`.
   - Card contents: Metric Title (14px, `#595959`), Large Value (24px, 600 weight, `#1F1F1F`), Trend Tag (`+12.5%` green / `-3.2%` red), Mini trend sparkline.
3. **Main Content Tier (2 Columns)**:
   - Left (65% width): Main analytical chart (QCustomPlot / QtCharts) in a White Card.
   - Right (35% width): Real-time system activity log or task queue list (`QListWidget`).

```
+-------------------------------------------------------------------------------+
| [ KPI Card 1 ]    | [ KPI Card 2 ]    | [ KPI Card 3 ]    | [ KPI Card 4 ]    |
+-------------------------------------------------------------------------------+
| Main Business Trend Chart (65%)                 | Activity / Task List (35%)  |
|                                                 |                             |
|                                                 |                             |
+-------------------------------------------------------------------------------+
```

---

## Archetype 2: Standard Query Table with Detail Drawer (标准查询与抽屉表格)

### Spatial Structure
1. **Top Filter Card**:
   - 3 to 4 query fields in an inline row (Task Name, Status, Date Range).
   - "Expand/Collapse" toggle for advanced filters.
   - Right-aligned "Reset" and "Query" buttons.
2. **Table Container Card**:
   - **Action Bar**: Left side has Primary "+ New Task", Danger "Batch Delete"; Right side has Refresh and Density buttons.
   - **Data Table**: 48px header `#FAFAFA`, 14px body, row hover `#E6F4FF`.
   - **Bottom Pagination**: Page size switcher and page jump controls aligned to the right.
3. **Right-Side Detail Drawer**:
   - Selecting a row opens a 380px right-sliding drawer.
   - Displays comprehensive metadata, status chips, and action logs without navigating away from the table.

---

## Archetype 3: Step-by-Step Configuration Wizard (分步业务向导)

### Spatial Structure
1. **Top Step Indicator**:
   - Horizontal step bar (Step 1: Basic Info $\to$ Step 2: Security Credentials $\to$ Step 3: Complete).
2. **Centered Form Container (Max-Width 600px)**:
   - Centering prevents wide-screen form stretching on 1080p/2K/4K desktop monitors.
   - Vertical form rows with 16px vertical gap.
3. **Sticky Bottom Action Bar**:
   - Fixed at the bottom of the card or window.
   - "Previous" (Default button) and "Next / Submit" (Primary button).

---

## Archetype 4: Profile & Process Timeline View (详情画像与流程时间轴)

### Spatial Structure
1. **PageHeader Banner**:
   - Breadcrumb navigation $\to$ Page Title (20px, 600 weight) $\to$ Status Tag $\to$ Key metrics list (Owner, Creation Time, Version).
2. **Tabbed Content Container**:
   - Tab 1: "Configuration Details" (Descriptions table with 2-column key-value pairs).
   - Tab 2: "Audit Trail & Timeline" (Vertical timeline nodes with status icons).
   - Tab 3: "Related Resources" (Sub-table of connected entities).
