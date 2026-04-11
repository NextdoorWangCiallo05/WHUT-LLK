#pragma once
#ifndef CLASSICRANKDIALOG_H
#define CLASSICRANKDIALOG_H

#include "windowdrag.h"
#include <QDialog>

class QTableWidget;
class QPushButton;

class ClassicRankDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClassicRankDialog(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void refreshTable();

    QTableWidget* table;
    QPushButton* btnExport;
    QPushButton* btnClear;
    QPushButton* btnClose;

    WindowDragState m_dragState;
};

#endif