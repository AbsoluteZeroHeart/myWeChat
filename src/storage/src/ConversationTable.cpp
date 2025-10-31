#include "ConversationTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

ConversationTable::ConversationTable(QSqlDatabase database, QObject *parent)
    : QObject(parent)
    , m_database(database)
{
}

bool ConversationTable::saveConversation(const Conversation& conversation) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO conversations
        (conversation_id, user_id, group_id, type, title, avatar, avatar_local_path, 
         last_message_content, last_message_time, unread_count, is_top)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(conversation.conversationId);
    query.addBindValue(conversation.userId);
    query.addBindValue(conversation.groupId);
    query.addBindValue(conversation.type);
    query.addBindValue(conversation.title);
    query.addBindValue(conversation.avatar);
    query.addBindValue(conversation.avatarLocalPath);
    query.addBindValue(conversation.lastMessageContent);
    query.addBindValue(conversation.lastMessageTime);
    query.addBindValue(conversation.unreadCount);
    query.addBindValue(conversation.isTop ? 1 : 0);

    return query.exec();
}

bool ConversationTable::updateConversationPartial(const Conversation& conversation) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QStringList updateFields;
    QVariantList bindValues;

    // 动态构建更新字段
    if (conversation.userId > 0) {
        updateFields << "user_id = ?";
        bindValues << conversation.userId;
    }
    if (conversation.groupId > 0) {
        updateFields << "group_id = ?";
        bindValues << conversation.groupId;
    }
    if (conversation.type >= 0) {
        updateFields << "type = ?";
        bindValues << conversation.type;
    }
    if (!conversation.title.isEmpty()) {
        updateFields << "title = ?";
        bindValues << conversation.title;
    }
    if (!conversation.avatar.isEmpty()) {
        updateFields << "avatar = ?";
        bindValues << conversation.avatar;
    }
    if (!conversation.avatarLocalPath.isEmpty()) {
        updateFields << "avatar_local_path = ?";
        bindValues << conversation.avatarLocalPath;
    }
    if (!conversation.lastMessageContent.isNull()) {
        updateFields << "last_message_content = ?";
        bindValues << conversation.lastMessageContent;
    }
    if (conversation.lastMessageTime > 0) {
        updateFields << "last_message_time = ?";
        bindValues << conversation.lastMessageTime;
    }
    if (conversation.unreadCount >= 0) {
        updateFields << "unread_count = ?";
        bindValues << conversation.unreadCount;
    }
    if (conversation.isTop) {
        updateFields << "is_top = ?";
        bindValues << conversation.isTop;
    }
    if (updateFields.isEmpty()) {
        return false;
    }

    QString sql = "UPDATE conversations SET " + updateFields.join(", ") + " WHERE conversation_id = ?";
    bindValues << conversation.conversationId;

    QSqlQuery query(m_database);
    query.prepare(sql);
    
    for (const QVariant& value : std::as_const(bindValues)) {
        query.addBindValue(value);
    }

    return query.exec();
}

bool ConversationTable::deleteConversation(qint64 conversationId) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    return query.exec();
}

QList<Conversation> ConversationTable::getAllConversations() {
    QList<Conversation> conversations;
    if (!m_database.isValid() || !m_database.isOpen()) return conversations;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations ORDER BY is_top DESC, last_message_time DESC");

    if (query.exec()) {
        while (query.next()) {
            conversations.append(Conversation(query));
        }
    }
    
    return conversations;
}

Conversation ConversationTable::getConversation(qint64 conversationId) {
    if (!m_database.isValid() || !m_database.isOpen()) return Conversation();

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (query.exec() && query.next()) {
        return Conversation(query);
    }
    
    return Conversation();
}

Conversation ConversationTable::getConversationByTarget(qint64 targetId, int type) {
    if (!m_database.isValid() || !m_database.isOpen()) return Conversation();

    QSqlQuery query(m_database);
    if (type == 0) {
        query.prepare("SELECT * FROM conversations WHERE user_id = ? AND type = ?");
    } else {
        query.prepare("SELECT * FROM conversations WHERE group_id = ? AND type = ?");
    }
    
    query.addBindValue(targetId);
    query.addBindValue(type);

    if (query.exec() && query.next()) {
        return Conversation(query);
    }
    
    return Conversation();
}

qint64 ConversationTable::getConversationTargetId(qint64 conversationId) {
    if (!m_database.isValid() || !m_database.isOpen()) return -1;

    QSqlQuery query(m_database);
    query.prepare("SELECT user_id, group_id, type FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (query.exec() && query.next()) {
        int type = query.value("type").toInt();
        if (type == 0) {
            return query.value("user_id").toLongLong();
        } else {
            return query.value("group_id").toLongLong();
        }
    }
    
    return -1;
}

bool ConversationTable::updateConversationLastMessage(qint64 conversationId, const QString& content, qint64 time) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare("UPDATE conversations SET last_message_content = ?, last_message_time = ? WHERE conversation_id = ?");
    query.addBindValue(content);
    query.addBindValue(time);
    query.addBindValue(conversationId);

    return query.exec();
}

bool ConversationTable::setConversationTop(qint64 conversationId, bool top) {
    if (!m_database.isValid() || !m_database.isOpen()) return false;

    QSqlQuery query(m_database);
    query.prepare("UPDATE conversations SET is_top = ? WHERE conversation_id = ?");
    query.addBindValue(top ? 1 : 0);
    query.addBindValue(conversationId);

    return query.exec();
}
