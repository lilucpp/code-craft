#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QFont>
#include <QStyle>
#include <QTest>
#include <QGraphicsDropShadowEffect>
#include <iostream>
#include <vector>
#include <functional>

// Helper to refresh dynamic QSS properties
void polishWidget(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
}

// Helper to apply Ant shadow
void applyAntShadow(QWidget* w, int blurRadius = 12, int yOffset = 2, int alpha = 20) {
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(w);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, yOffset);
    shadow->setColor(QColor(0, 0, 0, alpha));
    w->setGraphicsEffect(shadow);
}

// Data Model Definition
struct RuleItem {
    int id;
    QString name;
    QString desc;
    QString calls;
    QString status; // "运行中", "上线中", "异常", "已停用"
    QString time;
};

// Lightweight Tag container for Table cell without hardcoded backgrounds
QWidget* createStatusTag(const QString& status) {
    QWidget* container = new QWidget();
    container->setStyleSheet("background: transparent;");
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* tag = new QLabel(status);
    tag->setAlignment(Qt::AlignCenter);
    tag->setFixedHeight(22);
    tag->setMinimumWidth(60);

    if (status == "运行中") {
        tag->setProperty("tag", "success");
    } else if (status == "上线中") {
        tag->setProperty("tag", "processing");
    } else if (status == "异常") {
        tag->setProperty("tag", "error");
    } else {
        tag->setProperty("tag", "default");
    }
    polishWidget(tag);

    layout->addWidget(tag);
    return container;
}

int main(int argc, char *argv[]) {
    // 1. High-DPI Scaling Guardrails
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    QFont font = app.font();
    font.setFamily("Segoe UI, -apple-system, BlinkMacSystemFont, Microsoft YaHei, sans-serif");
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPointSize(10);
    app.setFont(font);

    // Load Ant Light Theme QSS
    QString qssLightPath = "demos/qss/ant_light_theme.qss";
    if (!QFile::exists(qssLightPath)) {
        qssLightPath = "../qss/ant_light_theme.qss";
    }
    QFile qssFile(qssLightPath);
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    // Mock 24 items dataset
    std::vector<RuleItem> allRules = {
        {1, "Rule-001-Gateway", "API 网关超时熔断策略", "12,450 万次", "运行中", "2026-11-05 10:20"},
        {2, "Rule-002-AuthService", "JWT 密钥自动定期轮换", "8,920 万次", "运行中", "2026-11-04 18:00"},
        {3, "Rule-003-OrderSync", "订单分布式异步落盘任务", "3,110 万次", "上线中", "2026-11-05 11:45"},
        {4, "Rule-004-PaymentCheck", "第三方对账平账自动化对冲", "540 万次", "异常", "2026-11-03 09:15"},
        {5, "Rule-005-LogArchiver", "冷数据定期归档 S3 压缩存储", "98 万次", "已停用", "2026-10-28 14:00"},
        {6, "Rule-006-K8sScaler", "HPA 自动扩缩容弹性响应规则", "45,200 万次", "运行中", "2026-11-05 12:00"},
        {7, "Rule-007-DDoSShield", "四层/七层流量突增智能防护", "98,100 万次", "运行中", "2026-11-05 08:30"},
        {8, "Rule-008-DataMasking", "生产环境敏感字段动态脱敏", "1,240 万次", "运行中", "2026-11-02 16:10"},
        {9, "Rule-009-CacheWarmup", "Redis 核心集群定时全量预热", "6,800 万次", "上线中", "2026-11-05 12:15"},
        {10, "Rule-010-DBVacuum", "PostgreSQL 脏页定期后台整理", "310 万次", "已停用", "2026-10-15 03:00"},
        {11, "Rule-011-KafkaRebalance", "Kafka 分区消费倾斜再平衡", "24,500 万次", "运行中", "2026-11-04 21:00"},
        {12, "Rule-012-CertRenewal", "HTTPS SSL 证书自动化延期", "12 万次", "运行中", "2026-11-01 00:00"},
        {13, "Rule-013-AuditTrail", "特权账号操作行为链条追踪", "850 万次", "运行中", "2026-11-05 09:00"},
        {14, "Rule-014-RateLimiter", "OpenAPI 单租户 QPS 阈值限流", "32,000 万次", "运行中", "2026-11-05 11:10"},
        {15, "Rule-015-ESReindex", "Elasticsearch 历史索引跨代迁移", "180 万次", "异常", "2026-11-04 14:40"},
        {16, "Rule-016-AlertThrottle", "告警风暴智能收敛与静默", "7,600 万次", "运行中", "2026-11-05 10:00"},
        {17, "Rule-017-SecretVault", "HashiCorp Vault 动态令牌续期", "4,300 万次", "上线中", "2026-11-05 12:20"},
        {18, "Rule-018-BackupSnapshot", "RDS 云原生跨可用区每日快照", "620 万次", "运行中", "2026-11-05 04:00"},
        {19, "Rule-019-TraceSampler", "OpenTelemetry 慢调用采样沉淀", "18,900 万次", "运行中", "2026-11-05 11:50"},
        {20, "Rule-020-SchemaValidator", "Protobuf 协议向前兼容性校验", "2,100 万次", "已停用", "2026-10-20 17:30"},
        {21, "Rule-021-DNSFailover", "GlobalAnycast 节点秒级漂移", "88,000 万次", "运行中", "2026-11-05 10:45"},
        {22, "Rule-022-CostOptimizer", "Spot 竞价实例闲时自动腾退", "510 万次", "上线中", "2026-11-05 07:15"},
        {23, "Rule-023-ModelInference", "TensorRT 模型推理并发批处理", "14,000 万次", "运行中", "2026-11-05 12:10"},
        {24, "Rule-024-EdgePurge", "CDN 全网边缘缓存批量主动刷新", "930 万次", "异常", "2026-11-03 19:20"}
    };

    std::vector<RuleItem> filteredRules = allRules;
    const int pageSize = 6;
    int currentPage = 1;
    int selectedRuleId = -1;

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("AntQt - Ant Design Pro 企业级高级查询表格与抽屉");
    mainWindow.resize(1180, 800);

    QWidget* centralWidget = new QWidget();
    centralWidget->setObjectName("centralWidget");
    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(16);

    // ==========================================
    // 1. 顶部查询表单卡片 (Filter Card)
    // ==========================================
    QFrame* filterCard = new QFrame();
    filterCard->setProperty("type", "card");
    applyAntShadow(filterCard, 12, 2, 18);
    QGridLayout* filterGrid = new QGridLayout(filterCard);
    filterGrid->setContentsMargins(16, 16, 16, 16);
    filterGrid->setHorizontalSpacing(20);
    filterGrid->setVerticalSpacing(12);

    QLabel* lblName = new QLabel("规则搜索:");
    lblName->setProperty("type", "secondary");
    QLineEdit* editName = new QLineEdit();
    editName->setPlaceholderText("按规则名或描述过滤");
    editName->setMinimumWidth(180);

    QLabel* lblStatus = new QLabel("服务状态:");
    lblStatus->setProperty("type", "secondary");
    QComboBox* comboStatus = new QComboBox();
    comboStatus->addItems({"全部状态", "运行中", "上线中", "异常", "已停用"});
    comboStatus->setMinimumWidth(140);

    QHBoxLayout* btnFilterLayout = new QHBoxLayout();
    btnFilterLayout->setSpacing(10);
    QPushButton* btnQuery = new QPushButton("查询");
    btnQuery->setProperty("type", "primary");
    polishWidget(btnQuery);
    QPushButton* btnReset = new QPushButton("重置");
    btnFilterLayout->addWidget(btnQuery);
    btnFilterLayout->addWidget(btnReset);

    filterGrid->addWidget(lblName, 0, 0);
    filterGrid->addWidget(editName, 0, 1);
    filterGrid->addWidget(lblStatus, 0, 2);
    filterGrid->addWidget(comboStatus, 0, 3);
    filterGrid->addLayout(btnFilterLayout, 0, 4);
    filterGrid->setColumnStretch(5, 1);

    rootLayout->addWidget(filterCard);

    // ==========================================
    // 2. 主体区 (表格卡片 + 右侧详情抽屉)
    // ==========================================
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(16);

    // 表格卡片
    QFrame* tableCard = new QFrame();
    tableCard->setProperty("type", "card");
    applyAntShadow(tableCard, 14, 2, 20);
    QVBoxLayout* tableCardLayout = new QVBoxLayout(tableCard);
    tableCardLayout->setContentsMargins(16, 16, 16, 16);
    tableCardLayout->setSpacing(14);

    // Toolbar
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    QLabel* tableTitle = new QLabel("查询表格 (Pro Table)");
    tableTitle->setProperty("type", "title-md");
    QLabel* countBadge = new QLabel();
    countBadge->setProperty("type", "muted");

    QPushButton* btnNew = new QPushButton("+ 新建规则");
    btnNew->setProperty("type", "primary");
    polishWidget(btnNew);
    QPushButton* btnExport = new QPushButton("批量导出");

    QPushButton* btnTheme = new QPushButton("切换暗黑主题");
    btnTheme->setFixedHeight(32);
    static bool isDark = false;
    QObject::connect(btnTheme, &QPushButton::clicked, [&]() {
        isDark = !isDark;
        QString nextQss = isDark ? "demos/qss/ant_dark_theme.qss" : "demos/qss/ant_light_theme.qss";
        if (!QFile::exists(nextQss)) {
            nextQss = isDark ? "../qss/ant_dark_theme.qss" : "../qss/ant_light_theme.qss";
        }
        QFile file(nextQss);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            app.setStyleSheet(file.readAll());
            file.close();
        }
        btnTheme->setText(isDark ? "切换亮色主题" : "切换暗黑主题");
    });

    toolbarLayout->addWidget(tableTitle);
    toolbarLayout->addWidget(countBadge);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(btnTheme);
    toolbarLayout->addWidget(btnExport);
    toolbarLayout->addWidget(btnNew);
    tableCardLayout->addLayout(toolbarLayout);

    // Table Widget
    QTableWidget* tableWidget = new QTableWidget(pageSize, 5);
    tableWidget->setHorizontalHeaderLabels({"规则名称", "描述信息", "服务调用数", "状态", "更新时间"});
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setShowGrid(false);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tableWidget->horizontalHeader()->setHighlightSections(false);
    tableWidget->setFocusPolicy(Qt::NoFocus);

    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    tableWidget->setColumnWidth(3, 110);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    tableCardLayout->addWidget(tableWidget, 1);

    // ==========================================
    // 2.1 经典 Ant Design Pagination 分页工具栏
    // ==========================================
    QHBoxLayout* paginationLayout = new QHBoxLayout();
    QLabel* pageInfoLabel = new QLabel();
    pageInfoLabel->setProperty("type", "secondary");
    paginationLayout->addWidget(pageInfoLabel);
    paginationLayout->addStretch();

    QPushButton* btnPrev = new QPushButton("<");
    btnPrev->setProperty("type", "page-btn");
    polishWidget(btnPrev);
    paginationLayout->addWidget(btnPrev);

    QHBoxLayout* pageNumbersLayout = new QHBoxLayout();
    pageNumbersLayout->setSpacing(6);
    paginationLayout->addLayout(pageNumbersLayout);

    QPushButton* btnNext = new QPushButton(">");
    btnNext->setProperty("type", "page-btn");
    polishWidget(btnNext);
    paginationLayout->addWidget(btnNext);

    tableCardLayout->addLayout(paginationLayout);
    bodyLayout->addWidget(tableCard, 1);

    // ==========================================
    // 3. 右侧联动抽屉 (Drawer)
    // ==========================================
    QFrame* drawer = new QFrame();
    drawer->setFixedWidth(340);
    drawer->setProperty("type", "card");
    applyAntShadow(drawer, 20, 2, 35);
    QVBoxLayout* drawerLayout = new QVBoxLayout(drawer);
    drawerLayout->setContentsMargins(18, 18, 18, 18);
    drawerLayout->setSpacing(14);

    QHBoxLayout* drawerHeader = new QHBoxLayout();
    QLabel* drawerTitle = new QLabel("规则详情抽屉");
    drawerTitle->setProperty("type", "title-md");
    QPushButton* btnCloseDrawer = new QPushButton("×");
    btnCloseDrawer->setProperty("type", "close");
    btnCloseDrawer->setFixedSize(28, 28);
    polishWidget(btnCloseDrawer);

    drawerHeader->addWidget(drawerTitle);
    drawerHeader->addStretch();
    drawerHeader->addWidget(btnCloseDrawer);
    drawerLayout->addLayout(drawerHeader);

    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #E8E8E8;");
    drawerLayout->addWidget(divider);

    QLabel* lblDNameKey = new QLabel("规则名称");
    lblDNameKey->setProperty("type", "muted");
    QLabel* lblDNameVal = new QLabel();
    lblDNameVal->setProperty("type", "title-md");

    QLabel* lblDDescKey = new QLabel("功能描述");
    lblDDescKey->setProperty("type", "muted");
    QLabel* lblDDescVal = new QLabel();
    lblDDescVal->setProperty("type", "secondary");
    lblDDescVal->setWordWrap(true);

    QLabel* lblDCallsKey = new QLabel("服务调用总数");
    lblDCallsKey->setProperty("type", "muted");
    QLabel* lblDCallsVal = new QLabel();
    lblDCallsVal->setStyleSheet("font-size: 16px; font-weight: 600; color: #1677FF;");

    QLabel* lblDStatusKey = new QLabel("当前运行状态");
    lblDStatusKey->setProperty("type", "muted");
    QLabel* lblDStatusVal = new QLabel();

    drawerLayout->addWidget(lblDNameKey);
    drawerLayout->addWidget(lblDNameVal);
    drawerLayout->addSpacing(4);
    drawerLayout->addWidget(lblDDescKey);
    drawerLayout->addWidget(lblDDescVal);
    drawerLayout->addSpacing(4);
    drawerLayout->addWidget(lblDCallsKey);
    drawerLayout->addWidget(lblDCallsVal);
    drawerLayout->addSpacing(4);
    drawerLayout->addWidget(lblDStatusKey);
    drawerLayout->addWidget(lblDStatusVal);
    drawerLayout->addStretch();

    // Drawer Actions: 编辑与状态切换
    QHBoxLayout* drawerActions = new QHBoxLayout();
    QPushButton* btnEditRule = new QPushButton("编辑规则");
    btnEditRule->setProperty("type", "primary");
    polishWidget(btnEditRule);
    QPushButton* btnToggleStatus = new QPushButton("停用规则");
    btnToggleStatus->setProperty("type", "danger");
    polishWidget(btnToggleStatus);
    drawerActions->addWidget(btnEditRule);
    drawerActions->addWidget(btnToggleStatus);
    drawerLayout->addLayout(drawerActions);

    drawer->hide();
    bodyLayout->addWidget(drawer);
    rootLayout->addLayout(bodyLayout, 1);

    // ==========================================
    // Function: 刷新表格数据与分页渲染
    // ==========================================
    std::function<void()> refreshTable;
    refreshTable = [&]() {
        int totalItems = static_cast<int>(filteredRules.size());
        int totalPages = (totalItems + pageSize - 1) / pageSize;
        if (totalPages == 0) totalPages = 1;
        if (currentPage > totalPages) currentPage = totalPages;
        if (currentPage < 1) currentPage = 1;

        countBadge->setText(QString("（共 %1 条规则配置）").arg(totalItems));
        pageInfoLabel->setText(QString("共 %1 条数据，第 %2 / %3 页").arg(totalItems).arg(currentPage).arg(totalPages));

        int startIdx = (currentPage - 1) * pageSize;
        int endIdx = std::min(startIdx + pageSize, totalItems);
        int rowsToShow = endIdx - startIdx;

        tableWidget->setRowCount(rowsToShow);

        for (int i = 0; i < rowsToShow; ++i) {
            const auto& item = filteredRules[startIdx + i];
            tableWidget->setRowHeight(i, 44);

            QTableWidgetItem* itName = new QTableWidgetItem(item.name);
            itName->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
            itName->setData(Qt::UserRole, item.id);
            tableWidget->setItem(i, 0, itName);

            tableWidget->setItem(i, 1, new QTableWidgetItem(item.desc));

            QTableWidgetItem* itCall = new QTableWidgetItem(item.calls);
            tableWidget->setItem(i, 2, itCall);

            tableWidget->setCellWidget(i, 3, createStatusTag(item.status));
            tableWidget->setItem(i, 4, new QTableWidgetItem(item.time));
        }

        // Render Page Numbers
        QLayoutItem* item;
        while ((item = pageNumbersLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        btnPrev->setEnabled(currentPage > 1);
        btnNext->setEnabled(currentPage < totalPages);

        for (int p = 1; p <= totalPages; ++p) {
            QPushButton* btnP = new QPushButton(QString::number(p));
            btnP->setProperty("type", "page-btn");
            if (p == currentPage) {
                btnP->setProperty("active", "true");
            }
            polishWidget(btnP);
            QObject::connect(btnP, &QPushButton::clicked, [&, p]() {
                currentPage = p;
                refreshTable();
            });
            pageNumbersLayout->addWidget(btnP);
        }
    };

    // Pagination Click Listeners
    QObject::connect(btnPrev, &QPushButton::clicked, [&]() {
        if (currentPage > 1) {
            currentPage--;
            refreshTable();
        }
    });
    QObject::connect(btnNext, &QPushButton::clicked, [&]() {
        int totalPages = (static_cast<int>(filteredRules.size()) + pageSize - 1) / pageSize;
        if (currentPage < totalPages) {
            currentPage++;
            refreshTable();
        }
    });

    // Query & Filter Listener
    auto performFilter = [&]() {
        QString kw = editName->text().trimmed();
        QString st = comboStatus->currentText();

        filteredRules.clear();
        for (const auto& r : allRules) {
            bool matchKw = kw.isEmpty() || r.name.contains(kw, Qt::CaseInsensitive) || r.desc.contains(kw, Qt::CaseInsensitive);
            bool matchSt = (st == "全部状态") || (r.status == st);
            if (matchKw && matchSt) {
                filteredRules.push_back(r);
            }
        }
        currentPage = 1;
        refreshTable();
    };

    QObject::connect(btnQuery, &QPushButton::clicked, performFilter);
    QObject::connect(editName, &QLineEdit::returnPressed, performFilter);

    // Reset Listener
    QObject::connect(btnReset, &QPushButton::clicked, [&]() {
        editName->clear();
        comboStatus->setCurrentIndex(0);
        filteredRules = allRules;
        currentPage = 1;
        refreshTable();
        tableWidget->clearSelection();
        drawer->hide();
    });

    // Update Drawer view
    auto updateDrawerContent = [&](int ruleId) {
        for (auto& r : allRules) {
            if (r.id == ruleId) {
                selectedRuleId = ruleId;
                lblDNameVal->setText(r.name);
                lblDDescVal->setText(r.desc);
                lblDCallsVal->setText(r.calls);
                lblDStatusVal->setText(r.status);
                if (r.status == "异常") {
                    lblDStatusVal->setStyleSheet("font-weight: 600; color: #FF4D4F;");
                    btnToggleStatus->setText("恢复运行");
                } else if (r.status == "已停用") {
                    lblDStatusVal->setStyleSheet("font-weight: 600; color: #8C8C8C;");
                    btnToggleStatus->setText("启用规则");
                } else {
                    lblDStatusVal->setStyleSheet("font-weight: 600; color: #52C41A;");
                    btnToggleStatus->setText("停用规则");
                }
                drawer->show();
                break;
            }
        }
    };

    // Table Row Click
    QObject::connect(tableWidget, &QTableWidget::cellClicked, [&](int row, int /*col*/) {
        if (row >= 0 && row < tableWidget->rowCount()) {
            int id = tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
            updateDrawerContent(id);
        }
    });

    QObject::connect(btnCloseDrawer, &QPushButton::clicked, [drawer]() {
        drawer->hide();
    });

    // Toggle Status in Drawer
    QObject::connect(btnToggleStatus, &QPushButton::clicked, [&]() {
        for (auto& r : allRules) {
            if (r.id == selectedRuleId) {
                if (r.status == "已停用") {
                    r.status = "运行中";
                } else {
                    r.status = "已停用";
                }
                r.time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
                break;
            }
        }
        performFilter();
        updateDrawerContent(selectedRuleId);
    });

    // Edit Rule Dialog
    QObject::connect(btnEditRule, &QPushButton::clicked, [&]() {
        RuleItem* target = nullptr;
        for (auto& r : allRules) {
            if (r.id == selectedRuleId) {
                target = &r;
                break;
            }
        }
        if (!target) return;

        QDialog dlg(&mainWindow);
        dlg.setWindowTitle("编辑规则配置");
        dlg.setFixedWidth(420);

        QVBoxLayout* dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setSpacing(14);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        QLabel* dlgTitle = new QLabel("编辑当前规则");
        dlgTitle->setProperty("type", "title-md");
        dlgLayout->addWidget(dlgTitle);

        QLabel* lN = new QLabel("规则名称:");
        lN->setProperty("type", "secondary");
        QLineEdit* inName = new QLineEdit(target->name);

        QLabel* lD = new QLabel("功能描述:");
        lD->setProperty("type", "secondary");
        QLineEdit* inDesc = new QLineEdit(target->desc);

        QLabel* lS = new QLabel("服务状态:");
        lS->setProperty("type", "secondary");
        QComboBox* inStatus = new QComboBox();
        inStatus->addItems({"运行中", "上线中", "异常", "已停用"});
        inStatus->setCurrentText(target->status);

        dlgLayout->addWidget(lN);
        dlgLayout->addWidget(inName);
        dlgLayout->addWidget(lD);
        dlgLayout->addWidget(inDesc);
        dlgLayout->addWidget(lS);
        dlgLayout->addWidget(inStatus);

        QHBoxLayout* dlgBtnLayout = new QHBoxLayout();
        dlgBtnLayout->addStretch();
        QPushButton* btnCancel = new QPushButton("取消");
        QPushButton* btnSave = new QPushButton("确定");
        btnSave->setProperty("type", "primary");
        polishWidget(btnSave);

        dlgBtnLayout->addWidget(btnCancel);
        dlgBtnLayout->addWidget(btnSave);
        dlgLayout->addLayout(dlgBtnLayout);

        QObject::connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
        QObject::connect(btnSave, &QPushButton::clicked, [&]() {
            target->name = inName->text();
            target->desc = inDesc->text();
            target->status = inStatus->currentText();
            target->time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
            dlg.accept();
        });

        if (dlg.exec() == QDialog::Accepted) {
            performFilter();
            updateDrawerContent(selectedRuleId);
        }
    });

    // Initial Load
    refreshTable();

    mainWindow.setCentralWidget(centralWidget);
    mainWindow.show();

    // Check for auto-test or screenshot flag
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--auto-test") {
            mainWindow.resize(1180, 800);
            qApp->processEvents();

            std::cout << "[Test 1] Capturing Page 1 with 24 records & Pagination..." << std::endl;
            mainWindow.grab().save("demo2_1_table_page1.png");

            std::cout << "[Test 2] Simulating click on Page 2..." << std::endl;
            btnNext->click();
            qApp->processEvents();
            mainWindow.grab().save("demo2_2_table_page2.png");

            std::cout << "[Test 3] Simulating filter for 'Gateway'..." << std::endl;
            editName->setText("Gateway");
            btnQuery->click();
            qApp->processEvents();
            mainWindow.grab().save("demo2_3_filter_result.png");

            std::cout << "[Test 4] Simulating click on Row to open Drawer..." << std::endl;
            tableWidget->cellClicked(0, 0);
            qApp->processEvents();
            mainWindow.grab().save("demo2_4_drawer_edit.png");

            // Open and capture Edit Dialog
            std::cout << "[Test 4.1] Capturing Edit Rule Dialog..." << std::endl;
            QTimer::singleShot(200, [&]() {
                QWidget* activeModal = QApplication::activeModalWidget();
                if (activeModal) {
                    activeModal->grab().save("demo2_4_edit_dialog.png");
                    activeModal->close();
                }
            });
            btnEditRule->click();
            qApp->processEvents();

            std::cout << "[Test 5] Simulating Reset and Switch to Dark..." << std::endl;
            btnReset->click();
            btnTheme->click();
            qApp->processEvents();
            mainWindow.grab().save("demo2_5_dark_pagination.png");

            std::cout << "[Test Success] All Demo 2 simulated clicks and pagination completed." << std::endl;
            return 0;
        }

        if (QString(argv[i]) == "--screenshot" && i + 1 < argc) {
            mainWindow.resize(1180, 800);
            qApp->processEvents();
            QPixmap pix = mainWindow.grab();
            pix.save(argv[i + 1]);
            return 0;
        }
    }

    return app.exec();
}
