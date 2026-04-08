#include "audiomanager.h"
#include "gamewindow.h"
#include "mainwindow.h"
#include "helpdialog.h"
#include "glassmessagebox.h"
#include "thememanager.h"
#include "uistyle.h"

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
        refreshBoard();
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

    connect(btnBack, &QPushButton::clicked, this, &GameWindow::backToMain);

    connect(btnReset, &QPushButton::clicked, this, [this]() {
        logic->shuffleMap();
        refreshBoard();
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

QIcon GameWindow::iconForValue(int v)
{
    auto it = m_iconCache.find(v);
    if (it != m_iconCache.end()) return it.value();

    const QString p = ThemeManager::instance().tileIconPath(v);
    QIcon icon(p);
    m_iconCache.insert(v, icon);
    return icon;
}

void GameWindow::setModeTitle(const QString& title)
{
    if (titleLabel) {
        titleLabel->setText(title);
    }
}

void GameWindow::backToMain()
{
    MainWindow* w = new MainWindow();
    w->move(this->pos());
    w->show();
    close();
}

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

        logic->setMapData(lastPoint.x, lastPoint.y, 0);
        logic->setMapData(cur.x, cur.y, 0);
        onPairRemoved();

        const quint64 epochAtSchedule = m_boardEpoch;
        QPointer<GameWindow> self(this);

        QTimer::singleShot(120, this, [self, epochAtSchedule]() {
            if (!self) return;
            if (self->m_boardEpoch != epochAtSchedule) return;
            if (!self->cells) return;

            self->clearLinkPath();
            self->refreshBoard();

            if (self->logic->isMapEmpty()) {
                self->onGameCleared();
                return;
            }

            if (!self->logic->hasAnySolution()) {
                QSettings s("YourCompany", "LLK_Refresh");
                bool autoShuffle = s.value("game/autoShuffle", true).toBool();

                if (autoShuffle) {
                    self->logic->shuffleUntilSolvable();
                    self->refreshBoard();
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

void GameWindow::refreshBoard()
{
    if (!cells) return;

    for (int x = 0; x < rows; x++) {
        for (int y = 0; y < cols; y++) {
            int v = logic->getMapData(x, y);
            int idx = x * cols + y;

            if (idx < 0 || idx >= m_cellsCount || !cells[idx]) continue;

            if (v == 0) {
                cells[idx]->setIcon(QIcon());
                cells[idx]->setStyleSheet(kCellStyleNormal);
            }
            else {
                cells[idx]->setIcon(iconForValue(v));
                cells[idx]->setStyleSheet(kCellStyleNormal);
            }
        }
    }

    hasHint = false;
    clearLinkPath();
}

void GameWindow::drawLinkPath(const std::vector<Point>& path)
{
    m_linkPath.clear();
    m_linkPath = path;
    m_showLink = true;
    update();
}

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

void GameWindow::mousePressEvent(QMouseEvent* event)
{
    if (handleWindowDragMousePress(this, event, m_dragState)) return;
    QWidget::mousePressEvent(event);
}

void GameWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (handleWindowDragMouseMove(this, event, m_dragState)) return;
    QWidget::mouseMoveEvent(event);
}

void GameWindow::mouseReleaseEvent(QMouseEvent* event)
{
    handleWindowDragMouseRelease(event, m_dragState);
    QWidget::mouseReleaseEvent(event);
}

void GameWindow::clearHintStyle()
{
    if (!hasHint || !cells) return;

    int i1 = hintA.x * cols + hintA.y;
    int i2 = hintB.x * cols + hintB.y;

    if (i1 >= 0 && i1 < m_cellsCount && cells[i1]) {
        cells[i1]->setStyleSheet(kCellStyleNormal);
    }
    if (i2 >= 0 && i2 < m_cellsCount && cells[i2]) {
        cells[i2]->setStyleSheet(kCellStyleNormal);
    }

    hasHint = false;
}

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
            refreshBoard();
        }
        else {
            GlassMessageBox::information(this, "提示", "当前无可消除对子，请手动点击“重排”。");
        }
    }
}

void GameWindow::clearLinkPath()
{
    m_showLink = false;
    m_linkPath.clear();
    update();
}

void GameWindow::onGameCleared()
{
    GlassMessageBox::information(this, "结算", "消除成功");
    backToMain();
}