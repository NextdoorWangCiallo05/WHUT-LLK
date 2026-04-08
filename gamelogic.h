#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>

struct Point {
    int x, y; // x=行 y=列
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
    bool operator==(const Point& p) const { return x == p.x && y == p.y; }
};

class GameLogic
{
public:
    GameLogic();
    ~GameLogic();

    void initMap(int rows, int cols);
    bool isConnected(Point a, Point b);
    void shuffleMap();
    int getMapData(int x, int y);
    void setMapData(int x, int y, int val);
    bool isMapEmpty();

    bool findPath(Point a, Point b, std::vector<Point>& path);
    bool findHintPair(Point& a, Point& b);
    bool hasAnySolution();
    bool shuffleUntilSolvable(int maxTry = 50);

    void setMaxType(int n);

private:
    void createPairNumbers();
    bool canConnectByBFS(Point a, Point b, std::vector<Point>* path = nullptr);
    void invalidateSolutionCache() { m_solutionCacheValid = false; }

    bool m_solutionCacheValid = false;
    bool m_hasSolutionCached = false;

private:
    std::vector<std::vector<int>> m_map;
    int m_rows;
    int m_cols;
    static constexpr int EMPTY = 0;
    int m_maxType = 20;
};

#endif