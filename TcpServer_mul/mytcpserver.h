#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"
#include <QThreadPool>
#include <QThread>
#include <QRunnable>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
    void buildConnection(qintptr socketDescriptor);

    static QList<MyTcpSocket*> m_tcpSocketList;

public slots:
    void offSocket(MyTcpSocket* mysocket);
};

class MyWork :public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MyWork(qintptr socket)
        : socketDescriptor_(socket)
    {
        setAutoDelete(false);
    }

    void run() override;
    qintptr socketDescriptor_;
};


#endif // MYTCPSERVER_H
