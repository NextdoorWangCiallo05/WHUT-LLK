#include "gamelogic.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <random> // 添加头文件
#include <queue> // 添加此头文件以修复 std::queue 未定义的问题
#include <array> // 添加此头文件以修复 std::array 未定义的问题

GameLogic::GameLogic()
    : m_map(nullptr), m_rows(0), m_cols(0)
{
    srand(static_cast<unsigned int>(time(nullptr))); // 使用标准C库的srand替换qsrand
}

GameLogic::~GameLogic()
{
    if (m_map) {
        for (int i = 0; i < m_rows; i++)
            delete[] m_map[i];
        delete[] m_map;
    }
}

void GameLogic::initMap(int rows, int cols)
{
    // 先释放旧内存（按旧 m_rows）
    if (m_map) {
        for (int i = 0; i < m_rows; i++) {
            delete[] m_map[i];
        }
        delete[] m_map;
        m_map = nullptr;
    }

    // 再设置新尺寸
    m_rows = rows;
    m_cols = cols;

    // 创建新地图
    m_map = new int* [m_rows];
    for (int i = 0; i < m_rows; i++) {
        m_map[i] = new int[m_cols];
        memset(m_map[i], 0, m_cols * sizeof(int));
    }

    createPairNumbers();
}

// 生成成对随机数（连连看核心：必须成对）
void GameLogic::createPairNumbers()
{
    int total = m_rows * m_cols;
    if (total % 2 != 0) total--; // 保证偶数

    std::vector<int> nums;
    nums.reserve(total);

    int maxType = 20; // 你的图片资源 1~20
    int pairCount = total / 2; // 需要多少对

    // 循环填充 pairCount 对（类型可重复出现）
    for (int i = 0; i < pairCount; ++i) {
        int v = (i % maxType) + 1; // 1..20 循环
        nums.push_back(v);
        nums.push_back(v);
    }

    static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
    std::shuffle(nums.begin(), nums.end(), rng);

    int idx = 0;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_map[r][c] = (idx < (int)nums.size()) ? nums[idx++] : EMPTY;
        }
    }
}

// 地图是否为空（胜利条件）
bool GameLogic::isMapEmpty()
{
    for (int i = 0; i < m_rows; i++)
        for (int j = 0; j < m_cols; j++)
            if (m_map[i][j] != EMPTY) return false;
    return true;
}

// 获取格子数据
int GameLogic::getMapData(int x, int y)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return EMPTY;
    return m_map[x][y];
}

// 设置格子数据
void GameLogic::setMapData(int x, int y, int val)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;
    m_map[x][y] = val;
}

// 重排地图
void GameLogic::shuffleMap()
{
    std::vector<int> allCells;
    for (int i = 0; i < m_rows; i++)
        for (int j = 0; j < m_cols; j++)
            if (m_map[i][j] != EMPTY)
                allCells.push_back(m_map[i][j]);

    // 打乱
    static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
    std::shuffle(allCells.begin(), allCells.end(), rng);

    // 重新填充
    int idx = 0;
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_map[i][j] != EMPTY && idx < (int)allCells.size())
                m_map[i][j] = allCells[idx++];
        }
    }
}

// ===================== 核心连通算法 =====================
bool GameLogic::isConnected(Point a, Point b)
{
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;

    return canConnectByBFS(a, b);
}

// 1. 直线连通（无拐点）
bool GameLogic::checkStraight(Point a, Point b)
{
    // 同一行
    if (a.x == b.x) {
        int minCol = std::min(a.y, b.y) + 1;
        int maxCol = std::max(a.y, b.y) - 1;
        for (int c = minCol; c <= maxCol; c++) {
            if (m_map[a.x][c] != EMPTY) return false;
        }
        return true;
    }

    // 同一列
    if (a.y == b.y) {
        int minRow = std::min(a.x, b.x) + 1;
        int maxRow = std::max(a.x, b.x) - 1;
        for (int r = minRow; r <= maxRow; r++) {
            if (m_map[r][a.y] != EMPTY) return false;
        }
        return true;
    }

    return false;
}

// 2. 一个拐点连通
bool GameLogic::checkOneCorner(Point a, Point b)
{
    // 拐点1
    Point p1(a.x, b.y);
    if (m_map[p1.x][p1.y] == EMPTY) {
        if (checkStraight(a, p1) && checkStraight(p1, b))
            return true;
    }

    // 拐点2
    Point p2(b.x, a.y);
    if (m_map[p2.x][p2.y] == EMPTY) {
        if (checkStraight(a, p2) && checkStraight(p2, b))
            return true;
    }

    return false;
}

// 3. 两个拐点连通（最常用）
bool GameLogic::checkTwoCorner(Point a, Point b)
{
    // 向右扫描
    for (int c = a.y + 1; c < m_cols; c++) {
        if (m_map[a.x][c] != EMPTY) break;
        Point p(a.x, c);
        if (checkOneCorner(p, b)) return true;
    }

    // 向左扫描
    for (int c = a.y - 1; c >= 0; c--) {
        if (m_map[a.x][c] != EMPTY) break;
        Point p(a.x, c);
        if (checkOneCorner(p, b)) return true;
    }

    // 向下扫描
    for (int r = a.x + 1; r < m_rows; r++) {
        if (m_map[r][a.y] != EMPTY) break;
        Point p(r, a.y);
        if (checkOneCorner(p, b)) return true;
    }

    // 向上扫描
    for (int r = a.x - 1; r >= 0; r--) {
        if (m_map[r][a.y] != EMPTY) break;
        Point p(r, a.y);
        if (checkOneCorner(p, b)) return true;
    }

    return false;
}

bool GameLogic::canConnectByBFS(Point a, Point b)
{
    // 扩展棋盘：四周加一圈空白，允许“绕外面”
    int R = m_rows + 2;
    int C = m_cols + 2;

    std::vector<std::vector<int>> ex(R, std::vector<int>(C, 0));
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            ex[i + 1][j + 1] = m_map[i][j];
        }
    }

    int sx = a.x + 1, sy = a.y + 1;
    int tx = b.x + 1, ty = b.y + 1;

    // 终点视为可进入
    ex[tx][ty] = 0;

    // 方向：下上右左
    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    struct Node {
        int x, y, dir, turns;
    };

    // dist[x][y][dir] = 到达该状态的最少拐点
    const int INF = 1e9;
    std::vector<std::vector<std::array<int, 4>>> dist(
        R, std::vector<std::array<int, 4>>(C, std::array<int, 4>{INF, INF, INF, INF})
    );

    std::queue<Node> q;

    // 从起点向四个方向“发射”
    for (int d = 0; d < 4; ++d) {
        int nx = sx + dx[d], ny = sy + dy[d];
        if (nx < 0 || nx >= R || ny < 0 || ny >= C) continue;
        if (ex[nx][ny] != 0 && !(nx == tx && ny == ty)) continue;

        dist[nx][ny][d] = 0;
        q.push({ nx, ny, d, 0 });
    }

    while (!q.empty()) {
        Node cur = q.front(); q.pop();

        if (cur.x == tx && cur.y == ty && cur.turns <= 2) {
            return true;
        }

        // 继续直走
        int nx = cur.x + dx[cur.dir];
        int ny = cur.y + dy[cur.dir];
        if (nx >= 0 && nx < R && ny >= 0 && ny < C) {
            if ((nx >= 0 && nx < R && ny >= 0 && ny < C && (ex[nx][ny] == 0)) || (nx == tx && ny == ty)) {
                if (dist[nx][ny][cur.dir] > cur.turns) {
                    dist[nx][ny][cur.dir] = cur.turns;
                    q.push({ nx, ny, cur.dir, cur.turns });
                }
            }
        }

        // 转向（+1拐点）
        if (cur.turns < 2) {
            for (int nd = 0; nd < 4; ++nd) {
                if (nd == cur.dir) continue;
                int tx2 = cur.x + dx[nd];
                int ty2 = cur.y + dy[nd];
                if (tx2 < 0 || tx2 >= R || ty2 < 0 || ty2 >= C) continue;
                if (ex[tx2][ty2] != 0 && !(tx2 == tx && ty2 == ty)) continue;

                if (dist[tx2][ty2][nd] > cur.turns + 1) {
                    dist[tx2][ty2][nd] = cur.turns + 1;
                    q.push({ tx2, ty2, nd, cur.turns + 1 });
                }
            }
        }
    }

    return false;
}

bool GameLogic::findPath(Point a, Point b, std::vector<Point>& path)
{
    path.clear();
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;
    return canConnectByBFSWithPath(a, b, path);
}

bool GameLogic::findHintPair(Point& a, Point& b)
{
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            if (m_map[i][j] == EMPTY) continue;
            for (int x = i; x < m_rows; ++x) {
                int yStart = (x == i) ? (j + 1) : 0;
                for (int y = yStart; y < m_cols; ++y) {
                    if (m_map[x][y] == EMPTY) continue;
                    if (m_map[i][j] != m_map[x][y]) continue;
                    if (isConnected(Point(i, j), Point(x, y))) {
                        a = Point(i, j);
                        b = Point(x, y);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool GameLogic::hasAnySolution()
{
    Point a, b;
    return findHintPair(a, b);
}

bool GameLogic::shuffleUntilSolvable(int maxTry)
{
    if (isMapEmpty()) return true;

    for (int t = 0; t < maxTry; ++t) {
        shuffleMap();
        if (hasAnySolution()) return true;
    }
    return false;
}

bool GameLogic::canConnectByBFSWithPath(Point a, Point b, std::vector<Point>& path)
{
    path.clear();

    int R = m_rows + 2;
    int C = m_cols + 2;

    std::vector<std::vector<int>> ex(R, std::vector<int>(C, 0));
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            ex[i + 1][j + 1] = m_map[i][j];
        }
    }

    int sx = a.x + 1, sy = a.y + 1;
    int tx = b.x + 1, ty = b.y + 1;
    ex[tx][ty] = 0; // 终点可进入

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    struct Node { int x, y, dir, turns; };
    struct Prev { int px, py, pdir; bool has; };

    const int INF = 1e9;
    std::vector<std::vector<std::array<int, 4>>> dist(
        R, std::vector<std::array<int, 4>>(C, std::array<int, 4>{INF, INF, INF, INF})
    );
    std::vector<std::vector<std::array<Prev, 4>>> pre(
        R, std::vector<std::array<Prev, 4>>(C, std::array<Prev, 4>{
        Prev{ -1,-1,-1,false }, Prev{ -1,-1,-1,false }, Prev{ -1,-1,-1,false }, Prev{ -1,-1,-1,false }
    })
    );

    std::queue<Node> q;

    // 起点向四方向发射
    for (int d = 0; d < 4; ++d) {
        int nx = sx + dx[d], ny = sy + dy[d];
        if (nx < 0 || nx >= R || ny < 0 || ny >= C) continue;
        if (ex[nx][ny] != 0 && !(nx == tx && ny == ty)) continue;

        dist[nx][ny][d] = 0;
        pre[nx][ny][d] = Prev{ sx, sy, -1, true }; // 前驱标记为起点
        q.push({ nx, ny, d, 0 });
    }

    int endDir = -1;
    while (!q.empty()) {
        Node cur = q.front(); q.pop();

        if (cur.x == tx && cur.y == ty && cur.turns <= 2) {
            endDir = cur.dir;
            break;
        }

        // 直走
        int nx = cur.x + dx[cur.dir], ny = cur.y + dy[cur.dir];
        if (nx >= 0 && nx < R && ny >= 0 && ny < C) {
            if (ex[nx][ny] == 0 || (nx == tx && ny == ty)) {
                if (dist[nx][ny][cur.dir] > cur.turns) {
                    dist[nx][ny][cur.dir] = cur.turns;
                    pre[nx][ny][cur.dir] = Prev{ cur.x, cur.y, cur.dir, true };
                    q.push({ nx, ny, cur.dir, cur.turns });
                }
            }
        }

        // 转向
        if (cur.turns < 2) {
            for (int nd = 0; nd < 4; ++nd) {
                if (nd == cur.dir) continue;
                int tx2 = cur.x + dx[nd], ty2 = cur.y + dy[nd];
                if (tx2 < 0 || tx2 >= R || ty2 < 0 || ty2 >= C) continue;
                if (ex[tx2][ty2] != 0 && !(tx2 == tx && ty2 == ty)) continue;

                if (dist[tx2][ty2][nd] > cur.turns + 1) {
                    dist[tx2][ty2][nd] = cur.turns + 1;
                    pre[tx2][ty2][nd] = Prev{ cur.x, cur.y, cur.dir, true };
                    q.push({ tx2, ty2, nd, cur.turns + 1 });
                }
            }
        }
    }

    if (endDir == -1) return false;

    // 回溯完整网格路径（扩展坐标）
    std::vector<Point> rev;
    int cx = tx, cy = ty, cd = endDir;
    rev.push_back(Point(cx - 1, cy - 1)); // 先放终点(棋盘坐标)

    while (true) {
        Prev p = pre[cx][cy][cd];
        if (!p.has) break;

        // 到了起点锚点
        if (p.px == sx && p.py == sy && p.pdir == -1) {
            rev.push_back(Point(sx - 1, sy - 1));
            break;
        }

        rev.push_back(Point(p.px - 1, p.py - 1));
        cx = p.px; cy = p.py; cd = p.pdir;
    }

    if (rev.empty()) return false;
    std::reverse(rev.begin(), rev.end());

    // 压缩成折线关键点（仅保留拐点 + 起终点）
    path.clear();
    path.push_back(rev.front());

    for (size_t i = 1; i + 1 < rev.size(); ++i) {
        Point p0 = rev[i - 1], p1 = rev[i], p2 = rev[i + 1];
        int dx1 = p1.x - p0.x, dy1 = p1.y - p0.y;
        int dx2 = p2.x - p1.x, dy2 = p2.y - p1.y;
        if (dx1 != dx2 || dy1 != dy2) {
            path.push_back(p1); // 拐点
        }
    }

    path.push_back(rev.back());
    return path.size() >= 2;
}