#include "MessageTable.h"
#include <QSqlQuery>
#include <QSqlError>

MessageTable::MessageTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool MessageTable::saveMessage(const QJsonObject& message)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO messages ("
                  "message_id, conversation_id, sender_id, type, content, "
                  "file_path, file_url, file_size, duration, thumbnail_path, msg_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(message.value("message_id").toVariant().toLongLong());
    query.addBindValue(message.value("conversation_id").toVariant().toLongLong());
    query.addBindValue(message.value("sender_id").toVariant().toLongLong());
    query.addBindValue(message.value("type").toInt());
    query.addBindValue(message.value("content").toString());
    query.addBindValue(message.value("file_path").toString());
    query.addBindValue(message.value("file_url").toString());
    query.addBindValue(message.value("file_size").toVariant());
    query.addBindValue(message.value("duration").toVariant());
    query.addBindValue(message.value("thumbnail_path").toString());
    query.addBindValue(message.value("msg_time").toVariant().toLongLong());

    if (!query.exec()) {
        qWarning() << "Save message failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MessageTable::updateMessage(const QJsonObject& message)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE messages SET "
                  "conversation_id = ?, sender_id = ?, type = ?, content = ?, "
                  "file_path = ?, file_url = ?, file_size = ?, duration = ?, "
                  "thumbnail_path = ?, msg_time = ? "
                  "WHERE message_id = ?");

    query.addBindValue(message.value("conversation_id").toVariant());
    query.addBindValue(message.value("sender_id").toVariant());
    query.addBindValue(message.value("type").toInt());
    query.addBindValue(message.value("content").toString());
    query.addBindValue(message.value("file_path").toString());
    query.addBindValue(message.value("file_url").toString());
    query.addBindValue(message.value("file_size").toVariant());
    query.addBindValue(message.value("duration").toVariant());
    query.addBindValue(message.value("thumbnail_path").toString());
    query.addBindValue(message.value("msg_time").toVariant());
    query.addBindValue(message.value("message_id").toVariant());

    if (!query.exec()) {
        qWarning() << "Update message failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool MessageTable::deleteMessage(qint64 messageId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec()) {
        qWarning() << "Delete message failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QJsonArray MessageTable::getMessages(qint64 conversationId, int limit, int offset)
{
    QJsonArray messages;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return messages;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "ORDER BY msg_time ASC LIMIT ? OFFSET ?");
    query.addBindValue(conversationId);
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        qWarning() << "Get messages failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        messages.append(messageFromQuery(query));
    }

    return messages;
}

QJsonObject MessageTable::getMessage(qint64 messageId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return messageFromQuery(query);
}

QJsonObject MessageTable::getLastMessage(qint64 conversationId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "ORDER BY msg_time DESC LIMIT 1");
    query.addBindValue(conversationId);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return messageFromQuery(query);
}

bool MessageTable::clearMessages()
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query("DELETE FROM messages", m_database);
    
    if (!query.exec()) {
        qWarning() << "Clear messages failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MessageTable::clearConversationMessages(qint64 conversationId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Clear conversation messages failed:" << query.lastError().text();
        return false;
    }

    return true;
}


QJsonArray MessageTable::getMessagesByTimeRange(qint64 conversationId, qint64 startTime, qint64 endTime)
{
    QJsonArray messages;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return messages;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "AND msg_time BETWEEN ? AND ? ORDER BY msg_time ASC");
    query.addBindValue(conversationId);
    query.addBindValue(startTime);
    query.addBindValue(endTime);

    if (!query.exec()) {
        qWarning() << "Get messages by time range failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        messages.append(messageFromQuery(query));
    }

    return messages;
}


// 私有辅助方法
QJsonObject MessageTable::messageFromQuery(const QSqlQuery& query)
{
    QJsonObject message;
    
    message["message_id"] = query.value("message_id").toLongLong();
    message["conversation_id"] = query.value("conversation_id").toLongLong();
    message["sender_id"] = query.value("sender_id").toLongLong();
    message["type"] = query.value("type").toInt();
    message["content"] = query.value("content").toString();
    message["file_path"] = query.value("file_path").toString();
    message["file_url"] = query.value("file_url").toString();
    message["file_size"] = query.value("file_size").toLongLong();
    message["duration"] = query.value("duration").toInt();
    message["thumbnail_path"] = query.value("thumbnail_path").toString();
    message["msg_time"] = query.value("msg_time").toLongLong();
    
    return message;
}


int MessageTable::getMessageCount(qint64 conversationId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Get message count failed:" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
