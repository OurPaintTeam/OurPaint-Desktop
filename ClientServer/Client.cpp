//
// Created by Eugene Bychkov on 13.10.2024.
//

#include "Client.h"
#include <QMessageBox>

Client::Client(QObject *parent) : QObject(parent), tcpSocket(new QTcpSocket(this)) {
    connect(tcpSocket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
}

void Client::connectToServer(const QString &ip, quint16 port) {
    tcpSocket->connectToHost(ip, port);
}

void Client::sendCommandToServer(const QString &command) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << command;
    tcpSocket->write(data);
}

void Client::onConnected() {
}
void Client::sendChatMessage(const QString &message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << QString("CHAT|%1").arg(message);
    tcpSocket->write(data);
}
void Client::onReadyRead() {
    QDataStream in(tcpSocket);
    while (!tcpSocket->atEnd()) {
        QString message;
        in >> message;

        if (message.startsWith("CHAT|")) {
            QString msg = message.mid(5);
            emit newChatMessageReceived(msg, "User");
        }
        else if (message == "SERVER_SHUTDOWN") {
            emit serverShutdown();
            tcpSocket->disconnectFromHost();
        }
        else {
            emit newStateReceived(message);
        }
    }
}

void Client::onDisconnected() {
    emit disconnectedFromServer();
}

void Client::disconnectFromServer() {
    tcpSocket->disconnectFromHost();
    emit onDisconnected();
}
