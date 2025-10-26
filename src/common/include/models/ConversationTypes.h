#ifndef CONVERSATIONTYPES_H
#define CONVERSATIONTYPES_H

#include <QDateTime>
#include <QString>

struct ConversationInfo
{
    qint64 conversationId = 0;     // 会话ID
    qint64 groupId = 0;            // 群聊目标ID（群聊时有效）
    qint64 userId = 0;             // 用户目标ID（单聊时有效）
    int type = 0;                  // 会话类型（0:单聊 1:群聊）

    QString title;                 // 会话标题
    QString avatar;                // 会话头像URL
    QString avatarLocalPath;       // 头像本地路径
    QString lastMessageContent;    // 最后一条消息内容
    qint64 lastMessageTime;     // 最后一条消息时间

    int unreadCount = 0;           // 未读数量
    bool isTop = false;            // 是否置顶

    // 判断是否为群聊
    bool isGroup() const { return type == 1; }

    // 获取目标ID（根据会话类型返回对应的ID）
    qint64 targetId() const { return isGroup() ? groupId : userId; }
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

#endif // CONVERSATIONTYPES_H
