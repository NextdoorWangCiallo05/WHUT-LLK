#pragma once
#ifndef LEVELSELECTDIALOG_H
#define LEVELSELECTDIALOG_H

#include <QDialog>

class QPushButton;

class LevelSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LevelSelectDialog(QWidget* parent = nullptr);

    int selectedLevel() const { return m_selectedLevel; }

private:
    void refreshLockState();

    QPushButton* btnL1;
    QPushButton* btnL2;
    QPushButton* btnL3;
    QPushButton* btnCancel;

    int m_selectedLevel = 0;
};

#endif