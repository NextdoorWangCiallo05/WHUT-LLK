#pragma once
#ifndef RELAXWINDOW_H
#define RELAXWINDOW_H
#include "gamewindow.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class RelaxWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit RelaxWindow(QWidget* parent = nullptr);
protected:
    void initGame() override;
    void startGame() override;

    QString helpTitle() const override;
    QString helpText() const override;
};

#endif