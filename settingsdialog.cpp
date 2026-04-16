#include "settingsdialog.h"
#include "uistyle.h"
#include "thememanager.h"
#include "audiomanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QAbstractSpinBox>
#include <QSignalBlocker>
#include <QComboBox>
#include <QListView>
#include <QFrame>

// 构造函数，设置窗口属性、加载主题、创建 UI 元素并连接信号槽
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setFixedSize(500, 600);
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

    setupGlassDialogTopBar(this, layout, "设置", "设置");

    QLabel* title = new QLabel("游戏设置", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    QLabel* volText = new QLabel("背景BGM音量：", this);
    volText->setStyleSheet(glassTextLabelStyle());

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            height: 8px;
            border-radius: 4px;
            background: rgba(255,255,255,0.16);
        }
        QSlider::sub-page:horizontal {
            border-radius: 4px;
            background: rgba(30,110,244,0.88);
        }
        QSlider::add-page:horizontal {
            border-radius: 4px;
            background: rgba(255,255,255,0.12);
        }
        QSlider::handle:horizontal {
            width: 16px;
            margin: -5px 0;
            border-radius: 8px;
            background: rgba(255,255,255,0.96);
            border: 1px solid rgba(30,110,244,0.55);
        }
    )");

    volumeLabel = new QLabel(this);
    volumeLabel->setMinimumWidth(30);
    volumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    volumeLabel->setStyleSheet(glassTextLabelStyle());

    volumeIconLabel = new QLabel(this);
    volumeIconLabel->setFixedWidth(46);
    volumeIconLabel->setAlignment(Qt::AlignCenter);
    volumeIconLabel->setStyleSheet(R"(
    QLabel{
        color: rgba(0,0,0,0.88);
        font-size: 8px;
        font-weight: 600;
        letter-spacing: 0.5px;
        background-color: rgba(255,255,255,0.10);
        border: 1px solid rgba(255,255,255,0.14);
        border-radius: 10px;
        padding: 2px 4px;
    }
)");

    QHBoxLayout* volRow = new QHBoxLayout();
    volRow->addWidget(volText);
    volRow->addWidget(volumeSlider, 1);
    volRow->addWidget(volumeLabel);
    volRow->addWidget(volumeIconLabel);

    muteCheck = new QCheckBox("BGM静音", this);
    sfxMuteCheck = new QCheckBox("音效静音", this);
    autoShuffleCheck = new QCheckBox("无解时自动重排", this);

    QString checkStyle = R"(
        QCheckBox{
            color: rgba(0,0,0,0.95);
            font-size: 15px;
            font-weight: 600;
            spacing: 8px;
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 10px;
            padding: 6px 10px;
        }
        QCheckBox::indicator{
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 1px solid rgba(255,255,255,0.70);
            background-color: rgba(255,255,255,0.18);
        }
        QCheckBox::indicator:hover{
            border: 1px solid rgba(30,110,244,0.95);
            background-color: rgba(255,255,255,0.28);
        }
        QCheckBox::indicator:checked{
            background-color: rgba(30,110,244,0.95);
            border: 1px solid rgba(255,255,255,0.95);
            image: url(:/images/check.svg);
        }
        QCheckBox::indicator:checked:hover{
            background-color: rgba(52,132,255,0.98);
        }
    )";

    muteCheck->setStyleSheet(checkStyle);
    sfxMuteCheck->setStyleSheet(checkStyle); 
    autoShuffleCheck->setStyleSheet(checkStyle);

    QLabel* timedText = new QLabel("倒计时：", this);
    timedText->setStyleSheet(R"(
        QLabel{
            color: rgba(0,0,0,0.92);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 10px;
            padding: 6px 10px;
        }
    )");
    timedText->setFixedWidth(110);

    timedMinutesSpin = new QSpinBox(this);
    timedMinutesSpin->setRange(1, 60);
    timedMinutesSpin->setAlignment(Qt::AlignCenter);
    timedMinutesSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    timedMinutesSpin->setFixedWidth(100);
    timedMinutesSpin->setStyleSheet(R"(
        QSpinBox {
            color: rgba(0,0,0,0.95);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.18);
            border: 1px solid rgba(255,255,255,0.22);
            border-radius: 10px;
            padding: 6px 10px;
            min-height: 18px;
        }
        QSpinBox:hover {
            border: 1px solid rgba(30,110,244,0.55);
            background-color: rgba(255,255,255,0.24);
        }
        QSpinBox:focus {
            border: 1px solid rgba(30,110,244,0.85);
            background-color: rgba(255,255,255,0.28);
        }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 18px;
            border: none;
            background: transparent;
        }
        QSpinBox::up-arrow {
            image: url(:/images/spin-up.svg);
            width: 10px; 
            height: 10px;
        }
        QSpinBox::down-arrow {
            image: url(:/images/spin-down.svg);
            width: 10px; 
            height: 10px;
        }
    )");

    QLabel* timedUnit = new QLabel("分钟", this);
    timedUnit->setAlignment(Qt::AlignCenter);
    timedUnit->setStyleSheet(R"(
        QLabel{
            color: rgba(0,0,0,0.92);
            font-size: 14px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 10px;
            padding: 6px 10px;
        }
    )");
    timedUnit->setFixedWidth(60);

    QWidget* timedCard = new QWidget(this);
    timedCard->setStyleSheet(glassCardStyle());

    QWidget* scoreCard = new QWidget(this);
    scoreCard->setStyleSheet(glassCardStyle()); 

    QLabel* scoreText = new QLabel("消除得分：", scoreCard);
    scoreText->setStyleSheet(R"(
        QLabel{
            color: rgba(0,0,0,0.92);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 10px;
            padding: 6px 10px;
        }
    )");
    scoreText->setFixedWidth(110);

    scorePerPairSpin = new QSpinBox(scoreCard);
    scorePerPairSpin->setRange(1, 100);
    scorePerPairSpin->setAlignment(Qt::AlignCenter);
    scorePerPairSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    scorePerPairSpin->setFixedWidth(100);
    scorePerPairSpin->setStyleSheet(R"(
        QSpinBox {
            color: rgba(0,0,0,0.95);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.18);
            border: 1px solid rgba(255,255,255,0.22);
            border-radius: 10px;
            padding: 6px 10px;
            min-height: 18px;
        }
        QSpinBox:hover { border: 1px solid rgba(30,110,244,0.55); }
        QSpinBox:focus { border: 1px solid rgba(30,110,244,0.85); }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 18px; border: none; background: transparent;
        }
        QSpinBox::up-arrow { image: url(:/images/spin-up.svg); width: 10px; height: 10px; }
        QSpinBox::down-arrow { image: url(:/images/spin-down.svg); width: 10px; height: 10px; }
    )");

    QLabel* scoreUnit = new QLabel("分/对", scoreCard);
    scoreUnit->setAlignment(Qt::AlignCenter);
    scoreUnit->setStyleSheet(scoreText->styleSheet());
    scoreUnit->setFixedWidth(60);

    QHBoxLayout* scoreRow = new QHBoxLayout(scoreCard);
    scoreRow->setContentsMargins(12, 8, 12, 8);
    scoreRow->setSpacing(10);
    scoreRow->setAlignment(Qt::AlignVCenter);

    scoreRow->addWidget(scoreText);
    scoreRow->addWidget(scorePerPairSpin);
    scoreRow->addWidget(scoreUnit);
    scoreRow->addStretch();

    QLabel* themeText = new QLabel("界面主题：", this);
    themeText->setStyleSheet(glassTextLabelStyle());

    themeCombo = new QComboBox(this);

    auto* lv = new QListView(themeCombo);
    lv->setFrameShape(QFrame::NoFrame);
    lv->setAttribute(Qt::WA_TranslucentBackground, false);
    themeCombo->setView(lv);

    themeCombo->setMinimumWidth(160);
    themeCombo->addItem("罗小黑", "classic");
    themeCombo->addItem("星露谷", "fruit");
    themeCombo->addItem("微软", "win");
    themeCombo->addItem("水果", "star");
    themeCombo->setStyleSheet(R"(
/* ===== 主体（你自己的风格）===== */
QComboBox {
    min-height: 34px;
    padding: 6px 36px 6px 12px;
    font-size: 14px;
    font-weight: 600;
    color: rgba(0,0,0,0.92);
    background-color: rgba(255,255,255,0.10);
    border: 1px solid rgba(255,255,255,0.22);
    border-radius: 14px;
}
QComboBox:hover {
    background-color: rgba(255,255,255,0.14);
    border: 1px solid rgba(255,255,255,0.32);
}
QComboBox:focus {
    background-color: rgba(255,255,255,0.18);
    border: 1px solid rgba(255,255,255,0.45);
}

/* 下拉箭头 */
    QComboBox::drop-down {
        subcontrol-origin: padding;
        subcontrol-position: top right;
        width: 28px;
        border: none;
        background: transparent;
        margin-right: 6px;
        border-top-right-radius: 14px;
        border-bottom-right-radius: 14px;
    }
    QComboBox::down-arrow {
        image: url(:/images/chevron.svg);
        width: 14px;
        height: 14px;
    }

    QComboBox QAbstractItemView {
        outline: 0;
        border: none;
        font-size: 14px;
        font-weight: 600;
        background-color: rgb(245,245,245);  /* 不透明浅底，防止黑底透出 */
        color: rgba(0,0,0,0.92);
        selection-background-color: transparent; /* 用 item:selected 自定义高亮 */
        selection-color: rgba(0,0,0,0.98);
        padding: 6px;
    }
    QComboBox QListView {
        border: none;
        outline: 0;
    }

    QComboBox QListView::item {
        min-height: 28px;
        margin: 2px 4px;
        padding: 4px 10px;
        border-radius: 10px;
        background: transparent;
}
    QComboBox QListView::item:hover {
        background: rgba(38,158,233,0.18);
}
    QComboBox QListView::item:selected {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:1,
            stop:0 rgba(76,185,255,0.95),
            stop:1 rgba(38,158,233,0.95)
    );
    color: #111;
    border: none;
    border-radius: 10px;
    font-weight: 700;
}
)");

    QHBoxLayout* themeRow = new QHBoxLayout();
    themeRow->addWidget(themeText);
    themeRow->addWidget(themeCombo);
    themeRow->addStretch();

    QHBoxLayout* timedRow = new QHBoxLayout(timedCard);
    timedRow->setContentsMargins(12, 8, 12, 8);
    timedRow->setSpacing(10);
    timedRow->setAlignment(Qt::AlignVCenter);

    timedRow->addWidget(timedText);
    timedRow->addWidget(timedMinutesSpin);
    timedRow->addWidget(timedUnit);
    timedRow->addStretch();

    btnRestoreDefault = new QPushButton("恢复默认值", this);
    btnClose = new QPushButton("关闭", this);

    QColor baseBlue(0, 145, 255);
    btnRestoreDefault->setStyleSheet(glassButtonStyle(baseBlue));
    btnClose->setStyleSheet(glassButtonStyle(baseBlue));
    btnRestoreDefault->setFixedHeight(40);
    btnRestoreDefault->setMinimumWidth(120);
    btnClose->setFixedHeight(40);
    btnClose->setMinimumWidth(120);

    applyGlassShadow(btnRestoreDefault, QColor(0, 0, 0, 55));
    applyGlassShadow(btnClose, QColor(0, 0, 0, 55));

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(btnRestoreDefault);
    btnRow->addSpacing(12);
    btnRow->addWidget(btnClose);
    btnRow->addStretch();

    layout->addWidget(title);
    layout->addLayout(volRow);
    layout->addWidget(muteCheck);
    layout->addWidget(sfxMuteCheck);
    layout->addWidget(autoShuffleCheck);
    layout->addWidget(timedCard);
    layout->addWidget(scoreCard);
    layout->addLayout(themeRow);
    layout->addStretch();
    layout->addLayout(btnRow);

    refreshUi();

    connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
        AudioManager::instance().setVolume(value);
        volumeLabel->setText(QString::number(value));
        volumeIconLabel->setText((muteCheck->isChecked() || volumeSlider->value() == 0) ? "静音" : "播放");
        });

    connect(muteCheck, &QCheckBox::toggled, this, [=](bool checked) {
        AudioManager::instance().setMuted(checked);
        volumeIconLabel->setText((checked || volumeSlider->value() == 0) ? "静音" : "播放");
        });

    connect(sfxMuteCheck, &QCheckBox::toggled, this, [=](bool checked) {
        AudioManager::instance().setSfxMuted(checked);
        });

    connect(autoShuffleCheck, &QCheckBox::toggled, this, [=](bool checked) {
        QSettings s("YourCompany", "LLK_Refresh");
        s.setValue("game/autoShuffle", checked);
        });

    connect(timedMinutesSpin, qOverload<int>(&QSpinBox::valueChanged), this, [=](int minutes) {
        QSettings s("YourCompany", "LLK_Refresh");
        s.setValue("game/timedModeTotalSec", minutes * 60);
        });

    connect(scorePerPairSpin, qOverload<int>(&QSpinBox::valueChanged), this, [=](int score) {
        QSettings s("YourCompany", "LLK_Refresh");
        s.setValue("game/scorePerPair", score);
        });

    connect(themeCombo, &QComboBox::currentIndexChanged, this, [=](int idx) {
        QString key = themeCombo->itemData(idx).toString();
        ThemeManager::instance().setCurrentTheme(key);
        });

    connect(btnRestoreDefault, &QPushButton::clicked, this, [=]() {
        restoreDefaults();
        });

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

// 刷新 UI 元素状态以匹配当前设置
void SettingsDialog::refreshUi()
{
    const QSignalBlocker b1(volumeSlider);
    const QSignalBlocker b2(muteCheck);
    const QSignalBlocker b3(sfxMuteCheck);
    const QSignalBlocker b4(autoShuffleCheck);
    const QSignalBlocker b5(timedMinutesSpin);

    volumeSlider->setValue(AudioManager::instance().volume());
    volumeLabel->setText(QString::number(AudioManager::instance().volume()));
    muteCheck->setChecked(AudioManager::instance().isMuted());
    sfxMuteCheck->setChecked(AudioManager::instance().isSfxMuted());

    QSettings s("YourCompany", "LLK_Refresh");
    bool autoShuffle = s.value("game/autoShuffle", true).toBool();
    autoShuffleCheck->setChecked(autoShuffle);

    scorePerPairSpin->setValue(s.value("game/scorePerPair", 5).toInt());

    int totalSec = s.value("game/timedModeTotalSec", 600).toInt();
    int minutes = totalSec / 60;
    if (minutes < 1) minutes = 1;
    if (minutes > 60) minutes = 60;
    timedMinutesSpin->setValue(minutes);

    volumeIconLabel->setText((muteCheck->isChecked() || volumeSlider->value() == 0) ? "静音" : "播放");

    const QSignalBlocker b(themeCombo);
    const QString cur = ThemeManager::instance().currentTheme();
    int idx = themeCombo->findData(cur);
    if (idx < 0) idx = 0;
    themeCombo->setCurrentIndex(idx);
}

// 恢复默认设置并保存到配置文件
void SettingsDialog::restoreDefaults()
{
    volumeSlider->setValue(50);
    muteCheck->setChecked(false);
    sfxMuteCheck->setChecked(false);
    autoShuffleCheck->setChecked(true);
    timedMinutesSpin->setValue(10);
    scorePerPairSpin->setValue(5);

    QSettings s("YourCompany", "LLK_Refresh");
    s.setValue("audio/volume", 50);
    s.setValue("audio/muted", false);
    s.setValue("audio/sfxMuted", false);
    s.setValue("game/autoShuffle", true);
    s.setValue("game/timedModeTotalSec", 600);
    s.setValue("game/scorePerPair", 5);

    AudioManager::instance().setVolume(50);
    AudioManager::instance().setMuted(false);
    AudioManager::instance().setSfxMuted(false);
    volumeLabel->setText("50");
    volumeIconLabel->setText("VOL");

}

void SettingsDialog::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QDialog::mousePressEvent(event);
}

void SettingsDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QDialog::mouseMoveEvent(event);
}

void SettingsDialog::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QDialog::mouseReleaseEvent(event);
}