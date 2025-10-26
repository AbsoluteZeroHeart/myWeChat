#ifndef MESSAGETABLE_H
#define MESSAGETABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
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
    int getMessageCount(qint64 conversationId);

    // 消息状态管理
    QJsonArray getMessagesByTimeRange(qint64 conversationId, qint64 startTime, qint64 endTime);

private:
    QSqlDatabase m_database;

    QJsonObject messageFromQuery(const QSqlQuery& query);

};

#endif // MESSAGETABLE_H
