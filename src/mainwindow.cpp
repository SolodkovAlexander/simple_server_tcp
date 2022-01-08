#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    server(new QTcpServer(this))
{
    ui->setupUi(this);

    //fill combo with addresses
    initComboAddress();

    //make connections
    connect(ui->buttonStart, &QPushButton::clicked, this, &MainWindow::startServer);
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onClientConnected);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected()
{
    QTcpSocket *socket = server->nextPendingConnection();
    log(QString("Client (%1:%2) connected.").arg(socket->peerAddress().toString(), QString::number(socket->peerPort())));
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onClientReadyRead);
    clients << socket;
}

void MainWindow::onClientDisconnected()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (!client)
        return;

    log(QString("Client (%1:%2) disconnected.").arg(client->peerAddress().toString(), QString::number(client->peerPort())));
    clients.removeOne(client);
}

void MainWindow::onClientReadyRead()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (!client)
        return;

    QByteArray messageBytes = client->readAll();
    log(QString("Message (\"%1\") [%2 bytes] received from client (%3:%4).")
        .arg(QString::fromUtf8(messageBytes),
             QString::number(messageBytes.count()),
             client->peerAddress().toString(),
             QString::number(client->peerPort())));

    //reply
    reply(QString("Server received message [%1 bytes]")
          .arg(messageBytes.count()),
          client);
}

void MainWindow::reply(const QString &message,
                       QTcpSocket *client)
{
    log(QString("Server started reply (\"%1\") [%2 bytes] to client (%3:%4).")
        .arg(message,
             QString::number(message.toUtf8().count()),
             client->peerAddress().toString(),
             QString::number(client->peerPort())));

    qint64 bytesWritten = client->write(message.toUtf8());
    log(QString("Server finished reply [%1 bytes].")
        .arg(bytesWritten));
}

void MainWindow::initComboAddress()
{
    for (const QHostAddress &address : QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol)
            ui->comboAddress->addItem(address.toString());
    }
}

void MainWindow::startServer()
{
    //stop server
    if (server->isListening())
        stopServer();

    //start server
    QHostAddress address = QHostAddress(ui->comboAddress->currentText());
    quint16 port = ui->editPort->text().toUShort();
    if (server->listen(address, port))
    {
        port = server->serverPort();
        log(QString("Server started listen to %1:%2.").arg(address.toString(), QString::number(port)));
    }
    else
    {
        log(QString("Error start to %1:%2 (%3)!").arg(address.toString(), QString::number(port), server->errorString()));
    }
}

void MainWindow::stopServer()
{
    //clear clients
    for (QTcpSocket *client : clients)
    {
        disconnect(client, &QTcpSocket::disconnected, this, &MainWindow::onClientDisconnected);
        client->disconnectFromHost();
        delete client;
    }
    clients.clear();

    //stop server
    QHostAddress address = server->serverAddress();
    quint16 port = server->serverPort();
    server->close();
    log(QString("Server stopped listen to %1:%2.").arg(address.toString(), QString::number(port)));
}

void MainWindow::log(const QString &message)
{
    ui->editLog->appendPlainText(message);
}
