#pragma once
#ifndef CLASSICWINDOW_H
#define CLASSICWINDOW_H
#include "gamewindow.h"
#include <QElapsedTimer>

class ClassicWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit ClassicWindow(QWidget* parent = nullptr);

protected:
    void initGame() override;
    void startGame() override;

    QString helpTitle() const override;
    QString helpText() const override;

    void onGameCleared() override;
    bool isGameFinished() const override;

private:
    QElapsedTimer m_elapsed;
    bool m_finished = false;
};

#endif