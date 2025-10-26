#include "ConversationTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>

ConversationTable::ConversationTable(QSqlDatabase database, QObject *parent)
    : QObject(parent)
    , m_database(database)
{
}

bool ConversationTable::saveConversation(const QJsonObject& conversation) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO conversations
        (conversation_id, group_id, user_id, type, title, avatar, avatar_local_path,
         last_message_content, last_message_time, unread_count, is_top)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    qint64 conversationId = conversation.value("conversation_id").toVariant().toLongLong();
    query.addBindValue(conversationId);

    int type = conversation.value("type").toInt();
    if (type == 0) { // 单聊
        query.addBindValue(QVariant(QMetaType::fromType<qint64>()));

        qint64 userId = 0;
        if (conversation.contains("user_id") && !conversation.value("user_id").isUndefined()) {
            userId = conversation.value("user_id").toVariant().toLongLong();
        }
        query.addBindValue(userId);
    } else { // 群聊
        qint64 groupId = 0;
        if (conversation.contains("group_id") && !conversation.value("group_id").isUndefined()) {
            groupId = conversation.value("group_id").toVariant().toLongLong();
        }
        query.addBindValue(groupId);
        query.addBindValue(QVariant(QMetaType::fromType<qint64>()));
    }

    query.addBindValue(type);

    QString title = conversation.value("title").toString();
    QString avatar = conversation.value("avatar").toString();
    QString avatarLocalPath = conversation.value("avatar_local_path").toString();
    QString lastMessageContent = conversation.value("last_message_content").toString();

    query.addBindValue(title);
    query.addBindValue(avatar);
    query.addBindValue(avatarLocalPath);
    query.addBindValue(lastMessageContent);

    qint64 lastMessageTime = 0;
    if (conversation.contains("last_message_time") && !conversation.value("last_message_time").isUndefined()) {
        lastMessageTime = conversation.value("last_message_time").toVariant().toLongLong();
    }
    query.addBindValue(lastMessageTime);

    int unreadCount = conversation.value("unread_count").toInt(0);
    bool isTop = conversation.value("is_top").toBool(false);

    query.addBindValue(unreadCount);
    query.addBindValue(isTop ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "Failed to save conversation:" << query.lastError().text();
        return false;
    }

    return true;
}


bool ConversationTable::updateConversationPartial(const QJsonObject& conversation)
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return false;
    }

    if (!conversation.contains("conversation_id")) {
        qWarning() << "conversation_id is required for update";
        return false;
    }

    qint64 conversationId = conversation.value("conversation_id").toVariant().toLongLong();

    // 构建动态的 UPDATE 语句
    QStringList updateFields;
    QList<QVariant> bindValues;

    // 检查并添加各个字段
    if (conversation.contains("group_id")) {
        updateFields << "group_id = ?";
        bindValues << conversation.value("group_id").toVariant().toLongLong();
    }

    if (conversation.contains("user_id")) {
        updateFields << "user_id = ?";
        bindValues << conversation.value("user_id").toVariant().toLongLong();
    }

    if (conversation.contains("type")) {
        updateFields << "type = ?";
        bindValues << conversation.value("type").toInt();
    }

    if (conversation.contains("title")) {
        updateFields << "title = ?";
        bindValues << conversation.value("title").toString();
    }

    if (conversation.contains("avatar")) {
        updateFields << "avatar = ?";
        bindValues << conversation.value("avatar").toString();
    }

    if (conversation.contains("avatar_local_path")) {
        updateFields << "avatar_local_path = ?";
        bindValues << conversation.value("avatar_local_path").toString();
    }

    if (conversation.contains("last_message_content")) {
        updateFields << "last_message_content = ?";
        bindValues << conversation.value("last_message_content").toString();
    }

    if (conversation.contains("last_message_time")) {
        updateFields << "last_message_time = ?";
        bindValues << conversation.value("last_message_time").toVariant().toLongLong();
    }

    if (conversation.contains("unread_count")) {
        updateFields << "unread_count = ?";
        bindValues << conversation.value("unread_count").toInt();
    }

    if (conversation.contains("is_top")) {
        updateFields << "is_top = ?";
        bindValues << (conversation.value("is_top").toBool() ? 1 : 0);
    }

    if (updateFields.isEmpty()) {
        qWarning() << "No fields to update";
        return false;
    }

    // 构建完整的 SQL 语句
    QString sql = QString("UPDATE conversations SET %1 WHERE conversation_id = ?")
                      .arg(updateFields.join(", "));

    QSqlQuery query(m_database);
    query.prepare(sql);

    // 添加绑定值
    for (const QVariant& value : std::as_const(bindValues)) {
        query.addBindValue(value);
    }

    // 添加 WHERE 条件
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to update conversation:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ConversationTable::deleteConversation(qint64 conversationId) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to delete conversation:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QJsonArray ConversationTable::getAllConversations() {
    QJsonArray conversations;
    
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return conversations;
    }


    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM conversations 
        ORDER BY is_top DESC, last_message_time DESC
    )");

    if (!query.exec()) {
        qWarning() << "Failed to get all conversations:" << query.lastError().text();
        return conversations;
    }

    while (query.next()) {
        QJsonObject conversation;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            // 处理可能为空的字段
            if (value.isNull()) {
                if (fieldName == "group_id" || fieldName == "user_id") {
                    conversation[fieldName] = QJsonValue::Null;
                } else {
                    conversation[fieldName] = QString();
                }
            } else {
                conversation[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        
        conversations.append(conversation);
    }

    return conversations;
}

QJsonObject ConversationTable::getConversation(qint64 conversationId) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to get conversation:" << query.lastError().text();
        return QJsonObject();
    }

    if (query.next()) {
        QJsonObject conversation;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            // 处理可能为空的字段
            if (value.isNull()) {
                if (fieldName == "group_id" || fieldName == "user_id") {
                    conversation[fieldName] = QJsonValue::Null;
                } else {
                    conversation[fieldName] = QString();
                }
            } else {
                conversation[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        
        return conversation;
    }

    return QJsonObject();
}

QJsonObject ConversationTable::getConversationByTarget(qint64 targetId, int type) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    
    if (type == 0) { // 单聊
        query.prepare("SELECT * FROM conversations WHERE user_id = ? AND type = 0");
    } else { // 群聊
        query.prepare("SELECT * FROM conversations WHERE group_id = ? AND type = 1");
    }
    
    query.addBindValue(targetId);

    if (!query.exec()) {
        qWarning() << "Failed to get conversation by target:" << query.lastError().text();
        return QJsonObject();
    }

    if (query.next()) {
        QJsonObject conversation;
        QSqlRecord record = query.record();
        
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            QVariant value = record.value(i);
            
            // 处理可能为空的字段
            if (value.isNull()) {
                if (fieldName == "group_id" || fieldName == "user_id") {
                    conversation[fieldName] = QJsonValue::Null;
                } else {
                    conversation[fieldName] = QString();
                }
            } else {
                conversation[fieldName] = QJsonValue::fromVariant(value);
            }
        }
        
        return conversation;
    }

    return QJsonObject();
}


qint64 ConversationTable::getConversationTargetId(qint64 conversationId)
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open (getConversationTargetId)";
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT type, group_id, user_id
        FROM conversations
        WHERE conversation_id = ?
    )");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to get conversation target ID: " << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        int type = query.value("type").toInt();
        if (type == 0) {
            return query.value("user_id").isNull() ? -1 : query.value("user_id").toLongLong();
        } else if (type == 1) {
            return query.value("group_id").isNull() ? -1 : query.value("group_id").toLongLong();
        } else {
            qWarning() << "Unknown conversation type for ID:" << conversationId;
            return -1;
        }
    } else {
        qWarning() << "Conversation not found for ID: " << conversationId;
        return -1;
    }
}

bool ConversationTable::updateConversationLastMessage(qint64 conversationId, const QString& content, qint64 time) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE conversations 
        SET last_message_content = ?, last_message_time = ? 
        WHERE conversation_id = ?
    )");
    
    query.addBindValue(content);
    query.addBindValue(time);
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to update conversation last message:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}


bool ConversationTable::setConversationTop(qint64 conversationId, bool top) {
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database is not valid or not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE conversations SET is_top = ? WHERE conversation_id = ?");
    query.addBindValue(top ? 1 : 0);
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to set conversation top:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}




