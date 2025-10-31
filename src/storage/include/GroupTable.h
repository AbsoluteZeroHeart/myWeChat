#ifndef GROUPTABLE_H
#define GROUPTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include "Group.h"

class GroupTable : public QObject
{
    Q_OBJECT

public:
    explicit GroupTable(QSqlDatabase database, QObject *parent = nullptr);

    // 群组管理
    bool saveGroup(const Group& group);
    bool updateGroup(const Group& group);
    bool deleteGroup(qint64 groupId);
    QList<Group> getAllGroups();
    Group getGroup(qint64 groupId);
    QString getLocalAvatarPath(qint64 groupId);
    
    // 群组信息更新
    bool updateGroupAnnouncement(qint64 groupId, const QString& announcement);
    bool updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath = "");
    bool updateGroupMemberCount(qint64 groupId, int memberCount);
    bool updateGroupNote(qint64 groupId, const QString& groupNote);
    
    // 搜索
    QList<Group> searchGroups(const QString& keyword);

private:
    QSqlDatabase m_database;
};

#endif // GROUPTABLE_H
