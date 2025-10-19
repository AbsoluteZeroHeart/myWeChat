#ifndef MESSAGETABLE_H
#define MESSAGETABLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class MessageTable : public QObject
{
    Q_OBJECT

public:
    explicit MessageTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 消息管理
    bool saveMessage(const QJsonObject& message);
    bool updateMessage(const QJsonObject& message);
    bool deleteMessage(qint64 messageId);
    QJsonArray getMessages(qint64 conversationId, int limit = 50, int offset = 0);
    QJsonObject getMessage(qint64 messageId);
    QJsonObject getLastMessage(qint64 conversationId);
    bool clearMessages();
    bool clearConversationMessages(qint64 conversationId);
    
    // 消息状态管理
    int getUnreadCount(qint64 conversationId);
    QJsonArray getUnreadMessages(qint64 conversationId);
    bool markMessagesAsRead(qint64 conversationId);
    QJsonArray getMessagesByTimeRange(qint64 conversationId, qint64 startTime, qint64 endTime);
    int getMessageCount(qint64 conversationId);

private:
    QSqlDatabase m_database;
};

#endif // MESSAGETABLE_H