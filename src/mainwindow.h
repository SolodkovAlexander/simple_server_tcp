#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QTcpServer;
class QTcpSocket;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void onClientConnected();
    void onClientDisconnected();
    void onClientReadyRead();
    void reply(const QString &message,
               QTcpSocket *client = Q_NULLPTR);

private:
    void initComboAddress();
    void startServer();
    void stopServer();
    void log(const QString &message);

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QList<QTcpSocket *> clients;
};

#endif // MAINWINDOW_H
