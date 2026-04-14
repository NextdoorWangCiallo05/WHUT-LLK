#include "customwindow.h"
#include "thememanager.h"
#include "uistyle.h"
#include "glassmessagebox.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

/// 这个文件实现了自定义模式窗口，允许玩家输入棋盘的行数和列数来创建一个新的游戏
// 自定义模式窗口实现
CustomWindow::CustomWindow(QWidget* parent) : GameWindow(parent)
{
    setWindowTitle("自定义模式");
    setModeTitle("自定义模式");
    setupCommonModeButtons(QColor(255, 45, 85), false, QSize(140, 44));

    int r = 8, c = 8;
    if (!askBoardSize(r, c)) {
        m_valid = false;
        return;
    }

    m_customRows = r;
    m_customCols = c;
    initGame();
    startGame();
    m_valid = true;
}

// 初始化游戏数据，设置棋盘大小和图案类型数量
bool CustomWindow::askBoardSize(int& r, int& c)
{
    QDialog dlg(this);
    dlg.setFixedSize(460, 300);
    dlg.setModal(true);

    auto applyDialogBg = [&dlg]() {
        const QString path = ThemeManager::instance().backgroundPathDialog();
        dlg.setStyleSheet(QString(
            "QDialog { border-image: url(%1) 0 0 0 0 stretch stretch; }"
        ).arg(path));
        };
    applyDialogBg();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, &dlg, applyDialogBg);

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    setupGlassDialogTopBar(&dlg, root, "自定义棋盘", "自定义棋盘");

    QLabel* title = new QLabel("请输入棋盘大小（行×列需为偶数）", &dlg);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(glassTitleLabelStyle());

    QWidget* card = new QWidget(&dlg);
    card->setStyleSheet(glassCardStyle());
    auto* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(14, 14, 14, 14);
    cardLay->setSpacing(10);

    QHBoxLayout* rowLay = new QHBoxLayout();
    QHBoxLayout* colLay = new QHBoxLayout();

    QLabel* labR = new QLabel("行数：", card);
    QLabel* labC = new QLabel("列数：", card);

    labR->setFixedWidth(64);
    labC->setFixedWidth(64);
    labR->setAlignment(Qt::AlignCenter);
    labC->setAlignment(Qt::AlignCenter);
    labR->setStyleSheet(R"(
        QLabel{
            color: rgba(0,0,0,0.95);
            font-size: 15px;
            font-weight: 700;
            background: transparent;
            border: none;
            padding: 0;
        }
    )");
    labC->setStyleSheet(labR->styleSheet());

    QSpinBox* spinR = new QSpinBox(card);
    QSpinBox* spinC = new QSpinBox(card);
    spinR->setRange(2, 30);
    spinC->setRange(2, 30);
    spinR->setValue(r);
    spinC->setValue(c);
    spinR->setAlignment(Qt::AlignCenter);
    spinC->setAlignment(Qt::AlignCenter);

    spinR->setKeyboardTracking(false);
    spinC->setKeyboardTracking(false);

    spinR->setStyleSheet(R"(
        QSpinBox {
            color: rgba(0,0,0,0.95);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.18);
            border: 1px solid rgba(255,255,255,0.22);
            border-radius: 10px;
            padding: 6px 10px;
            min-height: 18px;
        }
        QSpinBox:hover { border: 1px solid rgba(30,110,244,0.55); }
        QSpinBox:focus { border: 1px solid rgba(30,110,244,0.85); }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 18px; border: none; background: transparent;
        }
    )");
    spinC->setStyleSheet(spinR->styleSheet());

    rowLay->addWidget(labR);
    rowLay->addWidget(spinR, 1);

    colLay->addWidget(labC);
    colLay->addWidget(spinC, 1);

    cardLay->addLayout(rowLay);
    cardLay->addLayout(colLay);

    QPushButton* okBtn = new QPushButton("确定", &dlg);
    QPushButton* cancelBtn = new QPushButton("取消", &dlg);
    okBtn->setFixedHeight(40);
    cancelBtn->setFixedHeight(40);
    okBtn->setMinimumWidth(120);
    cancelBtn->setMinimumWidth(120);
    okBtn->setStyleSheet(glassButtonStyle(QColor(255, 45, 85)));
    cancelBtn->setStyleSheet(glassButtonStyle(QColor(120, 130, 145)));

    applyGlassShadow(okBtn, QColor(0, 0, 0, 70));
    applyGlassShadow(cancelBtn, QColor(0, 0, 0, 70));

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->addStretch();
    btnLay->addWidget(okBtn);
    btnLay->addSpacing(12);
    btnLay->addWidget(cancelBtn);
    btnLay->addStretch();

    root->addWidget(title);
    root->addWidget(card, 1);
    root->addLayout(btnLay);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, [&]() {
        const int rr = spinR->value();
        const int cc = spinC->value();

        if ((rr * cc) % 2 != 0) {
            GlassMessageBox::warning(&dlg, "无效布局", "行×列必须为偶数，请重新设置。");
            return;
        }

        r = rr;
        c = cc;
        dlg.accept();
        });

    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    return dlg.exec() == QDialog::Accepted;
}

// 初始化游戏数据，设置棋盘大小和图案类型数量
void CustomWindow::initGame()
{
    m_control->setMaxType(ThemeManager::instance().tileTypeCount());
    m_control->initMap(m_customRows, m_customCols);
}

// 开始游戏，创建棋盘并刷新界面
void CustomWindow::startGame()
{
    createBoard();
    refreshBoard();
}

// 返回帮助信息标题
QString CustomWindow::helpTitle() const
{
    return "自定义模式帮助";
}

// 返回帮助信息内容
QString CustomWindow::helpText() const
{
    return
        "自定义模式说明：\n"
        "1. 进入模式时可自定义棋盘行列。\n"
        "2. 行×列必须为偶数，才能保证图案成对生成。\n"
        "3. 规则与基础模式一致，无倒计时。";
}