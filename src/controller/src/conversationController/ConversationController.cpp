#include "ConversationController.h"
#include <QDebug>
#include "DataAccessContext.h"
#include <QJsonArray>
#include <QJsonObject>

ConversationController::ConversationController(QObject *parent)
    : QObject(parent)
    , m_chatListModel(new ChatListModel(this))
    , m_dataAccessContext(new DataAccessContext(this))
{
}

ConversationController::~ConversationController()
{
}

ChatListModel* ConversationController::chatListModel() const
{
    return m_chatListModel;
}

void ConversationController::loadConversations()
{
    loadConversationsFromDatabase();
    emit conversationLoaded();
}

bool ConversationController::createSingleChat(qint64 userId)
{
    // 检查是否已存在该单聊会话
    Conversation conv = m_dataAccessContext->conversationTable()->getConversationByTarget(userId, 0);
    if (conv.isValid()) {
        emit conversationCreated(conv.conversationId);
        return true;
    }

    // 创建新会话
    Conversation conversation;
    conversation.conversationId = QDateTime::currentSecsSinceEpoch(); // 简单生成ID
    conversation.userId = userId;
    conversation.type = 0; // 单聊
    conversation.title = getConversationTitle(0, userId);
    conversation.avatar = getConversationAvatar(0, userId);
    conversation.lastMessageContent = "";
    conversation.lastMessageTime = QDateTime::currentSecsSinceEpoch();
    conversation.unreadCount = 0;
    conversation.isTop = false;

    if (m_dataAccessContext->conversationTable()->saveConversation(conversation)) {
        // 重新加载会话列表
        loadConversationsFromDatabase();
        emit conversationCreated(conversation.conversationId);
        return true;
    } else {
        emit errorOccurred("创建会话失败");
        return false;
    }
}

bool ConversationController::createGroupChat(qint64 groupId)
{
    Conversation conv = m_dataAccessContext->conversationTable()->getConversationByTarget(groupId, 1);
    if (conv.isValid()) {
        emit conversationCreated(conv.conversationId);
        return true;
    }

    // 创建新会话
    Conversation conversation;
    conversation.conversationId = QDateTime::currentSecsSinceEpoch(); // 简单生成ID
    conversation.groupId = groupId;
    conversation.type = 1; // 群聊
    conversation.title = getConversationTitle(1, groupId);
    conversation.avatar = getConversationAvatar(1, groupId);
    conversation.lastMessageContent = "";
    conversation.lastMessageTime = QDateTime::currentSecsSinceEpoch();
    conversation.unreadCount = 0;
    conversation.isTop = false;

    if (m_dataAccessContext->conversationTable()->saveConversation(conversation)) {
        // 重新加载会话列表
        loadConversationsFromDatabase();
        emit conversationCreated(conversation.conversationId);
        return true;
    } else {
        emit errorOccurred("创建群聊会话失败");
        return false;
    }
}

void ConversationController::updateLastMessage(qint64 conversationId, const QString &message)
{
    qint64 lastMessageTime = QDateTime::currentSecsSinceEpoch();
    
    // 更新模型
    m_chatListModel->updateLastMessage(conversationId, message, lastMessageTime);

    m_dataAccessContext->conversationTable()->updateConversationLastMessage(conversationId, message, lastMessageTime);
    emit conversationUpdated(conversationId);
}


void ConversationController::updateUnreadCount(qint64 conversationId, int count)
{
    // 更新模型
    Conversation conversation = m_dataAccessContext->conversationTable()->getConversation(conversationId);
    if (conversation.isValid()) {
        conversation.unreadCount = count;
        m_chatListModel->updateConversation(conversation);
    }

    // 更新数据库
    Conversation updateData;
    updateData.conversationId = conversationId;
    updateData.unreadCount = count;
    
    m_dataAccessContext->conversationTable()->updateConversationPartial(updateData);
    emit conversationUpdated(conversationId);
}

void ConversationController::incrementUnreadCount(qint64 conversationId)
{
    Conversation conversation = m_dataAccessContext->conversationTable()->getConversation(conversationId);
    if (conversation.isValid()) {
        updateUnreadCount(conversationId, conversation.unreadCount + 1);
    }
}

void ConversationController::clearUnreadCount(qint64 conversationId)
{
    updateUnreadCount(conversationId, 0);
}

void ConversationController::handltoggleReadStatus(qint64 conversationId)
{
    Conversation conversation = m_dataAccessContext->conversationTable()->getConversation(conversationId);
    if (!conversation.isValid()) {
        return;
    }

    // 如果当前有未读消息，则标记为已读；如果已读，则设置为1条未读
    int newUnreadCount = (conversation.unreadCount > 0) ? 0 : 1;

    updateUnreadCount(conversationId, newUnreadCount);

    qDebug() << "Toggle read status for conversation:" << conversationId
             << "New unread count:" << newUnreadCount;
}


void ConversationController::toggleTopStatus(qint64 conversationId)
{
    Conversation conversation = m_dataAccessContext->conversationTable()->getConversation(conversationId);
    if (conversation.isValid()) {
        bool newTopStatus = !conversation.isTop;
        
        // 更新模型
        conversation.isTop = newTopStatus;
        m_chatListModel->updateConversation(conversation);

        // 更新数据库
        m_dataAccessContext->conversationTable()->setConversationTop(conversationId, newTopStatus);
        emit conversationUpdated(conversationId);
    }
}

void ConversationController::deleteConversation(qint64 conversationId)
{
    if (m_dataAccessContext->conversationTable()->deleteConversation(conversationId)) {
        m_chatListModel->removeConversation(conversationId);
        emit conversationDeleted(conversationId);
    } else {
        emit errorOccurred("删除会话失败");
    }
}

Conversation ConversationController::getConversation(qint64 conversationId)
{
    return m_dataAccessContext->conversationTable()->getConversation(conversationId);
}

QVariantMap ConversationController::getConversationInfo(qint64 conversationId)
{
    Conversation conversation = m_dataAccessContext->conversationTable()->getConversation(conversationId);
    QVariantMap result;
    
    if (conversation.isValid()) {
        result["conversationId"] = conversation.conversationId;
        result["groupId"] = conversation.groupId;
        result["userId"] = conversation.userId;
        result["type"] = conversation.type;
        result["title"] = conversation.title;
        result["avatar"] = conversation.avatar;
        result["avatarLocalPath"] = conversation.avatarLocalPath;
        result["lastMessageContent"] = conversation.lastMessageContent;
        result["lastMessageTime"] = conversation.lastMessageTime;
        result["unreadCount"] = conversation.unreadCount;
        result["isTop"] = conversation.isTop;
        result["isGroup"] = conversation.isGroup();
    }
    
    return result;
}

QVariantList ConversationController::searchConversations(const QString &keyword)
{
    QVariantList results;
    
    for (int i = 0; i < m_chatListModel->rowCount(); ++i) {
        Conversation conversation = m_chatListModel->getConversationAt(i);
        if (conversation.title.contains(keyword, Qt::CaseInsensitive) || 
            conversation.lastMessageContent.contains(keyword, Qt::CaseInsensitive)) {
            QVariantMap item;
            item["conversationId"] = conversation.conversationId;
            item["title"] = conversation.title;
            item["lastMessage"] = conversation.lastMessageContent;
            item["isGroup"] = conversation.isGroup();
            results.append(item);
        }
    }
    
    return results;
}

void ConversationController::onNewMessageReceived(qint64 conversationId, const QString &message, qint64 timestamp)
{
    Q_UNUSED(timestamp)
    
    // 更新最后消息
    updateLastMessage(conversationId, message);
    incrementUnreadCount(conversationId);
}

void ConversationController::loadConversationsFromDatabase()
{
    QList<Conversation> conversations = m_dataAccessContext->conversationTable()->getAllConversations();
    m_chatListModel->clearAll();
    for (const Conversation& conv : std::as_const(conversations)) {
        m_chatListModel->addConversation(conv);
    }
}

bool ConversationController::updateConversationInDatabase(const Conversation& conversation)
{
    return m_dataAccessContext->conversationTable()->updateConversationPartial(conversation);
}

QString ConversationController::getConversationTitle(int type, qint64 targetId)
{
    if (type == 0) {
        QString remarkName = m_dataAccessContext->contactTable()->getRemarkName(targetId);
        if(remarkName.isEmpty())
        {
            return m_dataAccessContext->userTable()->getNickname(targetId);
        }
        return remarkName;

    } else {
        Group group = m_dataAccessContext->groupTable()->getGroup(targetId);
        if (!group.isValid()) {
            QString group_note = group.groupNote;
            if(!group_note.isEmpty()) return group_note;

            return group.groupName;
        }
    }
    
    return QString("未知名");
}

QString ConversationController::getConversationAvatar(int type, qint64 targetId)
{
    if (type == 0) {
        return m_dataAccessContext->userTable()->getAvatarLocalPath(targetId);

    } else {
        return m_dataAccessContext->groupTable()->getLocalAvatarPath(targetId);
    }
    return QString();
}

void ConversationController::handleToggleTop(qint64 conversationId)
{
    toggleTopStatus(conversationId);
    loadConversations();
}

void ConversationController::handleToggleMute(qint64 conversationId)
{
    qDebug() << "Toggle mute for conversation:" << conversationId;
}

void ConversationController::handleOpenInWindow(qint64 conversationId)
{
    qDebug() << "Open conversation in window:" << conversationId;
    emit openConversationInWindow(conversationId);
}

void ConversationController::handleDelete(qint64 conversationId)
{
    deleteConversation(conversationId);
}
