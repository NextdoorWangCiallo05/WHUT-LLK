#include "glassmessagebox.h"
#include "uistyle.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialog>

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

void GlassMessageBox::information(QWidget* parent, const QString& title, const QString& text)
{
    showGlassMsg(parent, title, text, QColor(0, 145, 255));
}

void GlassMessageBox::warning(QWidget* parent, const QString& title, const QString& text)
{
    showGlassMsg(parent, title, text, QColor(255, 120, 70));
}