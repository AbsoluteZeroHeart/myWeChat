#ifndef GROUP_H
#define GROUP_H

#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDateTime>
#include <QFileInfo>

struct Group {
    qint64 groupId = 0;
    QString groupName;
    QString avatar;
    QString avatarLocalPath;
    QString announcement;
    int memberCount = 0;
    int maxMembers = 500;
    QString groupNote; // 群备注

    Group() = default;
    
    explicit Group(const QSqlQuery& query) {
        groupId = query.value("group_id").toLongLong();
        groupName = query.value("group_name").toString();
        avatar = query.value("avatar").toString();
        avatarLocalPath = query.value("avatar_local_path").toString();
        announcement = query.value("announcement").toString();
        memberCount = query.value("member_count").toInt();
        maxMembers = query.value("max_members").toInt();
        groupNote = query.value("group_note").toString();
    }

    QJsonObject toJson() const {
        return {
            {"group_id", QString::number(groupId)},
            {"group_name", groupName},
            {"avatar", avatar},
            {"avatar_local_path", avatarLocalPath},
            {"announcement", announcement},
            {"member_count", memberCount},
            {"max_members", maxMembers},
            {"group_note", groupNote}
        };
    }

    static Group fromJson(const QJsonObject& json) {
        Group group;
        group.groupId = json["group_id"].toString().toLongLong();
        group.groupName = json["group_name"].toString();
        group.avatar = json["avatar"].toString();
        group.avatarLocalPath = json["avatar_local_path"].toString();
        group.announcement = json["announcement"].toString();
        group.memberCount = json["member_count"].toInt();
        group.maxMembers = json["max_members"].toInt();
        group.groupNote = json["group_note"].toString();
        return group;
    }

    bool isValid() const {
        return groupId > 0 && !groupName.isEmpty();
    }

    // 便捷方法
    QString getDisplayName() const {
        return !groupNote.isEmpty() ? groupNote : groupName;
    }

    bool hasAvatar() const {
        return !avatar.isEmpty();
    }

    bool hasLocalAvatar() const {
        return !avatarLocalPath.isEmpty() && QFileInfo::exists(avatarLocalPath);
    }

    bool hasAnnouncement() const {
        return !announcement.isEmpty();
    }

    bool isFull() const {
        return memberCount >= maxMembers;
    }

    int getAvailableSlots() const {
        return maxMembers - memberCount;
    }

    QString getMemberCountText() const {
        return QString("%1/%2").arg(memberCount).arg(maxMembers);
    }

    void incrementMemberCount() {
        if (memberCount < maxMembers) {
            memberCount++;
        }
    }

    void decrementMemberCount() {
        if (memberCount > 0) {
            memberCount--;
        }
    }

    void updateAvatar(const QString& newAvatar, const QString& newLocalPath = "") {
        avatar = newAvatar;
        if (!newLocalPath.isEmpty()) {
            avatarLocalPath = newLocalPath;
        }
    }

    void updateAnnouncement(const QString& newAnnouncement) {
        announcement = newAnnouncement;
    }

    void updateGroupNote(const QString& note) {
        groupNote = note;
    }
};

Q_DECLARE_METATYPE(Group)


#endif // GROUP_H
