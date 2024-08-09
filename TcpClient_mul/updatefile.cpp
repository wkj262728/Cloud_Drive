#include "updatefile.h"
#include "tcpclient.h"
#include <QProgressDialog>
#include <QThread>

UpdateFile::UpdateFile(PDU *pdu)
    : initpdu(pdu)
{

}

void UpdateFile::init()
{
    tcpSocket = new QTcpSocket;
    connect(tcpSocket, SIGNAL(connected()),
            this, SLOT(showConnect()));
    connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));
    tcpSocket->connectToHost(QHostAddress(TcpClient::getInstance().m_strIP),TcpClient::getInstance().m_usPort);
}

void UpdateFile::showConnect()
{
    tcpSocket->write((char*)initpdu, initpdu->uiPDULen);
    free(initpdu);
    initpdu = nullptr;
}

void UpdateFile::startUpdate(const uint &num)
{
    QFile file(TcpClient::getInstance().updateFilePath);
    connect(this, SIGNAL(startDialog(uint)),
            &TcpClient::getInstance(), SLOT(onstartDialog(uint)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setDialog(uint)),
            &TcpClient::getInstance(), SLOT(onsetDialog(uint)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(endDialog()),
            &TcpClient::getInstance(), SLOT(onendDialog()), Qt::BlockingQueuedConnection);

    if(!file.open(QIODevice::ReadOnly))
    {
        file.close();
        return;
    }
    char *pBuffer = new char[4096];
    int ret = 0;
    emit(startDialog(num));
    for(uint i=0; i<=num; i+=4096)
    {
        ret = file.read(pBuffer, 4096);
        if(ret == 0)break;
        tcpSocket->write(pBuffer, ret);
        emit(setDialog(i));
        tcpSocket->waitForBytesWritten();
        if(TcpClient::getInstance().progressDialog->wasCanceled())
        {
            break;
        }
    }
    emit(endDialog());
    file.close();
    delete[](pBuffer);
    pBuffer = nullptr;
}

void UpdateFile::recvMsg()
{
    qDebug()<<QThread::currentThreadId();
    uint uiPDULen = 0;
    tcpSocket->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    tcpSocket->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_UPDATE_RESPOND:
        {

            startUpdate(QString(pdu->caData).toInt());
            break;
        }
        case ENUM_MSG_TYPE_UPDATE_SUCCESS_RESPOND:
        {
            tcpSocket->disconnectFromHost();
            disconnect(this, 0 ,0, 0);
            QThread::currentThread()->exit(0);
            break;
        }
        default:
            break;
    }
    free(pdu);
    pdu = NULL;
}

