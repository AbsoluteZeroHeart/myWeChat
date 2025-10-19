#ifndef CONVERSATIONTABLE_H
#define CONVERSATIONTABLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class ConversationTable : public QObject
{
    Q_OBJECT

public:
    explicit ConversationTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 会话管理
    bool saveConversation(const QJsonObject& conversation);
    bool updateConversation(const QJsonObject& conversation);
    bool deleteConversation(qint64 conversationId);
    QJsonArray getAllConversations();
    QJsonObject getConversation(qint64 conversationId);
    QJsonObject getConversationByTarget(qint64 targetId, int type);
    bool clearConversations();
    
    // 会话状态管理
    bool updateConversationLastMessage(qint64 conversationId, const QString& content, qint64 time);
    bool updateUnreadCount(qint64 conversationId, int count);
    bool setConversationTop(qint64 conversationId, bool top);
    bool incrementUnreadCount(qint64 conversationId);
    bool resetUnreadCount(qint64 conversationId);
    QJsonArray getTopConversations();
    QJsonArray getUnreadConversations();

private:
    QSqlDatabase m_database;
};

#endif // CONVERSATIONTABLE_H