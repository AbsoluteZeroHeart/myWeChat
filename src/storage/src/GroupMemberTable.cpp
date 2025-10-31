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

bool GroupMemberTable::saveGroupMember(const GroupMember& member)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    if (!member.isValid()) {
        qWarning() << "Invalid group member data";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO group_members ("
                  "group_id, user_id, nickname, role, join_time, is_contact"
                  ") VALUES (?, ?, ?, ?, ?, ?)");

    query.addBindValue(member.groupId);
    query.addBindValue(member.userId);
    query.addBindValue(member.nickname);
    query.addBindValue(member.role);
    query.addBindValue(member.joinTime);
    query.addBindValue(member.isContact);

    if (!query.exec()) {
        qWarning() << "Save group member failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GroupMemberTable::updateGroupMember(const GroupMember& member)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    if (!member.isValid()) {
        qWarning() << "Invalid group member data";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE group_members SET "
                  "nickname = ?, role = ?, join_time = ?, is_contact = ? "
                  "WHERE group_id = ? AND user_id = ?");

    query.addBindValue(member.nickname);
    query.addBindValue(member.role);
    query.addBindValue(member.joinTime);
    query.addBindValue(member.isContact);
    query.addBindValue(member.groupId);
    query.addBindValue(member.userId);

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

QList<GroupMember> GroupMemberTable::getGroupMembers(qint64 groupId)
{
    QList<GroupMember> members;

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

GroupMember GroupMemberTable::getGroupMember(qint64 groupId, qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return GroupMember();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND user_id = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        return GroupMember();
    }

    return memberFromQuery(query);
}

QList<GroupMember> GroupMemberTable::searchGroupMembers(qint64 groupId, const QString& keyword)
{
    QList<GroupMember> members;

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

bool GroupMemberTable::syncGroupMembers(qint64 groupId, const QList<GroupMember>& members)
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
        for (const GroupMember& member : members) {
            // 确保每个成员都有正确的groupId
            GroupMember memberToSave = member;
            memberToSave.groupId = groupId;

            if (!saveGroupMember(memberToSave)) {
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
    updateQuery.addBindValue(isContact);
    updateQuery.addBindValue(userId);

    if (!updateQuery.exec()) {
        qWarning() << "Refresh contact status failed:" << updateQuery.lastError().text();
        return false;
    }

    return true;
}

QList<GroupMember> GroupMemberTable::getGroupAdmins(qint64 groupId)
{
    QList<GroupMember> admins;

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
GroupMember GroupMemberTable::memberFromQuery(const QSqlQuery& query)
{
    return GroupMember::fromSqlQuery(query);
}