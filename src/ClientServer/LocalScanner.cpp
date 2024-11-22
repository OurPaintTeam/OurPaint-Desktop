//
// Created by Eugene Bychkov on 29.10.2024.
//

#include "LocalScanner.h"
#include "PortCheckTask.h"
#include <QThreadPool>
#include <QHostAddress>
#include <QNetworkInterface>

PortScanner::PortScanner(QObject *parent, quint16 port)
        : QObject(parent), m_port(port)
{
}

void PortScanner::addAvailableHost(const QString &host)
{
    QMutexLocker locker(&m_mutex);
    m_availableHosts.append(host);
}

QStringList PortScanner::scanNetwork()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QString lastAddress;
    m_availableHosts.clear();
    for (const QHostAddress &address : addresses){
        if (address.protocol() != QAbstractSocket::IPv4Protocol ||
            address.isLoopback()){
            continue;
        }
        QStringList parts = address.toString().split(".");
        for (int j = parts[3].toInt() - 10; j <= parts[3].toInt() + 10; ++j)
        {
            if (j == parts[3].toInt()){
                continue;
            }
            QString ipString = QString("192.168.%1.%2").arg(parts[2]).arg(j);
            QHostAddress hostAddress(ipString);
            PortCheckTask task(this, hostAddress, m_port);
            task.run();
       }
    }
    return m_availableHosts;
}