#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDebug>
#include <opdb.h>
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOff()));

    upState = false;
}
QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::startDownload()
{
    if(!m_file.open(QIODevice::ReadOnly))
    {
        PDU *respdu(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_RESPOND;
        respdu->caData[61]='g';
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        m_file.close();
        return;
    }
    char *pBuffer = new char[4096];
    int ret = 0;
    while(true)
    {
        ret = m_file.read(pBuffer, 4096);
        if(ret < 0 || ret > 4096)
        {
            PDU *respdu(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_RESPOND;
            respdu->caData[61]='g';
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        if(ret == 0)break;
        write(pBuffer, ret);
        waitForBytesWritten();
    }
    m_file.close();
    delete[](pBuffer);
    pBuffer = nullptr;
}

void MyTcpSocket::handleShareFileRequest(PDU *pdu, QString strSouName)
{
    int iUserNum = 0;
    char caFileName[32];
    sscanf(pdu -> caData, "%s %d", caFileName, &iUserNum);
    qDebug() << "分享文件：" << caFileName << " 人数：" << iUserNum;

    // 转发给被分享的好友分享文件通知
    const int iFilePathLen = pdu->uiMsgLen - iUserNum * 32;
    char caFilePath[iFilePathLen];
    PDU* resPdu = mkPDU(iFilePathLen);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
    memcpy(resPdu -> caData, strSouName.toStdString().c_str(), strSouName.size()); // 发送方
    memcpy(resPdu -> caData + 32, caFileName, 32); // 发送文件名
    memcpy(caFilePath, (char*)(pdu -> caMsg) + 32 * iUserNum, iFilePathLen);
    memcpy((char*)resPdu -> caMsg, caFilePath, iFilePathLen); // 发送文件路径
    // 遍历分享所有要接收文件的好友
    char caDesName[32]; // 目标好友名
    for(int i = 0; i < iUserNum; ++ i)
    {
        memcpy(caDesName, (char*)(pdu -> caMsg) + 32 * i, 32);
        MyTcpServer::getInstance().resend(caDesName, resPdu);
        qDebug() << caDesName;
    }
    free(resPdu);
    resPdu = NULL;

    // 回复发送方消息
    resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    strcpy(resPdu -> caData, "ok");

}

bool MyTcpSocket::copyDir(QString strOldPath, QString strNewPath)
{
    int ret = true;
    QDir dir; // 目录操作

    qDebug() << "分享目录：" << strOldPath << " " << strNewPath;
    dir.mkdir(strNewPath); // 新路径创建空目录
    dir.setPath(strOldPath); // 设置为源目录
    QFileInfoList fileInfoList = dir.entryInfoList(); // 获得源目录下文件列表
    // 对源目录下所有文件（分为普通文件、文件夹）进行递归拷贝
    QString strOldFile;
    QString strNewFile;
    for(QFileInfo fileInfo:fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
        {
            continue;
        }
        strOldFile = QString("%1/%2").arg(strOldPath).arg(fileInfo.fileName());
        strNewFile = QString("%1/%2").arg(strNewPath).arg(fileInfo.fileName());
        if(fileInfo.isFile())
        {
            ret = ret && QFile::copy(strOldFile, strNewFile);
        }
        else if(fileInfo.isDir())
        {
            ret = ret && copyDir(strOldFile, strNewFile);
        }
        qDebug() << strOldFile << " -> " << strNewFile;
    }

    return ret;
}

void MyTcpSocket::handleShareFileNoteRespond(PDU *pdu)
{
    int iOldPathLen = 0;
    int iNewPathLen = 0;
    sscanf(pdu -> caData, "%d %d", &iOldPathLen, &iNewPathLen);
    char caOldPath[iOldPathLen];
    char caNewDir[iNewPathLen];
    char* sender;
    sscanf((char*)pdu -> caMsg, "%s %s", caOldPath, caNewDir);

    // 获得文件新的路径
    char *pIndex = strrchr(caOldPath, '/'); // 获得最右侧的/的指针，找到文件名
    QString strNewPath = QString("%1/%2").arg(caNewDir).arg(pIndex + 1);
    qDebug() << "同意分享文件：" << caOldPath << " " << strNewPath;

    QFileInfo fileInfo(caOldPath);
    bool ret = false;
    if(fileInfo.isFile())
    {
        ret = QFile::copy(caOldPath, strNewPath);
    }
    else if(fileInfo.isDir())
    {
        ret = copyDir(caOldPath, strNewPath);
    }
    else
    {
        ret = false;
    }
    // 回复接收方
    PDU* respdu = mkPDU(0);
    respdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND_SENDER;
    if(ret)
    {
        memcpy(respdu -> caData, "ok", 32);
    }
    else
    {
        memcpy(respdu -> caData, "gg", 32);
    }
    // qDebug()<<"sender:"<<(char*)pdu->caMsg;
    // MyTcpServer::getInstance().resend(sender, respdu);
    respdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND_RECEIVER;
    write((char*)respdu, respdu->uiPDULen);
    free(respdu);
    respdu = NULL;
}

void MyTcpSocket::recvMsg()
{
    if(upState)
    {
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iReceive += buff.size();
        if(m_iReceive >= m_iTotal)
        {
            m_file.close();
            upState = false;
            m_iReceive = 0;
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_UPDATE_SUCCESS_RESPOND;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        return;
    }
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    switch (pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData + 32, 32);
        bool ret = opDB::getInstance().handleRegist(caName, caPwd);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData, "regist ok");
            QDir dir;
            dir.mkdir(QString("./file/%1").arg(caName));
        }
        else
        {
            strcpy(respdu->caData, "regist failed");
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData + 32, 32);
        bool ret = opDB::getInstance().handleLogin(caName, caPwd);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData, "login ok");
            m_strName = caName;
            QDir dir;
            dir.mkdir(QString("./file/%1").arg(caName));
        }
        else
        {
            strcpy(respdu->caData, "login failed");
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList ret = opDB::getInstance().handleAllOnline();
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i=0; i<ret.size(); i++)
        {
            memcpy(respdu->caMsg + i * 32, ret[i].toStdString().c_str(), ret[i].size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
    {
        int ret = opDB::getInstance().handleSearchUsr(pdu->caData);
        PDU *respdu = mkPDU(32);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(ret == -1)
        {
            strcpy(respdu->caData, "Not find");
        }
        else if(ret == 1)
        {
            strcpy(respdu->caData, "online");
        }
        else if(ret == 0)
        {
            strcpy(respdu->caData, "offline");
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        int ret = opDB::getInstance().handleAddFriend(pdu->caData, pdu->caData + 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        if(ret == -1)strcpy(respdu->caData, "Unknow Error");
        else if(ret == 3)strcpy(respdu->caData, "用户不存在");
        else if(ret == 2)strcpy(respdu->caData, "对方不在线");
        else if(ret == 1)
        {
            MyTcpServer::getInstance().resend(pdu->caData, pdu);
            strcpy(respdu->caData, "已向对方发送好友申请");
        }
        else if(ret == 0)strcpy(respdu->caData, "你与对方已经是好友");
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_AGREE_FRIEND_RESPOND:
    {
        MyTcpServer::getInstance().resend(pdu->caData + 32, pdu);
        opDB::getInstance().addFriend(pdu->caData, pdu->caData + 32);
        break;
    }
    case ENUM_MSG_TYPE_REFUSE_FRIEND_RESPOND:
    {
        MyTcpServer::getInstance().resend(pdu->caData + 32, pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_RFIEND_REQUEST:
    {
        QStringList ans = opDB::getInstance().getFriend(pdu->caData);
        uint uiMsgLen = ans.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_RFIEND_RESPOND;
        QList<int>state;
        for(int i=0; i<ans.size(); i++){
            memcpy(respdu->caMsg + i * 32, ans[i].toStdString().c_str(), ans[i].size());
            respdu->caData[i]=opDB::getInstance().isOnline(ans[i].toStdString().c_str()) == 1 ? '1' : '0';
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_RFIEND_REQUEST:
    {
        opDB::getInstance().delFriend(pdu->caData, pdu->caData + 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_RFIEND_RESPOND;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        if(!opDB::getInstance().isOnline(pdu->caData))
        {
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        MyTcpServer::getInstance().resend(pdu->caData, pdu);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND;
        write((char*)pdu, pdu->uiPDULen);
        break;
    }
    case ENUM_MSG_TYPE_CHAT_REQUEST:
    {
        MyTcpServer::getInstance().allsend(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREAT_DIR_REQUEST:
    {
        QDir dir;
        QString curPath = QString("%1").arg(pdu->caMsg);
        bool ret = dir.exists(curPath);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_CREAT_DIR_RESPOND;
        if(ret)
        {
            if(dir.exists(curPath + '/' + (pdu->caData + 32)))
            {
                strcpy(respdu->caData, "文件名已存在");
            }
            else
            {
                dir.mkdir(curPath + '/' + (pdu->caData + 32));
                strcpy(respdu->caData, "创建成功");
                strcpy(respdu->caData + 32, pdu->caData + 32);
            }
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
    {
        QDir dir(pdu->caMsg);
        QFileInfoList fileList = dir.entryInfoList();
        PDU *respdu = mkPDU((fileList.size()-2) * 32);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
        for(int i=2; i<fileList.size(); i++)
        {
            respdu->caData[i-2] = fileList[i].isFile()==1 ? '1' : '0';
            strcpy(respdu->caMsg + 32*(i-2), fileList[i].fileName().toStdString().c_str());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_REQUEST:
    {
        QString strPath = QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData);
        QFileInfo file(strPath);
        if(file.isDir())
        {
            QDir(strPath).removeRecursively();
        }
        else
        {
            QFile(strPath).remove();
        }
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_RESPOND;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_RENAME_REQUEST:
    {
        PDU *respdu = mkPDU(0);
        QString path = QString("%1").arg(pdu->caMsg);
        QDir dir;
        if(dir.exists(QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData + 32)))
        {
            strcpy(respdu->caData, "文件名已存在");
        }
        else
        {
            QFile(QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData)).rename(QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData + 32));
            strcpy(respdu->caData, pdu->caData + 32);
        }
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_RESPOND;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FILE_INTO_REQUEST:
    {
        QString path = QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData);
        if(QFileInfo(path).isFile())
        {
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_FILE_INTO_RESPOND;
            respdu->caData[0] = 'g';
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        QDir dir(path);
        QFileInfoList fileList = dir.entryInfoList();
        PDU *respdu = mkPDU((fileList.size()-2) * 32);
        respdu->uiMsgType = ENUM_MSG_TYPE_FILE_INTO_RESPOND;
        for(int i=2; i<fileList.size(); i++)
        {
            respdu->caData[i-2] = fileList[i].isFile()==1 ? '1' : '0';
            strcpy(respdu->caMsg + 32*(i-2), fileList[i].fileName().toStdString().c_str());
        }
        strcpy(respdu->caData + 32, pdu->caData);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FILE_BACK_REQUEST:
    {
        QDir dir(pdu->caMsg);
        QFileInfoList fileList = dir.entryInfoList();
        PDU *respdu = mkPDU((fileList.size()-2) * 32);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
        for(int i=2; i<fileList.size(); i++)
        {
            respdu->caData[i-2] = fileList[i].isFile()==1 ? '1' : '0';
            strcpy(respdu->caMsg + 32*(i-2), fileList[i].fileName().toStdString().c_str());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_UPDATE_REQUEST:
    {
        QString path = QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData);
        uint fileSize = QString("%1").arg(pdu->caData + 32).toUInt();
        QFile dir;
        if(dir.exists(path))
        {
            QString fileName = pdu->caData;
            QString lastName = pdu->caData;
            int index = fileName.indexOf('.');
            if(index>=0)
            {
                fileName.remove(index, fileName.size() - index + 1);
            }
            lastName.remove(0, fileName.size());
            uint cnt = 1;
            while(dir.exists(QString("%1/%2(%3)%4").arg(pdu->caMsg).arg(fileName).arg(cnt).arg(lastName)))
                cnt++;
            path = QString("%1/%2(%3)%4").arg(pdu->caMsg).arg(fileName).arg(cnt).arg(lastName);
        }
        m_file.setFileName(path);
        if(m_file.open(QIODevice::WriteOnly))
        {
            upState = true;
            m_iTotal = fileSize;
            m_iReceive = 0;
            qDebug()<<path;
        }
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPDATE_RESPOND;
        strcpy(respdu->caData, QString("%1").arg(fileSize).toStdString().c_str());
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_REQUEST:
    {
        QFileInfo fileInfo(pdu->caMsg);
        PDU *respdu = mkPDU(0);
        if(fileInfo.isDir())
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_RESPOND;
            respdu->caData[61] = 'n';
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        uint m_iTotal = fileInfo.size();
        m_file.setFileName(pdu->caMsg);
        respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_RESPOND;
        strcpy(respdu->caData, QString::number(m_iTotal).toStdString().c_str());
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_START_DOWNLOAD_REQUEST:
    {
        startDownload();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: // 分享文件请求
    {
        handleShareFileRequest(pdu, m_strName);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND: // 分享文件通知响应处理
    {
        if(pdu->uiMsgLen == 0)
        {
            PDU* respdu = mkPDU(0);
            respdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND_SENDER;
            memcpy(respdu -> caData, "no", 32);
            MyTcpServer::getInstance().resend(pdu->caData, respdu);
            free(respdu);
            respdu = NULL;
            break;
        }
        handleShareFileNoteRespond(pdu);
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;

}

void MyTcpSocket::clientOff()
{
    opDB::getInstance().handleOff(m_strName.toStdString().c_str());
    emit offline(this);
}
