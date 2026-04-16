#include "gamecontrol.h"
#include "glassmessagebox.h"

#include <QSettings>

// GameControl 类的实现：管理游戏状态、处理用户输入、与 GameLogic 交互、发出信号通知界面更新等
// 构造函数：初始化成员变量，创建 GameLogic 实例
GameControl::GameControl(QObject* parent)
    : QObject(parent),
      m_logic(new GameLogic()),
      m_rows(10),
      m_cols(16),
      m_hasSelected(false),
      m_showLink(false),
      m_hasHint(false),
      m_inputEnabled(true),
      m_boardEpoch(0)
{
}

GameControl::~GameControl()
{
    delete m_logic;
}

// 初始化地图，设置行列数，并让 GameLogic 创建地图数据
void GameControl::initMap(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    m_logic->initMap(rows, cols);
}

// 重置游戏状态，增加 epoch 以使任何未完成的动画或操作失效，清除选择和提示状态
void GameControl::resetState()
{
    ++m_boardEpoch;
    m_hasSelected = false;
    m_hasHint = false;
    clearLinkPath();
}

// 处理用户点击单元格的逻辑：选择、匹配、路径显示、状态更新等
void GameControl::handleCellClick(int x, int y)
{
    if (!m_inputEnabled) return;
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;

	// 如果点击的单元格为空，则直接返回；否则根据当前选择状态进行处理：
    // 如果没有选择则选中当前单元格；如果已经选择则尝试匹配，如果匹配成功则显示路径、更新状态、发出信号，并在短暂延迟后清除路径和检查游戏状态；
    // 如果匹配失败则转移选择到当前单元格
    Point cur(x, y);
    if (m_logic->getMapData(x, y) == 0) return;

    clearHintStyle();

	// 如果当前没有选择任何单元格，则选中当前单元格并发出选择信号；否则先发出取消选择的信号，如果点击的单元格与之前选择的相同则取消选择；
    // 如果不同则尝试匹配，如果匹配成功则显示路径、更新状态、发出匹配信号，并在短暂延迟后清除路径和检查游戏状态；如果匹配失败则转移选择到当前单元格
    if (!m_hasSelected) {
        m_lastPoint = cur;
        m_hasSelected = true;
        emit cellSelected(x, y);
        return;
    }

	// 已经有一个单元格被选中，先发出取消选择的信号
    emit cellDeselected(m_lastPoint.x, m_lastPoint.y);

    if (m_lastPoint.x == x && m_lastPoint.y == y) {
        m_hasSelected = false;
        return;
    }

	// 尝试找到连接路径，如果找到则显示路径、更新状态、发出匹配信号，并在短暂延迟后清除路径和检查游戏状态；如果没有找到，则转移选择到当前单元格
    std::vector<Point> path;
    if (m_logic->findPath(m_lastPoint, cur, path)) {
        drawLinkPath(path);

        m_logic->setMapData(m_lastPoint.x, m_lastPoint.y, 0);
        m_logic->setMapData(cur.x, cur.y, 0);

        emit pairMatched(m_lastPoint, cur, path);
        emit cellsRemoved(m_lastPoint, cur);

        const quint64 epochAtSchedule = m_boardEpoch;

        QTimer::singleShot(120, this, [this, epochAtSchedule]() {
            if (m_boardEpoch != epochAtSchedule) return;

            clearLinkPath();
            checkPostRemoveState();
            });

        m_hasSelected = false;
    }
    else {
        emit selectionTransferred(m_lastPoint.x, m_lastPoint.y, x, y);
        m_lastPoint = cur;
        emit cellSelected(x, y);
    }
}

// 显示提示：寻找一对可连接的单元格，如果找到则发出信号显示提示；如果没有找到，根据设置决定是否自动洗牌或提示用户需要手动洗牌
void GameControl::showHint()
{
    clearHintStyle();

    Point a, b;
    if (m_logic->findHintPair(a, b)) {
        m_hintA = a;
        m_hintB = b;
        m_hasHint = true;
        emit hintDisplayed(a, b);
    }
    else {
        QSettings s("YourCompany", "LLK_Refresh");
        bool autoShuffle = s.value("game/autoShuffle", true).toBool();

        if (autoShuffle) {
            shuffleUntilSolvable();
            emit noSolutionAutoShuffled();
        }
        else {
            emit noSolutionManualShuffleNeeded();
        }
    }
}

// 清除提示样式：如果当前有提示，则发出信号清除提示，并重置提示状态
void GameControl::clearHintStyle()
{
    if (!m_hasHint) return;

    emit hintCleared(m_hintA, m_hintB);
    m_hasHint = false;
}

// 绘制连接路径：接受一个点的路径，更新内部状态以显示连接，并发出信号通知界面更新
void GameControl::drawLinkPath(const std::vector<Point>& path)
{
    m_linkPath = path;
    m_showLink = true;
    emit linkPathUpdated();
}

// 清除连接路径：如果当前没有显示连接且路径为空，则直接返回；否则重置状态并发出信号通知界面更新
void GameControl::clearLinkPath()
{
    if (!m_showLink && m_linkPath.empty()) return;
    m_showLink = false;
    m_linkPath.clear();
    emit linkPathCleared();
}

// 洗牌：调用 GameLogic 的 shuffleMap 方法随机打乱地图数据
void GameControl::shuffle()
{
    m_logic->shuffleMap();
}

// 洗牌直到可解：调用 GameLogic 的 shuffleUntilSolvable 方法，尝试多次洗牌直到找到一个有解的地图，返回是否成功
bool GameControl::shuffleUntilSolvable(int maxTry)
{
    return m_logic->shuffleUntilSolvable(maxTry);
}

// 检查游戏是否完成：如果 GameLogic 存在且地图为空，则返回 true；否则返回 false
bool GameControl::isFinished() const
{
    return m_logic ? m_logic->isMapEmpty() : true;
}

// 检查地图是否为空：如果 GameLogic 存在且地图为空，则返回 true；否则返回 true（如果没有 GameLogic 则认为地图为空）
bool GameControl::isEmpty() const
{
    return m_logic ? m_logic->isMapEmpty() : true;
}

// 检查是否有任何解：如果 GameLogic 存在，则调用 hasAnySolution 方法返回结果；否则返回 false（如果没有 GameLogic 则认为没有解）
bool GameControl::hasAnySolution() const
{
    return m_logic ? m_logic->hasAnySolution() : false;
}

// 获取地图数据：如果 GameLogic 存在，则调用 getMapData 方法返回指定位置的数据；否则返回 0（如果没有 GameLogic 则认为该位置为空）
int GameControl::getMapData(int x, int y) const
{
    return m_logic ? m_logic->getMapData(x, y) : 0;
}

// 设置地图数据：如果 GameLogic 存在，则调用 setMapData 方法设置指定位置的数据；否则不执行任何操作（如果没有 GameLogic 则无法设置数据）
void GameControl::setMapData(int x, int y, int val)
{
    if (m_logic) m_logic->setMapData(x, y, val);
}

// 设置最大类型：如果 GameLogic 存在，则调用 setMaxType 方法设置最大类型；否则不执行任何操作（如果没有 GameLogic 则无法设置最大类型）
void GameControl::setMaxType(int n)
{
    if (m_logic) m_logic->setMaxType(n);
}

// 获取输入状态：返回当前输入是否启用的状态
bool GameControl::inputEnabled() const
{
    return m_inputEnabled;
}

// 设置输入状态：接受一个布尔值参数，更新内部状态以启用或禁用输入
void GameControl::setInputEnabled(bool enabled)
{
    m_inputEnabled = enabled;
}

// 获取当前棋盘状态的 epoch：返回一个递增的整数，表示当前棋盘状态的版本，用于使任何未完成的动画或操作失效
void GameControl::incrementEpoch()
{
    ++m_boardEpoch;
}

// 检查移除单元格后的状态：如果地图为空则发出游戏完成信号；如果没有任何解则根据设置决定是否自动洗牌或提示用户需要手动洗牌
void GameControl::checkPostRemoveState()
{
    if (m_logic->isMapEmpty()) {
        emit gameCleared();
        return;
    }

    if (!m_logic->hasAnySolution()) {
        QSettings s("YourCompany", "LLK_Refresh");
        bool autoShuffle = s.value("game/autoShuffle", true).toBool();

        if (autoShuffle) {
            m_logic->shuffleUntilSolvable();
            emit noSolutionAutoShuffled();
        }
        else {
            emit noSolutionManualShuffleNeeded();
        }
    }
}
