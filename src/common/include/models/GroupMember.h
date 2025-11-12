#ifndef GROUPMEMBER_H
#define GROUPMEMBER_H

#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDateTime>

struct GroupMember {
    qint64 groupId = 0;
    qint64 userId = 0;
    QString nickname;
    int role = 0;           // 0:普通成员, 1:管理员, 2:群主
    qint64 joinTime = 0;
    bool isContact = false;

    GroupMember() = default;
    
    static GroupMember fromSqlQuery(const QSqlQuery& query) {
        GroupMember member;
        member.groupId = query.value("group_id").toLongLong();
        member.userId = query.value("user_id").toLongLong();
        member.nickname = query.value("nickname").toString();
        member.role = query.value("role").toInt();
        member.joinTime = query.value("join_time").toLongLong();
        member.isContact = query.value("is_contact").toBool();
        return member;
    }

    QJsonObject toJson() const {
        return {
            {"group_id", QString::number(groupId)},
            {"user_id", QString::number(userId)},
            {"nickname", nickname},
            {"role", role},
            {"join_time", joinTime},
            {"is_contact", isContact}
        };
    }

    static GroupMember fromJson(const QJsonObject& json) {
        GroupMember member;
        member.groupId = json["group_id"].toString().toLongLong();
        member.userId = json["user_id"].toString().toLongLong();
        member.nickname = json["nickname"].toString();
        member.role = json["role"].toInt();
        member.joinTime = json["join_time"].toVariant().toLongLong();
        member.isContact = json["is_contact"].toBool();
        return member;
    }

    bool isValid() const {
        return groupId > 0 && userId > 0 && !nickname.isEmpty();
    }

    // 便捷方法
    bool isAdmin() const { return role >= 1; }
    bool isOwner() const { return role >= 2; }
    QString getDisplayName() const { return nickname; }
};

Q_DECLARE_METATYPE(GroupMember)


#endif // GROUPMEMBER_H
