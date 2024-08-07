#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

    static PrivateChat &getInstance();

    void setChatName(QString strName);
    void updateMsg(const PDU *pdu);
    void clenrMsg();
    void cleanInput();

    QString m_strChatName;

private slots:
    void on_sendMsgPB_clicked();

private:
    Ui::PrivateChat *ui;
};

#endif // PRIVATECHAT_H
