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
    QObject::connect(&w, &MainWindow::messageSubmitted, &bot, &TgBot::handleUiMessage);
    QObject::connect(&bot, &TgBot::uiReplyReady, &w, &MainWindow::receiveAgentReply);
    w.show();
    return a.exec();
}
