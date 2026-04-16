#pragma once
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include "windowdrag.h"

#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QSlider;
class QLabel;
class QCheckBox;
class QPushButton;
class QSpinBox;
class QComboBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);


protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void refreshUi();
    void restoreDefaults();

private:
    QSlider* volumeSlider;
    QLabel* volumeLabel;
    QLabel* volumeIconLabel;
    QCheckBox* muteCheck;
    QCheckBox* sfxMuteCheck;
    QCheckBox* autoShuffleCheck;
    QSpinBox* timedMinutesSpin;
    QSpinBox* scorePerPairSpin;
    QComboBox* themeCombo;

    QPushButton* btnRestoreDefault;
    QPushButton* btnClose;

    WindowDragState m_dragState;
};

#endif