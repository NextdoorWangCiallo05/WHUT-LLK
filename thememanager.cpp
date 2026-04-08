#include "thememanager.h"
#include <QSettings>
#include <QFile>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager ins;
    return ins;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    load();
}

QString ThemeManager::ensureThemeName(const QString& name) const
{
    QString n = name.trimmed().toLower();
    if (n.isEmpty()) return "classic";
    if (n != "classic" && n != "fruit" && n != "win" && n != "star") return "classic";
    return n;
}

bool ThemeManager::exists(const QString& qrcPath) const
{
    return QFile::exists(qrcPath);
}

void ThemeManager::load()
{
    QSettings s("YourCompany", "LLK_Refresh");
    m_theme = ensureThemeName(s.value("ui/theme", "classic").toString());
}

void ThemeManager::save()
{
    QSettings s("YourCompany", "LLK_Refresh");
    s.setValue("ui/theme", m_theme);
}

QString ThemeManager::currentTheme() const
{
    return m_theme;
}

void ThemeManager::setCurrentTheme(const QString& name)
{
    QString n = ensureThemeName(name);
    if (n == m_theme) return;
    m_theme = n;
    save();
    emit themeChanged();
}

QString ThemeManager::backgroundPathMain() const
{
    QString p = QString(":/images/%1/main.png").arg(m_theme);
    if (!exists(p)) p = ":/images/classic/main.png";
    return p;
}

QString ThemeManager::backgroundPathGame() const
{
    QString p = QString(":/images/%1/game.png").arg(m_theme);
    if (!exists(p)) p = ":/images/classic/game.png";
    return p;
}

QString ThemeManager::backgroundPathDialog() const
{
    QString p = QString(":/images/dialog.png");
    return p;
}

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

int ThemeManager::tileTypeCount() const
{
    if (m_theme == "classic" || m_theme == "win") return 12; // 主题1/3
    if (m_theme == "fruit") return 20;                       // 主题2
    if (m_theme == "star") return 18;                        // 主题4
    return 20;
}