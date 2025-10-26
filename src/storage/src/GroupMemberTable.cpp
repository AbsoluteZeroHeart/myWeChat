#include "GroupMemberTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <stdexcept>

GroupMemberTable::GroupMemberTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool GroupMemberTable::saveGroupMember(const QJsonObject& member)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO group_members ("
                  "group_id, user_id, nickname, role, join_time, is_contact"
                  ") VALUES (?, ?, ?, ?, ?, ?)");

    qint64 groupId = member.value("group_id").toVariant().toLongLong();
    qint64 userId = member.value("user_id").toVariant().toLongLong();
    QString nickname = member.value("nickname").toString();
    int role = member.value("role").toInt(0);

    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    qint64 joinTime = currentTime;
    if (member.contains("join_time") && !member.value("join_time").isUndefined()) {
        joinTime = member.value("join_time").toVariant().toLongLong();
    }

    int isContact = member.value("is_contact").toInt(0);

    query.addBindValue(groupId);
    query.addBindValue(userId);
    query.addBindValue(nickname);
    query.addBindValue(role);
    query.addBindValue(joinTime);
    query.addBindValue(isContact);

    if (!query.exec()) {
        qWarning() << "Save group member failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GroupMemberTable::updateGroupMember(const QJsonObject& member)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE group_members SET "
                  "nickname = ?, role = ?, join_time = ?, is_contact = ? "
                  "WHERE group_id = ? AND user_id = ?");

    qint64 groupId = member.value("group_id").toVariant().toLongLong();
    qint64 userId = member.value("user_id").toVariant().toLongLong();
    QString nickname = member.value("nickname").toString();
    int role = member.value("role").toInt(0);
    qint64 joinTime = member.value("join_time").toVariant().toLongLong();
    int isContact = member.value("is_contact").toInt(0);

    query.addBindValue(nickname);
    query.addBindValue(role);
    query.addBindValue(joinTime);
    query.addBindValue(isContact);
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Update group member failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupMemberTable::deleteGroupMember(qint64 groupId, qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM group_members WHERE group_id = ? AND user_id = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Delete group member failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupMemberTable::deleteAllGroupMembers(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM group_members WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Delete all group members failed:" << query.lastError().text();
        return false;
    }

    return true;
}

QJsonArray GroupMemberTable::getGroupMembers(qint64 groupId)
{
    QJsonArray members;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return members;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? ORDER BY role DESC, nickname");
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Get group members failed:" << query.lastError().text();
        return members;
    }

    while (query.next()) {
        members.append(memberFromQuery(query));
    }

    return members;
}

QJsonObject GroupMemberTable::getGroupMember(qint64 groupId, qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND user_id = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return memberFromQuery(query);
}

QJsonArray GroupMemberTable::searchGroupMembers(qint64 groupId, const QString& keyword)
{
    QJsonArray members;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return members;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND "
                  "(nickname LIKE ? OR user_id IN (SELECT user_id FROM users WHERE username LIKE ?)) "
                  "ORDER BY role DESC, nickname");

    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(groupId);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);

    if (!query.exec()) {
        qWarning() << "Search group members failed:" << query.lastError().text();
        return members;
    }

    while (query.next()) {
        members.append(memberFromQuery(query));
    }

    return members;
}

bool GroupMemberTable::syncGroupMembers(qint64 groupId, const QJsonArray& members)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    // 开始事务
    if (!m_database.transaction()) {
        qWarning() << "Begin transaction failed";
        return false;
    }

    try {
        // 先删除该群的所有成员
        if (!deleteAllGroupMembers(groupId)) {
            throw std::runtime_error("Delete old members failed");
        }

        // 批量插入新成员
        for (const QJsonValue& value : members) {
            QJsonObject member = value.toObject();
            member["group_id"] = groupId;

            if (!saveGroupMember(member)) {
                throw std::runtime_error("Save member failed");
            }
        }

        // 提交事务
        if (!m_database.commit()) {
            throw std::runtime_error("Commit transaction failed");
        }

        return true;
    } catch (const std::exception& e) {
        m_database.rollback();
        qWarning() << "Sync group members failed:" << e.what();
        return false;
    }
}

bool GroupMemberTable::updateMemberRole(qint64 groupId, qint64 userId, int role)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE group_members SET role = ? WHERE group_id = ? AND user_id = ?");
    query.addBindValue(role);
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Update member role failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupMemberTable::updateMemberNickname(qint64 groupId, qint64 userId, const QString& nickname)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE group_members SET nickname = ? WHERE group_id = ? AND user_id = ?");
    query.addBindValue(nickname);
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Update member nickname failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupMemberTable::refreshContactStatus(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    // 检查用户是否在联系人表中存在
    QSqlQuery checkQuery(m_database);
    checkQuery.prepare("SELECT COUNT(*) FROM contacts WHERE user_id = ?");
    checkQuery.addBindValue(userId);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "Check contact status failed";
        return false;
    }

    bool isContact = checkQuery.value(0).toInt() > 0;

    // 更新所有群中该用户的联系人状态
    QSqlQuery updateQuery(m_database);
    updateQuery.prepare("UPDATE group_members SET is_contact = ? WHERE user_id = ?");
    updateQuery.addBindValue(isContact ? 1 : 0);
    updateQuery.addBindValue(userId);

    if (!updateQuery.exec()) {
        qWarning() << "Refresh contact status failed:" << updateQuery.lastError().text();
        return false;
    }

    return true;
}

QJsonArray GroupMemberTable::getGroupAdmins(qint64 groupId)
{
    QJsonArray admins;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return admins;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND role >= 1 ORDER BY role DESC, nickname");
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Get group admins failed:" << query.lastError().text();
        return admins;
    }

    while (query.next()) {
        admins.append(memberFromQuery(query));
    }

    return admins;
}

int GroupMemberTable::getGroupMemberCount(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        qWarning() << "Get group member count failed:" << query.lastError().text();
        return -1;
    }

    return query.value(0).toInt();
}

// 私有辅助方法
QJsonObject GroupMemberTable::memberFromQuery(const QSqlQuery& query)
{
    QJsonObject member;

    member["group_id"] = query.value("group_id").toLongLong();
    member["user_id"] = query.value("user_id").toLongLong();
    member["nickname"] = query.value("nickname").toString();
    member["role"] = query.value("role").toInt();
    member["join_time"] = query.value("join_time").toLongLong();
    member["is_contact"] = query.value("is_contact").toInt();

    return member;
}
