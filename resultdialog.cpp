#include "resultdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ResultDialog::ResultDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("结算");
    setFixedSize(420, 300);
    setModal(true);

    titleLabel = new QLabel("结算", this);
    detailLabel = new QLabel(this);
    gradeLabel = new QLabel(this);
    okBtn = new QPushButton("确定", this);

    titleLabel->setAlignment(Qt::AlignCenter);
    detailLabel->setAlignment(Qt::AlignCenter);
    gradeLabel->setAlignment(Qt::AlignCenter);

    titleLabel->setStyleSheet("font-size:24px;font-weight:bold;color:#2c3e50;");
    detailLabel->setStyleSheet("font-size:16px;color:#34495e;");
    gradeLabel->setStyleSheet("font-size:28px;font-weight:bold;color:#e67e22;");
    okBtn->setFixedHeight(38);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(12);
    lay->addWidget(titleLabel);
    lay->addStretch();
    lay->addWidget(detailLabel);
    lay->addWidget(gradeLabel);
    lay->addStretch();
    lay->addWidget(okBtn, 0, Qt::AlignCenter);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

QString ResultDialog::calcGrade(bool win, int usedSec, int leftSec) const
{
    if (!win) return "D";
    if (leftSec >= 180) return "S";
    if (leftSec >= 90)  return "A";
    if (leftSec >= 30)  return "B";
    return "C";
}

void ResultDialog::setResult(bool win, int totalSec, int leftSec, int removedPairs)
{
    int usedSec = totalSec - leftSec;
    int usedMin = usedSec / 60, usedRem = usedSec % 60;
    int leftMin = leftSec / 60, leftRem = leftSec % 60;

    titleLabel->setText(win ? "挑战成功！" : "挑战失败");

    detailLabel->setText(QString("用时：%1:%2\n剩余：%3:%4\n消除对数：%5")
        .arg(usedMin, 2, 10, QChar('0'))
        .arg(usedRem, 2, 10, QChar('0'))
        .arg(leftMin, 2, 10, QChar('0'))
        .arg(leftRem, 2, 10, QChar('0'))
        .arg(removedPairs));

    gradeLabel->setText("评分：" + calcGrade(win, usedSec, leftSec));
}