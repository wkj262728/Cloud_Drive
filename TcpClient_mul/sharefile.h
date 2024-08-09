#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>

class shareFile : public QWidget
{
    Q_OBJECT
public:
    explicit shareFile(QWidget *parent = nullptr);
    void updateFriendList(QListWidget *pFriendList);

public slots:
    void selectAll(); // 全选
    void cancleSelect(); // 取消所有选择
    void affirmShare(); // 确定分享
    void cancleShare(); // 取消分享

signals:

private:
    QPushButton *m_pSelectAllPB; // 全选
    QPushButton *m_pCancleSelectPB; // 取消选择

    QPushButton *m_pAffirmPB; // 确认键
    QPushButton *m_pCanclePB; // 取消键

    QScrollArea *m_pFriendsSA; // 展示好友区
    QWidget *m_pFriendsWid; // 所有好友窗口
    QVBoxLayout *m_pFriendsVBL; // 好友信息垂直布局
    QButtonGroup *m_pFriendsBG;
};

#endif // SHAREFILE_H
