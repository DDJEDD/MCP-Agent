#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("System Status");
    resize(380, 200);


    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QFrame *card = new QFrame(this);
    card->setStyleSheet(
        "QFrame {"
        "   background-color: #1e1e2e;"
        "   border-radius: 12px;"
        "   border: 1px solid #313244;"
        "}"
        );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(15);
    cardLayout->setContentsMargins(20, 20, 20, 20);

    QHBoxLayout *statusLayout = new QHBoxLayout();

    QLabel *statusDot = new QLabel("●", this);
    statusDot->setStyleSheet("color: #a6e3a1; font-size: 20px; border: none; background: transparent;");

    QLabel *statusText = new QLabel("Bot / Service is Active", this);
    statusText->setStyleSheet("color: #cdd6f4; font-size: 16px; font-weight: bold; border: none; background: transparent;");

    statusLayout->addWidget(statusDot);
    statusLayout->addWidget(statusText);
    statusLayout->addStretch();


    uptimeLabel = new QLabel("Uptime: 00:00:00", this);
    uptimeLabel->setStyleSheet("color: #a6adc8; font-size: 13px; border: none; background: transparent;");


    QLabel *startedAtLabel = new QLabel("Started: " + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy"), this);
    startedAtLabel->setStyleSheet("color: #6c7086; font-size: 11px; border: none; background: transparent;");

    cardLayout->addLayout(statusLayout);
    cardLayout->addWidget(uptimeLabel);
    cardLayout->addWidget(startedAtLabel);


    mainLayout->addWidget(card);
    setCentralWidget(centralWidget);

    setStyleSheet("QMainWindow { background-color: #11111b; }");


    startTime.start();
    uptimeTimer = new QTimer(this);
    connect(uptimeTimer, &QTimer::timeout, this, &MainWindow::updateUptime);
    uptimeTimer->start(1000);
}

void MainWindow::updateUptime()
{
    qint64 secs = startTime.elapsed() / 1000;
    int hours = secs / 3600;
    int mins = (secs % 3600) / 60;
    int seconds = secs % 60;

    QString timeStr = QString("Uptime: %1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(mins, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'));

    uptimeLabel->setText(timeStr);
}

MainWindow::~MainWindow()
{
    delete ui;
}