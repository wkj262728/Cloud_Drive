#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"
#include "opdb.h"
#include "protocol.h"
#include "ui_tcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpServer;
}
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer()
    {
        opDB::getInstance().serverShutdown();
        delete ui;
    }
    void loadConfig();

private:
    Ui::TcpServer *ui;

    QString m_strIP;
    quint16 m_usPort;
};
#endif // TCPSERVER_H
