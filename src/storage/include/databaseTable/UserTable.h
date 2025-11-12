#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/User.h"

class UserTable : public QObject {
    Q_OBJECT
public:
    explicit UserTable(QObject *parent = nullptr);
    ~UserTable() override;


public slots:
    void init();

    // 当前用户管理
    void saveCurrentUser(int reqId, const User &user);
    void getCurrentUser(int reqId);
    void updateCurrentUser(int reqId, const User &user);
    void clearCurrentUser(int reqId);

    // 通用用户 CRUD
    void saveUser(int reqId, const User &user);
    void updateUser(int reqId, const User &user);
    void deleteUser(int reqId, qint64 userId);
    void getUser(int reqId, qint64 userId);

    // 辅助查询
    void getUserByAccount(int reqId, const QString &account);
    void getAllUsers(int reqId);

signals:
    // 当前用户信号
    void currentUserSaved(int reqId, bool ok, QString reason);
    void currentUserLoaded(int reqId, User user);

    // 通用的CRUD信号
    void userSaved(int reqId, bool ok, QString reason);
    void userDeleted(int reqId, bool ok, QString reason);
    void userLoaded(int reqId, User user);

    // 辅助结果
    void userByAccountLoaded(int reqId, User user);
    void allUsersLoaded(int reqId, QList<User> users);

    void dbError(int reqId, QString error);

private:
    QSharedPointer<QSqlDatabase> m_database;

    // 内部 helper
    bool ensureDbOpen(int reqId);
};
