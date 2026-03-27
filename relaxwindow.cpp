#include "relaxwindow.h"
#include "helpdialog.h"
#include <QMessageBox>
#include <QPushButton>

RelaxWindow::RelaxWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("休闲模式");

    QString btnStyle = R"(
    QPushButton {
        font-size: 14px;
        color: white;
        background-color: rgba(46, 204, 113, 0.88);   /* 主绿 */
        border: none;
        border-radius: 8px;
        padding: 8px 20px;
        min-width: 120px;
    }
    QPushButton:hover {
        background-color: rgba(72, 224, 141, 1);      /* 亮绿 */
    }
    QPushButton:pressed {
        background-color: rgba(39, 174, 96, 1);       /* 深绿 */
    }
)";

    btnBack = new QPushButton("返回主菜单");
    btnReset = new QPushButton("重排图案");
    btnHint = new QPushButton("提示");
    btnHelp = new QPushButton("帮助");

    btnBack->setStyleSheet(btnStyle);
    btnReset->setStyleSheet(btnStyle);
    btnHint->setStyleSheet(btnStyle);
	btnHelp->setStyleSheet(btnStyle);

    // 只使用父类已有 btnLayout，不要重建布局
    btnLayout->addWidget(btnBack);
    btnLayout->addWidget(btnReset);
    btnLayout->addWidget(btnHint);
	btnLayout->addWidget(btnHelp);

    connect(btnBack, &QPushButton::clicked, this, &RelaxWindow::backToMain);
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
        dlg.setHelpTitle("休闲模式帮助");
        dlg.setHelpText(
            "休闲模式说明：\n"
            "1. 节奏轻松，无倒计时压力。\n"
            "2. 连线规则同样为最多 2 拐点。\n"
            "3. 可使用提示与重排辅助通关。"
        );
        dlg.exec();
        });

    initGame();
    startGame();
}

void RelaxWindow::initGame()
{
    rows = 8;
    cols = 8;
    logic->initMap(rows, cols);
}

void RelaxWindow::startGame()
{
    createBoard();
    refreshBoard();
}