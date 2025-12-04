#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDateTime>
 
struct Conversation {
    qint64 conversationId = 0;
    qint64 groupId = -1;
    qint64 userId = -1;
    int type = 0;

    QString title;
    QString avatar;
    QString avatarLocalPath;
    QString lastMessageContent;
    qint64 lastMessageTime = 0;

    int unreadCount = 0;
    bool isTop = false;

    Conversation() = default;
    
    explicit Conversation(const QSqlQuery& query) {
        conversationId = query.value("conversation_id").toLongLong();
        groupId = query.value("group_id").toLongLong();
        userId = query.value("user_id").toLongLong();
        type = query.value("type").toInt();
        title = query.value("title").toString();
        avatar = query.value("avatar").toString();
        avatarLocalPath = query.value("avatar_local_path").toString();
        lastMessageContent = query.value("last_message_content").toString();
        lastMessageTime = query.value("last_message_time").toLongLong();
        unreadCount = query.value("unread_count").toInt();
        isTop = query.value("is_top").toBool();
    }

    QJsonObject toJson() const {
        return {
            {"conversation_id", QString::number(conversationId)},
            {"group_id", QString::number(groupId)},
            {"user_id", QString::number(userId)},
            {"type", type},
            {"title", title},
            {"avatar", avatar},
            {"avatar_local_path", avatarLocalPath},
            {"last_message_content", lastMessageContent},
            {"last_message_time", lastMessageTime},
            {"unread_count", unreadCount},
            {"is_top", isTop}
        };
    }

    static Conversation fromJson(const QJsonObject& json) {
        Conversation conv;
        conv.conversationId = json["conversation_id"].toString().toLongLong();
        conv.groupId = json["group_id"].toString().toLongLong();
        conv.userId = json["user_id"].toString().toLongLong();
        conv.type = json["type"].toInt();
        conv.title = json["title"].toString();
        conv.avatar = json["avatar"].toString();
        conv.avatarLocalPath = json["avatar_local_path"].toString();
        conv.lastMessageContent = json["last_message_content"].toString();
        conv.lastMessageTime = json["last_message_time"].toVariant().toLongLong();
        conv.unreadCount = json["unread_count"].toInt();
        conv.isTop = json["is_top"].toBool();
        return conv;
    }

    bool isValid() const {
        return conversationId > -1 ;
    }

    // 判断是否为群聊
    bool isGroup() const { return type == 1 && groupId>-1; }

    // 获取目标ID（根据会话类型返回对应的ID）
    qint64 targetId() const { return isGroup() ? groupId : userId; }

    QString getDisplayTime() const {
        QDateTime dt = QDateTime::fromSecsSinceEpoch(lastMessageTime);
        if (dt.date() == QDate::currentDate()) {
            return dt.toString("hh:mm");
        } else if (dt.daysTo(QDateTime::currentDateTime()) <= 7) {
            return dt.toString("ddd");
        } else {
            return dt.toString("MM/dd");
        }
    }

    bool hasUnread() const {
        return unreadCount > 0;
    }
};


// 自定义数据角色
enum ConversationRole {
    ConversationIdRole = Qt::UserRole + 1,
    GroupIdRole,
    UserIdRole,
    TypeRole,
    TitleRole,
    AvatarRole,
    AvatarLocalPathRole,
    LastMessageContentRole,
    LastMessageTimeRole,
    UnreadCountRole,
    IsTopRole,
    IsGroupRole,
    TargetIdRole
};

Q_DECLARE_METATYPE(Conversation)


#endif // CONVERSATION_H
