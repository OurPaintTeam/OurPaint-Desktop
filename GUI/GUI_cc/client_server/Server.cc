//
// Created by Eugene Bychkov on 13.10.2024.
//

#include "Server.h"

Server::Server(const QString& name, QObject *parent) : QObject(parent), tcpServer(new QTcpServer(this)) {
    serverName = name;
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

bool Server::startServer(quint16 port) {
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "Server could not start!";
        return false;
    }
    qDebug() << "Server started!";
    return true;
}

void Server::sendToClients(const QString &command) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << command;

    for (QTcpSocket *client : clients) {
        client->write(data);
    }
}

void Server::onNewConnection() {
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    clients.insert(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);
    emit newConnection();
}

void Server::onReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    QDataStream in(clientSocket);

    while (!clientSocket->atEnd()) {
        QString command;
        in >> command;
        if (command.startsWith("CHAT|")) {
            QString message = command.mid(5);
            QStringList args = message.split("|");
            sendChatToClients(args[1], args[0]);
            emit newMessageReceived(args[1], args[0]);
        }else{
            sendToClients(command);
            emit newCommandReceived(command);
        }
    }
}
void Server::setName(const QString& name){
    serverName = name;
}

void Server::onClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        clients.remove(clientSocket);
        clientSocket->deleteLater();
    }
}

void Server::sendChatToClients(const QString &message, const QString &name){
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << QString("CHAT|%1|%2").arg(name, message);
    for (QTcpSocket *client : clients) {
        client->write(data);
    }
}
void Server::stopServer() {
    QString shutdownMsg = "STOP|";
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << shutdownMsg;
    for(auto client : clients) {
        client->write(data);
        client->flush();
        client->disconnectFromHost();
    }
    clients.clear();
    if(tcpServer->isListening()) {
        tcpServer->close();
        qDebug() << "Сервер остановлен.";
    }
}
