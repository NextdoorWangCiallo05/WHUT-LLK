#include "gamelogic.h"

#include <algorithm>
#include <ctime>
#include <vector>
#include <random>
#include <queue>
#include <array>
#include <unordered_map>

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

GameLogic::GameLogic()
    : m_rows(0), m_cols(0)
{
}

GameLogic::~GameLogic()
{
}

void GameLogic::initMap(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    m_map.assign(m_rows, std::vector<int>(m_cols, EMPTY));
    createPairNumbers();
    invalidateSolutionCache();
}

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

bool GameLogic::isMapEmpty()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_map[i][j] != EMPTY) return false;
        }
    }
    return true;
}

int GameLogic::getMapData(int x, int y)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return EMPTY;
    return m_map[x][y];
}

void GameLogic::setMapData(int x, int y, int val)
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;
    if (m_map[x][y] == val) return;
    m_map[x][y] = val;
    invalidateSolutionCache();
}

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

bool GameLogic::isConnected(Point a, Point b)
{
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;

    return canConnectByBFS(a, b, nullptr);
}

bool GameLogic::findPath(Point a, Point b, std::vector<Point>& path)
{
    path.clear();
    if (a == b) return false;
    if (getMapData(a.x, a.y) == EMPTY) return false;
    if (getMapData(a.x, a.y) != getMapData(b.x, b.y)) return false;

    return canConnectByBFS(a, b, &path);
}

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

bool GameLogic::shuffleUntilSolvable(int maxTry)
{
    if (isMapEmpty()) return true;

    for (int t = 0; t < maxTry; ++t) {
        shuffleMap();
        if (hasAnySolution()) return true;
    }
    return false;
}

bool GameLogic::canConnectByBFS(Point a, Point b, std::vector<Point>* path)
{
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
    ex[tx][ty] = 0;

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    struct Node {
        int x, y, dir, turns;
    };

    struct Prev {
        int px, py, pdir;
        bool has;
    };

    const int INF = 1e9;
    std::vector<std::vector<std::array<int, 4>>> dist(
        R, std::vector<std::array<int, 4>>(C, std::array<int, 4>{INF, INF, INF, INF})
    );

    std::vector<std::vector<std::array<Prev, 4>>> pre;
    if (path) {
        pre.assign(
            R,
            std::vector<std::array<Prev, 4>>(C, std::array<Prev, 4>{
            Prev{ -1, -1, -1, false },
                Prev{ -1, -1, -1, false },
                Prev{ -1, -1, -1, false },
                Prev{ -1, -1, -1, false }
        })
        );
    }

    std::queue<Node> q;

    for (int d = 0; d < 4; ++d) {
        int nx = sx + dx[d], ny = sy + dy[d];
        if (nx < 0 || nx >= R || ny < 0 || ny >= C) continue;
        if (ex[nx][ny] != 0 && !(nx == tx && ny == ty)) continue;

        dist[nx][ny][d] = 0;
        if (path) {
            pre[nx][ny][d] = Prev{ sx, sy, -1, true };
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

        int nx = cur.x + dx[cur.dir];
        int ny = cur.y + dy[cur.dir];
        if (nx >= 0 && nx < R && ny >= 0 && ny < C) {
            if (ex[nx][ny] == 0 || (nx == tx && ny == ty)) {
                if (dist[nx][ny][cur.dir] > cur.turns) {
                    dist[nx][ny][cur.dir] = cur.turns;
                    if (path) {
                        pre[nx][ny][cur.dir] = Prev{ cur.x, cur.y, cur.dir, true };
                    }
                    q.push({ nx, ny, cur.dir, cur.turns });
                }
            }
        }

        if (cur.turns < 2) {
            for (int nd = 0; nd < 4; ++nd) {
                if (nd == cur.dir) continue;
                int tx2 = cur.x + dx[nd];
                int ty2 = cur.y + dy[nd];
                if (tx2 < 0 || tx2 >= R || ty2 < 0 || ty2 >= C) continue;
                if (ex[tx2][ty2] != 0 && !(tx2 == tx && ty2 == ty)) continue;

                if (dist[tx2][ty2][nd] > cur.turns + 1) {
                    dist[tx2][ty2][nd] = cur.turns + 1;
                    if (path) {
                        pre[tx2][ty2][nd] = Prev{ cur.x, cur.y, cur.dir, true };
                    }
                    q.push({ tx2, ty2, nd, cur.turns + 1 });
                }
            }
        }
    }

    if (endDir == -1) return false;
    if (!path) return true;

    std::vector<Point> rev;
    int cx = endX;
    int cy = endY;
    int cd = endDir;
    rev.push_back(Point(cx - 1, cy - 1));

    while (true) {
        Prev p = pre[cx][cy][cd];
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

void GameLogic::setMaxType(int n)
{
    if (n < 1) n = 1;
    m_maxType = n;
}