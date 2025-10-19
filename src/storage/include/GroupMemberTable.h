#ifndef GROUPMEMBERTABLE_H
#define GROUPMEMBERTABLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class GroupMemberTable : public QObject
{
    Q_OBJECT

public:
    explicit GroupMemberTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 群成员管理
    bool saveGroupMember(const QJsonObject& member);
    bool updateGroupMember(const QJsonObject& member);
    bool deleteGroupMember(qint64 groupId, qint64 userId);
    bool deleteAllGroupMembers(qint64 groupId);
    QJsonArray getGroupMembers(qint64 groupId);
    QJsonObject getGroupMember(qint64 groupId, qint64 userId);
    QJsonArray searchGroupMembers(qint64 groupId, const QString& keyword);
    bool syncGroupMembers(qint64 groupId, const QJsonArray& members);
    
    // 群成员状态管理
    bool updateMemberRole(qint64 groupId, qint64 userId, int role);
    bool updateMemberNickname(qint64 groupId, qint64 userId, const QString& nickname);
    bool refreshContactStatus(qint64 userId);
    QJsonArray getGroupAdmins(qint64 groupId);
    int getGroupMemberCount(qint64 groupId);

private:
    QSqlDatabase m_database;
};

#endif // GROUPMEMBERTABLE_H