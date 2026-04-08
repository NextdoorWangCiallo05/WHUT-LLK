#include "timedwindow.h"
#include "helpdialog.h"
#include "resultdialog.h"
#include "uistyle.h"
#include "thememanager.h"
#include "rankmanager.h"

#include <QInputDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <typeinfo>

QString TimedWindow::formatTime(int sec) const
{
    if (sec < 0) sec = 0;
    int min = sec / 60;
    int s = sec % 60;
    return QString("%1:%2")
        .arg(min, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

TimedWindow::TimedWindow(QWidget* parent, bool autoStart) : GameWindow(parent)
{
    setWindowTitle("计时模式");
    setModeTitle("计时模式");

    QSettings s("YourCompany", "LLK_Refresh");
    totalSec = s.value("game/timedModeTotalSec", 600).toInt();
    if (totalSec < 60) totalSec = 60;
    if (totalSec > 3600) totalSec = 3600;

    leftSec = totalSec;
    removedPairs = 0;
    gameFinished = false;
    isPaused = false;

    setupCommonModeButtons(QColor(255, 146, 48), true, QSize(115, 42));

    labTime = new QLabel(this);
    labTime->setMinimumWidth(220);
    labTime->setFixedHeight(42);
    labTime->setAlignment(Qt::AlignCenter);
    labTime->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    labTime->setStyleSheet(glassTimeLabelStyle());
    labTime->setText(QString("剩余 %1 / 总计 %2")
        .arg(formatTime(leftSec))
        .arg(formatTime(totalSec)));

    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(1, labTime, 0, Qt::AlignCenter);
    }

    btnLayout->addSpacing(20);

    pauseMask = new QWidget(boardWidget);
    pauseMask->setStyleSheet(R"(
    QWidget {
        background-color: rgba(160, 160, 160, 0.20);
        border: 1px solid rgba(255, 255, 255, 0.30);
        border-radius: 18px; 
    }
)");
    pauseMask->hide();

    pauseLabel = new QLabel("已暂停", pauseMask);
    pauseLabel->setAlignment(Qt::AlignCenter);
    pauseLabel->setStyleSheet(R"(
    QLabel{
        color: rgba(255,255,255,0.98);
        font-size: 46px;
        font-weight: 800;
        background: transparent;
        border: none; 
    }
)");

    pauseSubLabel = new QLabel("点击「继续」恢复游戏", pauseMask);
    pauseSubLabel->setAlignment(Qt::AlignCenter);
    pauseSubLabel->setStyleSheet(R"(
    QLabel{
        color: rgba(255,255,255,0.92);
        font-size: 16px;
        font-weight: 600;
        background: transparent;
        border: none;
    }
)");

    disconnect(btnBack, nullptr, nullptr, nullptr);
    connect(btnBack, &QPushButton::clicked, this, [=]() {
        if (timer) timer->stop();
        backToMain();
        });

    connect(btnPause, &QPushButton::clicked, this, [=]() {
        setPaused(!isPaused);
        });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimedWindow::onTimerTimeout);

    if (autoStart) {
        initGame();
        startGame();
    }
}

void TimedWindow::initGame()
{
    rows = 8;
    cols = 12;
    logic->setMaxType(ThemeManager::instance().tileTypeCount());
    logic->initMap(rows, cols);

    leftSec = totalSec;
    removedPairs = 0;
    gameFinished = false;
    isPaused = false;
    inputEnabled = true;

    if (labTime) {
        labTime->setText(QString("剩余 %1 / 总计 %2")
            .arg(formatTime(leftSec))
            .arg(formatTime(totalSec)));
    }

    if (btnPause) btnPause->setText("暂停");
    if (pauseMask) pauseMask->hide();
}

void TimedWindow::startGame()
{
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
        labTime->setText(QString("剩余 %1 / 总计 %2")
            .arg(formatTime(leftSec))
            .arg(formatTime(totalSec)));
    }

    if (leftSec <= 0) {
        finishTimedGame(false);
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
    if (gameFinished) return;
    gameFinished = true;

    if (timer) timer->stop();
    inputEnabled = false;

    // 仅“计时模式本体”写入排行榜；关卡模式(LevelWindow)不写入
    if (win && typeid(*this) == typeid(TimedWindow)) {
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
            RankManager::instance().addRecord(nickname, leftSec);
        }
    }

    ResultDialog dlg(this);
    dlg.setResult(win, totalSec, leftSec, removedPairs);
    dlg.exec();
    backToMain();
}

void TimedWindow::onGameCleared()
{
    finishTimedGame(true);
}

void TimedWindow::setPaused(bool paused)
{
    isPaused = paused;
    inputEnabled = !paused;

    if (paused) {
        if (timer) timer->stop();
        updatePauseMaskGeometry();
        if (pauseMask) {
            pauseMask->show();
            pauseMask->raise();
        }
        if (btnPause) btnPause->setText("继续");
    }
    else {
        if (pauseMask) pauseMask->hide();
        if (timer && !gameFinished) timer->start(1000);
        if (btnPause) btnPause->setText("暂停");
    }
}

void TimedWindow::updatePauseMaskGeometry()
{
    if (!pauseMask || !boardWidget) return;
    pauseMask->setGeometry(boardWidget->rect());

    const QRect r = pauseMask->rect();
    if (pauseLabel)    pauseLabel->setGeometry(0, r.height() / 2 - 58, r.width(), 58);
    if (pauseSubLabel) pauseSubLabel->setGeometry(0, r.height() / 2 + 6, r.width(), 28);
}

void TimedWindow::resizeEvent(QResizeEvent* event)
{
    GameWindow::resizeEvent(event);
    updatePauseMaskGeometry();
}

QString TimedWindow::helpTitle() const
{
    return "计时模式帮助";
}

QString TimedWindow::helpText() const
{
    return
        "计时模式说明：\n"
        "1. 在倒计时结束前尽可能完成消除。\n"
        "2. 连线最多 2 拐点，可绕棋盘外侧。\n"
        "3. 合理使用“提示”和“重排”提高效率。";
}