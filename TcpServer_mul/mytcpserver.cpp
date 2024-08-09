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
    // MyWork *task = new MyWork(socketDescriptor);
    // QThreadPool::globalInstance()->start(task);
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    QThread *thread = new QThread();
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    MyTcpServer::getInstance().m_tcpSocketList.append(pTcpSocket);
    pTcpSocket->moveToThread(thread);
    thread->start();
    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*))
            , this, SLOT(offSocket(MyTcpSocket*)));
}

void MyWork::run()
{
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor_);
    MyTcpServer::getInstance().m_tcpSocketList.append(pTcpSocket);
    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*))
            , &MyTcpServer::getInstance(), SLOT(offSocket(MyTcpSocket*)));
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
