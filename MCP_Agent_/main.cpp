#include "mainwindow.h"
#include "tgbot.h"
#include <QApplication>
#include "QLabel"
#include <QSslSocket>
int main(int argc, char *argv[])
{
    qputenv("QT_FORCE_STDERR_LOGGING", "1");
    QApplication a(argc, argv);
    MainWindow w;
    qInfo() << "SSL:" << QSslSocket::supportsSsl() << QSslSocket::sslLibraryVersionString();

    TgBot bot;
    w.show();
    return a.exec();
}
