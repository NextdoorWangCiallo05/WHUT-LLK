#include "classicrankdialog.h"
#include "classicrankmanager.h"
#include "thememanager.h"
#include "uistyle.h"
#include "glassmessagebox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QFileDialog>
#include <QStandardPaths>

// 将秒数格式化为 mm:ss 的形式
static QString formatSecClassic(int sec)
{
    if (sec < 0) sec = 0;
    int m = sec / 60;
    int s = sec % 60;
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

ClassicRankDialog::ClassicRankDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(560, 400);
    setModal(true);

    auto applyDialogBg = [this]() {
        const QString path = ThemeManager::instance().backgroundPathDialog();
        setStyleSheet(QString(
            "QDialog { border-image: url(%1) 0 0 0 0 stretch stretch; }"
        ).arg(path));
        };
    applyDialogBg();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, applyDialogBg);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(12);

    setupGlassDialogTopBar(this, lay, "经典模式排行榜", "经典模式排行榜");

    QLabel* title = new QLabel("经典模式排行榜（用时越短越好）", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    QWidget* tableCard = new QWidget(this);
    tableCard->setStyleSheet(glassCardStyle());

    auto* tableShadow = new QGraphicsDropShadowEffect(tableCard);
    tableShadow->setBlurRadius(18);
    tableShadow->setOffset(0, 4);
    tableShadow->setColor(QColor(0, 0, 0, 55));
    tableCard->setGraphicsEffect(tableShadow);

    table = new QTableWidget(tableCard);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({ "昵称", "用时", "日期" });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setAlternatingRowColors(true);
    table->setShowGrid(true);

    table->setStyleSheet(R"(
        QTableWidget {
            background-color: rgba(255, 255, 255, 0.12);
            alternate-background-color: rgba(255, 255, 255, 0.08);
            color: rgba(0, 0, 0, 0.92);
            gridline-color: rgba(255, 255, 255, 0.18);
            border: 1px solid rgba(255,255,255,0.18);
            border-radius: 12px;
            font-size: 14px;
            selection-background-color: rgba(30,110,244,0.25);
            selection-color: rgba(0,0,0,0.98);
        }
        QHeaderView::section {
            background-color: rgba(255,255,255,0.18);
            color: rgba(0,0,0,0.95);
            font-weight: 700;
            border: none;
            padding: 8px;
        }
        QTableWidget::item {
            padding: 6px;
        }
    )");

    QVBoxLayout* cardLay = new QVBoxLayout(tableCard);
    cardLay->setContentsMargins(8, 8, 8, 8);
    cardLay->addWidget(table);

    btnExport = new QPushButton("导出", this);
    btnClear = new QPushButton("清空", this);
    btnClose = new QPushButton("关闭", this);

    for (auto* b : { btnExport, btnClear, btnClose }) {
        b->setFixedHeight(40);
        b->setMinimumWidth(110);
        applyGlassShadow(b, QColor(0, 0, 0, 85));
    }

    btnExport->setStyleSheet(glassButtonStyle(QColor(48, 209, 88)));
    btnClear->setStyleSheet(glassButtonStyle(QColor(255, 120, 70)));
    btnClose->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));

    QHBoxLayout* bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    bottomRow->addWidget(btnExport);
    bottomRow->addSpacing(10);
    bottomRow->addWidget(btnClear);
    bottomRow->addSpacing(10);
    bottomRow->addWidget(btnClose);
    bottomRow->addStretch();

    lay->addWidget(title);
    lay->addWidget(tableCard, 1);
    lay->addLayout(bottomRow);

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    connect(btnClear, &QPushButton::clicked, this, [this]() {
        if (!GlassMessageBox::question(this, "确认", "确定要清空经典排行榜吗？")) return;
        ClassicRankManager::instance().clear();
        refreshTable();
        GlassMessageBox::information(this, "提示", "经典排行榜已清空。");
        });

    connect(btnExport, &QPushButton::clicked, this, [this]() {
        ClassicRankManager::instance().load();
        QString def = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + "/classic_rank.csv";
        QString file = QFileDialog::getSaveFileName(this, "导出经典排行榜", def, "CSV 文件 (*.csv)");
        if (file.isEmpty()) return;

        if (ClassicRankManager::instance().exportToCsv(file)) {
            GlassMessageBox::information(this, "提示", "导出成功。");
        }
        else {
            GlassMessageBox::warning(this, "提示", "导出失败，请检查路径权限。");
        }
        });

    refreshTable();
}

void ClassicRankDialog::refreshTable()
{
    ClassicRankManager::instance().load();
    QList<ClassicRankRecord> records = ClassicRankManager::instance().topRecords(10);

    table->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const ClassicRankRecord& r = records[i];

        auto* itemName = new QTableWidgetItem(r.nickname);
        auto* itemTime = new QTableWidgetItem(formatSecClassic(r.usedSec));
        auto* itemDate = new QTableWidgetItem(r.dateTime.toString("yyyy-MM-dd HH:mm"));

        itemName->setTextAlignment(Qt::AlignCenter);
        itemTime->setTextAlignment(Qt::AlignCenter);
        itemDate->setTextAlignment(Qt::AlignCenter);

        if (i == 0) {
            QColor gold(212, 175, 55);
            itemName->setBackground(gold.lighter(165));
            itemTime->setBackground(gold.lighter(165));
            itemDate->setBackground(gold.lighter(165));
        }
        else if (i == 1) {
            QColor silver(192, 192, 192);
            itemName->setBackground(silver.lighter(120));
            itemTime->setBackground(silver.lighter(120));
            itemDate->setBackground(silver.lighter(120));
        }
        else if (i == 2) {
            QColor bronze(205, 127, 50);
            itemName->setBackground(bronze.lighter(130));
            itemTime->setBackground(bronze.lighter(130));
            itemDate->setBackground(bronze.lighter(130));
        }

        table->setItem(i, 0, itemName);
        table->setItem(i, 1, itemTime);
        table->setItem(i, 2, itemDate);
    }
}

void ClassicRankDialog::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QDialog::mousePressEvent(event);
}

void ClassicRankDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QDialog::mouseMoveEvent(event);
}

void ClassicRankDialog::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QDialog::mouseReleaseEvent(event);
}