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
    void resend(const char *pername, PDU *pdu);
    void allsend(PDU *pdu);

signals:
    void offline(MyTcpSocket* mysocket);
    void resending(PDU *pdu);

public slots:
    void recvMsg();
    void clientOff();
    void onresend(PDU *pdu);

private:
    QString m_strName = "";
    QFile m_file;
    uint m_iTotal,m_iReceive;
    bool upState;

};

#endif // MYTCPSOCKET_H
