#include "tcpserver.h"
#include "opdb.h"
#include <QApplication>
#include <threadpool.h>

QMap<Qt::HANDLE, opDB*> opDB::databaseMap;
QList<MyTcpSocket*> MyTcpServer::m_tcpSocketList;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // ThreadPool::getInstance().setMode(PoolMode::MODE_CACHED);
    // ThreadPool::getInstance().start();
    // ThreadPool::init();
    // QThreadPool::globalInstance()->setMaxThreadCount(1024);
    TcpServer w;
    return a.exec();
}
