#include "mainwindow.h"
#include "tgbot.h"
#include <QApplication>
#include "QLabel"
#include <QSslSocket>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qInfo() << "SSL:" << QSslSocket::supportsSsl() << QSslSocket::sslLibraryVersionString();

    TgBot bot;
    w.show();
    return a.exec();
}
