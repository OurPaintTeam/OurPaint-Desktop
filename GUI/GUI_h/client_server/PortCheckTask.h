#ifndef OURPAINT_HEADERS_CLIENTSERVER_PORTCHECKTASK_H_
#define OURPAINT_HEADERS_CLIENTSERVER_PORTCHECKTASK_H_


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

#endif // ! OURPAINT_HEADERS_CLIENTSERVER_PORTCHECKTASK_H_
