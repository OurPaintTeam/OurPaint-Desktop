//
// Created by Eugene Bychkov on 13.10.2024.
//

#ifndef OURPAINT_SERVER_H
#define OURPAINT_SERVER_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QSet>
#include <QDataStream>
#include "../painter/paint.h"

class Server : public QObject {
Q_OBJECT

private:
    QTcpServer *tcpServer;
    QSet<QTcpSocket *> clients;
    QString serverName;

public:
    Server(const QString& name = "Server", QObject *parent = nullptr);

    bool startServer(quint16 port);

    void setName(const QString& name);

    void sendToClients(const QString& paint);

    void sendChatToClients(const QString& msg, const QString& name);

    void stopServer();

signals:
    void newMessageReceived(const QString &msg, const QString &name);
    void newCommandReceived(const QString &command);
    void newConnection();
private slots:

    void onNewConnection();

    void onReadyRead();

    void onClientDisconnected();
};


#endif //OURPAINT_SERVER_H
