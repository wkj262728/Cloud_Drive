#ifndef UPDATEFILE_H
#define UPDATEFILE_H

#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include <QWidget>
// #include <QProgressDialog>

class UpdateFile : public QObject
{
    Q_OBJECT

public:
    UpdateFile(PDU *pdu);
    void startUpdate(const uint &num);

signals:
    void startDialog(const uint &num);
    void setDialog(const uint &num);
    void endDialog();

public slots:
    void showConnect();
    void recvMsg();
    void init();

private:
    QTcpSocket *tcpSocket;
    PDU *initpdu;
    // QProgressDialog *progressDialog;
};

#endif // UPDATEFILE_H
