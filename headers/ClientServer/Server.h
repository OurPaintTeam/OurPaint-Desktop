#ifndef OURPAINT_HEADERS_CLIENTSERVER_SERVER_H_
#define OURPAINT_HEADERS_CLIENTSERVER_SERVER_H_


#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QSet>
#include <QDataStream>
#include "paint.h"

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


#endif // ! OURPAINT_HEADERS_CLIENTSERVER_SERVER_H_
