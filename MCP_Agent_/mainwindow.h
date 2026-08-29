#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateUptime();

private:
    Ui::MainWindow *ui;
    QLabel *uptimeLabel;
    QElapsedTimer startTime;
    QTimer *uptimeTimer;
};

#endif // MAINWINDOW_H