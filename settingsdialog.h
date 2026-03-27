#pragma once
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSlider;
class QLabel;
class QCheckBox;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    void refreshUi();

private:
    QSlider* volumeSlider;
    QLabel* volumeLabel;
    QCheckBox* muteCheck;
    QCheckBox* autoShuffleCheck;
    QPushButton* btnClose;
};

#endif