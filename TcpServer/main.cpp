#include "tcpserver.h"
#include "opdb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    opDB::getInstance().init();
    TcpServer w;
    w.show();
    return a.exec();
}
