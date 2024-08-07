#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opdb.h"
#include <QDir>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();
    void startDownload();
    void handleShareFileRequest(PDU* pdu, QString strSouName);
    void handleShareFileNoteRespond(PDU *pdu);
    bool copyDir(QString strOldPath, QString strNewPath);

signals:
    void offline(MyTcpSocket* mysocket);

public slots:
    void recvMsg();
    void clientOff();

private:
    QString m_strName;
    QFile m_file;
    uint m_iTotal,m_iReceive;
    bool upState;

};

#endif // MYTCPSOCKET_H
