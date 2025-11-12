#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/Group.h"
class GroupTable : public QObject {
    Q_OBJECT
public:
    explicit GroupTable(QObject *parent = nullptr);
    ~GroupTable() override;

public slots:
    void init();

    void saveGroup(int reqId, const Group &group);
    void updateGroup(int reqId, const Group &group);
    void deleteGroup(int reqId, qint64 groupId);

    void getAllGroups(int reqId);
    void getGroup(int reqId, qint64 groupId);
    void getLocalAvatarPath(int reqId, qint64 groupId);

    void updateGroupAnnouncement(int reqId, qint64 groupId, const QString &announcement);
    void updateGroupAvatar(int reqId, qint64 groupId, const QString &avatarUrl, const QString &localPath);
    void updateGroupMemberCount(int reqId, qint64 groupId, int memberCount);
    void updateGroupNote(int reqId, qint64 groupId, const QString &groupNote);

    void searchGroups(int reqId, const QString &keyword);

signals:
    // 操作结果信号
    void groupSaved(int reqId, bool ok, QString reason);
    void groupUpdated(int reqId, bool ok, QString reason);
    void groupDeleted(int reqId, bool ok, QString reason);

    void allGroupsLoaded(int reqId, QList<Group> groups);
    void groupLoaded(int reqId, Group group);
    void localAvatarPathLoaded(int reqId, QString localPath);

    void groupAnnouncementUpdated(int reqId, bool ok);
    void groupAvatarUpdated(int reqId, bool ok);
    void groupMemberCountUpdated(int reqId, bool ok);
    void groupNoteUpdated(int reqId, bool ok);

    void groupsSearched(int reqId, QList<Group> groups);

    // 通用错误信号（reqId 可为 -1 表示 init/全局错误）
    void dbError(int reqId, QString error);

private:
    QSharedPointer<QSqlDatabase> m_database;
};
