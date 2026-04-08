#pragma once
#ifndef RANKDIALOG_H
#define RANKDIALOG_H

#include "windowdrag.h"
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QTableWidget;
class QPushButton;

class RankDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RankDialog(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void refreshTable();

    QTableWidget* table;
    QPushButton* btnClose;

    WindowDragState m_dragState;
};

#endif