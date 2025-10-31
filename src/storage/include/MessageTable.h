#ifndef MESSAGETABLE_H
#define MESSAGETABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include "Message.h"
#include "MediaItem.h"

class MessageTable : public QObject
{
    Q_OBJECT

public:
    explicit MessageTable(QSqlDatabase database, QObject *parent = nullptr);

    // 消息管理
    bool saveMessage(const Message& message);
    bool updateMessage(const Message& message);
    bool deleteMessage(qint64 messageId);
    QVector<Message> getMessages(qint64 conversationId, int limit, int offset);
    Message getMessage(qint64 messageId);
    Message getLastMessage(qint64 conversationId);
    bool clearMessages();
    bool clearConversationMessages(qint64 conversationId);
    QList<Message> getMessagesByTimeRange(qint64 conversationId, qint64 startTime, qint64 endTime);
    int getMessageCount(qint64 conversationId);
    QList<MediaItem> getMediaItems(qint64 conversationId);

private:
    QSqlDatabase m_database;
};

#endif // MESSAGETABLE_H
