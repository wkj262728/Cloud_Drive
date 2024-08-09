#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include "sharefile.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void showFile(PDU *pdu);
    void removeDir();
    void addDir(const char* name);
    void changename(const QString name);
    QString m_strSharedFileName,m_strSharedFilePath;

signals:
    void startinit();

public slots:
    void creatDir();
    void flushFile();
    void delDir();
    void rename();
    void into(const QModelIndex &index);
    void back();
    void updateFile();
    void downloadFile();
    void sharFile();
    // void moveFile();
    // void moveDesDir();

private:
    QListWidget *m_pBookList;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreatDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pMoveDesDirDB;
    shareFile *m_pSharedFileFLW;
};

#endif // BOOK_H
