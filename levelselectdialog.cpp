#include "levelselectdialog.h"
#include "thememanager.h"
#include "uistyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QColor>

LevelSelectDialog::LevelSelectDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(520, 300);
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
    setupGlassDialogTopBar(this, lay, "选择关卡", "选择关卡");

    QLabel* title = new QLabel("请选择关卡", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    // 卡片容器
    QWidget* card = new QWidget(this);
    card->setStyleSheet(glassCardStyle());

    auto* cardShadow = new QGraphicsDropShadowEffect(card);
    cardShadow->setBlurRadius(18);
    cardShadow->setOffset(0, 4);
    cardShadow->setColor(QColor(0, 0, 0, 55));
    card->setGraphicsEffect(cardShadow);

    QVBoxLayout* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(14, 14, 14, 14);
    cardLay->setSpacing(12);

    // 关卡按钮（横向）
    QHBoxLayout* levelRow = new QHBoxLayout();
    levelRow->setSpacing(12);

    btnL1 = new QPushButton("第1关", card);
    btnL2 = new QPushButton("第2关", card);
    btnL3 = new QPushButton("第3关", card);
    btnCancel = new QPushButton("取消", this);

    const QSize levelBtnSize(140, 56);
    btnL1->setFixedSize(levelBtnSize);
    btnL2->setFixedSize(levelBtnSize);
    btnL3->setFixedSize(levelBtnSize);

    btnCancel->setFixedHeight(40);
    btnCancel->setMinimumWidth(120);

    // 默认（解锁）样式
    btnL1->setStyleSheet(glassButtonStyle(QColor(0, 145, 255)));
    btnL2->setStyleSheet(glassButtonStyle(QColor(48, 209, 88)));
    btnL3->setStyleSheet(glassButtonStyle(QColor(255, 146, 48)));
    btnCancel->setStyleSheet(glassButtonStyle(QColor(120, 130, 145)));

    applyGlassShadow(btnL1, QColor(0, 0, 0, 70));
    applyGlassShadow(btnL2, QColor(0, 0, 0, 70));
    applyGlassShadow(btnL3, QColor(0, 0, 0, 70));
    applyGlassShadow(btnCancel, QColor(0, 0, 0, 70));

    levelRow->addStretch();
    levelRow->addWidget(btnL1);
    levelRow->addWidget(btnL2);
    levelRow->addWidget(btnL3);
    levelRow->addStretch();

    cardLay->addLayout(levelRow);

    lay->addWidget(title);
    lay->addWidget(card, 1);

    QHBoxLayout* bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    bottomRow->addWidget(btnCancel);
    bottomRow->addStretch();
    lay->addLayout(bottomRow);

    refreshLockState();

    connect(btnL1, &QPushButton::clicked, this, [this]() { m_selectedLevel = 1; accept(); });
    connect(btnL2, &QPushButton::clicked, this, [this]() { m_selectedLevel = 2; accept(); });
    connect(btnL3, &QPushButton::clicked, this, [this]() { m_selectedLevel = 3; accept(); });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void LevelSelectDialog::refreshLockState()
{
    QSettings s("YourCompany", "LLK_Refresh");
    int unlockedMax = s.value("level/unlockedMax", 1).toInt();
    if (unlockedMax < 1) unlockedMax = 1;
    if (unlockedMax > 3) unlockedMax = 3;

    auto lockedStyle = []() {
        return QString(R"(
            QPushButton {
                font-size: 15px;
                font-weight: 600;
                color: rgba(230,230,230,0.95);
                background-color: qlineargradient(
                    x1:0, y1:0, x2:1, y2:1,
                    stop:0 rgba(140,140,140,210),
                    stop:0.5 rgba(120,120,120,200),
                    stop:1 rgba(100,100,100,210)
                );
                border: 1px solid rgba(255,255,255,70);
                border-radius: 14px;
                padding: 8px 12px;
            }
            QPushButton:hover {
                background-color: qlineargradient(
                    x1:0, y1:0, x2:1, y2:1,
                    stop:0 rgba(145,145,145,215),
                    stop:1 rgba(105,105,105,215)
                );
            }
            QPushButton:disabled {
                color: rgba(235,235,235,0.90);
                background-color: qlineargradient(
                    x1:0, y1:0, x2:1, y2:1,
                    stop:0 rgba(135,135,135,185),
                    stop:1 rgba(95,95,95,185)
                );
                border: 1px solid rgba(255,255,255,55);
            }
        )");
        };

    btnL1->setStyleSheet(glassButtonStyle(QColor(255, 141, 40)));
    btnL2->setStyleSheet(glassButtonStyle(QColor(0, 195, 208)));
    btnL3->setStyleSheet(glassButtonStyle(QColor(255, 204, 0)));

    btnL1->setEnabled(true);
    btnL2->setEnabled(unlockedMax >= 2);
    btnL3->setEnabled(unlockedMax >= 3);

    btnL1->setText("第1关");
    btnL2->setText(unlockedMax >= 2 ? "第2关" : "第2关（未解锁）");
    btnL3->setText(unlockedMax >= 3 ? "第3关" : "第3关（未解锁）");

    if (unlockedMax < 2) {
        btnL2->setStyleSheet(lockedStyle());
    }
    if (unlockedMax < 3) {
        btnL3->setStyleSheet(lockedStyle());
    }
}