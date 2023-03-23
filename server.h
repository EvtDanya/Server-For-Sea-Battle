#pragma once
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QObject>
#include <QNetworkInterface>
#include <QDateTime>
#include <QVector>
#include <QDataStream>
#include <QHostInfo>
#include <QTime>

#include <random>
#include <ctime>
#include <iostream>
#include <fstream>

constexpr auto PATH = "server_logs.txt";

int random(int min, int max);

class Logger
{
private:
    std::ofstream logging_stream;

public:
    Logger();
    ~Logger();

    void add(std::string);

    void start_new_log();
    void end_log();
};

struct Socket
{
    QTcpSocket *socket;
    QString nickname;
};

class Server: public QTcpServer
{
    Q_OBJECT
public:
    Server();
    ~Server();

    void startServer();
    void setPort(uint16_t port);
    void close();

public slots:
    void newConnection();
    void clientDisconnected();
    void slotReadyRead();

signals:
    void addText(QString);
    void getPort();

private:
    uint16_t port;
    QByteArray data;

    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;

    QVector<Socket> sockets;

    void sendToClient(QString);
    void prepareDataCommand(QString);
    void sendCommandToClients(QString);
    void sendCommandToClientByIndex(QString, int);
    quint16 getIndexOfSocket(QTcpSocket *);
    quint16 nextBlockSize;
    quint8 count;
    quint8 connections;
};

