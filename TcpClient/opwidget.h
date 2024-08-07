#ifndef OPWIDGET_H
#define OPWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "friend.h"
#include "book.h"
#include <QStackedWidget>

class opWidget : public QWidget
{
    Q_OBJECT
public:
    explicit opWidget(QWidget *parent = nullptr);
    static opWidget &getInstance();
    Friend *getFriend();
    Book *getBook();

signals:

private:
    QListWidget* m_pListW;
    Friend *m_pFriend;
    Book *m_pBook;

    QStackedWidget *m_pSW;
};

#endif // OPWIDGET_H
