#ifndef GROUPTABLE_H
#define GROUPTABLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class GroupTable : public QObject
{
    Q_OBJECT

public:
    explicit GroupTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 群组管理
    bool saveGroup(const QJsonObject& group);
    bool updateGroup(const QJsonObject& group);
    bool deleteGroup(qint64 groupId);
    QJsonArray getAllGroups();
    QJsonObject getGroup(qint64 groupId);
    bool clearGroups();
    
    // 群组信息更新
    bool updateGroupAnnouncement(qint64 groupId, const QString& announcement);
    bool updateGroupMemberCount(qint64 groupId, int memberCount);
    bool updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath = "");
    QJsonArray searchGroups(const QString& keyword);

private:
    QSqlDatabase m_database;
};

#endif // GROUPTABLE_H