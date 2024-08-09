#include "opdb.h"
#include <QMessageBox>
#include <QDebug>
#include <QThread>

opDB::opDB(QObject *parent)
    : QObject{parent}
{
    // m_db = QSqlDatabase::addDatabase("QSQLITE");
    this->m_db = QSqlDatabase::addDatabase("QSQLITE", QString::number((long long)QThread::currentThreadId()));
    this->m_db.setDatabaseName("F:\\Documents\\Cloud_Drive\\TcpServer\\cloud.db");
    this->m_db.setHostName("localhost");
    if(this->m_db.open())
    {
        qDebug() << "数据库读取成功";
    }
    else
    {
        qDebug() << "数据库读取失败";
    }
}

opDB *opDB::getInstance()
{
    if(!databaseMap.contains(QThread::currentThreadId())) {
        databaseMap.insert(QThread::currentThreadId(), new opDB());
    }
    return databaseMap[QThread::currentThreadId()];
    // static opDB instance;
    // return instance;
}

bool opDB::handleRegist(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL)
    {
        return false;
    }
    QString data = QString("insert into  usrinfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query(this->m_db);
    return query.exec(data);
}

bool opDB::handleLogin(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL)
    {
        return false;
    }
    QString data = QString("select * from usrinfo where name='%1' and pwd='%2' and online=0").arg(name).arg(pwd);
    QSqlQuery query(this->m_db);
    query.exec(data);
    if(query.next())
    {
        data = QString("update usrinfo set online=1 where name='%1' and pwd='%2'").arg(name).arg(pwd);
        query.exec(data);
        return true;
    }
    else
    {
        return false;
    }
}

void opDB::handleOff(const char *name)
{
    if(name == NULL)
    {
        qDebug() << "name is NULL,shutdown failed!";
        return;
    }
    QString data = QString("update usrinfo set online=0 where name='%1'").arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
}

QStringList opDB::handleAllOnline()
{
    QString data = QString("select name from usrinfo where online=1");
    QSqlQuery query(this->m_db);
    query.exec(data);
    QStringList result;
    result.clear();

    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int opDB::handleSearchUsr(const char *name)
{
    if(name == NULL)return -1;
    QString data = QString("select online from usrinfo where name='%1'").arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        if(ret == 0)return 0;
        else return 1;
    }
    else
    {
        return -1;
    }
}

int opDB::handleAddFriend(const char *pername, const char *name)
{
    if(pername == NULL || name == NULL)return -1;
    QString data = QString("select * from friend where (id=(select id from usrinfo where name='%1') and friendid=(select id from usrinfo where name='%2')) "
                           "or (id=(select id from usrinfo where name='%3') and friendid=(select id from usrinfo where name='%4'))")
                       .arg(name).arg(pername).arg(pername).arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
    if(query.next())return 0;
    else
    {
        data = QString("select online from usrinfo where name='%1'").arg(pername);
        query.exec(data);
        if(query.next())
        {
            int ret = query.value(0).toInt();
            if(ret == 1)return 1;
            else return 2;
        }
        else return 3;
    }
}

void opDB::addFriend(const char *pername, const char *name)
{
    if(pername == NULL || name == NULL)return;
    QString data = QString("select id from usrinfo where name='%1' or name='%2'").arg(pername).arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
    query.next();
    int val1 = query.value(0).toInt();
    query.next();
    int val2 = query.value(0).toInt();
    data = QString("insert into friend values('%1','%2')").arg(val1).arg(val2);
    query.exec(data);
    return;
}

void opDB::serverShutdown()
{
    QString data = QString("update usrinfo set online=0");
    QSqlQuery query(this->m_db);
    query.exec(data);
}

QStringList opDB::getFriend(const char *name)
{
    QStringList ans;
    QString data = QString("select friendid from friend where id=(select id from usrinfo where name='%1')").arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
    while(query.next())
    {
        data = QString("select name from usrinfo where id=%1").arg(query.value(0).toInt());
        QSqlQuery exquery(this->m_db);
        exquery.exec(data);
        exquery.next();
        ans.append(exquery.value(0).toString());
    }
    data = QString("select id from friend where friendid=(select id from usrinfo where name='%1')").arg(name);
    query.exec(data);
    while(query.next())
    {
        data = QString("select name from usrinfo where id=%1").arg(query.value(0).toInt());
        QSqlQuery exquery(this->m_db);
        exquery.exec(data);
        exquery.next();
        ans.append(exquery.value(0).toString());
    }
    return ans;
}

int opDB::isOnline(const char *name)
{
    QString data = QString("select online from usrinfo where name='%1'").arg(name);
    QSqlQuery query(this->m_db);
    query.exec(data);
    query.next();
    return query.value(0).toInt();
}

void opDB::delFriend(const char* pername, const char *name)
{
    if(pername == NULL || name == NULL)return;
    QString data = QString("select id from usrinfo where name='%1' or name='%2'").arg(name).arg(pername);
    QSqlQuery query(this->m_db);
    query.exec(data);
    query.next();
    int id1 = query.value(0).toInt();
    query.next();
    int id2 = query.value(0).toInt();
    data = QString("delete from friend where (id=%1 and friendid=%2) or (id=%3 and friendid=%4)").arg(id1).arg(id2).arg(id2).arg(id1);
    query.exec(data);
}

opDB::~opDB()
{
    m_db.close();
}
