#pragma once
#ifndef CLASSICWINDOW_H
#define CLASSICWINDOW_H
#include "gamewindow.h"

class ClassicWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit ClassicWindow(QWidget* parent = nullptr);
protected:
    void initGame() override;
    void startGame() override;
};

#endif