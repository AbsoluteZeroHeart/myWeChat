#ifndef GROUPCONTROLLER_H
#define GROUPCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "Group.h"

class DataAccessContext;

class GroupController : public QObject
{
    Q_OBJECT

public:
    explicit GroupController(QObject *parent = nullptr);
    ~GroupController();

    // 群组管理
    bool createGroup(const Group& group);
    bool updateGroup(const Group& group);
    bool deleteGroup(qint64 groupId);
    Group getGroup(qint64 groupId);
    QList<Group> getAllGroups();
    QList<Group> searchGroups(const QString& keyword);
    
    // 群组信息更新
    bool updateGroupAnnouncement(qint64 groupId, const QString& announcement);
    bool updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath = "");
    bool updateGroupMemberCount(qint64 groupId, int memberCount);
    bool updateGroupNote(qint64 groupId, const QString& groupNote);
    
    // 便捷方法
    QString getGroupName(qint64 groupId);
    QString getLocalAvatarPath(qint64 groupId);
    bool incrementMemberCount(qint64 groupId);
    bool decrementMemberCount(qint64 groupId);

signals:
    void groupCreated(const Group& group);
    void groupUpdated(const Group& group);
    void groupDeleted(qint64 groupId);
    void groupAnnouncementChanged(qint64 groupId, const QString& announcement);
    void groupAvatarChanged(qint64 groupId, const QString& avatarUrl);
    void groupMemberCountChanged(qint64 groupId, int memberCount);
    void groupsChanged();

private:
    DataAccessContext* m_dataAccessContext;
};

#endif // GROUPCONTROLLER_H