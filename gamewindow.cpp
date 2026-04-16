#include "audiomanager.h"
#include "gamewindow.h"
#include "mainwindow.h"
#include "helpdialog.h"
#include "glassmessagebox.h"
#include "thememanager.h"
#include "timedwindow.h"
#include "uistyle.h"
#include "classicrankmanager.h"

#include <QIcon>
#include <QPixmap>
#include <QLayoutItem>
#include <QPainter>
#include <QPaintEvent>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QBrush>
#include <QColor>

// GameWindow 实现文件，包含游戏主窗口的 UI 组件创建、布局、事件处理等逻辑
// 设计上采用了分层布局，顶部为标题栏和控制按钮，中部为游戏棋盘，底部为操作按钮。通过连接 GameControl 的信号和槽实现游戏状态与 UI 的同步更新。同时使用了 Qt 的样式表和图形效果来增强界面美观度，实现玻璃质感的视觉效果。
// 主要功能包括：
// 创建和布局 UI 组件：使用 Qt 的布局管理器创建标题栏、信息栏、棋盘区域和操作按钮区域，并设置样式和效果
// 响应用户交互：处理用户点击棋盘单元格、操作按钮等事件，调用 GameControl 的方法更新游戏状态，并刷新 UI 显示
// 显示游戏状态：根据 GameControl 的状态更新棋盘单元格的图标和样式，显示连接路径、提示等信息
// 管理游戏流程：处理游戏完成、无解等状态，显示提示信息或返回主菜单等操作

// UI 样式定义，使用原生 Qt 样式表，配合半透明背景和阴影效果实现玻璃质感
static QString topBarStyle()
{
    return R"(
    QWidget {
        background-color: rgba(255, 255, 255, 0.20);
        border: 1px solid rgba(255, 255, 255, 0.28);
        border-radius: 16px;
    }
)";
}

static QString topBarTitleStyle()
{
    return R"(
    QLabel {
        color: rgba(0, 0, 0, 0.82);
        font-size: 15px;
        font-weight: 600;
        background: transparent;
        border: none;
    }
)";
}

static QString topBarBtnMinStyle()
{
    return R"(
    QPushButton {
        background: transparent;
        border: none;
        font-size: 18px;
        color: rgba(0,0,0,0.75);
        border-radius: 10px;
    }
    QPushButton:hover {
        background-color: rgba(255,255,255,0.35);
    }
)";
}

static QString topBarBtnCloseStyle()
{
    return R"(
    QPushButton {
        background: transparent;
        border: none;
        font-size: 18px;
        color: rgba(0,0,0,0.75);
        border-radius: 10px;
    }
    QPushButton:hover {
        background-color: rgba(255,80,80,0.92);
        color: white;
    }
)";
}

// 棋盘单元格的样式，普通状态无边框，选中状态红色边框，提示状态黄色边框和半透明背景
static const char* kCellStyleNormal = "border: none; background: transparent;";
static const char* kCellStyleSelected = "border: 2px solid red; background: transparent;";
static const char* kCellStyleHint = "border: 2px solid yellow; background: rgba(255,255,0,0.15);";

// GameWindow 构造函数，初始化成员变量，设置窗口属性，创建 UI 组件并连接信号槽
GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent),
    topWidget(nullptr),
    infoWidget(nullptr),
    boardWidget(nullptr),
    bottomWidget(nullptr),
    gameLayout(nullptr),
    infoLayout(nullptr),
    btnLayout(nullptr),
    cells(nullptr),
    m_cellsCount(0),
    btnBack(nullptr),
    btnReset(nullptr),
    btnHint(nullptr),
    btnHelp(nullptr),
    btnPause(nullptr),
    appIcon(nullptr),
    titleLabel(nullptr),
    btnMin(nullptr),
    btnClose(nullptr),
    m_control(new GameControl(this)),
    cellSize(48),
    m_forceClosing(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setFixedSize(800, 600);
    setAutoFillBackground(false);

    m_bgPix = QPixmap(ThemeManager::instance().backgroundPathGame());
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        m_bgPix = QPixmap(ThemeManager::instance().backgroundPathGame());
        m_bgScaled = QPixmap();
        m_bgScaledSize = QSize();
        m_iconCache.clear();
        refreshBoard();
        update();
        });

    connect(m_control, &GameControl::cellSelected, this, &GameWindow::onControlCellSelected);
    connect(m_control, &GameControl::cellDeselected, this, &GameWindow::onControlCellDeselected);
    connect(m_control, &GameControl::selectionTransferred, this, &GameWindow::onControlSelectionTransferred);
    connect(m_control, &GameControl::pairMatched, this, &GameWindow::onControlPairMatched);
    connect(m_control, &GameControl::cellsRemoved, this, &GameWindow::onControlCellsRemoved);
    connect(m_control, &GameControl::hintDisplayed, this, &GameWindow::onControlHintDisplayed);
    connect(m_control, &GameControl::hintCleared, this, &GameWindow::onControlHintCleared);
    connect(m_control, &GameControl::linkPathUpdated, this, &GameWindow::onControlLinkPathUpdated);
    connect(m_control, &GameControl::linkPathCleared, this, &GameWindow::onControlLinkPathCleared);
    connect(m_control, &GameControl::gameCleared, this, &GameWindow::onControlGameCleared);
    connect(m_control, &GameControl::noSolutionAutoShuffled, this, &GameWindow::onControlNoSolutionAutoShuffled);
    connect(m_control, &GameControl::noSolutionManualShuffleNeeded, this, &GameWindow::onControlNoSolutionManualShuffleNeeded);

    QVBoxLayout* globalLayout = new QVBoxLayout(this);
    globalLayout->setContentsMargins(16, 16, 16, 16);
    globalLayout->setSpacing(14);

    topWidget = new QWidget(this);
    topWidget->setFixedHeight(44);
    topWidget->setStyleSheet(topBarStyle());

    auto* topShadow = new QGraphicsDropShadowEffect(topWidget);
    topShadow->setBlurRadius(22);
    topShadow->setOffset(0, 6);
    topShadow->setColor(QColor(0, 0, 0, 45));
    topWidget->setGraphicsEffect(topShadow);

    QHBoxLayout* topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(12, 6, 12, 6);
    topLayout->setSpacing(8);

    appIcon = new QLabel(topWidget);
    appIcon->setPixmap(QPixmap(":/images/app.ico").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    appIcon->setStyleSheet(R"(
    QLabel {
        background: transparent;
        border: none;
        padding: 0px;
        margin: 0px;
    }
)");
    appIcon->setFixedSize(18, 18);

    titleLabel = new QLabel("连连看", topWidget);
    titleLabel->setStyleSheet(topBarTitleStyle());

    btnMin = new QPushButton("–", topWidget);
    btnClose = new QPushButton("×", topWidget);

    btnMin->setFixedSize(28, 24);
    btnClose->setFixedSize(28, 24);
    btnMin->setStyleSheet(topBarBtnMinStyle());
    btnClose->setStyleSheet(topBarBtnCloseStyle());

    topLayout->addWidget(appIcon);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnMin);
    topLayout->addWidget(btnClose);

    connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);

    globalLayout->addWidget(topWidget);

    infoWidget = new QWidget(this);
    infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(0);
    infoLayout->setAlignment(Qt::AlignCenter);

    m_score = 0;
    m_scoreLabel = new QLabel("当前分数: 0", infoWidget);
    m_scoreLabel->setStyleSheet(R"(
        QLabel {
            color: rgba(0,0,0,0.92);
            font-size: 15px;
            font-weight: 600;
            background-color: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 10px;
            padding: 6px 16px;
        }
    )");
    infoLayout->addWidget(m_scoreLabel);

    globalLayout->addWidget(infoWidget);

    globalLayout->addStretch(1);

    boardWidget = new QWidget(this);
    QHBoxLayout* boardOuter = new QHBoxLayout(boardWidget);
    boardOuter->setContentsMargins(0, 0, 0, 0);
    boardOuter->setSpacing(0);
    boardOuter->setAlignment(Qt::AlignCenter);

    gameLayout = new QGridLayout();
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->setHorizontalSpacing(2);
    gameLayout->setVerticalSpacing(2);
    gameLayout->setAlignment(Qt::AlignCenter);

    boardOuter->addLayout(gameLayout);
    globalLayout->addWidget(boardWidget, 0, Qt::AlignCenter);

    bottomWidget = new QWidget(this);
    btnLayout = new QHBoxLayout(bottomWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(20);
    btnLayout->setAlignment(Qt::AlignCenter);

    globalLayout->addWidget(bottomWidget, 0, Qt::AlignCenter);
    globalLayout->addStretch(1);

    setLayout(globalLayout);

    m_linkFadeAnim = new QVariantAnimation(this);
    m_linkFadeAnim->setDuration(200);
    connect(m_linkFadeAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        m_linkAlpha = v.toReal();
        update();
        });
    connect(m_linkFadeAnim, &QVariantAnimation::finished, this, [this]() {
        m_control->clearLinkPath();
        });
}

GameWindow::~GameWindow()
{
    if (cells) {
        for (int i = 0; i < m_cellsCount; ++i) {
            delete cells[i];
            cells[i] = nullptr;
        }
        delete[] cells;
        cells = nullptr;
        m_cellsCount = 0;
    }
}

// 设置游戏模式通用的操作按钮，包括返回主菜单、重排、提示、帮助等，根据参数决定是否包含暂停按钮，并连接相应的槽函数处理用户交互
void GameWindow::setupCommonModeButtons(const QColor& baseColor, bool withPauseButton, const QSize& btnSize)
{
    btnBack = new QPushButton("返回主菜单");
    btnReset = new QPushButton("重排");
    btnHint = new QPushButton("提示");
    btnHelp = new QPushButton("帮助");

    auto initBtn = [&](QPushButton* b) {
        b->setStyleSheet(glassButtonStyle(baseColor));
        b->setFixedSize(btnSize);
        btnLayout->addWidget(b);
        };

    initBtn(btnBack);

    if (withPauseButton) {
        btnPause = new QPushButton("暂停");
        initBtn(btnPause);
    }

    initBtn(btnReset);
    initBtn(btnHint);
    initBtn(btnHelp);

    connect(btnBack, &QPushButton::clicked, this, [this]() {
        if (!m_forceClosing && !isGameFinished()) {
            if (!GlassMessageBox::question(this, "提示", "你真的要退出吗？退出后不会保存当前进度。")) return;
        }
        backToMain();
        });

    connect(btnReset, &QPushButton::clicked, this, [this]() {
        m_control->shuffle();
        refreshBoard();
        GlassMessageBox::information(this, "提示", "图案已重排！");
        });

    connect(btnHint, &QPushButton::clicked, this, [this]() {
        m_control->showHint();
        });

    connect(btnHelp, &QPushButton::clicked, this, [this]() {
        HelpDialog dlg(this);
        dlg.setHelpTitle(helpTitle());
        dlg.setHelpText(helpText());
        dlg.exec();
        });
}

// 获取指定数值对应的图标，使用缓存避免重复加载，提高性能
QIcon GameWindow::iconForValue(int v)
{
    auto it = m_iconCache.find(v);
    if (it != m_iconCache.end()) return it.value();

    const QString p = ThemeManager::instance().tileIconPath(v);
    QIcon icon(p);
    m_iconCache.insert(v, icon);
    return icon;
}

// 设置窗口标题，更新标题标签的文本
void GameWindow::setModeTitle(const QString& title)
{
    if (titleLabel) {
        titleLabel->setText(title);
    }
}

// 返回主菜单的操作，首先检查是否强制关闭标志，如果未设置则弹出确认对话框询问用户是否退出，若用户确认则创建主窗口实例并显示，同时关闭当前游戏窗口
void GameWindow::backToMain()
{
    if (m_forceClosing) return;
    m_forceClosing = true;

    MainWindow* w = new MainWindow();
    w->move(this->pos());
    w->show();

    close();
}

// 创建游戏棋盘，根据 GameControl 的行列数动态生成 QPushButton 作为单元格，并连接点击事件处理函数，同时根据窗口大小适应性调整单元格大小，确保在不同屏幕尺寸下都有良好的显示效果
void GameWindow::createBoard()
{
    m_control->resetState();

    if (cells) {
        for (int i = 0; i < m_cellsCount; ++i) {
            delete cells[i];
            cells[i] = nullptr;
        }
        delete[] cells;
        cells = nullptr;
        m_cellsCount = 0;
    }

    while (QLayoutItem* item = gameLayout->takeAt(0)) {
        delete item;
    }

    const int r = m_control->rows();
    const int c = m_control->cols();

    const int hSpacing = gameLayout->horizontalSpacing();
    const int vSpacing = gameLayout->verticalSpacing();

    const int safeBoardMaxW = 760;
    const int safeBoardMaxH = 360;

    int maxByW = (safeBoardMaxW - (c - 1) * hSpacing) / c;
    int maxByH = (safeBoardMaxH - (r - 1) * vSpacing) / r;

    int adaptive = qMin(maxByW, maxByH);
    if (adaptive < 24) adaptive = 24;
    if (adaptive > 48) adaptive = 48;
    cellSize = adaptive;

    m_cellsCount = r * c;
    cells = new QPushButton * [m_cellsCount];

    int boardW = c * cellSize + (c - 1) * hSpacing;
    int boardH = r * cellSize + (r - 1) * vSpacing;
    boardWidget->setFixedSize(boardW, boardH);

    for (int x = 0; x < r; ++x) {
        for (int y = 0; y < c; ++y) {
            int idx = x * c + y;
            cells[idx] = new QPushButton(boardWidget);
            cells[idx]->setFixedSize(cellSize, cellSize);
            cells[idx]->setIconSize(QSize(cellSize - 2, cellSize - 2));
            cells[idx]->setStyleSheet(kCellStyleNormal);

            gameLayout->addWidget(cells[idx], x, y, Qt::AlignCenter);

            connect(cells[idx], &QPushButton::clicked, this, [=]() {
                onCellClicked(x, y);
                });
        }
    }
}

// 刷新指定单元格的显示，根据 GameControl 中对应位置的数值更新图标和样式，数值为 0 时清除图标并设置普通样式，非 0 时设置对应图标并保持普通样式
void GameWindow::refreshCell(int x, int y)
{
    if (!cells) return;

    const int r = m_control->rows();
    const int c = m_control->cols();
    if (x < 0 || x >= r || y < 0 || y >= c) return;

    int idx = x * c + y;
    if (idx < 0 || idx >= m_cellsCount || !cells[idx]) return;

    int v = m_control->getMapData(x, y);
    if (v == 0) {
        cells[idx]->setIcon(QIcon());
        cells[idx]->setStyleSheet(kCellStyleNormal);
    }
    else {
        cells[idx]->setIcon(iconForValue(v));
        cells[idx]->setStyleSheet(kCellStyleNormal);
    }
}

// 刷新多个单元格的显示，接受一个 Point 类型的向量，依次调用 refreshCell 刷新每个指定位置的单元格
void GameWindow::refreshCells(const std::vector<Point>& pts)
{
    for (const auto& p : pts) {
        refreshCell(p.x, p.y);
    }
}

// 处理单元格点击事件，首先播放点击音效，然后调用 GameControl 的 handleCellClick 方法处理游戏逻辑，根据点击位置更新游戏状态并刷新 UI 显示
void GameWindow::onCellClicked(int x, int y)
{
    AudioManager::instance().playClickSfx();
    m_control->handleCellClick(x, y);
}

// 刷新整个棋盘的显示，遍历 GameControl 中的所有单元格位置，调用 refreshCell 刷新每个单元格的显示状态，同时清除连接路径以更新 UI 显示
void GameWindow::refreshBoard()
{
    if (!cells) return;

    const int r = m_control->rows();
    const int c = m_control->cols();

    for (int x = 0; x < r; ++x) {
        for (int y = 0; y < c; ++y) {
            refreshCell(x, y);
        }
    }

    m_control->clearLinkPath();
}

// 绘制连接路径，首先检查当前连接路径是否有效且包含至少两个点，如果无效则直接返回。然后使用 QPainter 绘制连接路径，设置抗锯齿和线条样式，根据 GameControl 中的连接路径数据计算每个单元格中心的位置，并依次绘制连接线段，最后根据动画状态更新连接线的透明度实现淡入淡出效果
void GameWindow::paintEvent(QPaintEvent* event)
{
    if (m_bgScaledSize != size()) {
        m_bgScaled = m_bgPix.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_bgScaledSize = size();
    }

    QPainter bg(this);
    bg.drawPixmap(rect(), m_bgScaled);

    QWidget::paintEvent(event);

    if (!m_control->isShowingLink() || m_control->linkPath().size() < 2) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(100, 255, 240), 4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    const auto& lpath = m_control->linkPath();

    auto cellCenter = [&](Point pt)->QPoint {
        int hSpacing = gameLayout->horizontalSpacing();
        int vSpacing = gameLayout->verticalSpacing();

        QPoint boardTopLeft = boardWidget->geometry().topLeft();
        int x0 = boardTopLeft.x();
        int y0 = boardTopLeft.y();

        int cx = x0 + pt.y * (cellSize + hSpacing) + cellSize / 2;
        int cy = y0 + pt.x * (cellSize + vSpacing) + cellSize / 2;
        return QPoint(cx, cy);
        };

    for (size_t i = 0; i + 1 < lpath.size(); ++i) {
        p.drawLine(cellCenter(lpath[i]), cellCenter(lpath[i + 1]));
    }
}

// 处理窗口拖动的鼠标事件，首先调用 handleWindowDragMousePress、handleWindowDragMouseMove 和 handleWindowDragMouseRelease 函数处理窗口拖动逻辑，如果事件被处理则直接返回，否则调用基类的事件处理函数继续处理其他鼠标事件
void GameWindow::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QWidget::mousePressEvent(event);
}

// 处理窗口拖动的鼠标事件，首先调用 handleWindowDragMousePress、handleWindowDragMouseMove 和 handleWindowDragMouseRelease 函数处理窗口拖动逻辑，如果事件被处理则直接返回，否则调用基类的事件处理函数继续处理其他鼠标事件
void GameWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QWidget::mouseMoveEvent(event);
}

// 处理窗口拖动的鼠标事件，首先调用 handleWindowDragMousePress、handleWindowDragMouseMove 和 handleWindowDragMouseRelease 函数处理窗口拖动逻辑，如果事件被处理则直接返回，否则调用基类的事件处理函数继续处理其他鼠标事件
void GameWindow::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QWidget::mouseReleaseEvent(event);
}

// 游戏完成时的处理函数，首先显示一个信息对话框提示用户消除成功，然后调用 backToMain 函数返回主菜单
void GameWindow::onGameCleared()
{
    GlassMessageBox::information(this, "结算", "消除成功");
    backToMain();
}

// 检查游戏是否完成，调用 GameControl 的 isFinished 方法判断当前游戏状态是否已完成，返回相应的布尔值
bool GameWindow::isGameFinished() const
{
    return m_control->isFinished();
}

// 在需要确认退出时弹出对话框询问用户是否退出，如果游戏已完成或强制关闭标志已设置则直接返回 true，否则显示一个问题对话框让用户选择是否退出，返回用户的选择结果
bool GameWindow::confirmExitIfNeeded()
{
    if (m_forceClosing || isGameFinished()) return true;
    return GlassMessageBox::question(this, "提示", "你真的要退出吗？退出后不会保存当前进度。");
}

// 处理窗口关闭事件，首先检查当前窗口是否为 TimedWindow 类型，如果是则直接调用基类的事件处理函数关闭窗口，否则根据强制关闭标志和游戏完成状态决定是否弹出确认对话框询问用户是否退出，如果用户确认则接受事件关闭窗口，否则忽略事件继续保持窗口打开状态
void GameWindow::closeEvent(QCloseEvent* event)
{
    if (dynamic_cast<TimedWindow*>(this) != nullptr) {
        QWidget::closeEvent(event);
        return;
    }

    if (m_forceClosing || confirmExitIfNeeded()) event->accept();
    else event->ignore();
}

// 处理 GameControl 的 cellSelected 信号，首先检查 cells 数组是否有效，然后根据传入的 x 和 y 坐标计算对应的单元格索引，如果索引有效且对应的单元格存在，则将该单元格的样式设置为选中状态
void GameWindow::onControlCellSelected(int x, int y)
{
    if (!cells) return;
    const int c = m_control->cols();
    int idx = x * c + y;
    if (idx >= 0 && idx < m_cellsCount && cells[idx]) {
        cells[idx]->setStyleSheet(kCellStyleSelected);
    }
}

// 处理 GameControl 的 cellDeselected 信号，首先检查 cells 数组是否有效，然后根据传入的 x 和 y 坐标计算对应的单元格索引，如果索引有效且对应的单元格存在，则将该单元格的样式设置为普通状态
void GameWindow::onControlCellDeselected(int x, int y)
{
    if (!cells) return;
    const int c = m_control->cols();
    int idx = x * c + y;
    if (idx >= 0 && idx < m_cellsCount && cells[idx]) {
        cells[idx]->setStyleSheet(kCellStyleNormal);
    }
}

// 处理 GameControl 的 selectionTransferred 信号，首先检查 cells 数组是否有效，然后根据传入的 fromX、fromY、toX 和 toY 坐标计算对应的单元格索引，如果目标索引有效且对应的单元格存在，则将该单元格的样式设置为选中状态
void GameWindow::onControlSelectionTransferred(int fromX, int fromY, int toX, int toY)
{
    Q_UNUSED(fromX);
    Q_UNUSED(fromY);
    if (!cells) return;
    const int c = m_control->cols();
    int idx = toX * c + toY;
    if (idx >= 0 && idx < m_cellsCount && cells[idx]) {
        cells[idx]->setStyleSheet(kCellStyleSelected);
    }
}

// 处理 GameControl 的 pairMatched 信号，首先播放消除成功的音效，然后根据传入的 a、b 和 path 参数更新 UI 显示连接路径和消除效果，最后调用 onPairRemoved 函数处理消除后的逻辑
void GameWindow::onControlPairMatched(Point a, Point b, const std::vector<Point>& path)
{
    Q_UNUSED(a);
    Q_UNUSED(b);
    Q_UNUSED(path);
    AudioManager::instance().playClearSfx();

    // 读取分数设置，默认 5 分
    QSettings s("YourCompany", "LLK_Refresh");
    int scorePerPair = s.value("game/scorePerPair", 5).toInt();

    m_score += scorePerPair;
    if (m_scoreLabel) {
        m_scoreLabel->setText(QString("当前分数: %1").arg(m_score));
    }
}

// 处理 GameControl 的 cellsRemoved 信号，首先调用 onPairRemoved 函数处理消除后的逻辑，然后根据传入的 a 和 b 坐标刷新对应单元格的显示状态，更新 UI 显示消除效果
void GameWindow::onControlCellsRemoved(Point a, Point b)
{
    onPairRemoved();
    refreshCell(a.x, a.y);
    refreshCell(b.x, b.y);
}

// 处理 GameControl 的 hintDisplayed 信号，首先检查 cells 数组是否有效，然后根据传入的 a 和 b 坐标计算对应的单元格索引，如果索引有效且对应的单元格存在，则将该单元格的样式设置为提示状态
void GameWindow::onControlHintDisplayed(Point a, Point b)
{
    if (!cells) return;

    const int c = m_control->cols();
    int i1 = a.x * c + a.y;
    int i2 = b.x * c + b.y;

    if (i1 >= 0 && i1 < m_cellsCount && cells[i1]) {
        cells[i1]->setStyleSheet(kCellStyleHint);
    }
    if (i2 >= 0 && i2 < m_cellsCount && cells[i2]) {
        cells[i2]->setStyleSheet(kCellStyleHint);
    }
}

// 处理 GameControl 的 hintCleared 信号，首先检查 cells 数组是否有效，然后根据传入的 a 和 b 坐标计算对应的单元格索引，如果索引有效且对应的单元格存在，则根据 GameControl 中对应位置的数值更新图标和样式，数值为 0 时清除图标并设置普通样式，非 0 时设置对应图标并保持普通样式，同时将样式设置为普通状态
void GameWindow::onControlHintCleared(Point a, Point b)
{
    if (!cells) return;

    const int c = m_control->cols();
    int i1 = a.x * c + a.y;
    int i2 = b.x * c + b.y;

    if (i1 >= 0 && i1 < m_cellsCount && cells[i1]) {
        if (m_control->getMapData(a.x, a.y) == 0) {
            cells[i1]->setIcon(QIcon());
        }
        cells[i1]->setStyleSheet(kCellStyleNormal);
    }
    if (i2 >= 0 && i2 < m_cellsCount && cells[i2]) {
        if (m_control->getMapData(b.x, b.y) == 0) {
            cells[i2]->setIcon(QIcon());
        }
        cells[i2]->setStyleSheet(kCellStyleNormal);
    }
}

// 处理 GameControl 的 linkPathUpdated 信号，调用 update 函数刷新窗口显示连接路径的更新效果
void GameWindow::onControlLinkPathUpdated()
{
    update();
}

// 处理 GameControl 的 linkPathCleared 信号，调用 update 函数刷新窗口显示连接路径被清除的效果
void GameWindow::onControlLinkPathCleared()
{
    update();
}

// 处理 GameControl 的 gameCleared 信号，调用 onGameCleared 函数处理游戏完成后的逻辑，显示提示信息并返回主菜单
void GameWindow::onControlGameCleared()
{
    onGameCleared();
}

// 处理 GameControl 的 noSolutionAutoShuffled 信号，首先刷新整个棋盘的显示状态，然后显示一个信息对话框提示用户当前无可消除对子并已自动重排
void GameWindow::onControlNoSolutionAutoShuffled()
{
    refreshBoard();
    GlassMessageBox::information(this, "提示", "当前无可消除对子，已自动重排。");
}

// 处理 GameControl 的 noSolutionManualShuffleNeeded 信号，显示一个信息对话框提示用户当前无可消除对子需要手动点击重排
void GameWindow::onControlNoSolutionManualShuffleNeeded()
{
    GlassMessageBox::information(this, "提示", "当前无可消除对子，请手动点击重排。");
}
