#include "classicwindow.h"
#include "thememanager.h"

ClassicWindow::ClassicWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("经典模式");
    setModeTitle("经典模式");

    setupCommonModeButtons(QColor(0, 145, 255), false, QSize(140, 44));

    initGame();
    startGame();
}

void ClassicWindow::initGame()
{
    rows = 9;
    cols = 14;
    logic->setMaxType(ThemeManager::instance().tileTypeCount());
    logic->initMap(rows, cols);
}

void ClassicWindow::startGame()
{
    createBoard();
    refreshBoard();
}

QString ClassicWindow::helpTitle() const
{
    return "经典模式帮助";
}

QString ClassicWindow::helpText() const
{
    return
        "经典模式说明：\n"
        "1. 棋盘大小为 9×14。\n"
        "2. 连线最多 2 个拐点，可绕外圈。\n"
        "3. 点击“提示”可高亮一对可消除图案。\n"
        "4. 点击“重排”可打乱当前图案。";
}