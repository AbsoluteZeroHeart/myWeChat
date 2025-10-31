#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "Conversation.h"

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ChatListModel(QObject *parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // 会话管理
    void addConversation(const Conversation &conversation);
    void updateConversation(const Conversation &conversation);
    void updateLastMessage(qint64 conversationId, const QString &message, qint64 time);
    void updateUnreadCount(qint64 conversationId, int count);
    void updateTopStatus(qint64 conversationId, bool isTop);
    void removeConversation(qint64 conversationId);
    void clearAll();

    // 查询方法
    Conversation getConversation(qint64 conversationId) const;
    Conversation getConversationAt(int index) const;
    int findConversationIndex(qint64 conversationId) const;

private:
    QVector<Conversation> m_conversations;
};

#endif // CHATLISTMODEL_H