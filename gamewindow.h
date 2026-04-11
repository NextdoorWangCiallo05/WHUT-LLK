#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QSize>
#include <QVector>
#include <QTimer>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QHash>
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QVariantAnimation>
#include <QCloseEvent>

#include "gamelogic.h"
#include "windowdrag.h"

class GameWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr);
    virtual ~GameWindow();

    void setModeTitle(const QString& title);

protected:
    virtual void initGame() = 0;
    virtual void startGame() = 0;

    void setupCommonModeButtons(
        const QColor& baseColor,
        bool withPauseButton = false,
        const QSize& btnSize = QSize(140, 44)
    );

    virtual QString helpTitle() const = 0;
    virtual QString helpText() const = 0;

    bool inputEnabled;
    void clearLinkPath();
    void backToMain();
    void createBoard();
    void onCellClicked(int x, int y);

    void refreshBoard();
    void refreshCell(int x, int y);
    void refreshCells(const std::vector<Point>& pts);

    virtual void onGameCleared();

    void showHint();
    void drawLinkPath(const std::vector<Point>& path);
    void clearHintStyle();

    virtual bool isGameFinished() const;

    bool confirmExitIfNeeded();
    void closeEvent(QCloseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    QWidget* topWidget;
    QWidget* infoWidget;
    QWidget* boardWidget;
    QWidget* bottomWidget;
    QGridLayout* gameLayout;
    QHBoxLayout* infoLayout;
    QHBoxLayout* btnLayout;

    QPushButton** cells;
    int m_cellsCount;

    QPushButton* btnBack;
    QPushButton* btnReset;
    QPushButton* btnHint;
    QPushButton* btnHelp;
    QPushButton* btnPause;

    QLabel* appIcon;
    QLabel* titleLabel;
    QPushButton* btnMin;
    QPushButton* btnClose;

    GameLogic* logic;
    int rows;
    int cols;
    int cellSize;
    Point lastPoint;
    bool hasSelected;

    std::vector<Point> m_linkPath;
    bool m_showLink;

    qreal m_linkAlpha = 1.0;
    QVariantAnimation* m_linkFadeAnim = nullptr;

    Point hintA, hintB;
    bool hasHint;

    virtual void onPairRemoved() {}

    quint64 m_boardEpoch;
    bool m_forceClosing = false;

private:
    QIcon iconForValue(int v);
    QHash<int, QIcon> m_iconCache;

    QPixmap m_bgPix;
    QPixmap m_bgScaled;
    QSize m_bgScaledSize;

    WindowDragState m_dragState;
    
};

#endif