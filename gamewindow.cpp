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
#include <QPointer>

// 这个文件实现了GameWindow类，包含了游戏窗口的构造函数、事件处理函数以及一些辅助函数
// GameWindow类是所有游戏模式窗口的基类，提供了公共的UI组件和功能，如顶部栏、棋盘、按钮等，同时定义了一些纯虚函数供子类实现特定模式的逻辑
// GameWindow类的构造函数设置了窗口的基本属性和布局，创建了顶部栏和相关按钮，并连接了按钮的信号与相应的槽函数
// GameWindow类还包含了一些静态函数，用于返回不同UI组件的样式表字符串，这些样式表定义了组件的外观，如背景颜色、边框、字体等
// GameWindow类还定义了一些成员变量，如棋盘的行数和列数、单元格大小、游戏逻辑对象、当前选中的单元格等，这些变量在游戏过程中会被更新和使用
// GameWindow类还定义了一些虚函数，如onGameCleared()、helpTitle()、helpText()等，这些函数在子类中会被重写以实现不同模式的特定行为和帮助信息
// GameWindow类还实现了一些事件处理函数，如paintEvent()、closeEvent()等，这些函数用于处理窗口的绘制和关闭事件，确保游戏界面能够正确显示和响应用户操作
// GameWindow类还包含了一些辅助函数，如refreshBoard()、refreshCell()、drawLinkPath()等，这些函数用于更新游戏界面、显示连接路径等功能，增强游戏的交互性和可玩性
// 总的来说，GameWindow类是整个游戏应用程序的核心窗口类，提供了一个基础框架和公共功能，供不同游戏模式的窗口类继承和扩展，实现丰富多样的游戏体验
// 以下是一些静态函数，用于返回不同UI组件的样式表字符串
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

static const char* kCellStyleNormal = "border: none; background: transparent;";
static const char* kCellStyleSelected = "border: 2px solid red; background: transparent;";
static const char* kCellStyleHint = "border: 2px solid yellow; background: rgba(255,255,0,0.15);";

// GameWindow类的构造函数，初始化窗口属性和布局，创建UI组件并连接信号与槽函数
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
    logic(new GameLogic()),
    rows(10),
    cols(16),
    cellSize(48),
    hasSelected(false),
    m_showLink(false),
    hasHint(false),
    inputEnabled(true),
    m_boardEpoch(0)
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
        refreshBoard(); // 换主题属于全盘变化
        update();
        });

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
        clearLinkPath();
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
    delete logic;
}

// 设置公共模式按钮，包含返回主菜单、重排、提示、帮助等按钮，并连接相应的槽函数
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
        // 普通模式：这里确认一次
        if (!m_forceClosing && !isGameFinished()) {
            if (!GlassMessageBox::question(this, "提示", "你真的要退出吗？退出后不会保存当前进度。")) return;
        }
        backToMain();
        });

    connect(btnReset, &QPushButton::clicked, this, [this]() {
        logic->shuffleMap();
        refreshBoard(); // 重排是全盘变化
        GlassMessageBox::information(this, "提示", "图案已重排！");
        });

    connect(btnHint, &QPushButton::clicked, this, [this]() {
        showHint();
        });

    connect(btnHelp, &QPushButton::clicked, this, [this]() {
        HelpDialog dlg(this);
        dlg.setHelpTitle(helpTitle());
        dlg.setHelpText(helpText());
        dlg.exec();
        });
}

// 获取指定值对应的图标，如果缓存中已有则直接返回，否则从主题路径加载并缓存后返回
QIcon GameWindow::iconForValue(int v)
{
    auto it = m_iconCache.find(v);
    if (it != m_iconCache.end()) return it.value();

    const QString p = ThemeManager::instance().tileIconPath(v);
    QIcon icon(p);
    m_iconCache.insert(v, icon);
    return icon;
}

// 设置窗口标题，如果标题标签存在则更新其文本
void GameWindow::setModeTitle(const QString& title)
{
    if (titleLabel) {
        titleLabel->setText(title);
    }
}

// 返回主菜单的槽函数，首先确认是否需要提示用户保存进度，如果确认则创建主窗口并关闭当前窗口
void GameWindow::backToMain()
{
    if (m_forceClosing) return;
    m_forceClosing = true;

    MainWindow* w = new MainWindow();
    w->move(this->pos());
    w->show();

    close();
}

// 创建棋盘的函数，首先增加棋盘版本号，重置选择和提示状态，清除连接路径，然后销毁旧的单元格和布局，最后根据行列数创建新的单元格并添加到布局中，同时连接单元格的点击信号与相应的槽函数
void GameWindow::createBoard()
{
    ++m_boardEpoch;
    hasSelected = false;
    hasHint = false;
    clearLinkPath();

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

    const int hSpacing = gameLayout->horizontalSpacing();
    const int vSpacing = gameLayout->verticalSpacing();

    const int safeBoardMaxW = 760;
    const int safeBoardMaxH = 360;

    int maxByW = (safeBoardMaxW - (cols - 1) * hSpacing) / cols;
    int maxByH = (safeBoardMaxH - (rows - 1) * vSpacing) / rows;

    int adaptive = qMin(maxByW, maxByH);
    if (adaptive < 24) adaptive = 24;
    if (adaptive > 48) adaptive = 48;
    cellSize = adaptive;

    m_cellsCount = rows * cols;
    cells = new QPushButton * [m_cellsCount];

    int boardW = cols * cellSize + (cols - 1) * hSpacing;
    int boardH = rows * cellSize + (rows - 1) * vSpacing;
    boardWidget->setFixedSize(boardW, boardH);

    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            int idx = x * cols + y;
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

// 刷新指定单元格的显示，根据逻辑数据更新图标和样式，如果坐标无效或单元格不存在则直接返回
void GameWindow::refreshCell(int x, int y)
{
    if (!cells) return;
    if (x < 0 || x >= rows || y < 0 || y >= cols) return;

    int idx = x * cols + y;
    if (idx < 0 || idx >= m_cellsCount || !cells[idx]) return;

    int v = logic->getMapData(x, y);
    if (v == 0) {
        cells[idx]->setIcon(QIcon());
        cells[idx]->setStyleSheet(kCellStyleNormal);
    }
    else {
        cells[idx]->setIcon(iconForValue(v));
        cells[idx]->setStyleSheet(kCellStyleNormal);
    }
}

// 刷新多个单元格的显示，遍历坐标列表并调用单个单元格的刷新函数
void GameWindow::refreshCells(const std::vector<Point>& pts)
{
    for (const auto& p : pts) {
        refreshCell(p.x, p.y);
    }
}

// 处理单元格点击事件的函数，首先检查输入是否启用、坐标是否合法以及单元格是否有内容，如果没有则直接返回。然后根据当前选择状态进行处理，如果没有选中则设置当前单元格为选中状态并保存坐标；如果已经选中则检查是否点击了同一个单元格，如果是则取消选择；如果不是则尝试寻找连接路径，如果找到则播放消除音效、绘制连接路径、更新逻辑数据、刷新界面，并在动画结束后检查游戏状态和可能的重排
void GameWindow::onCellClicked(int x, int y)
{
    if (!inputEnabled) return;
    if (!cells) return;
    if (x < 0 || x >= rows || y < 0 || y >= cols) return;

    Point cur(x, y);
    if (logic->getMapData(x, y) == 0) return;

    AudioManager::instance().playClickSfx();
    clearHintStyle();

    if (!hasSelected) {
        lastPoint = cur;
        int idx = x * cols + y;
        if (idx >= 0 && idx < m_cellsCount && cells[idx]) {
            cells[idx]->setStyleSheet(kCellStyleSelected);
        }
        hasSelected = true;
        return;
    }

    int prevIdx = lastPoint.x * cols + lastPoint.y;
    if (prevIdx >= 0 && prevIdx < m_cellsCount && cells[prevIdx]) {
        cells[prevIdx]->setStyleSheet(kCellStyleNormal);
    }

    if (lastPoint.x == x && lastPoint.y == y) {
        hasSelected = false;
        return;
    }

    std::vector<Point> path;
    if (logic->findPath(lastPoint, cur, path)) {
        AudioManager::instance().playClearSfx();

        drawLinkPath(path);

        // 先改数据
        logic->setMapData(lastPoint.x, lastPoint.y, 0);
        logic->setMapData(cur.x, cur.y, 0);
        onPairRemoved();

        // 局部刷新：仅刷新被消掉的两格
        refreshCell(lastPoint.x, lastPoint.y);
        refreshCell(cur.x, cur.y);

        const quint64 epochAtSchedule = m_boardEpoch;
        QPointer<GameWindow> self(this);

        QTimer::singleShot(120, this, [self, epochAtSchedule]() {
            if (!self) return;
            if (self->m_boardEpoch != epochAtSchedule) return;
            if (!self->cells) return;

            self->clearLinkPath();

            if (self->logic->isMapEmpty()) {
                self->onGameCleared();
                return;
            }

            if (!self->logic->hasAnySolution()) {
                QSettings s("YourCompany", "LLK_Refresh");
                bool autoShuffle = s.value("game/autoShuffle", true).toBool();

                if (autoShuffle) {
                    self->logic->shuffleUntilSolvable();
                    self->refreshBoard(); // 自动重排是全盘变化
                    GlassMessageBox::information(self, "提示", "当前无可消除对子，已自动重排。");
                }
                else {
                    GlassMessageBox::information(self, "提示", "当前无可消除对子，请手动点击“重排”。");
                }
            }
            });
    }

    hasSelected = false;
}

// 刷新整个棋盘的显示，遍历所有单元格并调用单个单元格的刷新函数，同时重置提示状态和连接路径
void GameWindow::refreshBoard()
{
    if (!cells) return;

    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            refreshCell(x, y);
        }
    }

    hasHint = false;
    clearLinkPath();
}

// 绘制连接路径的函数，接受一个坐标列表作为路径，保存路径并设置显示连接的标志，然后调用update()触发重绘事件，在paintEvent中根据路径绘制连接线
void GameWindow::drawLinkPath(const std::vector<Point>& path)
{
    m_linkPath = path;
    m_showLink = true;
    update();
}

// 重写paintEvent函数，在绘制事件中首先检查背景图是否需要缩放，如果需要则进行缩放并缓存结果。然后使用QPainter绘制背景图，并调用基类的paintEvent处理其他绘制操作。最后如果需要显示连接路径，则设置抗锯齿和画笔样式，并根据路径坐标计算单元格中心位置，绘制连接线
void GameWindow::paintEvent(QPaintEvent* event)
{
    if (m_bgScaledSize != size()) {
        m_bgScaled = m_bgPix.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_bgScaledSize = size();
    }

    QPainter bg(this);
    bg.drawPixmap(rect(), m_bgScaled);

    QWidget::paintEvent(event);

    if (!m_showLink || m_linkPath.size() < 2) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(50, 220, 120), 4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

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

    for (size_t i = 0; i + 1 < m_linkPath.size(); ++i) {
        p.drawLine(cellCenter(m_linkPath[i]), cellCenter(m_linkPath[i + 1]));
    }
}
// 重写鼠标事件处理函数，首先调用窗口拖动处理函数，如果处理函数返回true表示事件已被处理则直接返回，否则调用基类的事件处理函数继续处理其他逻辑
void GameWindow::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QWidget::mousePressEvent(event);
}

// 重写鼠标移动事件处理函数，首先调用窗口拖动处理函数，如果处理函数返回true表示事件已被处理则直接返回，否则调用基类的事件处理函数继续处理其他逻辑
void GameWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QWidget::mouseMoveEvent(event);
}

// 重写鼠标释放事件处理函数，首先调用窗口拖动处理函数，如果处理函数返回true表示事件已被处理则直接返回，否则调用基类的事件处理函数继续处理其他逻辑
void GameWindow::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QWidget::mouseReleaseEvent(event);
}

// 清除提示样式的函数，首先检查是否有提示和单元格存在，如果没有则直接返回。然后根据提示坐标计算单元格索引，如果索引有效且单元格存在则根据逻辑数据更新图标和样式，最后重置提示状态
void GameWindow::clearHintStyle()
{
    if (!hasHint || !cells) return;

    // 仅局部清理提示样式，不做全盘刷新
    int i1 = hintA.x * cols + hintA.y;
    int i2 = hintB.x * cols + hintB.y;

    if (i1 >= 0 && i1 < m_cellsCount && cells[i1]) {
        if (logic->getMapData(hintA.x, hintA.y) == 0) {
            cells[i1]->setIcon(QIcon());
        }
        cells[i1]->setStyleSheet(kCellStyleNormal);
    }
    if (i2 >= 0 && i2 < m_cellsCount && cells[i2]) {
        if (logic->getMapData(hintB.x, hintB.y) == 0) {
            cells[i2]->setIcon(QIcon());
        }
        cells[i2]->setStyleSheet(kCellStyleNormal);
    }

    hasHint = false;
}

// 显示提示的函数，首先清除之前的提示样式，然后调用逻辑对象的findHintPair函数寻找可消除的一对坐标，如果找到则保存提示坐标并设置提示样式，如果没有找到则根据设置决定是否自动重排或提示用户手动重排
void GameWindow::showHint()
{
    clearHintStyle();

    Point a, b;
    if (logic->findHintPair(a, b)) {
        hintA = a;
        hintB = b;
        hasHint = true;

        int i1 = a.x * cols + a.y;
        int i2 = b.x * cols + b.y;

        if (cells && i1 >= 0 && i1 < m_cellsCount && cells[i1]) {
            cells[i1]->setStyleSheet(kCellStyleHint);
        }
        if (cells && i2 >= 0 && i2 < m_cellsCount && cells[i2]) {
            cells[i2]->setStyleSheet(kCellStyleHint);
        }
    }
    else {
        QSettings s("YourCompany", "LLK_Refresh");
        bool autoShuffle = s.value("game/autoShuffle", true).toBool();

        if (autoShuffle) {
            GlassMessageBox::information(this, "提示", "当前无可消除对子，将自动重排。");
            logic->shuffleUntilSolvable();
            refreshBoard(); // 自动重排是全盘变化
        }
        else {
            GlassMessageBox::information(this, "提示", "当前无可消除对子，请手动点击“重排”。");
        }
    }
}

// 清除连接路径的函数，首先检查是否需要清除，如果不需要则直接返回。然后重置显示连接的标志，清空连接路径列表，并调用update()触发重绘事件
void GameWindow::clearLinkPath()
{
    if (!m_showLink && m_linkPath.empty()) return;
    m_showLink = false;
    m_linkPath.clear();
    update();
}

// 游戏消除成功后的处理函数，首先显示一个信息框提示用户消除成功，然后调用返回主菜单的函数
void GameWindow::onGameCleared()
{
    GlassMessageBox::information(this, "结算", "消除成功");
    backToMain();
}

// 判断游戏是否结束的函数，检查逻辑对象是否存在，如果存在则调用其isMapEmpty函数判断棋盘是否已清空，如果逻辑对象不存在则认为游戏已结束
bool GameWindow::isGameFinished() const
{
    return logic ? logic->isMapEmpty() : true;
}

// 确认退出的函数，如果强制关闭标志已设置或游戏已结束则直接返回true，否则显示一个询问框让用户确认是否退出，返回用户的选择结果
bool GameWindow::confirmExitIfNeeded()
{
    if (m_forceClosing || isGameFinished()) return true;
    return GlassMessageBox::question(this, "提示", "你真的要退出吗？退出后不会保存当前进度。");
}

// 重写窗口关闭事件处理函数，在关闭事件中首先调用确认退出的函数，如果用户确认则接受事件继续关闭窗口，否则忽略事件保持窗口打开
void GameWindow::closeEvent(QCloseEvent* event)
{
    // TimedWindow / LevelWindow 由 TimedWindow::closeEvent 自己处理
    if (dynamic_cast<TimedWindow*>(this) != nullptr) {
        QWidget::closeEvent(event);
        return;
    }

    if (m_forceClosing || confirmExitIfNeeded()) event->accept();
    else event->ignore();
}