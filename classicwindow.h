#pragma once
#ifndef CLASSICWINDOW_H
#define CLASSICWINDOW_H
#include "gamewindow.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

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
};

#endif