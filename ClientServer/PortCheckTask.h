//
// Created by Eugene Bychkov on 29.10.2024.
//

#ifndef OURPAINT_PORTCHECKTASK_H
#define OURPAINT_PORTCHECKTASK_H


#include <QRunnable>
#include <QHostAddress>

class PortScanner;

class PortCheckTask : public QRunnable
{
public:
    PortCheckTask(PortScanner *scanner, const QHostAddress &address, quint16 port);

    void run() override;

private:
    PortScanner *m_scanner;
    QHostAddress m_address;
    quint16 m_port;
};

#endif //OURPAINT_PORTCHECKTASK_H
