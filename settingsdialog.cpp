#include "settingsdialog.h"
#include "audiomanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>

// 如果你在头文件里有成员：QCheckBox* autoShuffleCheck;
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setFixedSize(420, 280);
    setModal(true);

    QLabel* title = new QLabel("游戏设置", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:22px;font-weight:bold;color:#2c3e50;");

    QLabel* volText = new QLabel("音量：", this);

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);

    volumeLabel = new QLabel(this);
    volumeLabel->setMinimumWidth(36);
    volumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout* volRow = new QHBoxLayout();
    volRow->addWidget(volText);
    volRow->addWidget(volumeSlider, 1);
    volRow->addWidget(volumeLabel);

    muteCheck = new QCheckBox("静音", this);
    autoShuffleCheck = new QCheckBox("无解时自动重排", this); // 注意：成员变量
    btnClose = new QPushButton("关闭", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addLayout(volRow);
    layout->addWidget(muteCheck);
    layout->addWidget(autoShuffleCheck);
    layout->addStretch();
    layout->addWidget(btnClose, 0, Qt::AlignCenter);

    refreshUi(); // 统一在这里刷新一次

    connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
        AudioManager::instance().setVolume(value);
        volumeLabel->setText(QString::number(value));
        });

    connect(muteCheck, &QCheckBox::toggled, this, [=](bool checked) {
        AudioManager::instance().setMuted(checked);
        });

    connect(autoShuffleCheck, &QCheckBox::toggled, this, [=](bool checked) {
        QSettings s("YourCompany", "LLK_Refresh");
        s.setValue("game/autoShuffle", checked);
        });

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

void SettingsDialog::refreshUi()
{
    volumeSlider->setValue(AudioManager::instance().volume());
    volumeLabel->setText(QString::number(AudioManager::instance().volume()));
    muteCheck->setChecked(AudioManager::instance().isMuted());

    QSettings s("YourCompany", "LLK_Refresh");
    bool autoShuffle = s.value("game/autoShuffle", true).toBool();
    autoShuffleCheck->setChecked(autoShuffle);
}