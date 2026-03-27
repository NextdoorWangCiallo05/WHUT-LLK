#include "helpdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HelpDialog::HelpDialog(QWidget* parent)
    : QDialog(parent),
    titleLabel(new QLabel("帮助说明", this)),
    contentLabel(new QLabel(this)),
    closeBtn(new QPushButton("我知道了", this))
{
    setWindowTitle("帮助");
    setFixedSize(520, 360);
    setModal(true);

    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel{
            font-size: 22px;
            font-weight: bold;
            color: #2c3e50;
        }
    )");

    contentLabel->setWordWrap(true);
    contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLabel->setStyleSheet(R"(
        QLabel{
            font-size: 16px;
            color: #34495e;
            background-color: rgba(255,255,255,0.75);
            border-radius: 8px;
            padding: 12px;
        }
    )");

    closeBtn->setFixedHeight(40);
    closeBtn->setStyleSheet(R"(
        QPushButton{
            font-size: 15px;
            color: white;
            background-color: rgba(52,152,219,0.95);
            border: none;
            border-radius: 6px;
            padding: 6px 16px;
        }
        QPushButton:hover{ background-color: rgba(64,172,245,1); }
        QPushButton:pressed{ background-color: rgba(41,128,185,1); }
    )");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(titleLabel);
    layout->addWidget(contentLabel, 1);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void HelpDialog::setHelpTitle(const QString& title)
{
    titleLabel->setText(title);
}

void HelpDialog::setHelpText(const QString& text)
{
    contentLabel->setText(text);
}