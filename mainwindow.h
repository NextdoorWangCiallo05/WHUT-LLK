#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "windowdrag.h"

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPoint>
#include <QPixmap>
#include <QSize>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void openClassicMode();
    void openRelaxMode();
    void openTimedMode();
    void openLevelMode();
    void openCustomMode();

private:
    void initUI();
    QWidget* centralWidget;
    QVBoxLayout* layout;
    QPushButton* btnClassic;
    QPushButton* btnRelax;
    QPushButton* btnTimed;
    QPushButton* btnLevel;
    QPushButton* btnCustom;
    QPushButton* btnExit;

    QPixmap m_bgPix;
    QPixmap m_bgScaled;
    QSize   m_bgScaledSize;
    int m_cornerRadius = 10;

    WindowDragState m_dragState;
};

#endif