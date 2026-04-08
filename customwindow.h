#pragma once
#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include "gamewindow.h"

class CustomWindow : public GameWindow
{
    Q_OBJECT
public:
    explicit CustomWindow(QWidget* parent = nullptr);
    bool isValid() const { return m_valid; }

protected:
    void initGame() override;
    void startGame() override;
    QString helpTitle() const override;
    QString helpText() const override;

private:
    bool askBoardSize(int& r, int& c);

private:
    bool m_valid = false; 
};

#endif