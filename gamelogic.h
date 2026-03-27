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

    // 新增：获取一条可连接路径（返回的是棋盘坐标，可能含外圈点）
    bool findPath(Point a, Point b, std::vector<Point>& path);

    // 新增：提示一对可消除的点
    bool findHintPair(Point& a, Point& b);

    // 新增：是否还有解
    bool hasAnySolution();

    // 新增：重排直到有解（可限制次数）
    bool shuffleUntilSolvable(int maxTry = 50);

private:
    bool checkStraight(Point a, Point b);
    bool checkOneCorner(Point a, Point b);
    bool checkTwoCorner(Point a, Point b);

    void createPairNumbers();

    // 原判定
    bool canConnectByBFS(Point a, Point b);

    // 新增：BFS + 路径输出
    bool canConnectByBFSWithPath(Point a, Point b, std::vector<Point>& path);

    int** m_map;
    int m_rows;
    int m_cols;
    const int EMPTY = 0;
};

#endif