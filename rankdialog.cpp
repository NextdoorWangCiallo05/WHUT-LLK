#include "rankdialog.h"
#include "rankmanager.h"
#include "thememanager.h"
#include "uistyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QColor>

static QString formatSec(int sec)
{
    if (sec < 0) sec = 0;
    int m = sec / 60;
    int s = sec % 60;
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

RankDialog::RankDialog(QWidget* parent)
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

    setupGlassDialogTopBar(this, lay, "计时模式排行榜", "计时模式排行榜");

    QLabel* title = new QLabel("计时模式排行榜", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    QWidget* tableCard = new QWidget(this);
    tableCard->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 12px;
        }
    )");

    auto* tableShadow = new QGraphicsDropShadowEffect(tableCard);
    tableShadow->setBlurRadius(18);
    tableShadow->setOffset(0, 4);
    tableShadow->setColor(QColor(0, 0, 0, 55));
    tableCard->setGraphicsEffect(tableShadow);

    table = new QTableWidget(tableCard);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({ "昵称", "剩余时间", "日期" });
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

    QVBoxLayout* tableLay = new QVBoxLayout(tableCard);
    tableLay->setContentsMargins(8, 8, 8, 8);
    tableLay->addWidget(table);

    btnClose = new QPushButton("关闭", this);
    btnClose->setFixedHeight(40);
    btnClose->setMinimumWidth(120);
    btnClose->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    applyGlassShadow(btnClose, QColor(0, 0, 0, 85));

    lay->addWidget(title);
    lay->addWidget(tableCard, 1);
    lay->addWidget(btnClose, 0, Qt::AlignCenter);

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    refreshTable();
}

void RankDialog::refreshTable()
{
    RankManager::instance().load();
    QList<RankRecord> records = RankManager::instance().topRecords(10);

    table->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const RankRecord& r = records[i];

        auto* itemName = new QTableWidgetItem(r.nickname);
        auto* itemTime = new QTableWidgetItem(formatSec(r.leftSec));
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

void RankDialog::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QDialog::mousePressEvent(event);
}

void RankDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QDialog::mouseMoveEvent(event);
}

void RankDialog::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QDialog::mouseReleaseEvent(event);
}