#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void openClassicMode();
    void openRelaxMode();
    void openTimedMode();

private:
    void initUI();
    QWidget* centralWidget;
    QVBoxLayout* layout;
    QPushButton* btnClassic;
    QPushButton* btnRelax;
    QPushButton* btnTimed;
    QPushButton* btnExit;
};

#endif