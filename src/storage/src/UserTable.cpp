#include "UserTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QDateTime>
#include <QSqlRecord>

UserTable::UserTable(QSqlDatabase database, QObject *parent)
    : QObject(parent)
    , m_database(database)
{
}

bool UserTable::saveCurrentUser(const QJsonObject& user) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    // 开始事务
    if (!m_database.transaction()) {
        return false;
    }

    try {
        // 先清除当前用户标记
        QSqlQuery clearQuery(m_database);
        clearQuery.prepare("UPDATE users SET is_current = 0 WHERE is_current = 1");
        if (!clearQuery.exec()) {
            throw std::runtime_error("Failed to clear current user flag");
        }

        // 保存新当前用户
        QSqlQuery query(m_database);
        query.prepare(R"(
            INSERT OR REPLACE INTO users
            (user_id, account, nickname, avatar, avatar_local_path, gender, region, signature, is_current)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");

        query.addBindValue(user["user_id"].toVariant().toLongLong());
        query.addBindValue(user["account"].toString());
        query.addBindValue(user["nickname"].toString());
        query.addBindValue(user["avatar"].toString());
        query.addBindValue(user["avatar_local_path"].toString());
        query.addBindValue(user["gender"].toInt());
        query.addBindValue(user["region"].toString());
        query.addBindValue(user["signature"].toString());
        query.addBindValue(1); // is_current = 1

        if (!query.exec()) {
            throw std::runtime_error("Failed to save current user");
        }

        // 提交事务
        if (!m_database.commit()) {
            throw std::runtime_error("Failed to commit transaction");
        }

        return true;

    } catch (const std::exception& e) {
        m_database.rollback();
        qCritical() << "Save current user failed:" << e.what();
        return false;
    }
}

QJsonObject UserTable::getCurrentUser() {
    if (!m_database.isValid() || !m_database.isOpen()) return QJsonObject();

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM users WHERE is_current = 1 LIMIT 1");

    if (query.exec() && query.next()) {
        QJsonObject user;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            user[fieldName] = QJsonValue::fromVariant(record.value(i));
        }
        
        return user;
    }
    
    return QJsonObject();
}

bool UserTable::updateCurrentUser(const QJsonObject& user) {
    return saveCurrentUser(user);
}

bool UserTable::clearCurrentUser() {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    return query.exec("UPDATE users SET is_current = 0 WHERE is_current = 1");
}

bool UserTable::saveUser(const QJsonObject& user) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO users
        (user_id, account, nickname, avatar, avatar_local_path, gender, region, signature, is_current)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(user["user_id"].toVariant().toLongLong());
    query.addBindValue(user["account"].toString());
    query.addBindValue(user["nickname"].toString());
    query.addBindValue(user["avatar"].toString());
    query.addBindValue(user["avatar_local_path"].toString());
    query.addBindValue(user["gender"].toInt());
    query.addBindValue(user["region"].toString());
    query.addBindValue(user["signature"].toString());
    query.addBindValue(user.value("is_current").toInt(0));

    return query.exec();
}

bool UserTable::updateUser(const QJsonObject& user) {
    return saveUser(user);
}

bool UserTable::deleteUser(qint64 userId) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM users WHERE user_id = ?");
    query.addBindValue(userId);

    return query.exec();
}

QJsonObject UserTable::getUser(qint64 userId) {
    if (!m_database.isValid() || !m_database.isOpen()) return QJsonObject();

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM users WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        QJsonObject user;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            user[fieldName] = QJsonValue::fromVariant(record.value(i));
        }
        
        return user;
    }
    
    return QJsonObject();
}

qint64 UserTable::getCurrentUserId()
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open (getCurrentUserId)";
        return -1;
    }

    // 准备查询当前用户的ID
    QSqlQuery query(m_database);
    query.prepare("SELECT user_id FROM users WHERE is_current = 1 LIMIT 1");

    if (!query.exec()) {
        qWarning() << "Failed to query current user ID: " << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value("user_id").toLongLong();
    } else {
        qWarning() << "No current user found in database";
        return -1;
    }
}

QString UserTable::getAvatarLocalPath(qint64 userId)
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open (getAvatarLocalPath)";
        return "";
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT avatar_local_path FROM users WHERE user_id = ?");
    query.addBindValue(userId); // 绑定用户ID参数

    if (!query.exec()) {
        qWarning() << "Failed to get avatar local path: " << query.lastError().text();
        return "";
    }

    if (query.next()) {
        return query.value("avatar_local_path").toString();
    } else {
        qWarning() << "No user found for ID: " << userId;
        return "";
    }
}

QString UserTable::getNickname(qint64 userId)
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open (getNickname)";
        return "";
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT nickname FROM users WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Failed to get nickname: " << query.lastError().text();
        return "";
    }

    if (query.next()) {
        return query.value("nickname").toString();
    } else {
        qWarning() << "No user found for ID: " << userId;
        return "";
    }
}

QJsonObject UserTable::getUserByAccount(const QString& account) {
    if (!m_database.isValid() || !m_database.isOpen()) return QJsonObject();

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM users WHERE account = ?");
    query.addBindValue(account);

    if (query.exec() && query.next()) {
        QJsonObject user;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            user[fieldName] = QJsonValue::fromVariant(record.value(i));
        }
        
        return user;
    }
    
    return QJsonObject();
}

QJsonArray UserTable::getAllUsers() {
    QJsonArray users;
    if (!m_database.isValid() || !m_database.isOpen()) return users;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM users ORDER BY user_id");

    if (query.exec()) {
        while (query.next()) {
            QJsonObject user;
            QSqlRecord record = query.record();
            
            for (int i = 0; i < record.count(); ++i) {
                QString fieldName = record.fieldName(i);
                user[fieldName] = QJsonValue::fromVariant(record.value(i));
            }
            
            users.append(user);
        }
    }
    
    return users;
}

bool UserTable::userExists(qint64 userId) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM users WHERE user_id = ? LIMIT 1");
    query.addBindValue(userId);

    return query.exec() && query.next();
}
