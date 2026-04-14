#pragma once
#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include <vector>
#include <QTimer>

#include "gamelogic.h"

class GameControl : public QObject
{
    Q_OBJECT
public:
    explicit GameControl(QObject* parent = nullptr);
    ~GameControl();

    void initMap(int rows, int cols);
    void resetState();

    void handleCellClick(int x, int y);

    void showHint();
    void clearHintStyle();

    void drawLinkPath(const std::vector<Point>& path);
    void clearLinkPath();

    void shuffle();
    bool shuffleUntilSolvable(int maxTry = 50);

    bool isFinished() const;
    bool isEmpty() const;
    bool hasAnySolution() const;
    int getMapData(int x, int y) const;
    void setMapData(int x, int y, int val);
    void setMaxType(int n);

    bool inputEnabled() const;
    void setInputEnabled(bool enabled);

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    quint64 boardEpoch() const { return m_boardEpoch; }
    void incrementEpoch();

    bool hasSelectedCell() const { return m_hasSelected; }
    Point selectedPoint() const { return m_lastPoint; }

    bool isShowingLink() const { return m_showLink; }
    const std::vector<Point>& linkPath() const { return m_linkPath; }

    bool hasActiveHint() const { return m_hasHint; }
    Point hintPointA() const { return m_hintA; }
    Point hintPointB() const { return m_hintB; }

    GameLogic* logic() { return m_logic; }
    const GameLogic* logic() const { return m_logic; }

signals:
    void cellSelected(int x, int y);
    void cellDeselected(int x, int y);
    void selectionTransferred(int fromX, int fromY, int toX, int toY);
    void pairMatched(Point a, Point b, const std::vector<Point>& path);
    void cellsRemoved(Point a, Point b);
    void hintDisplayed(Point a, Point b);
    void hintCleared(Point a, Point b);
    void linkPathUpdated();
    void linkPathCleared();
    void boardNeedsRefresh();
    void cellsNeedRefresh(const std::vector<Point>& pts);
    void gameCleared();
    void noSolutionAutoShuffled();
    void noSolutionManualShuffleNeeded();

private:
    void checkPostRemoveState();

private:
    GameLogic* m_logic;

    int m_rows;
    int m_cols;

    Point m_lastPoint;
    bool m_hasSelected;

    std::vector<Point> m_linkPath;
    bool m_showLink;

    Point m_hintA;
    Point m_hintB;
    bool m_hasHint;

    bool m_inputEnabled;
    quint64 m_boardEpoch;
};

#endif
