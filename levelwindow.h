#pragma once
#ifndef LEVELWINDOW_H
#define LEVELWINDOW_H

#include "timedwindow.h"

class LevelWindow : public TimedWindow
{
    Q_OBJECT
public:
    explicit LevelWindow(int startLevel = 1, QWidget* parent = nullptr);

protected:
    void initGame() override;
    void onGameCleared() override;

    QString helpTitle() const override;
    QString helpText() const override;

private:
    void applyLevelSize();

private:
    int m_level = 1;
    bool m_switchingLevel = false;
};

#endif