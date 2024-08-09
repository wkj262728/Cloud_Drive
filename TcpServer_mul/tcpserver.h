#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"
#include "opdb.h"
#include "protocol.h"
#include "ui_tcpserver.h"



class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer()
    {
        opDB::getInstance()->serverShutdown();
    }
    void loadConfig();

private:

    QString m_strIP;
    quint16 m_usPort;
};
#endif // TCPSERVER_H
