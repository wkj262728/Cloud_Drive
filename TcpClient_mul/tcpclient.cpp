#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <opwidget.h>
#include <QListWidget>
#include <messagetips.h>
#include "privatechat.h"
#include <QThread>
#include <QWaitCondition>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(500, 250);
    loadConfig();
    downloadState  = false;
    connect(&m_tcpSocket, SIGNAL(connected()),
            this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));
    connect(&m_tcpSocket, SIGNAL(disconnected())
            , this, SLOT(serverOff()));
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly)){
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\r\n"," ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
    }
    else{
        QMessageBox::critical(this,"open config","open config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

void TcpClient::showConnect()
{
    MessageTips *mMessageTips = new MessageTips("连接服务器成功", this);
    mMessageTips->show();
}

void TcpClient::recvMsg()
{
    if(downloadState)
    {
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        m_ireceive += buffer.size();
        progressDialog->setValue(m_ireceive);
        if(m_ireceive == m_iTotal)
        {
            downloadState = false;
            m_file.close();
            delete progressDialog;
            progressDialog = nullptr;
            m_ireceive = 0;
            m_file.close();
            MessageTips *mMessageTips = new MessageTips("文件下载成功", this);
            mMessageTips->show();
        }
        return;
    }
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    switch (pdu->uiMsgType) {
        case SERVER_SHUTDOWN:
        {
            QMessageBox::information(this, "wanring", "服务器断开连接");
            break;
        }
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(!strcmp(pdu->caData, "regist ok"))
            {
                QMessageBox::information(this, "注册", "注册成功");
            }
            else
            {
                QMessageBox::warning(this, "注册", "注册失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(!strcmp(pdu->caData, "login ok"))
            {
                MessageTips *mMessageTips = new MessageTips("登陆成功", this);
                mMessageTips->show();
                CurrentPath = QString("./file/%1").arg(m_LoginName);
                opWidget::getInstance().show();
                hide();
            }
            else
            {
                MessageTips *mMessageTips = new MessageTips("登录失败", this);
                mMessageTips->show();
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            opWidget::getInstance().getFriend()->showAllOnline(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            if(!strcmp(pdu->caData, "Not find"))
            {
                QMessageBox::information(this, "搜素", QString("%1: 用户不存在").arg(opWidget::getInstance().getFriend()->m_strSearchName));
            }
            else
            {
                if(!strcmp(pdu->caData, "online"))opWidget::getInstance().getFriend()->showResult(opWidget::getInstance().getFriend()->m_strSearchName.toStdString().c_str(), 1);
                else opWidget::getInstance().getFriend()->showResult(opWidget::getInstance().getFriend()->m_strSearchName.toStdString().c_str(), 0);
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            int ret = QMessageBox::information(this, "添加好友", QString("%1 想要和你成为好友").arg(pdu->caData + 32)
                                     , QMessageBox::Yes, QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 64);
            if(ret == QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_AGREE_FRIEND_RESPOND;
                QListWidgetItem *item = new QListWidgetItem(QIcon(":/source/online.jpg"), pdu->caData + 32, opWidget::getInstance().getFriend()->m_pFriendLW);
                opWidget::getInstance().getFriend()->m_pFriendLW->addItem(item);
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_REFUSE_FRIEND_RESPOND;
            }
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            MessageTips *mMessageTips = new MessageTips(pdu->caData, this);
            mMessageTips->show();
            break;
        }
        case ENUM_MSG_TYPE_AGREE_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "添加好友", QString("%1 同意了您的好友请求").arg(pdu->caData));
            QListWidgetItem *item = new QListWidgetItem(QIcon(":/source/online.jpg"), pdu->caData, opWidget::getInstance().getFriend()->m_pFriendLW);
            opWidget::getInstance().getFriend()->m_pFriendLW->addItem(item);
            break;
        }
        case ENUM_MSG_TYPE_REFUSE_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "添加好友", QString("%1 拒绝了您的好友请求").arg(pdu->caData));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_RFIEND_RESPOND:
        {
            opWidget::getInstance().getFriend()->updateFriend(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_RFIEND_RESPOND:
        {
            MessageTips *mMessageTips = new MessageTips("删除好友成功", this);
            mMessageTips->show();
            QListWidgetItem* item = opWidget::getInstance().getFriend()->m_pFriendLW->currentItem();
            opWidget::getInstance().getFriend()->m_pFriendLW->removeItemWidget(item);
            delete item;
            item = nullptr;
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND:
        {
            if(pdu->uiMsgLen == 0)
            {
                MessageTips *mMessageTips = new MessageTips("对方不在线，消息发送失败", this);
                mMessageTips->show();
            }
            else PrivateChat::getInstance().updateMsg(pdu);
            PrivateChat::getInstance().cleanInput();
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CHAT_REQUEST:
        {
            opWidget::getInstance().getFriend()->updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREAT_DIR_RESPOND:
        {
            if(!strcmp(pdu->caData, "文件名已存在"))
            {
                MessageTips *mMessageTips = new MessageTips(pdu->caData, this);
                mMessageTips->show();
            }
            else opWidget::getInstance().getBook()->addDir(pdu->caData + 32);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            opWidget::getInstance().getBook()->showFile(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_RESPOND:
        {
            opWidget::getInstance().getBook()->removeDir();
            break;
        }
        case ENUM_MSG_TYPE_RENAME_RESPOND:
        {
            if(!strcmp(pdu->caData, "文件名已存在"))
            {
                MessageTips *mMessageTips = new MessageTips(pdu->caData, this);
                mMessageTips->show();
            }
            else opWidget::getInstance().getBook()->changename(QString(pdu->caData));
            break;
        }
        case ENUM_MSG_TYPE_FILE_INTO_RESPOND:
        {
            if(pdu->caData[0] == 'g')break;
            opWidget::getInstance().getBook()->showFile(pdu);
            CurrentPath = QString("%1/%2").arg(CurrentPath).arg(pdu->caData + 32);
            break;
        }
        case ENUM_MSG_TYPE_FILE_BACK_RESPOND:
        {
            opWidget::getInstance().getBook()->showFile(pdu);
            break;
        }

        case ENUM_MSG_TYPE_DOWNLOAD_RESPOND:
        {
            if(pdu->caData[61] == 'g')
            {
                downloadState = false;
                QMessageBox::critical(this, "下载文件", "网盘文件打开失败");
                break;
            }
            if(pdu->caData[61] == 'n')
            {
                downloadState = false;
                QMessageBox::critical(this, "下载文件", "无法下载文件夹");
                break;
            }
            downloadState = true;
            m_iTotal = QString("%1").arg(pdu->caData).toUInt();
            PDU *respdu = mkPDU(0);
            QFont font("ZYSong18030", 12);
            progressDialog = new QProgressDialog(opWidget::getInstance().getBook());
            progressDialog->setFont(font);
            progressDialog->setWindowModality(Qt::WindowModal);
            progressDialog->setMinimumDuration(5);
            progressDialog->setWindowTitle(tr("Please Wait"));
            progressDialog->setLabelText(tr("Downloading..."));
            progressDialog->setCancelButtonText(tr("Cancel"));
            progressDialog->setRange(0, m_iTotal);
            respdu->uiMsgType = ENUM_MSG_TYPE_START_DOWNLOAD_REQUEST;
            m_file.setFileName(downloadPath);
            m_file.open(QIODevice::WriteOnly);
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            MessageTips *mMessageTips = new MessageTips("已向对方发送分享文件请求", this);
            mMessageTips->show();
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char caFileName[32];
            char caSouName[32];
            int iFilePathLen = pdu -> uiMsgLen;
            char caFilePath[iFilePathLen];

            memcpy(caSouName, pdu -> caData, 32);
            memcpy(caFileName, pdu -> caData + 32, 32);
            QString strShareNote = QString("%1 想要分享文件: %2 给您 \n是否接收？").arg(caSouName).arg(caFileName);
            QMessageBox::StandardButton sbShareNote = QMessageBox::question(this, "分享文件", strShareNote);
            PDU *resPdu = NULL;
            if(sbShareNote == QMessageBox::No)
            {
                resPdu = mkPDU(0);
                strcpy(resPdu->caData, caSouName);
            }
            else
            {
                memcpy(caFilePath, (char*)pdu -> caMsg, iFilePathLen);
                QString strRootDir = "./file/" + m_LoginName; // 用户根目录
                resPdu = mkPDU(iFilePathLen + strRootDir.toUtf8().size() + 1);
                sprintf(resPdu -> caData, "%d %lld", iFilePathLen, strRootDir.toUtf8().size());
                sprintf((char*)resPdu -> caMsg, "%s %s", caFilePath, strRootDir.toStdString().c_str());
            }
            resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
            m_tcpSocket.write((char*)resPdu, resPdu -> uiPDULen);
            free(resPdu);
            resPdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND_SENDER:
        {
            if(!strcmp(pdu->caData, "ok"))QMessageBox::information(this, "分享文件", "成功向对方分享文件");
            else if(!strcmp(pdu->caData, "no"))QMessageBox::information(this, "分享文件", "对方拒绝了请求");
            else if(!strcmp(pdu->caData, "gg"))QMessageBox::information(this, "分享文件", "文件分享失败");
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND_RECEIVER:
        {
            if(!strcmp(pdu->caData, "ok"))
            {
                opWidget::getInstance().getBook()->flushFile();
            }
            else if(!strcmp(pdu->caData, "gg"))QMessageBox::information(this, "分享文件", "文件接收失败");
            break;
        }
        default:
            break;
    }
    free(pdu);
    pdu = NULL;
}

void TcpClient::serverOff()
{
    qDebug()<<"server gg";
    QMessageBox::information(this, "warning", "服务器断开连接");
}

void TcpClient::check()
{
    qDebug()<<"readyRead!"<<(++cnt);
}

void TcpClient::onstartDialog(const uint &num)
{
    progressDialog = new QProgressDialog(opWidget::getInstance().getBook());
    QFont font("ZYSong18030", 12);
    progressDialog->setFont(font);
    progressDialog->setWindowModality(Qt::NonModal);
    progressDialog->setMinimumDuration(5);
    progressDialog->setWindowTitle(tr("Please Wait"));
    progressDialog->setLabelText(tr("Updating..."));
    progressDialog->setRange(0, num);
}

void TcpClient::onsetDialog(const uint &num)
{
    progressDialog->setValue(num);

}

void TcpClient::onendDialog()
{
    delete progressDialog;
    progressDialog = nullptr;
    MessageTips *mMessageTips = new MessageTips("文件上传成功", opWidget::getInstance().getBook());
    mMessageTips->show();
    m_tcpSocket.waitForBytesWritten();
    opWidget::getInstance().getBook()->flushFile();
}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_LoginName = strName;
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        MessageTips *mMessageTips = new MessageTips("用户名或密码不能为空", this);
        mMessageTips->show();
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "注册", "用户名或密码不能为空");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}

