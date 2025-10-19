#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

struct ConversationsInfo
{
    QString id;          // 唯一标识
    QString title;       // 昵称
    QString avatar;      // 头像（可为空，用默认头像）
    QString lastMsg;     // 最后一条消息
    QDateTime lastTime;  // 最后消息时间
    int unreadCount;     // 未读消息数
    bool isGroup;        // 是否为群聊
};

// 自定义数据角色
enum ConversationsRole{
    IdRole = Qt::UserRole + 1,
    TitleRole,
    AvatarRole,
    LastMsgRole,
    LastTimeRole,
    UnreadCountRole,
    IsGroupRole,
};


//自定义聊天列表模型
class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ChatListModel(QObject *parent = nullptr);

    //设置模型数据行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 获取数据（根据角色返回对应数据）
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //设置数据
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)override;

    //角色名称（用于qml或数据映射）
    QHash<int, QByteArray> roleNames()const override;

    //添加好友
    void addFriend(const ConversationsInfo &conversationsInfo);

    //更新好友最后一条消息
    void updateLastMessage(const QString &conversationsId, const QString &msg, const QString &time);

    //更新未读消息数
    void updateUnreadCount(const QString &conversationsId, int count);

    //获取好友信息
    ConversationsInfo getConversationsInfo(const QString &friendId)const;


private:
    QVector<ConversationsInfo>conversationsList; //会话数据表
};

#endif // CHATLISTMODEL_H
