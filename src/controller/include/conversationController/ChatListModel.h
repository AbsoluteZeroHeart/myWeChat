#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QAbstractListModel>
#include "models/ConversationTypes.h"

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ChatListModel(QObject *parent = nullptr);

    // 基本模型方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    // 业务方法
    void addConversation(const ConversationInfo &conversationInfo);
    void updateLastMessage(qint64 conversationId, const QString &message, const qint64 &time);
    void updateUnreadCount(qint64 conversationId, int count);
    void updateTopStatus(qint64 conversationId, bool isTop);
    ConversationInfo getConversationInfo(qint64 conversationId) const;
    ConversationInfo getConversationInfoAt(int index) const;
    void removeConversation(qint64 conversationId);
    void clearAll();

private:
    QVector<ConversationInfo> m_conversations;
    int findConversationIndex(qint64 conversationId) const;
};

#endif // CHATLISTMODEL_H
