#include "glassmessagebox.h"
#include "uistyle.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialog>

/// 这个文件实现了玻璃风格的消息框，提供信息、警告和询问三种类型的消息框
// 显示玻璃风格的消息框，参数包括父窗口、标题、文本内容和按钮颜色
static void showGlassMsg(QWidget* parent, const QString& title, const QString& text, const QColor& btnColor)
{
    QDialog dlg(parent);
    dlg.setFixedSize(360, 200);
    dlg.setModal(true);

    auto applyDialogBg = [&dlg]() {
        const QString path = ThemeManager::instance().backgroundPathDialog();
        dlg.setStyleSheet(QString(
            "QDialog { border-image: url(%1) 0 0 0 0 stretch stretch; }"
        ).arg(path));
        };
    applyDialogBg();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, &dlg, applyDialogBg);

    auto* lay = new QVBoxLayout(&dlg);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(12);

    setupGlassDialogTopBar(&dlg, lay, title, title);

    QLabel* titleLabel = new QLabel(title, &dlg);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(glassTitleLabelStyle());

    QLabel* textLabel = new QLabel(text, &dlg);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet(glassTextLabelStyle());

    QPushButton* okBtn = new QPushButton("确定", &dlg);
    okBtn->setFixedHeight(40);
    okBtn->setMinimumWidth(120);
    okBtn->setStyleSheet(glassButtonStyle(btnColor));
    applyGlassShadow(okBtn, QColor(0, 0, 0, 85));

    lay->addWidget(titleLabel);
    lay->addWidget(textLabel, 1);
    lay->addWidget(okBtn, 0, Qt::AlignCenter);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

// 显示信息类型的消息框，按钮颜色为蓝色
void GlassMessageBox::information(QWidget* parent, const QString& title, const QString& text)
{
    showGlassMsg(parent, title, text, QColor(0, 145, 255));
}

// 显示警告类型的消息框，按钮颜色为橙色
void GlassMessageBox::warning(QWidget* parent, const QString& title, const QString& text)
{
    showGlassMsg(parent, title, text, QColor(255, 120, 70));
}

// 显示询问类型的消息框，包含确定和取消按钮，返回用户的选择结果
bool GlassMessageBox::question(QWidget* parent, const QString& title, const QString& text)
{
    QDialog dlg(parent);
    dlg.setFixedSize(380, 220);
    dlg.setModal(true);

    auto applyDialogBg = [&dlg]() {
        const QString path = ThemeManager::instance().backgroundPathDialog();
        dlg.setStyleSheet(QString(
            "QDialog { border-image: url(%1) 0 0 0 0 stretch stretch; }"
        ).arg(path));
        };
    applyDialogBg();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, &dlg, applyDialogBg);

    auto* lay = new QVBoxLayout(&dlg);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(12);

    setupGlassDialogTopBar(&dlg, lay, title, title);

    QLabel* titleLabel = new QLabel(title, &dlg);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(glassTitleLabelStyle());

    QLabel* textLabel = new QLabel(text, &dlg);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet(glassTextLabelStyle());

    QPushButton* okBtn = new QPushButton("确定", &dlg);
    QPushButton* cancelBtn = new QPushButton("取消", &dlg);

    okBtn->setFixedHeight(40);
    cancelBtn->setFixedHeight(40);
    okBtn->setMinimumWidth(120);
    cancelBtn->setMinimumWidth(120);

    okBtn->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    cancelBtn->setStyleSheet(glassButtonStyle(QColor(120, 130, 145)));

    applyGlassShadow(okBtn, QColor(0, 0, 0, 85));
    applyGlassShadow(cancelBtn, QColor(0, 0, 0, 85));

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->addStretch();
    btnLay->addWidget(okBtn);
    btnLay->addSpacing(12);
    btnLay->addWidget(cancelBtn);
    btnLay->addStretch();

    lay->addWidget(titleLabel);
    lay->addWidget(textLabel, 1);
    lay->addLayout(btnLay);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    return dlg.exec() == QDialog::Accepted;
}