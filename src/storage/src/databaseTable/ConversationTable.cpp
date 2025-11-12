#include "ConversationTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include "DbConnectionManager.h"
#include <QDebug>

ConversationTable::ConversationTable(QObject *parent)
    : QObject(parent),m_database(nullptr)
{
}

ConversationTable::~ConversationTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void ConversationTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

void ConversationTable::saveConversation(int reqId, const Conversation &conversation)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit conversationSaved(reqId, false, "Database not open");
        return;
    }

    QSqlQuery query(*m_database);
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

    if (!query.exec()) {
        emit conversationSaved(reqId, false, query.lastError().text());
        return;
    }
    emit conversationSaved(reqId, true, QString());
}

void ConversationTable::updateConversationPartial(int reqId, const Conversation &conversation)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit conversationUpdated(reqId, false, "Database not open");
        return;
    }

    QStringList updateFields;
    QVariantList bindValues;

    if (conversation.userId > 0)       { updateFields << "user_id = ?"; bindValues << conversation.userId; }
    if (conversation.groupId > 0)      { updateFields << "group_id = ?"; bindValues << conversation.groupId; }
    if (conversation.type >= 0)        { updateFields << "type = ?"; bindValues << conversation.type; }
    if (!conversation.title.isEmpty())  { updateFields << "title = ?"; bindValues << conversation.title; }
    if (!conversation.avatar.isEmpty()) { updateFields << "avatar = ?"; bindValues << conversation.avatar; }
    if (!conversation.avatarLocalPath.isEmpty()) { updateFields << "avatar_local_path = ?"; bindValues << conversation.avatarLocalPath; }
    if (!conversation.lastMessageContent.isNull()) { updateFields << "last_message_content = ?"; bindValues << conversation.lastMessageContent; }
    if (conversation.lastMessageTime > 0) { updateFields << "last_message_time = ?"; bindValues << conversation.lastMessageTime; }
    if (conversation.unreadCount >= 0) { updateFields << "unread_count = ?"; bindValues << conversation.unreadCount; }
    if (conversation.isTop)      { updateFields << "is_top = ?"; bindValues << (conversation.isTop ? 1 : 0); }

    if (updateFields.isEmpty()) {
        emit conversationUpdated(reqId, false, "No fields to update");
        return;
    }

    QString sql = "UPDATE conversations SET " + updateFields.join(", ") + " WHERE conversation_id = ?";
    bindValues << conversation.conversationId;

    QSqlQuery query(*m_database);
    query.prepare(sql);
    for (const QVariant &v : std::as_const(bindValues)) query.addBindValue(v);

    if (!query.exec()) {
        emit conversationUpdated(reqId, false, query.lastError().text());
        return;
    }
    bool ok = query.numRowsAffected() > 0;
    emit conversationUpdated(reqId, ok, ok ? QString() : "No rows affected");
}

void ConversationTable::deleteConversation(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit conversationDeleted(reqId, false, conversationId);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        emit conversationDeleted(reqId, false, conversationId);
        return;
    }
    emit conversationDeleted(reqId, query.numRowsAffected() > 0, conversationId);
}

void ConversationTable::setUnreadCount(int reqId, qint64 conversationId, int unreadCount)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        return;
    }

    // 确保未读计数不小于0
    if (unreadCount < 0) {
        unreadCount = 0;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE conversations SET unread_count = ? WHERE conversation_id = ?");
    query.addBindValue(unreadCount);
    query.addBindValue(conversationId);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        return;
    }
}

void ConversationTable::toggleTopStatus(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        return;
    }

    // 使用一条SQL语句直接切换置顶状态
    QSqlQuery query(*m_database);
    query.prepare("UPDATE conversations SET is_top = CASE WHEN is_top = 1 THEN 0 ELSE 1 END WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        return;
    }
    emit topStatusToggled(reqId, conversationId);
}

void ConversationTable::getAllConversations(int reqId)
{
    QList<Conversation> conversations;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit allConversationsLoaded(reqId, conversations);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM conversations ORDER BY is_top DESC, last_message_time DESC");
    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit allConversationsLoaded(reqId, conversations);
        return;
    }

    while (query.next()) conversations.append(Conversation(query));
    emit allConversationsLoaded(reqId, conversations);
}

void ConversationTable::getConversation(int reqId, qint64 conversationId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit conversationLoaded(reqId, Conversation());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec() || !query.next()) {
        emit conversationLoaded(reqId, Conversation());
        return;
    }

    emit conversationLoaded(reqId, Conversation(query));
}
