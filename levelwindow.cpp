#include "levelwindow.h"
#include "thememanager.h"
#include "uistyle.h"
#include "glassmessagebox.h"

#include <QTimer>
#include <QSettings>

// 关卡模式窗口，包含三关不同大小的棋盘，通关后自动进入下一关并解锁下一关按钮
LevelWindow::LevelWindow(int startLevel, QWidget* parent)
    : TimedWindow(parent, false),
    m_level(startLevel),
    m_switchingLevel(false)
{
    if (m_level < 1) m_level = 1;
    if (m_level > 3) m_level = 3;

    setWindowTitle("关卡模式");
    setModeTitle(QString("关卡模式 - 第%1关").arg(m_level));

    const QString levelBtnStyle = glassButtonStyle(QColor(219, 52, 242));
    if (btnBack)  btnBack->setStyleSheet(levelBtnStyle);
    if (btnPause) btnPause->setStyleSheet(levelBtnStyle);
    if (btnReset) btnReset->setStyleSheet(levelBtnStyle);
    if (btnHint)  btnHint->setStyleSheet(levelBtnStyle);
    if (btnHelp)  btnHelp->setStyleSheet(levelBtnStyle);

    initGame();
    startGame();
}

// 根据当前关卡设置棋盘大小
void LevelWindow::applyLevelSize()
{
    if (m_level == 1) {
        rows = 8;  cols = 7;
    }
    else if (m_level == 2) {
        rows = 9;  cols = 10;
    }
    else {
        rows = 10; cols = 13;
    }
}

// 初始化游戏状态，设置棋盘大小，重置计时器和相关变量
void LevelWindow::initGame()
{
    applyLevelSize();

    logic->setMaxType(ThemeManager::instance().tileTypeCount());
    logic->initMap(rows, cols);

    leftSec = totalSec;
    removedPairs = 0;
    gameFinished = false;
    isPaused = false;
    inputEnabled = true;

    if (labTime) {
        labTime->setText(QString("第%1关  剩余 %2 / 总计 %3")
            .arg(m_level)
            .arg(formatTime(leftSec))
            .arg(formatTime(totalSec)));
    }

    if (btnPause) btnPause->setText("暂停");
    if (pauseMask) pauseMask->hide();

    m_switchingLevel = false;
}

// 关卡通关处理，解锁下一关按钮，自动进入下一关，显示过关提示
void LevelWindow::onGameCleared()
{
    if (m_switchingLevel) return;
    m_switchingLevel = true;

    {
        QSettings s("YourCompany", "LLK_Refresh");
        int unlockedMax = s.value("level/unlockedMax", 1).toInt();
        if (m_level >= unlockedMax && m_level < 3) {
            s.setValue("level/unlockedMax", m_level + 1);
        }
    }

    if (m_level >= 3) {
        finishTimedGame(true);
        return;
    }

    inputEnabled = false;
    isPaused = true;
    gameFinished = true;
    if (timer) timer->stop();

    ++m_level;

    QTimer::singleShot(0, this, [this]() {
        setModeTitle(QString("关卡模式 - 第%1关").arg(m_level));

        initGame();
        createBoard();
        refreshBoard();

        if (timer) timer->start(1000);

        GlassMessageBox::information(this, "过关", QString("恭喜通过，进入第%1关！").arg(m_level));
        });
}

QString LevelWindow::helpTitle() const
{
    return "关卡模式帮助";
}

QString LevelWindow::helpText() const
{
    return
        "关卡模式说明：\n"
        "1. 共三关，棋盘依次为 8×7、9×10、10×13。\n"
        "2. 每关倒计时使用设置中的计时模式时长。\n"
        "3. 通关当前关卡后自动进入下一关，并解锁下一关按钮。\n"
        "4. 任意关卡时间耗尽则挑战失败。";
}