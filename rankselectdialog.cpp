#include "rankselectdialog.h"
#include "thememanager.h"
#include "uistyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QColor>

// 选择排行榜类型的对话框，提供计时模式榜和经典模式榜两种选择
RankSelectDialog::RankSelectDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(520, 280);
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

    setupGlassDialogTopBar(this, lay, "排行榜", "排行榜");

    QLabel* title = new QLabel("请选择排行榜类型", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    QWidget* card = new QWidget(this);
    card->setStyleSheet(glassCardStyle());
    QHBoxLayout* cardLay = new QHBoxLayout(card);
    cardLay->setContentsMargins(14, 14, 14, 14);
    cardLay->setSpacing(12);

    btnTimed = new QPushButton("计时模式榜", card);
    btnClassic = new QPushButton("经典模式榜", card);

    btnTimed->setFixedSize(180, 56);
    btnClassic->setFixedSize(180, 56);

    btnTimed->setStyleSheet(glassButtonStyle(QColor(255, 146, 48)));
    btnClassic->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));

    applyGlassShadow(btnTimed, QColor(0, 0, 0, 70));
    applyGlassShadow(btnClassic, QColor(0, 0, 0, 70));

    cardLay->addStretch();
    cardLay->addWidget(btnTimed);
    cardLay->addWidget(btnClassic);
    cardLay->addStretch();

    btnCancel = new QPushButton("取消", this);
    btnCancel->setFixedHeight(40);
    btnCancel->setMinimumWidth(120);
    btnCancel->setStyleSheet(glassButtonStyle(QColor(120, 130, 145)));
    applyGlassShadow(btnCancel, QColor(0, 0, 0, 70));

    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->addStretch();
    bottom->addWidget(btnCancel);
    bottom->addStretch();

    lay->addWidget(title);
    lay->addWidget(card, 1);
    lay->addLayout(bottom);

    connect(btnTimed, &QPushButton::clicked, this, [this]() {
        m_selectedType = 1;
        accept();
        });

    connect(btnClassic, &QPushButton::clicked, this, [this]() {
        m_selectedType = 2;
        accept();
        });

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}