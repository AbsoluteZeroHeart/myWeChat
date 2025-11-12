#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/GroupMember.h"

class GroupMemberTable : public QObject {
    Q_OBJECT
public:
    explicit GroupMemberTable(QObject *parent = nullptr);
    ~GroupMemberTable() override;

public slots:
    void init();

    // 异步操作（全部带 reqId）
    void saveGroupMember(int reqId, GroupMember member);
    void updateGroupMember(int reqId, GroupMember member);
    void deleteGroupMember(int reqId, qint64 groupId, qint64 userId);
    void deleteAllGroupMembers(int reqId, qint64 groupId);

    void getGroupMembers(int reqId, qint64 groupId);
    void getGroupMember(int reqId, qint64 groupId, qint64 userId);
    void searchGroupMembers(int reqId, qint64 groupId, const QString &keyword);

    void syncGroupMembers(int reqId, qint64 groupId, QList<GroupMember> members);

    void updateMemberRole(int reqId, qint64 groupId, qint64 userId, int role);
    void updateMemberNickname(int reqId, qint64 groupId, qint64 userId, const QString &nickname);

    void refreshContactStatus(int reqId, qint64 userId);

    void getGroupAdmins(int reqId, qint64 groupId);
    void getGroupMemberCount(int reqId, qint64 groupId);

signals:
    // 存储/修改/删除结果
    void groupMemberSaved(int reqId, bool ok, QString reason);
    void groupMemberUpdated(int reqId, bool ok, QString reason);
    void groupMemberDeleted(int reqId, bool ok, QString reason);
    void groupMembersDeletedAll(int reqId, bool ok, QString reason);

    // 查询结果
    void groupMembersLoaded(int reqId, QList<GroupMember> members);
    void groupMemberLoaded(int reqId, GroupMember member);
    void searchGroupMembersResult(int reqId, QList<GroupMember> members);

    // 同步批量结果
    void syncGroupMembersDone(int reqId, bool ok, QString reason);

    // 单字段修改结果
    void memberRoleUpdated(int reqId, bool ok);
    void memberNicknameUpdated(int reqId, bool ok);

    // 刷新联系人状态结果
    void refreshContactStatusDone(int reqId, bool ok);

    // 其它查询
    void groupAdminsLoaded(int reqId, QList<GroupMember> admins);
    void groupMemberCountLoaded(int reqId, int count);

    // 通用错误（reqId 可为 -1 表示 init/全局错误）
    void dbError(int reqId, QString error);

private:
    QSharedPointer<QSqlDatabase> m_database;

    // 私有辅助
    GroupMember memberFromQuery(const QSqlQuery &query) const;
};
