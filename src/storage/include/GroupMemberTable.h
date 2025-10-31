#ifndef GROUPMEMBERTABLE_H
#define GROUPMEMBERTABLE_H

#include <QtSql/QSqlDatabase>
#include <QObject>
#include <QList>
#include "GroupMember.h"

class GroupMemberTable : public QObject
{
    Q_OBJECT

public:
    explicit GroupMemberTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 使用结构体的接口
    bool saveGroupMember(const GroupMember& member);
    bool updateGroupMember(const GroupMember& member);
    bool deleteGroupMember(qint64 groupId, qint64 userId);
    bool deleteAllGroupMembers(qint64 groupId);
    QList<GroupMember> getGroupMembers(qint64 groupId);
    GroupMember getGroupMember(qint64 groupId, qint64 userId);
    QList<GroupMember> searchGroupMembers(qint64 groupId, const QString& keyword);
    bool syncGroupMembers(qint64 groupId, const QList<GroupMember>& members);
    bool updateMemberRole(qint64 groupId, qint64 userId, int role);
    bool updateMemberNickname(qint64 groupId, qint64 userId, const QString& nickname);
    bool refreshContactStatus(qint64 userId);
    QList<GroupMember> getGroupAdmins(qint64 groupId);
    int getGroupMemberCount(qint64 groupId);

private:
    QSqlDatabase m_database;
    
    GroupMember memberFromQuery(const QSqlQuery& query);
};

#endif // GROUPMEMBERTABLE_H
