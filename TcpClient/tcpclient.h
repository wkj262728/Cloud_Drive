#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opwidget.h"
#include "friend.h"
#include <QProgressDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    void startUpdate(const uint &num);

    static TcpClient& getInstance();
    QTcpSocket &getTcpSocket();
    QString m_LoginName;
    QString CurrentPath;
    QString updateFilePath,downloadPath;
    uint m_iTotal,m_ireceive;
    bool downloadState;

public slots:
    void showConnect();
    void recvMsg();
    void serverOff();
    void check();

private slots:
    // void on_send_pd_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    int cnt=0;
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    bool flag;
    QFile m_file;
    QProgressDialog *progressDialog;

    QTcpSocket m_tcpSocket;
};
#endif // TCPCLIENT_H
