#ifndef CHATMESSAGESMODEL_H
#define CHATMESSAGESMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "Message.h"

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
        FullMessageRole
    };

    explicit ChatMessagesModel(QObject *parent = nullptr);
    explicit ChatMessagesModel(int currentUserId, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // 消息管理
    void addMessage(const Message &message);
    void insertMessage(int row, const Message &message);
    void removeMessage(int row);
    void removeMessageById(qint64 messageId);
    void updateMessage(const Message &message);
    Message getMessage(int row) const;
    Message getMessageById(qint64 messageId) const;

    // 批量操作
    void addMessages(const QVector<Message> &messages);
    void clearAll();

    // 查询方法
    int findMessageIndexById(qint64 messageId) const;
    bool containsMessage(qint64 messageId) const;

    // 当前用户设置
    void setCurrentUserId(qint64 userId);
    qint64 currentUserId() const;

    // 会话相关
    void setConversationId(qint64 conversationId);
    qint64 conversationId() const;

private:
    QVector<Message> m_messages;
    qint64 m_currentUserId = 0;
    qint64 m_currentConversationId = 0;
};

#endif // CHATMESSAGESMODEL_H
