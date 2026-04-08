#pragma once
#ifndef TIMEDWINDOW_H
#define TIMEDWINDOW_H

#include "gamewindow.h"
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>

class TimedWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit TimedWindow(QWidget* parent = nullptr, bool autoStart = true);

protected:
    void initGame() override;
    void startGame() override;

    QString helpTitle() const override;
    QString helpText() const override;

    void resizeEvent(QResizeEvent* event) override;

    QString formatTime(int sec) const;
    void finishTimedGame(bool win);
    void setPaused(bool paused);
    void updatePauseMaskGeometry();

    QTimer* timer = nullptr;
    QLabel* labTime = nullptr;
    QProgressBar* timeBar = nullptr;
    QLabel* pauseSubLabel = nullptr;

    int leftSec = 0;
    int totalSec = 0;
    int removedPairs = 0;

    QWidget* pauseMask = nullptr;
    QLabel* pauseLabel = nullptr;

    bool isPaused = false;
    bool gameFinished = false;

    void onGameCleared() override;
    void onPairRemoved() override;

private slots:
    void onTimerTimeout();
};

#endif