#include "mainwindow.h"
#include "audiomanager.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 应用图标
    a.setWindowIcon(QIcon(":/images/app.ico"));

    AudioManager::instance().init();
    AudioManager::instance().playBgm(":/audio/bgm.mp3");

    MainWindow w;
    w.show();
    return a.exec();
}