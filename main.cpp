#include "mainwindow.h"
#include "audiomanager.h"
#include "thememanager.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 应用图标
    a.setWindowIcon(QIcon(":/images/app.ico"));

    auto pickThemeBgm = []() {
        const QString t = ThemeManager::instance().currentTheme();
        if (t == "classic") return QString(":/audio/1.mp3");
        if (t == "fruit")   return QString(":/audio/2.mp3");
        if (t == "win")     return QString(":/audio/3.mp3");
        if (t == "star")    return QString(":/audio/4.mp3");
        return QString(":/audio/1.mp3");
        };

    AudioManager::instance().init();
    AudioManager::instance().playBgm(pickThemeBgm());

    // 主题变化时自动切歌
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, &a, [=]() {
        AudioManager::instance().playBgm(pickThemeBgm());
        });

    MainWindow w;
    w.show();
    return a.exec();
}