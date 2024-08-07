#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include "messagetips.h"
#include <qfiledialog.h>
#include "friend.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_pBookList = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreatDirPB = new QPushButton("创建文件夹");
    m_pRenamePB = new QPushButton("重命名");
    m_pFlushFilePB = new QPushButton("刷新文件列表");

    QFont font(QStringLiteral("HGHT_CNKI"), 12);
    m_pBookList->setFont(font);

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreatDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除选中文件");
    m_pShareFilePB = new QPushButton("分享文件");

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookList);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    m_pSharedFileFLW = new shareFile;
    m_pSharedFileFLW->hide();

    connect(m_pCreatDirPB, SIGNAL(clicked(bool))
            , this, SLOT(creatDir()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool))
            , this, SLOT(flushFile()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool))
            , this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool))
            , this, SLOT(rename()));
    connect(m_pBookList, SIGNAL(doubleClicked(QModelIndex))
            , this, SLOT(into(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool))
            , this, SLOT(back()));
    connect(m_pUploadPB, SIGNAL(clicked(bool))
            , this, SLOT(updateFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool))
            , this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool))
            , this, SLOT(sharFile()));
    // connect(m_pMoveFilePB, SIGNAL(clicked(bool)),
    //         this, SLOT(moveFile()));
    // connect(m_pMoveDesDirDB, SIGNAL(clicked(bool)),
    //         this, SLOT(moveDesDir()));

}

void Book::showFile(PDU *pdu)
{
    m_pBookList->clear();
    uint len = pdu->uiMsgLen / 32;
    QListWidgetItem *item;
    for(uint i=0; i<len; i++)
    {
        if(pdu->caData[i] == '0')
        {
            item = new QListWidgetItem(QIcon(":/source/folder.jpg"), pdu->caMsg + i*32, m_pBookList);
            m_pBookList->addItem(item);
        }
    }
    for(uint i=0; i<len; i++)
    {
        if(pdu->caData[i] == '1')
        {
            item = new QListWidgetItem(QIcon(":/source/file.jpg"), pdu->caMsg + i*32, m_pBookList);
            m_pBookList->addItem(item);
        }
    }
}

void Book::creatDir()
{
    bool ok;
    QString newDir = QInputDialog::getText(this, "新建文件夹", "文件夾名字",QLineEdit::Normal, QString(), &ok);
    if(ok == false)return;
    if(newDir.isEmpty())
    {
        MessageTips *mMessageTips = new MessageTips("文件夾名字不能为空", this);
        mMessageTips->show();
        return;
    }
    QString dir = TcpClient::getInstance().CurrentPath;
    PDU *pdu = mkPDU(dir.toUtf8().size()+1);
    strcpy(pdu->caMsg, dir.toStdString().c_str());
    strcpy(pdu->caData, TcpClient::getInstance().m_LoginName.toStdString().c_str());
    strcpy(pdu->caData + 32, newDir.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_CREAT_DIR_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::flushFile()
{
    QString dir = TcpClient::getInstance().CurrentPath;
    PDU *pdu = mkPDU(dir.toUtf8().size()+1);
    strcpy(pdu->caMsg, dir.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString dir = TcpClient::getInstance().CurrentPath;
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL)return;
    PDU *pdu = mkPDU(dir.toUtf8().size()+1);
    strcpy(pdu->caMsg, dir.toStdString().c_str());
    strcpy(pdu->caData, pItem->text().toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::rename()
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL)return;
    bool ok;
    QString newDir = QInputDialog::getText(this, "重命名", "重命名", QLineEdit::Normal, QString(), &ok);
    if(ok == false)return;
    if(newDir.isEmpty())
    {
        MessageTips *mMessageTips = new MessageTips("文件名字不能为空", this);
        mMessageTips->show();
        return;
    }
    QString dir = TcpClient::getInstance().CurrentPath;
    PDU *pdu = mkPDU(dir.toUtf8().size()+1);
    strcpy(pdu->caMsg, dir.toStdString().c_str());
    strcpy(pdu->caData, pItem->text().toStdString().c_str());
    strcpy(pdu->caData + 32, newDir.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::into(const QModelIndex &index)
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL)return;
    QString dir = TcpClient::getInstance().CurrentPath;
    PDU *pdu = mkPDU(dir.toUtf8().size()+1);
    strcpy(pdu->caMsg, dir.toStdString().c_str());
    strcpy(pdu->caData, index.data().toString().toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_INTO_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Book::back()
{
    QString path = TcpClient::getInstance().CurrentPath;
    if(path == QString("./file/%1").arg(TcpClient::getInstance().m_LoginName))return;
    int index = path.lastIndexOf('/');
    path.remove(index, path.toUtf8().size()-index);
    TcpClient::getInstance().CurrentPath = path;
    PDU *pdu = mkPDU(path.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FILE_BACK_REQUEST;
    strcpy(pdu->caMsg, path.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::updateFile()
{
    QString path = TcpClient::getInstance().CurrentPath;
    QString filePath = QFileDialog::getOpenFileName();
    TcpClient::getInstance().updateFilePath = filePath;
    if(filePath.isEmpty())return;
    int index = filePath.lastIndexOf('/');
    QString fileName = filePath.right(filePath.size() - index - 1);
    QFile file(filePath);
    uint fileSize = file.size();
    PDU *pdu = mkPDU(path.toUtf8().size() + 1);
    strcpy(pdu->caMsg, path.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_UPDATE_REQUEST;
    strcpy(pdu->caData, fileName.toStdString().c_str());
    strcpy(pdu->caData + 32, QString::number(fileSize).toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL)return;
    QString path = TcpClient::getInstance().CurrentPath + '/' + pItem->text();
    QString filePath = QFileDialog::getSaveFileName();
    TcpClient::getInstance().downloadPath = filePath;
    if(filePath.isEmpty())return;
    PDU *pdu = mkPDU(path.toUtf8().size() + 1);
    strcpy(pdu->caMsg, path.toStdString().c_str());
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_REQUEST;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::sharFile()
{
    QListWidgetItem *pFileItem = m_pBookList->currentItem();
    if(NULL == pFileItem)return ;
    m_strSharedFileName = pFileItem->text();
    m_strSharedFilePath = QString("%1/%2").arg(TcpClient::getInstance().CurrentPath)
                              .arg(m_strSharedFileName);
    opWidget::getInstance().getFriend()->flushFriend();
    QListWidget *friendLW = opWidget::getInstance().getFriend()->getFriendList();
    // 选择好友窗口展示
    m_pSharedFileFLW->updateFriendList(friendLW);
    if(m_pSharedFileFLW->isHidden()) // 如果窗口隐藏，则显示出来
    {
        m_pSharedFileFLW->show();
    }
}

void Book::addDir(const char *name)
{
    if(!strcmp(name, ""))return;
    QListWidgetItem *item = new QListWidgetItem(QIcon(":/source/folder.jpg"), name);
    m_pBookList->insertItem(0, item);
}

void Book::changename(const QString name)
{
    m_pBookList->currentItem()->setText(name);
}

void Book::removeDir()
{
    m_pBookList->takeItem(m_pBookList->currentRow());
}
