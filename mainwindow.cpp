#include "mainwindow.h"
#include "classicwindow.h"
#include "relaxwindow.h"
#include "timedwindow.h"
#include "helpdialog.h"
#include "settingsdialog.h"
#include "audiomanager.h"

#include <QApplication>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("连连看 - 主菜单");
    setFixedSize(800, 600);

    QPixmap bgPix(":/images/bg1.png");
    QPalette pal;
    pal.setBrush(QPalette::Window, QBrush(bgPix.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    setPalette(pal);
    setAutoFillBackground(true);

    initUI();
}

MainWindow::~MainWindow() {}

void MainWindow::initUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 根布局：上（音乐） 中（左侧主按钮） 下（功能按钮）
    QVBoxLayout* root = new QVBoxLayout(centralWidget);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(0);

    // ---------------- 顶部栏 ----------------
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addStretch();

    QPushButton* btnMusic = new QPushButton(this);
    btnMusic->setFixedSize(110, 44);
    btnMusic->setStyleSheet(R"(
        QPushButton{
            font-size: 20px;
            color: #2c3e50;
            background-color: rgba(255,255,255,0.78);
            border: 1px solid rgba(60,60,60,0.25);
            border-radius: 4px;
        }
        QPushButton:hover{ background-color: rgba(255,255,255,0.95); }
        QPushButton:pressed{ background-color: rgba(235,235,235,0.95); }
    )");
    btnMusic->setText(AudioManager::instance().isMuted() ? "音乐：关" : "音乐：开");
    topBar->addWidget(btnMusic);

    root->addLayout(topBar);

    // ---------------- 中间区域 ----------------
    QHBoxLayout* centerArea = new QHBoxLayout();
    centerArea->setContentsMargins(0, 10, 0, 10);

    QVBoxLayout* leftModes = new QVBoxLayout();
    leftModes->setSpacing(22);
    leftModes->addStretch();

    btnClassic = new QPushButton("经典模式");
    btnRelax = new QPushButton("休闲模式");
    btnTimed = new QPushButton("计时模式");

    btnClassic->setFixedSize(128, 64);
    btnRelax->setFixedSize(128, 64);
    btnTimed->setFixedSize(128, 64);

    // 三模式对应颜色：经典蓝、休闲绿、计时橙
    QString classicStyle = R"(
        QPushButton{
            font-size: 20px;
            color: white;
            background-color: rgba(52,152,219,0.90);
            border: 1px solid rgba(255,255,255,0.45);
            border-radius: 4px;
        }
        QPushButton:hover{ background-color: rgba(64,172,245,1); }
        QPushButton:pressed{ background-color: rgba(41,128,185,1); }
    )";

    QString relaxStyle = R"(
        QPushButton{
            font-size: 20px;
            color: white;
            background-color: rgba(46,204,113,0.90);
            border: 1px solid rgba(255,255,255,0.45);
            border-radius: 4px;
        }
        QPushButton:hover{ background-color: rgba(72,224,141,1); }
        QPushButton:pressed{ background-color: rgba(39,174,96,1); }
    )";

    QString timedStyle = R"(
        QPushButton{
            font-size: 20px;
            color: white;
            background-color: rgba(230,126,34,0.92);
            border: 1px solid rgba(255,255,255,0.45);
            border-radius: 4px;
        }
        QPushButton:hover{ background-color: rgba(243,156,18,1); }
        QPushButton:pressed{ background-color: rgba(211,84,0,1); }
    )";

    btnClassic->setStyleSheet(classicStyle);
    btnRelax->setStyleSheet(relaxStyle);
    btnTimed->setStyleSheet(timedStyle);

    leftModes->addWidget(btnClassic, 0, Qt::AlignLeft);
    leftModes->addWidget(btnRelax, 0, Qt::AlignLeft);
    leftModes->addWidget(btnTimed, 0, Qt::AlignLeft);
    leftModes->addStretch();

    centerArea->addLayout(leftModes);
    centerArea->addStretch();

    root->addLayout(centerArea, 1);

    // ---------------- 底部栏 ----------------
    QHBoxLayout* bottomBar = new QHBoxLayout();
    bottomBar->addStretch();

    QPushButton* btnSetting = new QPushButton("设置");
    QPushButton* btnHelp = new QPushButton("帮助");
    QPushButton* btnRank = new QPushButton("排行榜");
    QPushButton* btnAbout = new QPushButton("关于");

    auto setSmallBtnStyle = [](QPushButton* b) {
        b->setFixedSize(86, 46);
        b->setStyleSheet(R"(
            QPushButton{
                font-size: 20px;
                color: #2c3e50;
                background-color: rgba(255,255,255,0.78);
                border: 1px solid rgba(60,60,60,0.25);
                border-radius: 4px;
            }
            QPushButton:hover{ background-color: rgba(255,255,255,0.95); }
            QPushButton:pressed{ background-color: rgba(235,235,235,0.95); }
        )");
        };

    setSmallBtnStyle(btnSetting);
    setSmallBtnStyle(btnHelp);
    setSmallBtnStyle(btnRank);
    setSmallBtnStyle(btnAbout);

    bottomBar->addWidget(btnSetting);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnHelp);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnRank);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnAbout);

    root->addLayout(bottomBar);

    // 保留成员兼容（你原头文件有 btnExit）
    btnExit = btnAbout;

    // ---------------- 事件连接 ----------------
    connect(btnClassic, &QPushButton::clicked, this, &MainWindow::openClassicMode);
    connect(btnRelax, &QPushButton::clicked, this, &MainWindow::openRelaxMode);
    connect(btnTimed, &QPushButton::clicked, this, &MainWindow::openTimedMode);

    connect(btnMusic, &QPushButton::clicked, this, [=]() {
        bool nowMuted = AudioManager::instance().isMuted();
        AudioManager::instance().setMuted(!nowMuted);
        btnMusic->setText(AudioManager::instance().isMuted() ? "音乐：关" : "音乐：开");
        });

    connect(btnSetting, &QPushButton::clicked, this, [=]() {
        SettingsDialog dlg(this);
        dlg.exec();
        btnMusic->setText(AudioManager::instance().isMuted() ? "音乐：关" : "音乐：开");
        });

    connect(btnHelp, &QPushButton::clicked, this, [=]() {
        HelpDialog dlg(this);
        dlg.setHelpTitle("连连看游戏帮助");
        dlg.setHelpText(
            "【基本规则】\n"
            "1. 点击两张相同图案的牌进行消除。\n"
            "2. 连线最多允许2个拐点，可绕棋盘外侧。\n"
            "3. 使用“提示”可高亮可消除对子，使用“重排”可打乱牌面。\n\n"
            "【模式说明】\n"
            "- 经典模式：标准棋盘挑战。\n"
            "- 休闲模式：更轻松的节奏。\n"
            "- 计时模式：在倒计时内完成消除。"
        );
        dlg.exec();
        });

    connect(btnRank, &QPushButton::clicked, this, [=]() {
        QMessageBox::information(this, "排行榜", "排行榜功能开发中");
        });

    connect(btnAbout, &QPushButton::clicked, this, [=]() {
        QMessageBox::information(this, "关于", "连连看_QtRefresh\n版本：1.0.0.0\n使用QT6重构");
        });
}

void MainWindow::openClassicMode() {
    ClassicWindow* w = new ClassicWindow();
    w->move(this->pos());
    w->show();
    close();
}

void MainWindow::openRelaxMode() {
    RelaxWindow* w = new RelaxWindow();
    w->move(this->pos());
    w->show();
    close();
}

void MainWindow::openTimedMode() {
    TimedWindow* w = new TimedWindow();
    w->move(this->pos());
    w->show();
    close();
}