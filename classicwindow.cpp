#include "classicwindow.h"
#include "helpdialog.h"
#include <QPushButton>
#include <QMessageBox>

ClassicWindow::ClassicWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("经典模式");

    QString btnStyle = R"(
    QPushButton {
        font-size: 14px;
        color: white;
        background-color: rgba(52, 152, 219, 0.88);   /* 主蓝 */
        border: none;
        border-radius: 8px;
        padding: 8px 20px;
        min-width: 120px;
    }
    QPushButton:hover {
        background-color: rgba(64, 172, 245, 1);      /* 亮蓝 */
    }
    QPushButton:pressed {
        background-color: rgba(41, 128, 185, 1);      /* 深蓝 */
    }
)";

    btnBack = new QPushButton("返回菜单");
    btnReset = new QPushButton("重排");
    btnHint = new QPushButton("提示");
    btnHelp = new QPushButton("帮助");

    btnBack->setStyleSheet(btnStyle);
    btnReset->setStyleSheet(btnStyle);
    btnHint->setStyleSheet(btnStyle);
    btnHelp->setStyleSheet(btnStyle);

    btnLayout->addWidget(btnBack);
    btnLayout->addWidget(btnReset);
    btnLayout->addWidget(btnHint);
    btnLayout->addWidget(btnHelp);


    connect(btnBack, &QPushButton::clicked, this, &ClassicWindow::backToMain);
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
        dlg.setHelpTitle("经典模式帮助");
        dlg.setHelpText(
            "经典模式说明：\n"
            "1. 棋盘大小为 9×14。\n"
            "2. 连线最多 2 个拐点，可绕外圈。\n"
            "3. 点击“提示”可高亮一对可消除图案。\n"
            "4. 点击“重排”可打乱当前图案。"
        );
        dlg.exec();
        });

    initGame();
    startGame();
}

void ClassicWindow::initGame() {
    rows = 9;
    cols = 14;
    logic->initMap(rows, cols);
}

void ClassicWindow::startGame() {
    createBoard();
    refreshBoard();
}