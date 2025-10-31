#include "GroupTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

GroupTable::GroupTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool GroupTable::saveGroup(const Group& group)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO groups ("
                  "group_id, group_name, avatar, avatar_local_path, "
                  "announcement, member_count, max_members, group_note"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(group.groupId);
    query.addBindValue(group.groupName);
    query.addBindValue(group.avatar);
    query.addBindValue(group.avatarLocalPath);
    query.addBindValue(group.announcement);
    query.addBindValue(group.memberCount);
    query.addBindValue(group.maxMembers);
    query.addBindValue(group.groupNote);

    if (!query.exec()) {
        qWarning() << "Save group failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GroupTable::updateGroup(const Group& group)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE groups SET "
                  "group_name = ?, avatar = ?, avatar_local_path = ?, "
                  "announcement = ?, member_count = ?, max_members = ?, group_note = ? "
                  "WHERE group_id = ?");

    query.addBindValue(group.groupName);
    query.addBindValue(group.avatar);
    query.addBindValue(group.avatarLocalPath);
    query.addBindValue(group.announcement);
    query.addBindValue(group.memberCount);
    query.addBindValue(group.maxMembers);
    query.addBindValue(group.groupNote);
    query.addBindValue(group.groupId);

    if (!query.exec()) {
        qWarning() << "Update group failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupTable::deleteGroup(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Delete group failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Group> GroupTable::getAllGroups()
{
    QList<Group> groups;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return groups;
    }

    QSqlQuery query("SELECT * FROM groups ORDER BY group_name", m_database);
    
    if (!query.exec()) {
        qWarning() << "Get all groups failed:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        groups.append(Group(query));
    }

    return groups;
}

Group GroupTable::getGroup(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return Group();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        return Group();
    }

    return Group(query);
}

QString GroupTable::getLocalAvatarPath(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QString();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT avatar_local_path FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        qWarning() << "Get local avatar path failed for group:" << groupId;
        return QString();
    }

    QString localPath = query.value("avatar_local_path").toString();

    // 检查本地文件是否存在
    if (!localPath.isEmpty() && QFileInfo::exists(localPath)) {
        return localPath;
    }

    return QString();
}

bool GroupTable::updateGroupAnnouncement(qint64 groupId, const QString& announcement)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE groups SET announcement = ? WHERE group_id = ?");
    query.addBindValue(announcement);
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Update group announcement failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupTable::updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    
    if (!localPath.isEmpty()) {
        query.prepare("UPDATE groups SET avatar = ?, avatar_local_path = ? WHERE group_id = ?");
        query.addBindValue(avatarUrl);
        query.addBindValue(localPath);
        query.addBindValue(groupId);
    } else {
        query.prepare("UPDATE groups SET avatar = ? WHERE group_id = ?");
        query.addBindValue(avatarUrl);
        query.addBindValue(groupId);
    }

    if (!query.exec()) {
        qWarning() << "Update group avatar failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupTable::updateGroupMemberCount(qint64 groupId, int memberCount)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE groups SET member_count = ? WHERE group_id = ?");
    query.addBindValue(memberCount);
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Update group member count failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool GroupTable::updateGroupNote(qint64 groupId, const QString& groupNote)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE groups SET group_note = ? WHERE group_id = ?");
    query.addBindValue(groupNote);
    query.addBindValue(groupId);

    if (!query.exec()) {
        qWarning() << "Update group note failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Group> GroupTable::searchGroups(const QString& keyword)
{
    QList<Group> groups;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return groups;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM groups WHERE "
                  "group_name LIKE ? OR announcement LIKE ? OR group_note LIKE ? "
                  "ORDER BY group_name");
    
    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);

    if (!query.exec()) {
        qWarning() << "Search groups failed:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        groups.append(Group(query));
    }

    return groups;
}