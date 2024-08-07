#include "privatechat.h"
#include "ui_privatechat.h"
#include "tcpclient.h"
#include <QMessageBox>

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(pdu == NULL)return;
    QString strMsg = QString("%1 says:  %2").arg(pdu->caData + 32).arg(pdu->caMsg);
    ui->showMsgTE->append(strMsg);
}

void PrivateChat::clenrMsg()
{
    ui->showMsgTE->clear();
}

void PrivateChat::cleanInput()
{
    ui->inputsMsgLE->clear();
}

void PrivateChat::on_sendMsgPB_clicked()
{
    QString strMsg = ui->inputsMsgLE->text();
    if(strMsg.isEmpty())return;
    PDU *pdu = mkPDU(strMsg.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
    strcpy(pdu->caData, m_strChatName.toStdString().c_str());
    strcpy(pdu->caData + 32, TcpClient::getInstance().m_LoginName.toStdString().c_str());
    strcpy(pdu->caMsg, strMsg.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

