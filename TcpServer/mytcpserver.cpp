#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer() {}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);
    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*))
            , this, SLOT(offSocket(MyTcpSocket*)));
}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if(pername == NULL || pdu == NULL)return;
    for(auto it:m_tcpSocketList){
        if(pername == it->getName())
        {
            it->write((char*)pdu, pdu->uiPDULen);
            it->waitForBytesWritten();
            break;
        }
    }
}

void MyTcpServer::allsend(PDU *pdu)
{
    if(pdu == NULL)return;
    for(auto it:m_tcpSocketList){
        it->write((char*)pdu, pdu->uiPDULen);
    }
}

void MyTcpServer::offSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(; iter != m_tcpSocketList.end(); iter++)
    {
        if(mysocket == *iter)
        {
            disconnect(mysocket, 0, 0, 0);
            mysocket->close();
            (*iter)->deleteLater();
            // delete *iter;
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
}
