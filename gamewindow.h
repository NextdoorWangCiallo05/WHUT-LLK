#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QSize>
#include <QTimer>
#include "gamelogic.h"

class GameWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr);
    virtual ~GameWindow();

protected:
    virtual void initGame() = 0;
    virtual void startGame() = 0;

    bool inputEnabled;
    void clearLinkPath();
    void backToMain();
    void createBoard();
    void onCellClicked(int x, int y);
    void refreshBoard();

    virtual void onGameCleared();

    // 新增
    void showHint();
    void drawLinkPath(const std::vector<Point>& path);
    void clearHintStyle();

    // 重写绘制
    void paintEvent(QPaintEvent* event) override;

    QWidget* boardWidget;
    QWidget* bottomWidget;
    QGridLayout* gameLayout;
    QHBoxLayout* btnLayout;

    QPushButton** cells;
    QPushButton* btnBack;
    QPushButton* btnReset;
    QPushButton* btnHint;
    QPushButton* btnHelp;

    GameLogic* logic;
    int rows;
    int cols;
    int cellSize;
    Point lastPoint;
    bool hasSelected;

    // 连线显示数据
    std::vector<Point> m_linkPath;
    bool m_showLink;

    // 提示高亮
    Point hintA, hintB;
    bool hasHint;

    virtual void onPairRemoved() {}
};

#endif