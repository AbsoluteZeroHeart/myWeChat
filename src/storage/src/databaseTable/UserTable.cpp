#include "UserTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QDateTime>
#include <QSqlRecord>
#include "DbConnectionManager.h"
#include <QDebug>
#include <stdexcept>

UserTable::UserTable(QObject *parent)
    : QObject(parent)
{
}

UserTable::~UserTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void UserTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
    }
}

bool UserTable::ensureDbOpen(int reqId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        if (reqId >= 0) emit dbError(reqId, "Database is not valid or not open");
        return false;
    }
    return true;
}

// ----- 当前用户管理 -----
void UserTable::saveCurrentUser(int reqId, const User &user)
{
    if (!ensureDbOpen(reqId)) { emit currentUserSaved(reqId, false, "Database not open"); return; }

    if (!m_database->transaction()) {
        emit currentUserSaved(reqId, false, "Begin transaction failed");
        return;
    }

    try {
        QSqlQuery clearQuery(*m_database);
        clearQuery.prepare("UPDATE users SET is_current = 0 WHERE is_current = 1");
        if (!clearQuery.exec()) {
            throw std::runtime_error(clearQuery.lastError().text().toStdString());
        }

        QSqlQuery q(*m_database);
        q.prepare(R"(
            INSERT OR REPLACE INTO users
            (user_id, account, nickname, avatar, avatar_local_path, gender, region, signature, is_current)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");
        q.addBindValue(user.userId);
        q.addBindValue(user.account);
        q.addBindValue(user.nickname);
        q.addBindValue(user.avatar);
        q.addBindValue(user.avatarLocalPath);
        q.addBindValue(user.gender);
        q.addBindValue(user.region);
        q.addBindValue(user.signature);
        q.addBindValue(1);

        if (!q.exec()) throw std::runtime_error(q.lastError().text().toStdString());

        if (!m_database->commit()) throw std::runtime_error(m_database->lastError().text().toStdString());

        emit currentUserSaved(reqId, true, QString());
    } catch (const std::exception &e) {
        m_database->rollback();
        QString reason = QString::fromUtf8(e.what());
        qCritical() << "Save current user failed:" << reason;
        emit currentUserSaved(reqId, false, reason);
    }
}

void UserTable::getCurrentUser(int reqId)
{
    if (!ensureDbOpen(reqId)) { emit currentUserLoaded(reqId, User()); return; }

    QSqlQuery q(*m_database);
    q.prepare("SELECT * FROM users WHERE is_current = 1 LIMIT 1");
    if (!q.exec() || !q.next()) {
        emit currentUserLoaded(reqId, User());
        return;
    }
    emit currentUserLoaded(reqId, User(q));
}

void UserTable::updateCurrentUser(int reqId, const User &user)
{
    saveCurrentUser(reqId, user);
}

void UserTable::clearCurrentUser(int reqId)
{
    if (!ensureDbOpen(reqId)) { emit currentUserSaved(reqId, false, "Database not open"); return; }

    QSqlQuery q(*m_database);
    if (!q.exec("UPDATE users SET is_current = 0 WHERE is_current = 1")) {
        emit currentUserSaved(reqId, false, q.lastError().text());
        return;
    }
    emit currentUserSaved(reqId, true, QString());
}

// ----- 通用的增删改查（CRUD） -----
void UserTable::saveUser(int reqId, const User &user)
{
    if (!ensureDbOpen(reqId)) { emit userSaved(reqId, false, "Database not open"); return; }

    QSqlQuery q(*m_database);
    q.prepare(R"(
        INSERT OR REPLACE INTO users
        (user_id, account, nickname, avatar, avatar_local_path, gender, region, signature, is_current)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    q.addBindValue(user.userId);
    q.addBindValue(user.account);
    q.addBindValue(user.nickname);
    q.addBindValue(user.avatar);
    q.addBindValue(user.avatarLocalPath);
    q.addBindValue(user.gender);
    q.addBindValue(user.region);
    q.addBindValue(user.signature);
    q.addBindValue(user.isCurrent ? 1 : 0);

    if (!q.exec()) {
        emit userSaved(reqId, false, q.lastError().text());
        return;
    }
    emit userSaved(reqId, true, QString());
}

void UserTable::updateUser(int reqId, const User &user)
{
    // 暂时用不到，懒得写了，先用saveUser哈哈
    saveUser(reqId, user);
}

void UserTable::deleteUser(int reqId, qint64 userId)
{
    if (!ensureDbOpen(reqId)) { emit userDeleted(reqId, false, "Database not open"); return; }

    QSqlQuery q(*m_database);
    q.prepare("DELETE FROM users WHERE user_id = ?");
    q.addBindValue(userId);

    if (!q.exec()) {
        emit userDeleted(reqId, false, q.lastError().text());
        return;
    }
    emit userDeleted(reqId, q.numRowsAffected() > 0, QString());
}

void UserTable::getUser(int reqId, qint64 userId)
{
    if (!ensureDbOpen(reqId)) { emit userLoaded(reqId, User()); return; }

    QSqlQuery q(*m_database);
    q.prepare("SELECT * FROM users WHERE user_id = ?");
    q.addBindValue(userId);

    if (!q.exec() || !q.next()) {
        emit userLoaded(reqId, User());
        return;
    }
    emit userLoaded(reqId, User(q));
}

// ----- 辅助查询 -----

void UserTable::getUserByAccount(int reqId, const QString &account)
{
    if (!ensureDbOpen(reqId)) { emit userByAccountLoaded(reqId, User()); return; }

    QSqlQuery q(*m_database);
    q.prepare("SELECT * FROM users WHERE account = ?");
    q.addBindValue(account);
    if (!q.exec() || !q.next()) {
        emit userByAccountLoaded(reqId, User());
        return;
    }
    emit userByAccountLoaded(reqId, User(q));
}

void UserTable::getAllUsers(int reqId)
{
    QList<User> users;
    if (!ensureDbOpen(reqId)) { emit allUsersLoaded(reqId, users); return; }

    QSqlQuery q(*m_database);
    q.prepare("SELECT * FROM users ORDER BY user_id");
    if (!q.exec()) {
        emit dbError(reqId, q.lastError().text());
        emit allUsersLoaded(reqId, users);
        return;
    }
    while (q.next()) users.append(User(q));
    emit allUsersLoaded(reqId, users);
}

