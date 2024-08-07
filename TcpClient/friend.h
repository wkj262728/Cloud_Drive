#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"
#include "protocol.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnline(PDU *pdu);
    void updateFriend(PDU *pdu);
    void showResult(const char* name, int state);
    QListWidget *getFriendList();

    QString m_strSearchName;
    Online *m_pOnline;
    QListWidget *m_pFriendLW;

public slots:
    void showOnline();
    void searchUsr();
    void flushFriend();
    void delFriend();
    void privateChat();
    void Chat();
    void updateMsg(const PDU *pdu);

signals:

private:
    QTextEdit *m_pShowMsgTE;
    QLineEdit *m_pInputMsgLE;
    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlinePB;
    QPushButton *m_pSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;


};

#endif // FRIEND_H
