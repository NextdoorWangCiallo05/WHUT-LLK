#include "thememanager.h"
#include <QSettings>
#include <QFile>

// 获取 ThemeManager 单例实例
ThemeManager& ThemeManager::instance()
{
    static ThemeManager ins;
    return ins;
}

// 构造函数，调用 load() 加载主题设置
ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    load();
}

// 确保主题名称合法，返回标准化的主题名称
QString ThemeManager::ensureThemeName(const QString& name) const
{
    QString n = name.trimmed().toLower();
    if (n.isEmpty()) return "classic";
    if (n != "classic" && n != "fruit" && n != "win" && n != "star") return "classic";
    return n;
}

// 检查指定的资源路径是否存在
bool ThemeManager::exists(const QString& qrcPath) const
{
    return QFile::exists(qrcPath);
}

// 从配置中加载当前主题设置，如果没有则使用默认值 "classic"
void ThemeManager::load()
{
    QSettings s("YourCompany", "LLK_Refresh");
    m_theme = ensureThemeName(s.value("ui/theme", "classic").toString());
}

// 将当前主题设置保存到配置中
void ThemeManager::save()
{
    QSettings s("YourCompany", "LLK_Refresh");
    s.setValue("ui/theme", m_theme);
}

// 获取当前主题名称
QString ThemeManager::currentTheme() const
{
    return m_theme;
}

// 设置当前主题名称，如果发生变化则保存并发出 themeChanged 信号
void ThemeManager::setCurrentTheme(const QString& name)
{
    QString n = ensureThemeName(name);
    if (n == m_theme) return;
    m_theme = n;
    save();
    emit themeChanged();
}

// 获取主界面背景图路径，如果当前主题没有则使用经典主题的背景图
QString ThemeManager::backgroundPathMain() const
{
    QString p = QString(":/images/%1/main.png").arg(m_theme);
    if (!exists(p)) p = ":/images/classic/main.png";
    return p;
}

// 获取游戏界面背景图路径，如果当前主题没有则使用经典主题的背景图
QString ThemeManager::backgroundPathGame() const
{
    QString p = QString(":/images/%1/game.png").arg(m_theme);
    if (!exists(p)) p = ":/images/classic/game.png";
    return p;
}

// 获取对话框背景图路径，当前所有主题都使用同一张图
QString ThemeManager::backgroundPathDialog() const
{
    QString p = QString(":/images/dialog.png");
    return p;
}

// 获取指定数值对应的方块图标路径，根据当前主题和数值映射到相应的图标，如果当前主题没有则使用经典主题的图标
QString ThemeManager::tileIconPath(int value) const
{
    if (value <= 0) return QString();

    // 每个主题支持的元素数量
    int count = 20;
    if (m_theme == "classic" || m_theme == "win") count = 12;   // 主题1/3：12个
    else if (m_theme == "fruit") count = 20;                    // 主题2：20个
    else if (m_theme == "star") count = 18;                     // 主题4：18个

    // 将任意 value 映射到 [1..count]
    int mapped = ((value - 1) % count) + 1;

    // 主题路径（按你实际目录调整）
    QString p = QString(":/images/%1/%2.png").arg(m_theme).arg(mapped);
    if (exists(p)) return p;

    // 兜底：classic
    p = QString(":/images/classic/%1.png").arg(((value - 1) % 12) + 1);
    if (exists(p)) return p;

    return QString();
}

// 获取当前主题支持的方块类型数量，根据主题不同返回不同的数量
int ThemeManager::tileTypeCount() const
{
    if (m_theme == "classic" || m_theme == "win") return 12; // 主题1/3
    if (m_theme == "fruit") return 20;                       // 主题2
    if (m_theme == "star") return 18;                        // 主题4
    return 20;
}