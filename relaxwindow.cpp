#include "relaxwindow.h"
#include "thememanager.h"

/// 休闲模式窗口实现
RelaxWindow::RelaxWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("休闲模式");
    setModeTitle("休闲模式");

    setupCommonModeButtons(QColor(48, 209, 88), false, QSize(140, 44));

    initGame();
    startGame();
}

/// 初始化游戏数据，设置地图大小和类型数量
void RelaxWindow::initGame()
{
    rows = 8;
    cols = 8;
    logic->setMaxType(ThemeManager::instance().tileTypeCount());
    logic->initMap(rows, cols);
}

/// 开始游戏，创建地图并刷新界面
void RelaxWindow::startGame()
{
    createBoard();
    refreshBoard();
}

QString RelaxWindow::helpTitle() const
{
    return "休闲模式帮助";
}

QString RelaxWindow::helpText() const
{
    return
        "休闲模式说明：\n"
        "1. 节奏轻松，无倒计时压力。\n"
        "2. 连线规则同样为最多 2 拐点。\n"
        "3. 可使用提示与重排辅助通关。";
}