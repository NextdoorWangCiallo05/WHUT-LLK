#include "gamelogic.h"

#include <algorithm>
#include <ctime>
#include <vector>
#include <random>
#include <queue>
#include <array>
#include <unordered_map>
#include <limits>

// 这个文件实现了游戏的核心逻辑，包括地图初始化、连接判断、路径寻找、提示功能和解决方案缓存等
// GameLogic 类，负责管理游戏状态和提供核心功能
// 主要成员变量包括地图数据、地图尺寸、BFS 相关的缓冲区以及解决方案缓存

// 使用全局随机数生成器，确保在整个游戏过程中使用同一个随机数引擎，避免重复种子导致的随机性问题
namespace {
    static std::mt19937& globalRng()
    {
        static std::mt19937 rng([] {
            std::random_device rd;
            std::seed_seq seq{
                rd(), rd(), rd(), rd(),
                static_cast<unsigned int>(std::time(nullptr))
            };
            return std::mt19937(seq);
            }());
        return rng;
    }
}

// 构造函数和析构函数，初始化成员变量
GameLogic::GameLogic()
    : m_rows(0), m_cols(0)
{
}

GameLogic::~GameLogic()
{
}

// 初始化地图，设置行数和列数，并生成配对的数字填充地图，同时重置解决方案缓存
void GameLogic::initMap(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    m_map.assign(m_rows, std::vector<int>(m_cols, EMPTY));
    createPairNumbers();
    invalidateSolutionCache();
}

// 设置图案类型数量，影响地图生成时的配对数字范围，同时重置解决方案缓存
void GameLogic::createPairNumbers()
{
    int total = m_rows * m_cols;
    if (total % 2 != 0) total--;

    std::vector<int> nums;
    nums.reserve(total);

    int maxType = m_maxType;
    int pairCount = total / 2;

    for (int i = 0; i < pairCount; ++i) {
        int v = (i % maxType) + 1;
        nums.push_back(v);
        nums.push_back(v);
    }

    std::shuffle(nums.begin(), nums.end(), globalRng());

    int idx = 0;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_map[r][c] = (idx < static_cast<int>(nums.size())) ? nums[idx++] : EMPTY;
        }
    }
}

// 判断地图是否为空，即所有格子都是 EMPTY
bool GameLogic::isMapEmpty()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_map[i][j] != EMPTY) return false;
        }
    }
    return true;
}

// 获取指定位置的地图数据，如果超出范围返回 EMPTY
int GameLogic::getMapData(int x, int y)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return EMPTY;
    return m_map[x][y];
}

// 设置指定位置的地图数据，如果超出范围或值未改变则不操作，否则更新地图并重置解决方案缓存
void GameLogic::setMapData(int x, int y, int val)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;
    if (m_map[x][y] == val) return;
    m_map[x][y] = val;
    invalidateSolutionCache();
}

// 洗牌地图，将非 EMPTY 的格子中的数字随机打乱位置，同时重置解决方案缓存
void GameLogic::shuffleMap()
{
    std::vector<int> allCells;
    allCells.reserve(m_rows * m_cols);

    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_map[i][j] != EMPTY)
                allCells.push_back(m_map[i][j]);
        }
    }

    std::shuffle(allCells.begin(), allCells.end(), globalRng());

    int idx = 0;
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_map[i][j] != EMPTY && idx < static_cast<int>(allCells.size()))
                m_map[i][j] = allCells[idx++];
        }
    }
    invalidateSolutionCache();
}

// 判断两个点是否可以连接，条件是它们不相同、都非 EMPTY、值相同，并且通过 BFS 判断是否存在合法路径
bool GameLogic::isConnected(Point a, Point b)
{
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;

    return canConnectByBFS(a, b, nullptr);
}

// 寻找两个点之间的连接路径，如果存在合法路径则返回 true，并将路径存储在 path 中，路径包含起点和终点的拐点坐标
bool GameLogic::findPath(Point a, Point b, std::vector<Point>& path)
{
    path.clear();
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;

    return canConnectByBFS(a, b, &path);
}

// 寻找任意一对可连接的点，如果存在则返回 true，并将这对点的坐标存储在 a 和 b 中
bool GameLogic::findHintPair(Point& a, Point& b)
{
    std::unordered_map<int, std::vector<Point>> buckets;
    buckets.reserve(32);

    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            int v = m_map[i][j];
            if (v != EMPTY) {
                buckets[v].emplace_back(i, j);
            }
        }
    }

    for (auto& kv : buckets) {
        auto& vec = kv.second;
        const int n = static_cast<int>(vec.size());
        if (n < 2) continue;

        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (isConnected(vec[i], vec[j])) {
                    a = vec[i];
                    b = vec[j];
                    return true;
                }
            }
        }
    }

    return false;
}

// 判断地图上是否存在任何一对可连接的点，如果缓存有效则直接返回缓存结果，否则通过 findHintPair 计算并更新缓存
bool GameLogic::hasAnySolution()
{
    if (m_solutionCacheValid) {
        return m_hasSolutionCached;
    }

    Point a, b;
    m_hasSolutionCached = findHintPair(a, b);
    m_solutionCacheValid = true;
    return m_hasSolutionCached;
}

// 洗牌地图直到存在可连接的点，最多尝试 maxTry 次，如果成功则返回 true，否则返回 false
bool GameLogic::shuffleUntilSolvable(int maxTry)
{
    if (isMapEmpty()) return true;

    for (int t = 0; t < maxTry; ++t) {
        shuffleMap();
        if (hasAnySolution()) return true;
    }
    return false;
}

// 确保 BFS 相关的缓冲区大小足够，参数 R 和 C 是扩展地图的行数和列数，needPath 表示是否需要路径缓冲
void GameLogic::ensureBfsBuffers(int R, int C, bool needPath)
{
    bool needResizeCore = (R > m_bufR || C > m_bufC);
    if (needResizeCore) {
        m_bufR = std::max(m_bufR, R);
        m_bufC = std::max(m_bufC, C);

        m_exBuf.assign(m_bufR, std::vector<int>(m_bufC, 0));
        m_distBuf.assign(
            m_bufR,
            std::vector<std::array<int, 4>>(m_bufC, std::array<int, 4>{0, 0, 0, 0})
        );
        m_preBuf.clear(); // 核心尺寸变化，路径缓冲重建
    }

    if (needPath && m_preBuf.empty()) {
        Prev d{ -1, -1, -1, false };
        m_preBuf.assign(
            m_bufR,
            std::vector<std::array<Prev, 4>>(m_bufC, std::array<Prev, 4>{ d, d, d, d })
        );
    }
}

// 使用 BFS 判断两个点是否可以连接，路径最多包含 2 个拐点，允许绕外圈连接，如果 needPath 不为 nullptr 则同时记录路径信息
bool GameLogic::canConnectByBFS(Point a, Point b, std::vector<Point>* path)
{
    const int R = m_rows + 2;
    const int C = m_cols + 2;
    const bool needPath = (path != nullptr);
    const int INF = std::numeric_limits<int>::max() / 4;

    ensureBfsBuffers(R, C, needPath);

    // 只重置本次使用区域 [0..R), [0..C)
    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < C; ++j) {
            m_exBuf[i][j] = 0;
            m_distBuf[i][j] = std::array<int, 4>{ INF, INF, INF, INF };

            if (needPath) {
                Prev d{ -1, -1, -1, false };
                m_preBuf[i][j] = std::array<Prev, 4>{ d, d, d, d };
            }
        }
    }

    // 填扩展地图
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            m_exBuf[i + 1][j + 1] = m_map[i][j];
        }
    }

    const int sx = a.x + 1, sy = a.y + 1;
    const int tx = b.x + 1, ty = b.y + 1;
    m_exBuf[tx][ty] = 0;

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    struct Node {
        int x, y, dir, turns;
    };

    std::queue<Node> q;

    // 从起点向四个方向“发射”
    for (int d = 0; d < 4; ++d) {
        int nx = sx + dx[d], ny = sy + dy[d];
        if (nx < 0 || nx >= R || ny < 0 || ny >= C) continue;
        if (m_exBuf[nx][ny] != 0 && !(nx == tx && ny == ty)) continue;

        m_distBuf[nx][ny][d] = 0;
        if (needPath) {
            m_preBuf[nx][ny][d] = Prev{ sx, sy, -1, true };
        }
        q.push({ nx, ny, d, 0 });
    }

    int endDir = -1;
    int endX = -1, endY = -1;

    while (!q.empty()) {
        Node cur = q.front();
        q.pop();

        if (cur.x == tx && cur.y == ty && cur.turns <= 2) {
            endDir = cur.dir;
            endX = cur.x;
            endY = cur.y;
            break;
        }

        // 直行
        {
            int nx = cur.x + dx[cur.dir];
            int ny = cur.y + dy[cur.dir];
            if (nx >= 0 && nx < R && ny >= 0 && ny < C) {
                if (m_exBuf[nx][ny] == 0 || (nx == tx && ny == ty)) {
                    if (m_distBuf[nx][ny][cur.dir] > cur.turns) {
                        m_distBuf[nx][ny][cur.dir] = cur.turns;
                        if (needPath) {
                            m_preBuf[nx][ny][cur.dir] = Prev{ cur.x, cur.y, cur.dir, true };
                        }
                        q.push({ nx, ny, cur.dir, cur.turns });
                    }
                }
            }
        }

        // 拐弯
		// 注意：拐弯后 turns 增加 1，且不能超过 2
        if (cur.turns < 2) {
            for (int nd = 0; nd < 4; ++nd) {
                if (nd == cur.dir) continue;

                int tx2 = cur.x + dx[nd];
                int ty2 = cur.y + dy[nd];
                if (tx2 < 0 || tx2 >= R || ty2 < 0 || ty2 >= C) continue;
                if (m_exBuf[tx2][ty2] != 0 && !(tx2 == tx && ty2 == ty)) continue;

                if (m_distBuf[tx2][ty2][nd] > cur.turns + 1) {
                    m_distBuf[tx2][ty2][nd] = cur.turns + 1;
                    if (needPath) {
                        m_preBuf[tx2][ty2][nd] = Prev{ cur.x, cur.y, cur.dir, true };
                    }
                    q.push({ tx2, ty2, nd, cur.turns + 1 });
                }
            }
        }
    }

    if (endDir == -1) return false;
    if (!needPath) return true;

    // 还原路径
    std::vector<Point> rev;
    int cx = endX;
    int cy = endY;
    int cd = endDir;
    rev.push_back(Point(cx - 1, cy - 1));

    while (true) {
        Prev p = m_preBuf[cx][cy][cd];
        if (!p.has) break;

        if (p.px == sx && p.py == sy && p.pdir == -1) {
            rev.push_back(Point(sx - 1, sy - 1));
            break;
        }

        rev.push_back(Point(p.px - 1, p.py - 1));
        cx = p.px;
        cy = p.py;
        cd = p.pdir;
    }

    if (rev.empty()) return false;
    std::reverse(rev.begin(), rev.end());

    path->clear();
    path->push_back(rev.front());

    // 压缩为“拐点路径”
    for (size_t i = 1; i + 1 < rev.size(); ++i) {
        Point p0 = rev[i - 1], p1 = rev[i], p2 = rev[i + 1];
        int dx1 = p1.x - p0.x, dy1 = p1.y - p0.y;
        int dx2 = p2.x - p1.x, dy2 = p2.y - p1.y;
        if (dx1 != dx2 || dy1 != dy2) {
            path->push_back(p1);
        }
    }

    path->push_back(rev.back());
    return path->size() >= 2;
}

// 设置图案类型数量，影响地图生成时的配对数字范围，同时重置解决方案缓存
void GameLogic::setMaxType(int n)
{
    if (n < 1) n = 1;
    m_maxType = n;
}