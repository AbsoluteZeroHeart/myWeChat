#include "MessageTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include "DbConnectionManager.h"
#include <QDebug>

MessageTable::MessageTable(QObject *parent)
    : QObject(parent)
{
}

MessageTable::~MessageTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void MessageTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

void MessageTable::saveMessage(int reqId, Message message)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit messageSaved(reqId, false, "Database is not open");
        return;
    }

    if (!message.isValid()) {
        emit messageSaved(reqId, false, "Invalid message");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("INSERT INTO messages ("
                  "conversation_id, sender_id, consignee_id, type, content, "
                  "file_path, file_url, file_size, duration, thumbnail_path, msg_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(message.conversationId);
    query.addBindValue(message.senderId);
    query.addBindValue(message.consigneeId);
    query.addBindValue(static_cast<int>(message.type));
    query.addBindValue(message.content);
    query.addBindValue(message.filePath);
    query.addBindValue(message.fileUrl);
    query.addBindValue(message.fileSize);
    query.addBindValue(message.duration);
    query.addBindValue(message.thumbnailPath);
    query.addBindValue(message.timestamp);

    if (!query.exec()) {
        emit messageSaved(reqId, false, query.lastError().text());
        return;
    }
    emit messageSaved(reqId, true, QString());
}

void MessageTable::updateMessage(int reqId, Message message)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit messageUpdated(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE messages SET "
                  "conversation_id = ?, sender_id = ?, consignee_id = ?, type = ?, content = ?, "
                  "file_path = ?, file_url = ?, file_size = ?, duration = ?, "
                  "thumbnail_path = ?, msg_time = ? "
                  "WHERE message_id = ?");

    query.addBindValue(message.conversationId);
    query.addBindValue(message.senderId);
    query.addBindValue(message.consigneeId);
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
        emit messageUpdated(reqId, false, query.lastError().text());
        return;
    }

    emit messageUpdated(reqId, query.numRowsAffected() > 0, QString());
}

void MessageTable::deleteMessage(int reqId, qint64 messageId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit messageDeleted(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec()) {
        emit messageDeleted(reqId, false, query.lastError().text());
        return;
    }

    emit messageDeleted(reqId, query.numRowsAffected() > 0, QString());
}


void MessageTable::getMessages(int reqId, qint64 conversationId, int limit, int offset)
{
    QVector<Message> messages;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit messagesLoaded(reqId, messages);
        return;
    }

    QSqlQuery query(*m_database);
    // 联表查询SQL：关联users（必选）和contacts（可选），一次性获取senderName和avatar
    query.prepare(R"(
        SELECT
            m.*,
            -- 若存在联系人记录则用备注名，否则用用户昵称
            CASE WHEN c.user_id IS NOT NULL THEN c.remark_name ELSE u.nickname END AS senderName,
            -- 头像直接取自users表的本地路径
            u.avatar_local_path AS avatar
        FROM messages m
        -- 必联users表（sender_id必然存在于users中）
        INNER JOIN users u ON m.sender_id = u.user_id
        -- 左联contacts表（仅联系人有记录）
        LEFT JOIN contacts c ON m.sender_id = c.user_id
        WHERE m.conversation_id = ?
        ORDER BY m.msg_time DESC
        LIMIT ? OFFSET ?
    )");
    query.addBindValue(conversationId);
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit messagesLoaded(reqId, messages);
        return;
    }

    while (query.next()){
        Message message;
        // 读取messages表的字段
        message.messageId = query.value("message_id").toLongLong();
        message.conversationId = query.value("conversation_id").toLongLong();
        message.senderId = query.value("sender_id").toLongLong();
        message.consigneeId = query.value("consignee_id").toLongLong();
        message.type = static_cast<MessageType>(query.value("type").toInt());
        message.content = query.value("content").toString();
        message.filePath = query.value("file_path").toString();
        message.fileUrl = query.value("file_url").toString();
        message.fileSize = query.value("file_size").toLongLong();
        message.duration = query.value("duration").toInt();
        message.thumbnailPath = query.value("thumbnail_path").toString();
        message.timestamp = query.value("msg_time").toLongLong();

        // 在联表结果中读取senderName和avatar
        message.senderName = query.value("senderName").toString();
        message.avatar = query.value("avatar").toString();

        messages.insert(0, message);
    }
    emit messagesLoaded(reqId, messages);
}

void MessageTable::getMessage(int reqId, qint64 messageId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit messageLoaded(reqId, Message());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec() || !query.next()) {
        emit messageLoaded(reqId, Message());
        return;
    }

    emit messageLoaded(reqId, Message(query));
}

void MessageTable::getLastMessage(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit lastMessageLoaded(reqId, Message());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "ORDER BY msg_time DESC LIMIT 1");
    query.addBindValue(conversationId);

    if (!query.exec() || !query.next()) {
        emit lastMessageLoaded(reqId, Message());
        return;
    }

    emit lastMessageLoaded(reqId, Message(query));
}

void MessageTable::clearMessages(int reqId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit messagesCleared(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query("DELETE FROM messages", *m_database);
    if (!query.exec()) {
        emit messagesCleared(reqId, false, query.lastError().text());
        return;
    }

    emit messagesCleared(reqId, true, QString());
}

void MessageTable::clearConversationMessages(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit conversationMessagesCleared(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        emit conversationMessagesCleared(reqId, false, query.lastError().text());
        return;
    }

    emit conversationMessagesCleared(reqId, true, QString());
}

void MessageTable::getMessagesByTimeRange(int reqId, qint64 conversationId, qint64 startTime, qint64 endTime)
{
    QList<Message> messages;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit messagesByTimeRangeLoaded(reqId, messages);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "AND msg_time BETWEEN ? AND ? ORDER BY msg_time ASC");
    query.addBindValue(conversationId);
    query.addBindValue(startTime);
    query.addBindValue(endTime);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit messagesByTimeRangeLoaded(reqId, messages);
        return;
    }

    while (query.next()) messages.append(Message(query));
    emit messagesByTimeRangeLoaded(reqId, messages);
}

void MessageTable::getMessageCount(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit messageCountLoaded(reqId, -1);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT COUNT(*) FROM messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec() || !query.next()) {
        emit dbError(reqId, query.lastError().text());
        emit messageCountLoaded(reqId, -1);
        return;
    }

    emit messageCountLoaded(reqId, query.value(0).toInt());
}

void MessageTable::getMediaItems(int reqId, qint64 conversationId)
{
    QList<MediaItem> mediaItems;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit mediaItemsLoaded(reqId, mediaItems);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM messages WHERE conversation_id = ? "
                  "AND type IN (1, 2) "
                  "AND (file_path IS NOT NULL OR file_url IS NOT NULL) "
                  "ORDER BY msg_time ASC");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit mediaItemsLoaded(reqId, mediaItems);
        return;
    }

    while (query.next()) {
        MediaItem media = MediaItem::fromSqlQuery(query);
        if (media.isValid()) mediaItems.append(media);
    }

    emit mediaItemsLoaded(reqId, mediaItems);
}

