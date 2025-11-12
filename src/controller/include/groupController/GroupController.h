#ifndef GROUPCONTROLLER_H
#define GROUPCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QAtomicInt>
#include "DatabaseManager.h"
#include "Group.h"

class GroupController : public QObject
{
    Q_OBJECT

public:
    explicit GroupController(DatabaseManager* dbManager, QObject* parent = nullptr);
    ~GroupController();

    int generateReqId();

    // 公共接口方法
    void createGroup(const Group& group);
    void updateGroup(const Group& group);
    void deleteGroup(qint64 groupId);
    void getGroup(qint64 groupId);
    void getAllGroups();
    void searchGroups(const QString& keyword);
    void updateGroupAnnouncement(qint64 groupId, const QString& announcement);
    void updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath);
    void updateGroupMemberCount(qint64 groupId, int memberCount);
    void updateGroupNote(qint64 groupId, const QString& groupNote);
    void getLocalAvatarPath(qint64 groupId);

signals:
    // 异步操作请求信号
    void createGroupRequested(int reqId, const Group& group);
    void updateGroupRequested(int reqId, const Group& group);
    void deleteGroupRequested(int reqId, qint64 groupId);
    void getGroupRequested(int reqId, qint64 groupId);
    void getAllGroupsRequested(int reqId);
    void searchGroupsRequested(int reqId, const QString& keyword);
    void updateGroupAnnouncementRequested(int reqId, qint64 groupId, const QString& announcement);
    void updateGroupAvatarRequested(int reqId, qint64 groupId, const QString& avatarUrl, const QString& localPath);
    void updateGroupMemberCountRequested(int reqId, qint64 groupId, int memberCount);
    void updateGroupNoteRequested(int reqId, qint64 groupId, const QString& groupNote);
    void getLocalAvatarPathRequested(int reqId, qint64 groupId);

    // 操作结果信号
    void groupCreated(int reqId, bool success, const QString& error);
    void groupUpdated(int reqId, bool success, const QString& error);
    void groupDeleted(int reqId, bool success, const QString& error);
    void groupLoaded(int reqId, const Group& group);
    void allGroupsLoaded(int reqId, const QList<Group>& groups);
    void groupsSearched(int reqId, const QList<Group>& groups);
    void groupAnnouncementUpdated(int reqId, bool success);
    void groupAvatarUpdated(int reqId, bool success);
    void groupMemberCountUpdated(int reqId, bool success);
    void groupNoteUpdated(int reqId, bool success);
    void localAvatarPathLoaded(int reqId, const QString& localPath);
    void dbError(int reqId, const QString& error);

    // 状态变更信号
    void groupCreated(const Group& group);
    void groupUpdated(const Group& group);
    void groupDeleted(qint64 groupId);
    void groupAnnouncementChanged(qint64 groupId, const QString& announcement);
    void groupAvatarChanged(qint64 groupId, const QString& avatarUrl);
    void groupMemberCountChanged(qint64 groupId, int memberCount);
    void groupNameLoaded(int reqId, const QString& groupName);
    void groupsChanged();

private slots:
    // 数据库操作结果处理槽函数
    void onGroupSaved(int reqId, bool success, const QString& error);
    void onGroupUpdated(int reqId, bool success, const QString& error);
    void onGroupDeleted(int reqId, bool success, const QString& error);
    void onGroupLoaded(int reqId, const Group& group);
    void onAllGroupsLoaded(int reqId, const QList<Group>& groups);
    void onGroupsSearched(int reqId, const QList<Group>& groups);
    void onGroupAnnouncementUpdated(int reqId, bool success);
    void onGroupAvatarUpdated(int reqId, bool success);
    void onGroupMemberCountUpdated(int reqId, bool success);
    void onGroupNoteUpdated(int reqId, bool success);
    void onLocalAvatarPathLoaded(int reqId, const QString& localPath);
    void onDbError(int reqId, const QString& error);

private:
    void connectSignals();
    void disconnectSignals();
    void connectAsyncSignals();

private:
    DatabaseManager* m_dbManager;
    GroupTable* m_groupTable;
    QAtomicInt m_reqIdCounter;
    QMap<int, Group> m_pendingGroups;
    QMap<int, qint64> m_pendingGroupIds;
    QMap<int, QPair<QString, QString>> m_pendingAvatarUpdates;
};

#endif // GROUPCONTROLLER_H
