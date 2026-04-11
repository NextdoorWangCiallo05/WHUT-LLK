#include "classicwindow.h"
#include "thememanager.h"
#include "classicrankmanager.h"
#include "glassmessagebox.h"

#include <QInputDialog>
#include <QLineEdit>

/// 这个文件实现了经典模式窗口，玩家需要在一个固定大小的棋盘上完成消除任务，记录用时并保存成绩
// 经典模式窗口类，继承自 GameWindow
ClassicWindow::ClassicWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("经典模式");
    setModeTitle("经典模式");

    setupCommonModeButtons(QColor(0, 145, 255), false, QSize(140, 44));

    initGame();
    startGame();
}

// 初始化游戏数据，设置棋盘大小和图案类型数量
void ClassicWindow::initGame()
{
    rows = 9;
    cols = 14;
    logic->setMaxType(ThemeManager::instance().tileTypeCount());
    logic->initMap(rows, cols);
    m_finished = false;
}

// 开始游戏，创建棋盘并刷新界面，同时启动计时器
void ClassicWindow::startGame()
{
    createBoard();
    refreshBoard();
    m_elapsed.start();
}

// 游戏完成后的处理，记录成绩并显示结算信息
void ClassicWindow::onGameCleared()
{
    if (m_finished) return;
    m_finished = true;

    int usedSec = int(m_elapsed.elapsed() / 1000);
    if (usedSec < 0) usedSec = 0;

    bool ok = false;
    QString nickname = QInputDialog::getText(
        this,
        "记录成绩",
        "请输入昵称：",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (ok) {
        ClassicRankManager::instance().addRecord(nickname, usedSec);
    }

    int m = usedSec / 60, s = usedSec % 60;
    GlassMessageBox::information(this, "结算",
        QString("消除成功！\n本局用时：%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));

    backToMain();
}

// 判断游戏是否结束，条件是玩家完成消除或者棋盘上没有可消除的图案
bool ClassicWindow::isGameFinished() const
{
    return m_finished || (logic && logic->isMapEmpty());
}

// 返回帮助信息的标题
QString ClassicWindow::helpTitle() const
{
    return "经典模式帮助";
}

// 返回帮助信息的内容，包含游戏规则和操作说明
QString ClassicWindow::helpText() const
{
    return
        "经典模式说明：\n"
        "1. 棋盘大小为 9×14。\n"
        "2. 连线最多 2 个拐点，可绕外圈。\n"
        "3. 点击“提示”可高亮一对可消除图案。\n"
        "4. 点击“重排”可打乱当前图案。";
}