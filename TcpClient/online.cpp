#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

Online &Online::getInstance()
{
    static Online instance;
    return instance;
}

void Online::showUsr(PDU *pdu)
{
    if(pdu == NULL)return;
    uint uiSize = pdu->uiMsgLen / 32;
    char caTmp[32];
    ui->online_lw->clear();
    for(uint i=0; i<uiSize; i++)
    {
        memcpy(caTmp, pdu->caMsg + i*32, 32);
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/source/online.jpg"), caTmp, ui->online_lw);
        // item->setSizeHint(QSize(40, 40)); //设置item宽度
        ui->online_lw->addItem(item); //插入item
        // ui->online_lw->addItem(caTmp);
    }
}

void Online::showResult(const char *name, int state)
{
    ui->online_lw->clear();
    QListWidgetItem *item;
    if(state)item = new QListWidgetItem(QIcon(":/source/online.jpg"), name, Online::ui->online_lw);
    else item = new QListWidgetItem(QIcon(":/source/off.jpg"), name, Online::ui->online_lw);
    ui->online_lw->addItem(item);
    if(opWidget::getInstance().getFriend()->m_pOnline->isHidden())
    {
        opWidget::getInstance().getFriend()->m_pOnline->show();
    }
}

void Online::on_addFriend_pb_clicked()
{
    QListWidgetItem *pItem = ui->online_lw->currentItem();
    if(pItem == nullptr)return;
    QString PerUsrName = pItem->text();
    QString LoginName = TcpClient::getInstance().m_LoginName;
    if(PerUsrName == LoginName)return;
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, PerUsrName.toStdString().c_str(), PerUsrName.size());
    memcpy(pdu->caData + 32, LoginName.toStdString().c_str(), LoginName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

