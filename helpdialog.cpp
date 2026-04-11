#include "helpdialog.h"
#include "thememanager.h"
#include "uistyle.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QColor>

// HelpDialog.cpp
// 这个对话框用于显示游戏的帮助信息，包含一个标题和一段说明文本，以及一个关闭按钮
HelpDialog::HelpDialog(QWidget* parent)
    : QDialog(parent),
    titleLabel(new QLabel(this)),
    contentLabel(new QLabel(this)),
    closeBtn(new QPushButton("我知道了", this))
{
    setFixedSize(520, 360);
    setModal(true);

    auto applyDialogBg = [this]() {
        const QString path = ThemeManager::instance().backgroundPathDialog();
        setStyleSheet(QString(
            "QDialog { border-image: url(%1) 0 0 0 0 stretch stretch; }"
        ).arg(path));
        };
    applyDialogBg();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, applyDialogBg);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // 统一自定义顶栏
    setupGlassDialogTopBar(this, layout, "帮助", "帮助");

    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(glassTitleLabelStyle());

    contentLabel->setWordWrap(true);
    contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLabel->setStyleSheet(glassTextLabelStyle());

    closeBtn->setFixedHeight(40);
    closeBtn->setMinimumWidth(140);
    closeBtn->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    applyGlassShadow(closeBtn, QColor(0, 0, 0, 85));

    layout->addWidget(titleLabel);
    layout->addWidget(contentLabel, 1);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

// 设置帮助对话框的标题文本
void HelpDialog::setHelpTitle(const QString& title)
{
    titleLabel->setText(title);
}

// 设置帮助对话框的内容文本
void HelpDialog::setHelpText(const QString& text)
{
    contentLabel->setText(text);
}
