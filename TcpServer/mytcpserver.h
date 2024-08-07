#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char *pername, PDU *pdu);
    void allsend(PDU *pdu);

    QList<MyTcpSocket*> m_tcpSocketList;

public slots:
    void offSocket(MyTcpSocket* mysocket);
};

#endif // MYTCPSERVER_H
