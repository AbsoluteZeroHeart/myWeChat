#ifndef USERTABLE_H
#define USERTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class UserTable : public QObject
{
    Q_OBJECT

public:
    explicit UserTable(QSqlDatabase database, QObject *parent = nullptr);

    // 当前用户管理
    bool saveCurrentUser(const QJsonObject& user);
    QJsonObject getCurrentUser();
    bool updateCurrentUser(const QJsonObject& user);
    bool clearCurrentUser();

    // 用户基本信息管理
    bool saveUser(const QJsonObject& user);
    bool updateUser(const QJsonObject& user);
    bool deleteUser(qint64 userId);
    QJsonObject getUser(qint64 userId);
    qint64 getCurrentUserId();
    QString getAvatarLocalPath(qint64 userId);
    QString getNickname(qint64 userId);
    QJsonObject getUserByAccount(const QString& account);
    QJsonArray getAllUsers();
    bool userExists(qint64 userId);

private:
    QSqlDatabase m_database;
};

#endif // USERTABLE_H
