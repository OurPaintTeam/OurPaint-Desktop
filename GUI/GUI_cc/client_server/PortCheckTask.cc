#include "PortCheckTask.h"
#include "LocalScanner.h"
#include <QTcpSocket>

PortCheckTask::PortCheckTask(PortScanner *scanner, const QHostAddress &address, quint16 port)
        : m_scanner(scanner), m_address(address), m_port(port)
{
}

void PortCheckTask::run()
{
    QTcpSocket socket;
    socket.connectToHost(m_address, m_port);
    if (socket.waitForConnected(20))
    {
        m_scanner->addAvailableHost(m_address.toString());
        socket.disconnectFromHost();
    }
}