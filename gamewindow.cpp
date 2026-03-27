#include "gamewindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QLayoutItem>
#include <QPainter>
#include <QPaintEvent>

GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent),
    boardWidget(nullptr),
    bottomWidget(nullptr),
    gameLayout(nullptr),
    btnLayout(nullptr),
    cells(nullptr),
    btnBack(nullptr),
    btnReset(nullptr),
    btnHint(nullptr),
    btnHelp(nullptr),
    logic(new GameLogic()),
    rows(10),
    cols(16),
    cellSize(48),
    hasSelected(false),
    m_showLink(false),
    hasHint(false),
    inputEnabled(true)
{
    setFixedSize(800, 600);
    setAutoFillBackground(true);

    // 背景图
    QPixmap bg(":/images/bg2.png");
    QPalette pal;
    pal.setBrush(QPalette::Window, bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    setPalette(pal);

    // 全局布局：上留白 + 棋盘 + 按钮 + 下留白
    QVBoxLayout* globalLayout = new QVBoxLayout(this);
    globalLayout->setContentsMargins(16, 16, 16, 16);
    globalLayout->setSpacing(14);

    globalLayout->addStretch(1);

    // 棋盘容器（核心：固定区域 + 居中）
    boardWidget = new QWidget(this);
    QHBoxLayout* boardOuter = new QHBoxLayout(boardWidget);
    boardOuter->setContentsMargins(0, 0, 0, 0);
    boardOuter->setSpacing(0);
    boardOuter->setAlignment(Qt::AlignCenter);

    gameLayout = new QGridLayout();
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->setHorizontalSpacing(2);
    gameLayout->setVerticalSpacing(2);
    gameLayout->setAlignment(Qt::AlignCenter);

    boardOuter->addLayout(gameLayout);
    globalLayout->addWidget(boardWidget, 0, Qt::AlignCenter);

    // 底部按钮容器
    bottomWidget = new QWidget(this);
    btnLayout = new QHBoxLayout(bottomWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(20);
    btnLayout->setAlignment(Qt::AlignCenter);

    globalLayout->addWidget(bottomWidget, 0, Qt::AlignCenter);
    globalLayout->addStretch(1);

    setLayout(globalLayout);
}

GameWindow::~GameWindow()
{
    if (cells) {
        delete[] cells;
        cells = nullptr;
    }
    delete logic;
}

void GameWindow::backToMain()
{
    MainWindow* w = new MainWindow();
    w->move(this->pos());
    w->show();
    close();
}

void GameWindow::createBoard()
{
    // 清旧按钮
    if (cells) {
        for (int i = 0; i < rows * cols; i++) {
            if (cells[i]) cells[i]->deleteLater();
        }
        delete[] cells;
        cells = nullptr;
    }

    // 清空layout残留item
    while (QLayoutItem* item = gameLayout->takeAt(0)) {
        delete item;
    }

    cells = new QPushButton * [rows * cols];

    // 动态计算棋盘容器尺寸，保证“整块棋盘”居中显示
    int hSpacing = gameLayout->horizontalSpacing();
    int vSpacing = gameLayout->verticalSpacing();
    int boardW = cols * cellSize + (cols - 1) * hSpacing;
    int boardH = rows * cellSize + (rows - 1) * vSpacing;
    boardWidget->setFixedSize(boardW, boardH);

    for (int x = 0; x < rows; x++) {
        for (int y = 0; y < cols; y++) {
            int idx = x * cols + y;
            cells[idx] = new QPushButton(this);
            cells[idx]->setFixedSize(cellSize, cellSize);
            cells[idx]->setIconSize(QSize(cellSize - 2, cellSize - 2));
            cells[idx]->setStyleSheet("border: none; background: transparent;");

            gameLayout->addWidget(cells[idx], x, y, Qt::AlignCenter);

            connect(cells[idx], &QPushButton::clicked, this, [=]() {
                onCellClicked(x, y);
                });
        }
    }
}

void GameWindow::onCellClicked(int x, int y)
{
    if (!inputEnabled) return;

    Point cur(x, y);
    if (logic->getMapData(x, y) == 0) return;

    clearHintStyle();

    if (!hasSelected) {
        lastPoint = cur;
        cells[x * cols + y]->setStyleSheet("border: 2px solid red; background: transparent;");
        hasSelected = true;
        return;
    }

    cells[lastPoint.x * cols + lastPoint.y]->setStyleSheet("border: none; background: transparent;");

    if (lastPoint.x == x && lastPoint.y == y) {
        hasSelected = false;
        return;
    }

    std::vector<Point> path;
    if (logic->findPath(lastPoint, cur, path)) {
        drawLinkPath(path); // 先画线

        logic->setMapData(lastPoint.x, lastPoint.y, 0);
        logic->setMapData(cur.x, cur.y, 0);
        onPairRemoved();

        // 延迟一点再刷新，让用户看到连线
        QTimer::singleShot(120, this, [=]() {
            clearLinkPath();
            refreshBoard();

            if (logic->isMapEmpty()) {
                onGameCleared();
                return;
            }

            // 若无解自动重排
            if (!logic->hasAnySolution()) {
                logic->shuffleUntilSolvable();
                refreshBoard();
                QMessageBox::information(this, "提示", "当前无可消除对子，已自动重排。");
            }
            });
    }

    hasSelected = false;
}

void GameWindow::refreshBoard()
{
    for (int x = 0; x < rows; x++) {
        for (int y = 0; y < cols; y++) {
            int v = logic->getMapData(x, y);
            int idx = x * cols + y;

            if (v == 0) {
                cells[idx]->setIcon(QIcon());
                cells[idx]->setStyleSheet("border: none; background: transparent;");
            }
            else {
                cells[idx]->setIcon(QIcon(":/images/" + QString::number(v) + ".png"));
                cells[idx]->setStyleSheet("border: none; background: transparent;");
            }
        }
    }
    hasHint = false;
    clearLinkPath();
}


void GameWindow::drawLinkPath(const std::vector<Point>& path)
{
    m_linkPath.clear();
    m_linkPath = path;
    m_showLink = true;
    update();
}

void GameWindow::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (!m_showLink || m_linkPath.size() < 2) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 绿色连线（可见性高）
    QPen pen(QColor(50, 220, 120), 4);   // 原来是红色
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    auto cellCenter = [&](Point pt)->QPoint {
        int hSpacing = gameLayout->horizontalSpacing();
        int vSpacing = gameLayout->verticalSpacing();

        QPoint boardTopLeft = boardWidget->geometry().topLeft();
        int x0 = boardTopLeft.x();
        int y0 = boardTopLeft.y();

        int cx = x0 + pt.y * (cellSize + hSpacing) + cellSize / 2;
        int cy = y0 + pt.x * (cellSize + vSpacing) + cellSize / 2;
        return QPoint(cx, cy);
        };

    for (size_t i = 0; i + 1 < m_linkPath.size(); ++i) {
        p.drawLine(cellCenter(m_linkPath[i]), cellCenter(m_linkPath[i + 1]));
    }
}

// ===================== 提示功能 =====================
void GameWindow::clearHintStyle()
{
    if (!hasHint || !cells) return;

    int i1 = hintA.x * cols + hintA.y;
    int i2 = hintB.x * cols + hintB.y;

    if (i1 >= 0 && i1 < rows * cols && cells[i1]) {
        cells[i1]->setStyleSheet("border: none; background: transparent;");
    }
    if (i2 >= 0 && i2 < rows * cols && cells[i2]) {
        cells[i2]->setStyleSheet("border: none; background: transparent;");
    }

    hasHint = false;
}

void GameWindow::showHint()
{
    // 先清理旧提示样式
    clearHintStyle();

    Point a, b;
    if (logic->findHintPair(a, b)) {
        hintA = a;
        hintB = b;
        hasHint = true;

        int i1 = a.x * cols + a.y;
        int i2 = b.x * cols + b.y;

        if (cells && i1 >= 0 && i1 < rows * cols && cells[i1]) {
            cells[i1]->setStyleSheet("border: 2px solid yellow; background: rgba(255,255,0,0.15);");
        }
        if (cells && i2 >= 0 && i2 < rows * cols && cells[i2]) {
            cells[i2]->setStyleSheet("border: 2px solid yellow; background: rgba(255,255,0,0.15);");
        }
    }
    else {
        QMessageBox::information(this, "提示", "当前无可消除对子，将自动重排。");
        logic->shuffleUntilSolvable();
        refreshBoard();
    }
}

void GameWindow::clearLinkPath()
{
    m_showLink = false;
    m_linkPath.clear();
    update();
}

void GameWindow::onGameCleared()
{
    QMessageBox::information(this, "胜利", "恭喜你，全部消除！");
    backToMain();
}