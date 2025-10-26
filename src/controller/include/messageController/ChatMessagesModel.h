#ifndef CHATMESSAGESMODEL_H
#define CHATMESSAGESMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "models/ChatMessage.h"

class ChatMessagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MessageRoles {
        MessageIdRole = Qt::UserRole + 1,
        ConversationIdRole,
        SenderIdRole,
        TypeRole,
        ContentRole,
        FilePathRole,
        FileUrlRole,
        FileSizeRole,
        DurationRole,
        ThumbnailPathRole,
        TimestampRole,
        SenderNameRole,
        AvatarRole,
        IsOwnRole,
        IsTextRole,
        IsImageRole,
        IsVideoRole,
        IsFileRole,
        IsVoiceRole,
        IsMediaRole,
        HasFileRole,
        HasThumbnailRole,
        FormattedFileSizeRole,
        FormattedDurationRole,
        FullMessageRole  // 返回完整的消息对象
    };

    explicit ChatMessagesModel(QObject *parent = nullptr);
    explicit ChatMessagesModel(int currentUserId, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 消息管理
    void addMessage(const ChatMessage &message);
    void insertMessage(int row, const ChatMessage &message);
    void removeMessage(int row);
    void removeMessageById(int messageId);
    void updateMessage(const ChatMessage &message);
    ChatMessage getMessage(int row) const;
    ChatMessage getMessageById(int messageId) const;

    // 批量操作
    void addMessages(const QVector<ChatMessage> &messages);
    void clearAll();

    // 查询方法
    int findMessageIndexById(int messageId) const;
    bool containsMessage(int messageId) const;

    // 当前用户设置
    void setCurrentUserId(int userId);
    int currentUserId() const;

    // 会话相关
    void setConversationId(int conversationId);
    int conversationId() const;

private:
    QVector<ChatMessage> m_messages;
    int m_currentUserId = 0;
    int m_currentConversationId = 0;
};

#endif // CHATMESSAGESMODEL_H
