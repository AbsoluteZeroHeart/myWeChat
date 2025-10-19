#ifndef CHATMESSAGELISTMODEL_H
#define CHATMESSAGELISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "ChatMessage.h"

class ConversationsModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ConversationsModel(QObject* parent = nullptr);

    // QAbstractItemModel接口重写
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // 管理消息列表
    void addMessage(const ChatMessage& message);
    void insertMessage(int row, const ChatMessage& message);
    void removeMessage(int row);
    ChatMessage getMessage(int row) const;
    void clearAll();

private:
    QList<ChatMessage> m_messages; // 存储聊天消息的列表
};

#endif // CHATMESSAGELISTMODEL_H
