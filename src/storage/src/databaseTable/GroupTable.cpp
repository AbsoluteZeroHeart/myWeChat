#include "GroupTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include "DbConnectionManager.h"
#include <QDebug>

GroupTable::GroupTable(QObject *parent)
    : QObject(parent)
{}

GroupTable::~GroupTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void GroupTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

void GroupTable::saveGroup(int reqId, const Group &group)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupSaved(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
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
        emit groupSaved(reqId, false, query.lastError().text());
        return;
    }
    emit groupSaved(reqId, true, QString());
}

void GroupTable::updateGroup(int reqId, const Group &group)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupUpdated(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
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
        emit groupUpdated(reqId, false, query.lastError().text());
        return;
    }
    emit groupUpdated(reqId, query.numRowsAffected() > 0, QString());
}

void GroupTable::deleteGroup(int reqId, qint64 groupId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupDeleted(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit groupDeleted(reqId, false, query.lastError().text());
        return;
    }
    emit groupDeleted(reqId, query.numRowsAffected() > 0, QString());
}

void GroupTable::getAllGroups(int reqId)
{
    QList<Group> groups;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit allGroupsLoaded(reqId, groups);
        return;
    }

    QSqlQuery query(*m_database);
    // 使用 prepare + exec 保证在一些驱动下行为一致
    query.prepare("SELECT * FROM groups ORDER BY group_name");
    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit allGroupsLoaded(reqId, groups);
        return;
    }

    while (query.next()) groups.append(Group(query));
    emit allGroupsLoaded(reqId, groups);
}

void GroupTable::getGroup(int reqId, qint64 groupId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit groupLoaded(reqId, Group());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        emit groupLoaded(reqId, Group());
        return;
    }

    emit groupLoaded(reqId, Group(query));
}

void GroupTable::getLocalAvatarPath(int reqId, qint64 groupId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit localAvatarPathLoaded(reqId, QString());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT avatar_local_path FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        emit localAvatarPathLoaded(reqId, QString());
        return;
    }

    QString localPath = query.value("avatar_local_path").toString();
    if (!localPath.isEmpty() && QFileInfo::exists(localPath)) {
        emit localAvatarPathLoaded(reqId, localPath);
        return;
    }

    emit localAvatarPathLoaded(reqId, QString());
}

void GroupTable::updateGroupAnnouncement(int reqId, qint64 groupId, const QString &announcement)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupAnnouncementUpdated(reqId, false);
        emit dbError(reqId, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE groups SET announcement = ? WHERE group_id = ?");
    query.addBindValue(announcement);
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit groupAnnouncementUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit groupAnnouncementUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupTable::updateGroupAvatar(int reqId, qint64 groupId, const QString &avatarUrl, const QString &localPath)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupAvatarUpdated(reqId, false);
        emit dbError(reqId, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
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
        emit groupAvatarUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit groupAvatarUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupTable::updateGroupMemberCount(int reqId, qint64 groupId, int memberCount)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupMemberCountUpdated(reqId, false);
        emit dbError(reqId, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE groups SET member_count = ? WHERE group_id = ?");
    query.addBindValue(memberCount);
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit groupMemberCountUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit groupMemberCountUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupTable::updateGroupNote(int reqId, qint64 groupId, const QString &groupNote)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupNoteUpdated(reqId, false);
        emit dbError(reqId, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE groups SET group_note = ? WHERE group_id = ?");
    query.addBindValue(groupNote);
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit groupNoteUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit groupNoteUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupTable::searchGroups(int reqId, const QString &keyword)
{
    QList<Group> groups;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit groupsSearched(reqId, groups);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM groups WHERE "
                  "group_name LIKE ? OR announcement LIKE ? OR group_note LIKE ? "
                  "ORDER BY group_name");

    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit groupsSearched(reqId, groups);
        return;
    }

    while (query.next()) groups.append(Group(query));
    emit groupsSearched(reqId, groups);
}
