#include "MessageTable.h"
#include <QSqlQuery>
#include <QSqlError>

MessageTable::MessageTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool MessageTable::saveMessage(const Message& message)
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

    query.addBindValue(message.messageId);
    query.addBindValue(message.conversationId);
    query.addBindValue(message.senderId);
    query.addBindValue(static_cast<int>(message.type));
    query.addBindValue(message.content);
    query.addBindValue(message.filePath);
    query.addBindValue(message.fileUrl);
    query.addBindValue(message.fileSize);
    query.addBindValue(message.duration);
    query.addBindValue(message.thumbnailPath);
    query.addBindValue(message.timestamp);

    if (!query.exec()) {
        qWarning() << "Save message failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MessageTable::updateMessage(const Message& message)
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

    query.addBindValue(message.conversationId);
    query.addBindValue(message.senderId);
    query.addBindValue(static_cast<int>(message.type));
    query.addBindValue(message.content);
    query.addBindValue(message.filePath);
    query.addBindValue(message.fileUrl);
    query.addBindValue(message.fileSize);
    query.addBindValue(message.duration);
    query.addBindValue(message.thumbnailPath);
    query.addBindValue(message.timestamp);
    query.addBindValue(message.messageId);

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

QList<Message> MessageTable::getMessages(qint64 conversationId, int limit, int offset)
{
    QVector<Message> messages;
    
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return messages;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "ORDER BY msg_time DESC LIMIT ? OFFSET ?");
    query.addBindValue(conversationId);
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        qWarning() << "Get messages failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        messages.append(Message(query));
    }

    return messages;
}

Message MessageTable::getMessage(qint64 messageId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return Message();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec() || !query.next()) {
        return Message();
    }

    return Message(query);
}

Message MessageTable::getLastMessage(qint64 conversationId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return Message();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "ORDER BY msg_time DESC LIMIT 1");
    query.addBindValue(conversationId);

    if (!query.exec() || !query.next()) {
        return Message();
    }

    return Message(query);
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

QList<Message> MessageTable::getMessagesByTimeRange(qint64 conversationId, qint64 startTime, qint64 endTime)
{
    QList<Message> messages;
    
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
        messages.append(Message(query));
    }

    return messages;
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


QList<MediaItem> MessageTable::getMediaItems(qint64 conversationId)
{
    QList<MediaItem> mediaItems;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return mediaItems;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "AND type IN (1, 2) "
                  "AND (file_path IS NOT NULL OR file_url IS NOT NULL) "
                  "ORDER BY msg_time ASC");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Get media items failed:" << query.lastError().text();
        return mediaItems;
    }

    while (query.next()) {
        MediaItem media = MediaItem::fromSqlQuery(query);
        if (media.isValid()) {
            mediaItems.append(media);
        }
    }

    return mediaItems;
}
