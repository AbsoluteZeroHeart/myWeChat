#ifndef USERTABLE_H
#define USERTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include "User.h"

class UserTable : public QObject
{
    Q_OBJECT

public:
    explicit UserTable(QSqlDatabase database, QObject *parent = nullptr);

    bool saveCurrentUser(const User& user);
    User getCurrentUser();
    bool updateCurrentUser(const User& user);
    bool clearCurrentUser();
    
    bool saveUser(const User& user);
    bool updateUser(const User& user);
    bool deleteUser(qint64 userId);
    User getUser(qint64 userId);
    qint64 getCurrentUserId();
    QString getAvatarLocalPath(qint64 userId);
    QString getNickname(qint64 userId);
    User getUserByAccount(const QString& account);
    QList<User> getAllUsers();
    bool userExists(qint64 userId);

private:
    QSqlDatabase m_database;
};

#endif // USERTABLE_H
