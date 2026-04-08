#include "resultdialog.h"
#include "uistyle.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QColor>

ResultDialog::ResultDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(460, 340);
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

    // 统一玻璃顶栏
    setupGlassDialogTopBar(this, lay, "结算", "结算");

    // 标题卡
    titleLabel = new QLabel("结算", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(glassTitleLabelStyle());

    // 内容卡片
    QWidget* detailCard = new QWidget(this);
    detailCard->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 12px;
        }
    )");

    auto* cardShadow = new QGraphicsDropShadowEffect(detailCard);
    cardShadow->setBlurRadius(18);
    cardShadow->setOffset(0, 4);
    cardShadow->setColor(QColor(0, 0, 0, 55));
    detailCard->setGraphicsEffect(cardShadow);

    QVBoxLayout* cardLay = new QVBoxLayout(detailCard);
    cardLay->setContentsMargins(14, 14, 14, 14);
    cardLay->setSpacing(10);

    detailLabel = new QLabel(detailCard);
    detailLabel->setAlignment(Qt::AlignCenter);
    detailLabel->setWordWrap(true);
    detailLabel->setStyleSheet(R"(
        QLabel{
            font-size: 15px;
            font-weight: 600;
            color: rgba(0,0,0,0.94);
            background: transparent;
            border: none;
            line-height: 1.35;
        }
    )");

    gradeLabel = new QLabel(detailCard);
    gradeLabel->setAlignment(Qt::AlignCenter);
    gradeLabel->setStyleSheet(R"(
        QLabel{
            font-size: 34px;
            font-weight: 900;
            color: rgba(230,126,34,0.98);
            background-color: rgba(255,255,255,0.18);
            border: 1px solid rgba(255,255,255,0.20);
            border-radius: 12px;
            padding: 8px 12px;
        }
    )");

    cardLay->addWidget(detailLabel);
    cardLay->addWidget(gradeLabel);

    okBtn = new QPushButton("确定", this);
    okBtn->setFixedHeight(40);
    okBtn->setMinimumWidth(120);
    okBtn->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    applyGlassShadow(okBtn, QColor(0, 0, 0, 85));

    lay->addWidget(titleLabel);
    lay->addWidget(detailCard, 1);
    lay->addWidget(okBtn, 0, Qt::AlignCenter);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

QString ResultDialog::calcGrade(bool win, int usedSec, int leftSec) const
{
    Q_UNUSED(usedSec);
    if (!win) return "D";
    if (leftSec >= 180) return "S";
    if (leftSec >= 90)  return "A";
    if (leftSec >= 30)  return "B";
    return "C";
}

void ResultDialog::setResult(bool win, int totalSec, int leftSec, int removedPairs)
{
    if (leftSec < 0) leftSec = 0;
    if (totalSec < 0) totalSec = 0;
    if (leftSec > totalSec) leftSec = totalSec;

    int usedSec = totalSec - leftSec;
    int usedMin = usedSec / 60, usedRem = usedSec % 60;
    int leftMin = leftSec / 60, leftRem = leftSec % 60;
    int totalMin = totalSec / 60, totalRem = totalSec % 60;

    titleLabel->setText(win ? "挑战成功！" : "挑战失败");

    detailLabel->setText(QString(
        "总时长：%1:%2\n"
        "用时：%3:%4\n"
        "剩余：%5:%6\n"
        "消除对数：%7")
        .arg(totalMin, 2, 10, QChar('0'))
        .arg(totalRem, 2, 10, QChar('0'))
        .arg(usedMin, 2, 10, QChar('0'))
        .arg(usedRem, 2, 10, QChar('0'))
        .arg(leftMin, 2, 10, QChar('0'))
        .arg(leftRem, 2, 10, QChar('0'))
        .arg(removedPairs));

    const QString g = calcGrade(win, usedSec, leftSec);
    gradeLabel->setText("评分  " + g);

    // 胜负差异化配色
    if (win) {
        gradeLabel->setStyleSheet(R"(
            QLabel{
                font-size: 34px;
                font-weight: 900;
                color: rgba(32,140,85,0.98);
                background-color: rgba(255,255,255,0.20);
                border: 1px solid rgba(255,255,255,0.20);
                border-radius: 12px;
                padding: 8px 12px;
            }
        )");
    }
    else {
        gradeLabel->setStyleSheet(R"(
            QLabel{
                font-size: 34px;
                font-weight: 900;
                color: rgba(210,70,70,0.98);
                background-color: rgba(255,255,255,0.20);
                border: 1px solid rgba(255,255,255,0.20);
                border-radius: 12px;
                padding: 8px 12px;
            }
        )");
    }
}