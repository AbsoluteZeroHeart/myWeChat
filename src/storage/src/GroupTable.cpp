#include "GroupTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

GroupTable::GroupTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool GroupTable::saveGroup(const QJsonObject& group)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO groups ("
                  "group_id, group_name, avatar, avatar_local_path, "
                  "announcement, member_count, max_members"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(group.value("group_id").toVariant().toLongLong());
    query.addBindValue(group.value("group_name").toString());
    query.addBindValue(group.value("avatar").toString());
    query.addBindValue(group.value("avatar_local_path").toString());
    query.addBindValue(group.value("announcement").toString());
    query.addBindValue(group.value("member_count").toInt(0));
    query.addBindValue(group.value("max_members").toInt(500));

    if (!query.exec()) {
        qWarning() << "Save group failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GroupTable::updateGroup(const QJsonObject& group)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE groups SET "
                  "group_name = ?, avatar = ?, avatar_local_path = ?, "
                  "announcement = ?, member_count = ?, max_members = ? "
                  "WHERE group_id = ?");

    query.addBindValue(group.value("group_name").toString());
    query.addBindValue(group.value("avatar").toString());
    query.addBindValue(group.value("avatar_local_path").toString());
    query.addBindValue(group.value("announcement").toString());
    query.addBindValue(group.value("member_count").toInt(0));
    query.addBindValue(group.value("max_members").toInt(500));
    query.addBindValue(group.value("group_id").toVariant().toLongLong());

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

QJsonArray GroupTable::getAllGroups()
{
    QJsonArray groups;
    
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
        groups.append(groupFromQuery(query));
    }

    return groups;
}

QJsonObject GroupTable::getGroup(qint64 groupId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM groups WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return groupFromQuery(query);
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

QJsonArray GroupTable::searchGroups(const QString& keyword)
{
    QJsonArray groups;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return groups;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM groups WHERE "
                  "group_name LIKE ? OR announcement LIKE ? "
                  "ORDER BY group_name");
    
    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);

    if (!query.exec()) {
        qWarning() << "Search groups failed:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        groups.append(groupFromQuery(query));
    }

    return groups;
}

// 私有辅助方法
QJsonObject GroupTable::groupFromQuery(const QSqlQuery& query)
{
    QJsonObject group;
    
    group["group_id"] = query.value("group_id").toLongLong();
    group["group_name"] = query.value("group_name").toString();
    group["avatar"] = query.value("avatar").toString();
    group["avatar_local_path"] = query.value("avatar_local_path").toString();
    group["announcement"] = query.value("announcement").toString();
    group["member_count"] = query.value("member_count").toInt();
    group["max_members"] = query.value("max_members").toInt();
    
    return group;
}
