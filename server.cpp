#include "server.h"

int random(int min, int max)
{
    static std::mt19937 gen(time(NULL));
    std::uniform_int_distribution<> uid(min, max);
    return uid(gen);
}


Logger::Logger()
{
    this->start_new_log();
}

Logger::~Logger()
{
    this->end_log();
}

void Logger::add(std::string text)
{
    logging_stream.open(PATH, std::ios::app);
    if (logging_stream.is_open())
    {
        logging_stream << "[" <<  QTime::currentTime().toString("HH:mm:ss:zzz").toStdString() << "] >> " << text << std::endl;     
    }
    logging_stream.close();
}

void Logger::start_new_log()
{
    logging_stream.open(PATH, std::ios::app);
    if (logging_stream.is_open())
    {
        logging_stream << "\n|------------[ New log, "<< QDateTime::currentDateTime().toString("dd.MM.yyyy").toStdString() <<" ]-------------|\n";
    }
    logging_stream.close();
}

void Logger::end_log()
{
    logging_stream.open(PATH, std::ios::app);
    if (logging_stream.is_open())
    {
        logging_stream << "|-----------------[ End of log ]-----------------|\n";
    }
    logging_stream.close();
}

Server::Server():nextBlockSize(0), count(0),connections(0)
{
    this->tcpServer = new QTcpServer(this);
    this->tcpServer->setMaxPendingConnections(2);
    this->tcpSocket = nullptr;
}

Server::~Server()
{   
    delete this->tcpServer;
    emit addText("Server's destructor of class is called!");
}

void Server::startServer()
{
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::newConnection);
    if (!this->tcpServer->listen(QHostAddress::Any, this->port))
        emit addText("Server is not started");
    else
    {
        emit addText("Server is started successfully!");
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
                emit addText("Your server " + address.toString() + ":" + QString::number(this->port));
        }
    }
}

void Server::setPort(uint16_t port)
{
    this->port = port;
}

void Server::close()
{
    this->tcpServer->close();
}

void Server::newConnection()
{
    this->connections++;
    this->tcpSocket = this->tcpServer->nextPendingConnection();

    connect(tcpSocket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &Server::clientDisconnected);
    //connect(tcpSocket, &QTcpSocket::disconnected, tcpSocket, &QTcpSocket::deleteLater);

    this->sockets.push_back({this->tcpSocket,""});

    emit addText("New connection! Count of connections " + QString::number(this->connections));
    emit addText("Ip of incoming connection >> " + this->tcpSocket->peerAddress().toString());
}

void Server::clientDisconnected()
{  
    this->connections--;
    emit addText("Client disconnected! Count of connections " + QString::number(this->connections));
    emit addText("Ip of disconnected Client >> " + this->tcpSocket->peerAddress().toString());

    auto ind = this->getIndexOfSocket(this->tcpSocket);
    if (this->connections != 0)
        this->sendCommandToClientByIndex("ClientDisconnected",1-ind);
    emit addText("deleting");
    //this->tcpSocket->close();
    //this->tcpSocket->deleteLater();
    //this->sockets[i].socket->close();
    //this->sockets[ind].socket->deleteLater();
    this->sockets.remove(ind);
    emit addText("deleted");
    this->count = 0;
}
void Server::slotReadyRead()
{
    this->tcpSocket = (QTcpSocket*)sender();
    QDataStream in(this->tcpSocket);
    in.setVersion(QDataStream::Qt_6_4);
    if (in.status() == QDataStream::Ok)
    {
        emit addText("Read message from client...");
        for (;;)
        {
            if (this->nextBlockSize == 0)
            {
                emit addText("nextBlockSize = 0");
                if (this->tcpSocket->bytesAvailable() < 2)
                {
                    emit addText("data < 2, break");
                    break;
                }
                in >> this->nextBlockSize;
                emit addText("nextBlockSize = " + QString::number(this->nextBlockSize));
            }
            if (this->tcpSocket->bytesAvailable() < this->nextBlockSize)
            {
                emit addText("data is not full, break");
                break;
            }
            quint8 typeOfData;
            QString str;
            this->nextBlockSize = 0;
            in >> typeOfData;
            emit addText("type " + QString::number(typeOfData));
            switch (typeOfData)
            {
                case 0:
                {
                    QTime time;
                    in >> time >> str;
                    emit addText(str);
                    this->sendToClient(str);
                    break;
                }
                case 1:
                {
                    in >> str;
                    QStringList pieces = str.split(" ");
                    if (pieces.at(0) == "SetNickname")
                    {
                        for (auto &pair : this->sockets)
                            if (pair.socket->peerAddress() == this->tcpSocket->peerAddress()
                                    && pair.socket->peerPort() == this->tcpSocket->peerPort())
                            {
                                    pair.nickname = pieces[1].remove(0,1);
                                    pair.nickname.remove(pair.nickname.size()-1,pair.nickname.size());
                                    this->count++;
                                    emit addText("/Set nickname " + pair.nickname + " (" + QString::number(this->count)+')');
                                    break;
                            }
                        if (this->count == 2)
                        {
                            this->sendCommandToClients("CanPlaceShips");
                            emit addText("/CanPlaceShips");
                            this->count=0;
                            break;
                        }
                    }
                    else if (pieces.at(0) == "ShipsPlaced")
                    {
                        this->count++;
                        emit addText("/Ships Placed (" + QString::number(this->count)+')');
                        if (this->count == 2)
                        {
                            emit addText("/CanPlay");
                            this->count=0;
                            int firstPlayer = random(0,1);
                            this->sendCommandToClientByIndex("FirstPlayer", firstPlayer);
                            this->sendCommandToClientByIndex("SecondPlayer", 1-firstPlayer);
                            break;
                        }
                    }
                    else if (pieces.at(0) == "Shoot")
                    {
                        emit addText("/Shoot " + pieces.at(1) + ' ' + pieces.at(2));
                        this->sendCommandToClientByIndex(str, 1-this->getIndexOfSocket(this->tcpSocket));
                    }
                    else if (pieces.at(0) == "ShotResult")
                    {
                        emit addText("/ShotResult " + pieces.at(1));
                        this->sendCommandToClientByIndex(str, 1-this->getIndexOfSocket(this->tcpSocket));
                    }
                    else if (pieces.at(0) == "IncorrectCoordsOfAttack")
                    {
                        emit addText("/IncorrectCoordsOfAttack");
                        this->sendCommandToClientByIndex(str, 1-this->getIndexOfSocket(this->tcpSocket));
                    }
                    else if (pieces.at(0) == "PassMove")
                    {
                        this->sendCommandToClientByIndex(str, 1-this->getIndexOfSocket(this->tcpSocket));
                    }
                    else
                    {
                        emit addText("Unknown command from client >> " + pieces.at(0));
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }

    }
    else
        emit addText("DataStream error occured!");
}

void Server::sendToClient(QString str)
{
    for (auto &pair : this->sockets)
    {
        if (pair.socket->peerAddress() == this->tcpSocket->peerAddress()
                && pair.socket->peerPort() == this->tcpSocket->peerPort())
        {
            if (pair.nickname.isEmpty())
                str = ", [ !NicknameIsEmpty! ] " + str;
            else
                str = ", [" + pair.nickname + "] " + str;
            break;
        }
    }
    this->data.clear();
    QDataStream out(&this->data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << quint16(0) << quint8(0) << QTime::currentTime() << str;
    out.device()->seek(0);
    out << quint16(data.size()-sizeof(quint16));
    for (auto &pair : this->sockets)
        pair.socket->write(this->data);
}

void Server::prepareDataCommand(QString str)
{
    this->data.clear();
    QDataStream out(&this->data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << quint16(0) << quint8(1) << str;
    out.device()->seek(0);
    out << quint16(data.size()-sizeof(quint16));
}

void Server::sendCommandToClients(QString str)
{
    this->prepareDataCommand(str);
    for (auto &pair : this->sockets)
        pair.socket->write(this->data);
}

void Server::sendCommandToClientByIndex(QString str, int ind)
{
    this->prepareDataCommand(str);
    this->sockets.at(ind).socket->write(this->data);
}

quint16 Server::getIndexOfSocket(QTcpSocket *socket)
{
    for (auto i = 0; i < this->sockets.size(); i++)
        if (this->sockets[i].socket->peerAddress() == socket->peerAddress()
                && this->sockets[i].socket->peerPort() == socket->peerPort())
        {
            return i; break;
        }
    return 0;
}

