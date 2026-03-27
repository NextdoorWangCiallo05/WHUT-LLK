#pragma once
#ifndef TIMEDWINDOW_H
#define TIMEDWINDOW_H
#include "gamewindow.h"
#include <QTimer>
#include <QLabel>
#include <QProgressBar>

class TimedWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit TimedWindow(QWidget* parent = nullptr);
protected:
    void initGame() override;
    void startGame() override;
private slots:
    void onTimerTimeout();
private:
    QTimer* timer;
    QLabel* labTime;
    QProgressBar* timeBar;

    int leftSec;
    int totalSec;
    int removedPairs;

    void onGameCleared() override;
    void onPairRemoved() override;
    void finishTimedGame(bool win);

    QWidget* pauseMask;
    QLabel* pauseLabel;
    void setPaused(bool paused);

	QPushButton* btnPause;  // 暂停/继续按钮
    bool isPaused;          
    bool gameFinished;

    void updatePauseMaskGeometry();
};

#endif