#pragma once
#ifndef RELAXWINDOW_H
#define RELAXWINDOW_H
#include "gamewindow.h"

class RelaxWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit RelaxWindow(QWidget* parent = nullptr);
protected:
    void initGame() override;
    void startGame() override;
};

#endif