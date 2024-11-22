//
// Created by Eugene Bychkov on 13.10.2024.
//

#ifndef OURPAINT_CLIENT_H
#define OURPAINT_CLIENT_H

#include <QTcpSocket>
#include <QObject>
#include "paint.h"
#include <QMessageBox>
class Client : public QObject {
Q_OBJECT

public:
    Client(const QString& name="User", QObject *parent = nullptr);
    void setName(const QString& name);
    void connectToServer(const QString &ip, quint16 port);
    void sendCommandToServer(const QString &command);
    void sendChatMessage(const QString &message);
    void disconnectFromServer(); // Добавленный метод для ручного отключения

signals:
    void newChatMessageReceived(const QString& message, const QString& user);
    void newStateReceived(const QString& command);
    void disconnectedFromServer();
    void serverShutdown(); // Новый сигнал для обработки отключения сервера

private slots:
    void onConnected();

    void onReadyRead();
    void onDisconnected();

private:
    QString userName;
    QTcpSocket *tcpSocket;
};


#endif //OURPAINT_CLIENT_H
