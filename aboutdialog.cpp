#include "aboutdialog.h"
#include "thememanager.h"
#include "uistyle.h"
#include "windowdrag.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QColor>

// aboutdialog.cpp
// 这个文件实现了关于对话框的功能，显示应用程序的版本信息和简介

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent),
    titleLabel(new QLabel(this)),
    contentLabel(new QLabel(this)),
    closeBtn(new QPushButton("关闭", this))
{
    setFixedSize(420, 280);
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

    setupGlassDialogTopBar(this, layout, "关于", "关于");

    titleLabel->setText("关于连连看");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(glassTitleLabelStyle());

    contentLabel->setText(
        "连连看  版本：1.0.6.1\n\n"
        "这是一个使用 Qt6 开发的连连看小游戏。\n"
        "支持经典、休闲、计时、关卡和自定义五种模式，\n"
        "并包含提示、重排、排行榜、4种主题样式和结算功能。\n"
        "最后更改于2026.4.14。"
    );
    contentLabel->setWordWrap(true);
    contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    contentLabel->setStyleSheet(glassTextLabelStyle());

    closeBtn->setFixedHeight(40);
    closeBtn->setMinimumWidth(120);
    closeBtn->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    applyGlassShadow(closeBtn, QColor(0, 0, 0, 85));

    layout->addWidget(titleLabel);
    layout->addWidget(contentLabel, 1);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
