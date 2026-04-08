#pragma once
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "windowdrag.h"
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QLabel;
class QPushButton;

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);


private:
    QLabel* titleLabel;
    QLabel* contentLabel;
    QPushButton* closeBtn;

    WindowDragState m_dragState;
};

#endif