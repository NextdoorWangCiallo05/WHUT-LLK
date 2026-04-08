#include "mainwindow.h"
#include "classicwindow.h"
#include "relaxwindow.h"
#include "timedwindow.h"
#include "levelwindow.h"
#include "levelselectdialog.h"
#include "customwindow.h"
#include "helpdialog.h"
#include "rankdialog.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "audiomanager.h"
#include "uistyle.h"
#include "windowround.h"
#include "thememanager.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPalette>
#include <QPoint>
#include <QPixmap>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QBrush>
#include <QColor>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("连连看 - 主菜单");
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    m_bgPix = QPixmap(ThemeManager::instance().backgroundPathMain());
    m_bgScaled = QPixmap();
    m_bgScaledSize = QSize();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        m_bgPix = QPixmap(ThemeManager::instance().backgroundPathMain());
        m_bgScaled = QPixmap();
        m_bgScaledSize = QSize();
        update();
        });

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    initUI();
}

MainWindow::~MainWindow() {}

void MainWindow::initUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* root = new QVBoxLayout(centralWidget);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(0);

    QWidget* topBar = new QWidget(this);
    topBar->setFixedHeight(44);
    topBar->setStyleSheet(R"(
    QWidget {
        background-color: rgba(255, 255, 255, 0.20);
        border: 1px solid rgba(255, 255, 255, 0.28);
        border-radius: 16px;
    }
)");

    auto* topShadow = new QGraphicsDropShadowEffect(topBar);
    topShadow->setBlurRadius(22);
    topShadow->setOffset(0, 6);
    topShadow->setColor(QColor(0, 0, 0, 45));
    topBar->setGraphicsEffect(topShadow);

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(12, 6, 12, 6);
    topLayout->setSpacing(8);

    QLabel* appIcon = new QLabel(topBar);
    appIcon->setPixmap(QPixmap(":/images/app.ico").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    appIcon->setStyleSheet("background: transparent; border: none; padding: 0; margin: 0;");
    appIcon->setFixedSize(18, 18);

    QLabel* titleLabel = new QLabel("连连看 - 主菜单", topBar);
    titleLabel->setStyleSheet("color: rgba(0, 0, 0, 0.82); font-size: 15px; font-weight: 600; background: transparent; border: none;");

    QPushButton* btnMin = new QPushButton("–", topBar);
    QPushButton* btnClose = new QPushButton("×", topBar);

    btnMin->setFixedSize(28, 24);
    btnClose->setFixedSize(28, 24);

    btnMin->setStyleSheet(R"(
    QPushButton { background: transparent; border: none; font-size: 18px; color: rgba(0,0,0,0.75); border-radius: 10px; }
    QPushButton:hover { background-color: rgba(255,255,255,0.35); }
)");
    btnClose->setStyleSheet(R"(
    QPushButton { background: transparent; border: none; font-size: 18px; color: rgba(0,0,0,0.75); border-radius: 10px; }
    QPushButton:hover { background-color: rgba(255,80,80,0.92); color: white; }
)");

    topLayout->addWidget(appIcon);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnMin);
    topLayout->addWidget(btnClose);

    connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);

    root->addWidget(topBar);

    QHBoxLayout* centerArea = new QHBoxLayout();
    centerArea->setContentsMargins(0, 10, 0, 10);

    QVBoxLayout* leftModes = new QVBoxLayout();
    leftModes->setSpacing(16);
    leftModes->addStretch();

    btnClassic = new QPushButton("经典模式");
    btnRelax = new QPushButton("休闲模式");
    btnTimed = new QPushButton("计时模式");
    btnLevel = new QPushButton("关卡模式");
    btnCustom = new QPushButton("自定义模式");

    const QSize modeBtnSize(170, 58);
    btnClassic->setFixedSize(modeBtnSize);
    btnRelax->setFixedSize(modeBtnSize);
    btnTimed->setFixedSize(modeBtnSize);
    btnLevel->setFixedSize(modeBtnSize);
    btnCustom->setFixedSize(modeBtnSize);

    btnClassic->setStyleSheet(mainMenuBigButtonStyle(QColor(0, 145, 255)));
    btnRelax->setStyleSheet(mainMenuBigButtonStyle(QColor(48, 209, 88)));
    btnTimed->setStyleSheet(mainMenuBigButtonStyle(QColor(255, 146, 48)));
    btnLevel->setStyleSheet(mainMenuBigButtonStyle(QColor(219, 52, 242)));
    btnCustom->setStyleSheet(mainMenuBigButtonStyle(QColor(255, 45, 85)));

    applyGlassShadow(btnClassic, QColor(0, 0, 0, 100));
    applyGlassShadow(btnRelax, QColor(0, 0, 0, 100));
    applyGlassShadow(btnTimed, QColor(0, 0, 0, 100));
    applyGlassShadow(btnLevel, QColor(0, 0, 0, 100));
    applyGlassShadow(btnCustom, QColor(0, 0, 0, 100));

    leftModes->addWidget(btnClassic, 0, Qt::AlignLeft);
    leftModes->addWidget(btnRelax, 0, Qt::AlignLeft);
    leftModes->addWidget(btnTimed, 0, Qt::AlignLeft);
    leftModes->addWidget(btnLevel, 0, Qt::AlignLeft);
    leftModes->addWidget(btnCustom, 0, Qt::AlignLeft);
    leftModes->addStretch();

    centerArea->addLayout(leftModes);
    centerArea->addStretch();
    root->addLayout(centerArea, 1);

    QHBoxLayout* bottomBar = new QHBoxLayout();
    bottomBar->addStretch();

    QPushButton* btnSetting = new QPushButton("设置");
    QPushButton* btnHelp = new QPushButton("帮助");
    QPushButton* btnRank = new QPushButton("排行榜");
    QPushButton* btnAbout = new QPushButton("关于");

    for (auto* b : { btnSetting, btnHelp, btnRank, btnAbout }) {
        b->setFixedSize(92, 46);
        b->setStyleSheet(glassButtonStyle(QColor(100, 240, 255)));
        applyGlassShadow(b, QColor(0, 0, 0, 70));
    }

    bottomBar->addWidget(btnSetting);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnHelp);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnRank);
    bottomBar->addSpacing(10);
    bottomBar->addWidget(btnAbout);

    root->addLayout(bottomBar);

    btnExit = btnAbout;

    connect(btnClassic, &QPushButton::clicked, this, &MainWindow::openClassicMode);
    connect(btnRelax, &QPushButton::clicked, this, &MainWindow::openRelaxMode);
    connect(btnTimed, &QPushButton::clicked, this, &MainWindow::openTimedMode);
    connect(btnLevel, &QPushButton::clicked, this, &MainWindow::openLevelMode);
    connect(btnCustom, &QPushButton::clicked, this, &MainWindow::openCustomMode);

    connect(btnSetting, &QPushButton::clicked, this, [=]() {
        SettingsDialog dlg(this);
        dlg.exec();
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
            "- 计时模式：在倒计时内完成消除。\n"
            "- 关卡模式：先选关，通关可解锁下一关。\n"
            "- 自定义模式：可自由设置棋盘行列（行×列必须为偶数）。"
        );
        dlg.exec();
        });

    connect(btnRank, &QPushButton::clicked, this, [=]() {
        RankDialog dlg(this);
        dlg.exec();
        });

    connect(btnAbout, &QPushButton::clicked, this, [=]() {
        AboutDialog dlg(this);
        dlg.exec();
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

void MainWindow::openLevelMode() {
    LevelSelectDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    int level = dlg.selectedLevel();
    if (level < 1 || level > 3) return;

    LevelWindow* w = new LevelWindow(level);
    w->move(this->pos());
    w->show();
    close();
}

void MainWindow::openCustomMode() {
    CustomWindow* w = new CustomWindow();
    if (!w->isValid()) {      // 取消输入时不展示空白棋盘
        delete w;
        return;
    }
    w->move(this->pos());
    w->show();
    close();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    m_bgScaled = QPixmap();
    m_bgScaledSize = QSize();
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath clip;
    clip.addRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
    p.setClipPath(clip);

    if (!m_bgPix.isNull()) {
        if (m_bgScaledSize != size() || m_bgScaled.isNull()) {
            m_bgScaled = m_bgPix.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            m_bgScaledSize = size();
        }
        p.drawPixmap(rect(), m_bgScaled);
    }
    else {
        p.fillPath(clip, QColor(30, 30, 30));
    }
}