#include "timedwindow.h"
#include "helpdialog.h"
#include "resultdialog.h"
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

TimedWindow::TimedWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("计时模式");

    // 先初始化计时相关状态（关键修复）
    totalSec = 600;
    leftSec = totalSec;
    removedPairs = 0;
    gameFinished = false;
    isPaused = false;

    QString btnStyle = R"(
    QPushButton {
        font-size: 14px;
        color: white;
        background-color: rgba(230, 126, 34, 0.90);   /* 主橙 */
        border: none;
        border-radius: 8px;
        padding: 8px 20px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: rgba(243, 156, 18, 1);      /* 亮橙 */
    }
    QPushButton:pressed {
        background-color: rgba(211, 84, 0, 1);        /* 深橙 */
    }
)";

    btnBack = new QPushButton("返回菜单");
    btnPause = new QPushButton("暂停");
    btnReset = new QPushButton("重排");
    btnHint = new QPushButton("提示");
    btnHelp = new QPushButton("帮助");

    btnBack->setStyleSheet(btnStyle);
    btnPause->setStyleSheet(btnStyle);
    btnReset->setStyleSheet(btnStyle);
    btnHint->setStyleSheet(btnStyle);
    btnHelp->setStyleSheet(btnStyle);

    btnLayout->addWidget(btnBack);
    btnLayout->addWidget(btnPause);
    btnLayout->addWidget(btnReset);
    btnLayout->addWidget(btnHint);
    btnLayout->addWidget(btnHelp);
    btnLayout->addSpacing(20);

    labTime = new QLabel(this);
    labTime->setMinimumWidth(100);   // 原来 180 太窄
    labTime->setAlignment(Qt::AlignCenter);
    labTime->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    labTime->setStyleSheet(
        "QLabel {"
        " color: white;"
        " font-size: 14px;"
        " font-weight: bold;"
        " background-color: rgba(0, 0, 0, 90);"
        " border-radius: 8px;"
        " padding: 6px 12px;"
        "}"
    );
    labTime->setText(QString("剩余 %1 秒").arg(leftSec));
    btnLayout->addWidget(labTime);

    // 关键：一定加到按钮布局
    btnLayout->addWidget(timeBar);

    pauseMask = new QWidget(boardWidget);
    pauseMask->setStyleSheet("background-color: rgba(0,0,0,140);");
    pauseMask->hide();

    pauseLabel = new QLabel("已暂停", pauseMask);
    pauseLabel->setAlignment(Qt::AlignCenter);
    pauseLabel->setStyleSheet("color:white;font-size:42px;font-weight:bold;");

    connect(btnBack, &QPushButton::clicked, this, [=]() {
        if (timer) timer->stop();
        backToMain();
        });

    connect(btnPause, &QPushButton::clicked, this, [=]() {
        setPaused(!isPaused);
        });

    connect(btnReset, &QPushButton::clicked, this, [=]() {
        logic->shuffleMap();
        refreshBoard();
        QMessageBox::information(this, "提示", "图案已重排！", QMessageBox::Ok);
        });

    connect(btnHint, &QPushButton::clicked, this, [=]() {
        showHint();
        });

    connect(btnHelp, &QPushButton::clicked, this, [=]() {
        HelpDialog dlg(this);
        dlg.setHelpTitle("计时模式帮助");
        dlg.setHelpText(
            "计时模式说明：\n"
            "1. 在倒计时结束前尽可能完成消除。\n"
            "2. 连线最多 2 拐点，可绕棋盘外侧。\n"
            "3. 合理使用“提示”和“重排”提高效率。"
        );
        dlg.exec();
        });

    // 只创建一次 timer（关键修复）
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimedWindow::onTimerTimeout);

    initGame();
    startGame();
}

void TimedWindow::initGame() {
    rows = 8;
    cols = 12;
    logic->initMap(rows, cols);

    leftSec = totalSec;
    removedPairs = 0;
    gameFinished = false;

    if (labTime) {
        labTime->setText(QString("剩余 %1 秒").arg(leftSec));
    }
}

void TimedWindow::startGame() {
    createBoard();
    refreshBoard();
    if (timer) timer->start(1000);
}

void TimedWindow::onTimerTimeout()
{
    if (gameFinished || isPaused) return;
    if (leftSec <= 0) return;

    --leftSec;
    if (labTime) {
        labTime->setText(QString("剩余 %1 秒").arg(leftSec));
    }
}

void TimedWindow::onPairRemoved()
{
    if (gameFinished) return;
    removedPairs++;
    if (logic->isMapEmpty()) {
        if (timer) timer->stop();
        finishTimedGame(true);
    }
}

void TimedWindow::finishTimedGame(bool win)
{
    if (gameFinished) return;   // 关键：只执行一次
    gameFinished = true;

    if (timer) timer->stop();
    inputEnabled = false;

    ResultDialog dlg(this);
    dlg.setResult(win, totalSec, leftSec, removedPairs);
    dlg.exec();
    backToMain();
}

void TimedWindow::onGameCleared()
{
    finishTimedGame(true); // 直接进入计时模式结算页
}

void TimedWindow::setPaused(bool paused)
{
    isPaused = paused;
    inputEnabled = !paused;

    if (paused) {
        timer->stop();
        updatePauseMaskGeometry();
        pauseMask->show();
        pauseMask->raise();
        btnPause->setText("继续");
    }
    else {
        pauseMask->hide();
        timer->start(1000);
        btnPause->setText("暂停");
    }
}

void TimedWindow::updatePauseMaskGeometry()
{
    if (!pauseMask || !boardWidget) return;
    pauseMask->setGeometry(boardWidget->rect()); 
    pauseLabel->setGeometry(pauseMask->rect());
}