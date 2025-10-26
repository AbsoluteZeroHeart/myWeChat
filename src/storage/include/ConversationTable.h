#ifndef CONVERSATIONTABLE_H
#define CONVERSATIONTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class ConversationTable : public QObject
{
    Q_OBJECT

public:
    explicit ConversationTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 会话管理
    bool saveConversation(const QJsonObject& conversation);
    bool updateConversationPartial(const QJsonObject& conversation);
    bool deleteConversation(qint64 conversationId);
    QJsonArray getAllConversations();
    QJsonObject getConversation(qint64 conversationId);
    QJsonObject getConversationByTarget(qint64 targetId, int type);
    qint64 getConversationTargetId(qint64 conversationId);
    
    // 会话状态管理
    bool updateConversationLastMessage(qint64 conversationId, const QString& content, qint64 time);
    bool setConversationTop(qint64 conversationId, bool top);

private:
    QSqlDatabase m_database;
};

#endif // CONVERSATIONTABLE_H
