#pragma once
#ifndef UISTYLE_H
#define UISTYLE_H

#include <QString>
#include <QColor>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QObject>


// 这个文件定义了一些玻璃风格的UI样式和辅助函数，供整个应用程序使用
static inline QString glassButtonStyle(const QColor& baseColor)
{
    const QColor c1 = baseColor.lighter(120);
    const QColor c2 = baseColor.darker(112);

    return QString(R"(
        QPushButton {
            font-size: 15px;
            font-weight: 600;
            color: rgba(255, 255, 255, 0.96);
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%1, %2, %3, 220),
                stop: 0.45 rgba(%4, %5, %6, 165),
                stop: 1 rgba(%7, %8, %9, 200)
            );
            border: 1px solid rgba(255, 255, 255, 90);
            border-top: 1px solid rgba(255, 255, 255, 150);
            border-left: 1px solid rgba(255, 255, 255, 120);
            border-radius: 14px;
            padding: 8px 18px;
        }
        QPushButton:hover {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%10, %11, %12, 240),
                stop: 0.5 rgba(%13, %14, %15, 190),
                stop: 1 rgba(%16, %17, %18, 220)
            );
            border: 1px solid rgba(255, 255, 255, 140);
            border-top: 1px solid rgba(255, 255, 255, 180);
        }
        QPushButton:pressed {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%19, %20, %21, 190),
                stop: 1 rgba(%22, %23, %24, 170)
            );
            border: 1px solid rgba(255, 255, 255, 70);
        }
    )")
        .arg(c1.red()).arg(c1.green()).arg(c1.blue())
        .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue())
        .arg(c2.red()).arg(c2.green()).arg(c2.blue())
        .arg(c1.red()).arg(c1.green()).arg(c1.blue())
        .arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue())
        .arg(c2.red()).arg(c2.green()).arg(c2.blue())
        .arg(c1.red()).arg(c1.green()).arg(c1.blue())
        .arg(c2.red()).arg(c2.green()).arg(c2.blue());
}

/// 玻璃风格的卡片、标题标签、文本标签和时间标签样式
static inline QString glassCardStyle()
{
    return R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.10);
            border: 1px solid rgba(255, 255, 255, 0.14);
            border-radius: 12px;
        }
    )";
}

// 玻璃风格的标题标签、文本标签和时间标签样式
static inline QString glassTitleLabelStyle()
{
    return R"(
        QLabel{
            font-size: 24px;
            font-weight: 600;
            color: rgba(0,0,0,0.98);
            background-color: rgba(255,255,255,0.16);
            border: 1px solid rgba(255,255,255,0.18);
            border-radius: 14px;
            padding: 10px 14px;
        }
    )";
}

// 玻璃风格的文本标签样式，适合正文内容显示
static inline QString glassTextLabelStyle()
{
    return R"(
        QLabel{
            font-size: 15px;
            color: rgba(0,0,0,0.95);
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 12px;
            padding: 14px 16px;
        }
    )";
}

// 玻璃风格的时间标签样式，适合显示日期、时间等信息
static inline QString glassTimeLabelStyle()
{
    return R"(
        QLabel {
            font-size: 14px;
            font-weight: 600;
            color: rgba(255, 255, 255, 0.98);
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(135, 145, 155, 220),
                stop: 0.5 rgba(105, 115, 125, 195),
                stop: 1 rgba(80, 90, 100, 215)
            );
            border: 1px solid rgba(255, 255, 255, 80);
            border-top: 1px solid rgba(255, 255, 255, 130);
            border-radius: 14px;
            padding: 6px 14px;
        }
    )";
}


// 给按钮添加玻璃风格的阴影效果，默认使用半透明黑色阴影
static inline void applyGlassShadow(QPushButton* btn, const QColor& color = QColor(0, 0, 0, 85))
{
    auto* shadow = new QGraphicsDropShadowEffect(btn);
    shadow->setBlurRadius(22);
    shadow->setOffset(0, 6);
    shadow->setColor(color);
    btn->setGraphicsEffect(shadow);
}


// 给对话框添加玻璃风格的标题栏，包含应用图标、标题文本、最小化和关闭按钮
static inline void setupGlassDialogTopBar(
    QDialog* dlg,
    QVBoxLayout* mainLayout,
    const QString& titleText,
    const QString& windowTitleText)
{
    dlg->setWindowTitle(windowTitleText);
    dlg->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dlg->setAttribute(Qt::WA_TranslucentBackground, false);

    QWidget* topWidget = new QWidget(dlg);
    topWidget->setFixedHeight(44);
    topWidget->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.20);
            border: 1px solid rgba(255, 255, 255, 0.28);
            border-radius: 16px;
        }
    )");

    auto* topShadow = new QGraphicsDropShadowEffect(topWidget);
    topShadow->setBlurRadius(22);
    topShadow->setOffset(0, 6);
    topShadow->setColor(QColor(0, 0, 0, 45));
    topWidget->setGraphicsEffect(topShadow);

    QHBoxLayout* topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(12, 6, 12, 6);
    topLayout->setSpacing(8);

    QLabel* appIcon = new QLabel(topWidget);
    appIcon->setPixmap(QPixmap(":/images/app.ico").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    appIcon->setFixedSize(18, 18);
    appIcon->setStyleSheet("background: transparent; border: none;");

    QLabel* titleLabel = new QLabel(titleText, topWidget);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: rgba(0, 0, 0, 0.82);
            font-size: 15px;
            font-weight: 600;
            background: transparent;
            border: none;
        }
    )");

    QPushButton* btnMin = new QPushButton("–", topWidget);
    QPushButton* btnClose = new QPushButton("×", topWidget);

    btnMin->setFixedSize(28, 24);
    btnClose->setFixedSize(28, 24);

    btnMin->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            font-size: 18px;
            color: rgba(0,0,0,0.75);
            border-radius: 10px;
        }
        QPushButton:hover {
            background-color: rgba(255,255,255,0.35);
        }
    )");

    btnClose->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            font-size: 18px;
            color: rgba(0,0,0,0.75);
            border-radius: 10px;
        }
        QPushButton:hover {
            background-color: rgba(255,80,80,0.92);
            color: white;
        }
    )");

    topLayout->addWidget(appIcon);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnMin);
    topLayout->addWidget(btnClose);

    QObject::connect(btnMin, &QPushButton::clicked, dlg, &QWidget::showMinimized);
    QObject::connect(btnClose, &QPushButton::clicked, dlg, &QDialog::reject);

    mainLayout->insertWidget(0, topWidget);
}


// 玻璃风格的主菜单大按钮样式，适合放在主界面或设置界面等重要位置，突出显示功能入口
static inline QString mainMenuBigButtonStyle(const QColor& baseColor)
{
    const QColor top = baseColor.lighter(125);
    const QColor mid = baseColor;
    const QColor bottom = baseColor.darker(115);

    return QString(R"(
        QPushButton {
            font-size: 22px;
            font-weight: 700;
            color: rgba(255, 255, 255, 0.98);
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%1, %2, %3, 235),
                stop: 0.5 rgba(%4, %5, %6, 210),
                stop: 1 rgba(%7, %8, %9, 235)
            );
            border: 1px solid rgba(255, 255, 255, 80);
            border-top: 1px solid rgba(255, 255, 255, 150);
            border-left: 1px solid rgba(255, 255, 255, 110);
            border-radius: 24px;
            padding: 14px 24px;
        }
        QPushButton:hover {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%10, %11, %12, 245),
                stop: 0.5 rgba(%13, %14, %15, 225),
                stop: 1 rgba(%16, %17, %18, 245)
            );
            border: 1px solid rgba(255, 255, 255, 120);
            border-top: 1px solid rgba(255, 255, 255, 180);
        }
        QPushButton:pressed {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 1,
                stop: 0 rgba(%19, %20, %21, 210),
                stop: 1 rgba(%22, %23, %24, 190)
            );
            border: 1px solid rgba(255, 255, 255, 60);
        }
    )")
        .arg(top.red()).arg(top.green()).arg(top.blue())
        .arg(mid.red()).arg(mid.green()).arg(mid.blue())
        .arg(bottom.red()).arg(bottom.green()).arg(bottom.blue())
        .arg(top.red()).arg(top.green()).arg(top.blue())
        .arg(mid.red()).arg(mid.green()).arg(mid.blue())
        .arg(bottom.red()).arg(bottom.green()).arg(bottom.blue())
        .arg(top.red()).arg(top.green()).arg(top.blue())
        .arg(bottom.red()).arg(bottom.green()).arg(bottom.blue());
}
#endif