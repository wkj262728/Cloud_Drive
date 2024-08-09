#include "friend.h"
#include "tcpclient.h"
#include <QInputDialog>
#include "privatechat.h"

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendLW = new QListWidget;
    m_pInputMsgLE = new QLineEdit;
    m_pShowOnlinePB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pPrivateChatPB = new QPushButton("私聊");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pOnline = new Online;

    QFont font(QStringLiteral("HGHT_CNKI"), 18);
    m_pFriendLW->setFont(font);
    m_pShowMsgTE->setEnabled(false);

    QVBoxLayout *pLeftPBVBL = new QVBoxLayout;
    pLeftPBVBL->addWidget(m_pDelFriendPB);
    pLeftPBVBL->addWidget(m_pFlushFriendPB);
    pLeftPBVBL->addWidget(m_pShowOnlinePB);
    pLeftPBVBL->addWidget(m_pSearchUsrPB);
    // pLeftPBVBL->addWidget(m_pMsgSendPB);
    pLeftPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopPBHBL = new QHBoxLayout;
    pTopPBHBL->addWidget(m_pShowMsgTE);
    pTopPBHBL->addWidget(m_pFriendLW);
    pTopPBHBL->addLayout(pLeftPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopPBHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlinePB, SIGNAL(clicked(bool))
            , this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(delFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool))
            , this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool))
            , this, SLOT(Chat()));

}

void Friend::showAllOnline(PDU *pdu)
{
    if(pdu == NULL)return;
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriend(PDU *pdu)
{
    if(pdu == NULL)return;
    uint uiSize = pdu->uiMsgLen / 32;
    m_pFriendLW->clear();
    QListWidgetItem *item;
    for(uint i=0; i<uiSize; i++)
    {
        if(pdu->caData[i] == '1')item = new QListWidgetItem(QIcon(":/source/online.jpg"), pdu->caMsg + i*32, m_pFriendLW);
        else item = new QListWidgetItem(QIcon(":/source/off.jpg"), pdu->caMsg + i*32, m_pFriendLW);
        m_pFriendLW->addItem(item);
    }
}

void Friend::delFriend()
{
    if(m_pFriendLW->currentItem() == NULL)return;
    QString strName = m_pFriendLW->currentItem()->text();
    PDU *pdu = mkPDU(0);
    strcpy(pdu->caData, strName.toStdString().c_str());
    strcpy(pdu->caData + 32, TcpClient::getInstance().m_LoginName.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_RFIEND_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::privateChat()
{
    if(m_pFriendLW->currentItem() != NULL)
    {
        PrivateChat::getInstance().setChatName(m_pFriendLW->currentItem()->text());
        if(PrivateChat::getInstance().isHidden())PrivateChat::getInstance().show();
        PrivateChat::getInstance().setWindowTitle(QString("与 %1% 的私聊").arg(m_pFriendLW->currentItem()->text()));
    }
}

void Friend::Chat()
{
    QString strMsg = m_pInputMsgLE->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_CHAT_REQUEST;
        strcpy(pdu->caData, TcpClient::getInstance().m_LoginName.toStdString().c_str());
        strcpy(pdu->caMsg, strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    m_pInputMsgLE->clear();
}

void Friend::updateMsg(const PDU *pdu)
{
    if(pdu == NULL)return;
    QString strMsg = QString("%1 says:  %2").arg(pdu->caData).arg(pdu->caMsg);
    m_pShowMsgTE->append(strMsg);
}



void Friend::showResult(const char *name, int state)
{
    m_pOnline->showResult(name, state);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendLW;
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    if(!m_strSearchName.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.toUtf8().size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_RFIEND_REQUEST;
    strcpy(pdu->caData, TcpClient::getInstance().m_LoginName.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}
