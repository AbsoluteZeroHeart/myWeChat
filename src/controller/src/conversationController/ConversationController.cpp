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
    QJsonObject conv = m_dataAccessContext->conversationTable()->getConversationByTarget(userId, 0);
    if (!conv.isEmpty()) {
        emit conversationCreated(conv.value("conversation_id").toVariant().toLongLong());
        return true;
    }

    // 创建新会话
    QJsonObject conversation;
    conversation["conversation_id"] = QDateTime::currentSecsSinceEpoch(); // 简单生成ID
    conversation["user_id"] = userId;
    conversation["type"] = 0; // 单聊
    conversation["title"] = getConversationTitle(0, userId);
    conversation["avatar"] = getConversationAvatar(0, userId);
    conversation["last_message_content"] = "";
    conversation["last_message_time"] = QDateTime::currentSecsSinceEpoch();
    conversation["unread_count"] = 0;
    conversation["is_top"] = false;

    if (m_dataAccessContext->conversationTable()->saveConversation(conversation)) {
        // 重新加载会话列表
        loadConversationsFromDatabase();
        emit conversationCreated(conversation["conversation_id"].toVariant().toLongLong());
        return true;
    } else {
        emit errorOccurred("创建会话失败");
        return false;
    }
}

bool ConversationController::createGroupChat(qint64 groupId)
{
    QJsonObject conv = m_dataAccessContext->conversationTable()->getConversationByTarget(groupId, 1);
    if (!conv.isEmpty()) {
        emit conversationCreated(conv.value("conversation_id").toVariant().toLongLong());
        return true;
    }

    // 创建新会话
    QJsonObject conversation;
    conversation["conversation_id"] = QDateTime::currentSecsSinceEpoch(); // 简单生成ID
    conversation["group_id"] = groupId;
    conversation["type"] = 1; // 群聊
    conversation["title"] = getConversationTitle(1, groupId);
    conversation["avatar"] = getConversationAvatar(1, groupId);
    conversation["last_message_content"] = "";
    conversation["last_message_time"] = QDateTime::currentSecsSinceEpoch();
    conversation["unread_count"] = 0;
    conversation["is_top"] = false;

    if (m_dataAccessContext->conversationTable()->saveConversation(conversation)) {
        // 重新加载会话列表
        loadConversationsFromDatabase();
        emit conversationCreated(conversation["conversation_id"].toVariant().toLongLong());
        return true;
    } else {
        emit errorOccurred("创建群聊会话失败");
        return false;
    }
}

void ConversationController::updateLastMessage(qint64 conversationId, const QString &message)
{
    qint64 last_message_time = QDateTime::currentSecsSinceEpoch();;
    m_chatListModel->updateLastMessage(conversationId, message, QDateTime::currentSecsSinceEpoch());

    m_dataAccessContext->conversationTable()->updateConversationLastMessage(conversationId, message, last_message_time);
    emit conversationUpdated(conversationId);
}

void ConversationController::updateUnreadCount(qint64 conversationId, int count)
{
    m_chatListModel->updateUnreadCount(conversationId, count);

    // 更新数据库
    QJsonObject conversation;
    conversation["conversation_id"] = conversationId;
    conversation["unread_count"] = count;
    
    m_dataAccessContext->conversationTable()->updateConversationPartial(conversation);
    emit conversationUpdated(conversationId);
}

void ConversationController::incrementUnreadCount(qint64 conversationId)
{
    ConversationInfo info = m_chatListModel->getConversationInfo(conversationId);
    if (info.conversationId != 0) {
        updateUnreadCount(conversationId, info.unreadCount + 1);
    }
}

void ConversationController::clearUnreadCount(qint64 conversationId)
{
    updateUnreadCount(conversationId, 0);
}

void ConversationController::toggleReadStatus(qint64 conversationId)
{
    ConversationInfo info = m_chatListModel->getConversationInfo(conversationId);
    if (info.conversationId != 0) {
        // 如果当前有未读消息，则标记为已读；如果已读，则设置为1条未读
        int newUnreadCount = (info.unreadCount > 0) ? 0 : 1;
        m_chatListModel->updateUnreadCount(conversationId, newUnreadCount);

        // 更新数据库
        QJsonObject conversation;
        conversation["conversation_id"] = conversationId;
        conversation["unread_count"] = newUnreadCount;

        m_dataAccessContext->conversationTable()->updateConversationPartial(conversation);
        emit conversationUpdated(conversationId);

        qDebug() << "Toggle read status for conversation:" << conversationId
                 << "New unread count:" << newUnreadCount;
    }
}

void ConversationController::toggleTopStatus(qint64 conversationId)
{
    ConversationInfo info = m_chatListModel->getConversationInfo(conversationId);
    if (info.conversationId != 0) {
        bool newTopStatus = !info.isTop;
        m_chatListModel->updateTopStatus(conversationId, newTopStatus);

        // 更新数据库
        QJsonObject conversation;
        conversation["conversation_id"] = conversationId;
        conversation["is_top"] = newTopStatus;
        
        m_dataAccessContext->conversationTable()->updateConversationPartial(conversation);
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

QVariantMap ConversationController::getConversationInfo(qint64 conversationId)
{
    ConversationInfo info = m_chatListModel->getConversationInfo(conversationId);
    QVariantMap result;
    
    if (info.conversationId != 0) {
        result["conversationId"] = info.conversationId;
        result["groupId"] = info.groupId;
        result["userId"] = info.userId;
        result["type"] = info.type;
        result["title"] = info.title;
        result["avatar"] = info.avatar;
        result["avatarLocalPath"] = info.avatarLocalPath;
        result["lastMessageContent"] = info.lastMessageContent;
        result["lastMessageTime"] = info.lastMessageTime;
        result["unreadCount"] = info.unreadCount;
        result["isTop"] = info.isTop;
        result["isGroup"] = info.isGroup();
    }
    
    return result;
}

QVariantList ConversationController::searchConversations(const QString &keyword)
{
    QVariantList results;
    
    // 这里可以实现搜索逻辑，暂时简单实现
    for (int i = 0; i < m_chatListModel->rowCount(); ++i) {
        ConversationInfo info = m_chatListModel->getConversationInfoAt(i);
        if (info.title.contains(keyword, Qt::CaseInsensitive) || 
            info.lastMessageContent.contains(keyword, Qt::CaseInsensitive)) {
            QVariantMap item;
            item["conversationId"] = info.conversationId;
            item["title"] = info.title;
            item["lastMessage"] = info.lastMessageContent;
            item["isGroup"] = info.isGroup();
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
    
    // 增加未读计数（如果当前不在这个会话中）
    // 这里需要根据业务逻辑判断是否增加未读计数
    incrementUnreadCount(conversationId);
}

void ConversationController::loadConversationsFromDatabase()
{
    QJsonArray conversations = m_dataAccessContext->conversationTable()->getAllConversations();
    m_chatListModel->clearAll();
    for (const QJsonValue &value : std::as_const(conversations)) {
        QJsonObject conversationData = value.toObject();
        ConversationInfo info = createConversationInfo(conversationData);
        m_chatListModel->addConversation(info);
    }
}

bool ConversationController::updateConversationInDatabase(const ConversationInfo &conversationInfo)
{
    QJsonObject conversation;
    conversation["conversation_id"] = conversationInfo.conversationId;
    conversation["group_id"] = conversationInfo.groupId;
    conversation["user_id"] = conversationInfo.userId;
    conversation["type"] = conversationInfo.type;
    conversation["title"] = conversationInfo.title;
    conversation["avatar"] = conversationInfo.avatar;
    conversation["avatar_local_path"] = conversationInfo.avatarLocalPath;
    conversation["last_message_content"] = conversationInfo.lastMessageContent;
    conversation["last_message_time"] = conversationInfo.lastMessageTime;
    conversation["unread_count"] = conversationInfo.unreadCount;
    conversation["is_top"] = conversationInfo.isTop;

    return m_dataAccessContext->conversationTable()->updateConversationPartial(conversation);
}

ConversationInfo ConversationController::createConversationInfo(const QJsonObject &dbData)
{
    ConversationInfo info;
    info.conversationId = dbData.value("conversation_id").toVariant().toLongLong();
    info.groupId = dbData.value("group_id").toVariant().toLongLong();
    info.userId = dbData.value("user_id").toVariant().toLongLong();
    info.type = dbData.value("type").toInt();
    info.title = dbData.value("title").toString();
    info.avatar = dbData.value("avatar").toString();
    info.avatarLocalPath = dbData.value("avatar_local_path").toString();
    info.lastMessageContent = dbData.value("last_message_content").toString();
    info.lastMessageTime = dbData.value("last_message_time").toVariant().toLongLong();
    info.unreadCount = dbData.value("unread_count").toInt();
    info.isTop = dbData.value("is_top").toInt() == 1;

    return info;
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
        QJsonObject group = m_dataAccessContext->groupTable()->getGroup(targetId);
        if (!group.isEmpty()) {
            QString group_note = group.value("group_note").toString();
            if(!group_note.isEmpty()) return group_note;

            return group.value("group_name").toString();
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

void ConversationController::handleMarkAsUnread(qint64 conversationId)
{
    toggleReadStatus(conversationId);
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
