#ifndef OURPAINT_HEADERS_CLIENTSERVER_PORTSCANNER_H_
#define OURPAINT_HEADERS_CLIENTSERVER_PORTSCANNER_H_

#include <QObject>
#include <QStringList>
#include <QHostAddress>
#include <QMutex>

class PortScanner : public QObject
{
Q_OBJECT

public:
    explicit PortScanner(QObject *parent = nullptr, quint16 port = 2005);
    QStringList scanNetwork();
    void addAvailableHost(const QString &host);

private:
    quint16 m_port;
    QStringList m_availableHosts;
    QMutex m_mutex;

    QList<QPair<QHostAddress, int>> getLocalNetworks();
};



#endif // ! OURPAINT_HEADERS_CLIENTSERVER_PORTSCANNER_H_