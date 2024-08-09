#ifndef OPDB_H
#define OPDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QMap>

class opDB : public QObject
{
    Q_OBJECT
public:
    explicit opDB(QObject *parent = nullptr);
    static opDB *getInstance();
    bool handleRegist(const char* name, const char* pwd);
    bool handleLogin(const char* name, const char* pwd);
    void handleOff(const char* name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char* name);
    int handleAddFriend(const char* pername, const char* name);
    void addFriend(const char* pername, const char* name);
    void serverShutdown();
    QStringList getFriend(const char* name);
    int isOnline(const char* name);
    void delFriend(const char* pername, const char* name);

    ~opDB();

signals:

private:
    QSqlDatabase m_db;
    static QMap<Qt::HANDLE, opDB*> databaseMap;
};

#endif // OPDB_H
