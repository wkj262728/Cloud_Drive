#include "opwidget.h"
#include "tcpclient.h"

opWidget::opWidget(QWidget *parent)
    : QWidget{parent}
{
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("网盘");

    QFont font(QStringLiteral("HGHT_CNKI"), 18);
    m_pListW->setFont(font);

    m_pFriend = new Friend;
    m_pBook = new Book;
    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    getFriend()->flushFriend();
    TcpClient::getInstance().getTcpSocket().waitForBytesWritten();
    getBook()->flushFile();

    connect(m_pListW, SIGNAL(currentRowChanged(int))
            , m_pSW, SLOT(setCurrentIndex(int)));
}

opWidget &opWidget::getInstance()
{
    static opWidget instance;
    return instance;
}

Friend *opWidget::getFriend()
{
    return m_pFriend;
}

Book *opWidget::getBook()
{
    return m_pBook;
}
