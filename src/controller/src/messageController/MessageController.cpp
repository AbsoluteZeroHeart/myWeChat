#include "MessageController.h"
#include "ChatMessagesModel.h"
#include "DataAccessContext.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QDebug>

MessageController::MessageController(QObject *parent)
    : QObject(parent)
    , m_messagesModel(new ChatMessagesModel(this))
    , m_dataAccessContext(new DataAccessContext(this))
    , m_currentConversationId(-1)
    , m_currentUserId(-1)
    , m_loading(false)
    , m_hasMoreMessages(false)
    , m_currentOffset(0)
    , m_temporaryIdCounter(1000000) // 从大数开始避免与真实ID冲突
    , m_isSearchMode(false)
{
}

MessageController::~MessageController()
{
    // 清理下载计时器
    for (auto &timer : m_downloadTimers) {
        if (timer && timer->isActive()) {
            timer->stop();
        }
    }
    m_downloadTimers.clear();
}

ChatMessagesModel* MessageController::messagesModel() const
{
    return m_messagesModel;
}

int MessageController::currentConversationId() const
{
    return m_currentConversationId;
}

int MessageController::currentUserId() const
{
    return m_currentUserId;
}

bool MessageController::loading() const
{
    return m_loading;
}

bool MessageController::hasMoreMessages() const
{
    return m_hasMoreMessages;
}

void MessageController::setCurrentConversationId(int conversationId)
{
    if (m_currentConversationId != conversationId) {
        m_currentConversationId = conversationId;
        m_messagesModel->setConversationId(conversationId);
        m_currentOffset = 0;
        m_isSearchMode = false;

        // 加载新会话的消息
        loadRecentMessages();
        setCurrentUserId(m_dataAccessContext->userTable()->getCurrentUserId());
    }
}

void MessageController::setCurrentUserId(int userId)
{
    if (m_currentUserId != userId) {
        m_currentUserId = userId;
        m_messagesModel->setCurrentUserId(userId);
        emit currentUserIdChanged();
    }
}

void MessageController::sendTextMessage(const QString& content)
{
    if (content.trimmed().isEmpty() || m_currentConversationId == 0) {
        emit errorOccurred("无法发送空消息或未选择会话");
        return;
    }

    ChatMessage message = createMessage(MessageType::TEXT, content);
    int temporaryId = generateTemporaryId();
    m_temporaryMessages[temporaryId] = message;

    // 先添加到模型（立即显示）
    m_messagesModel->addMessage(message);

     QTimer::singleShot(0, [this, temporaryId]() {
        auto it = m_temporaryMessages.find(temporaryId);
        if (it != m_temporaryMessages.end()) {
            ChatMessage& message = it.value();
            if (saveMessageToDatabase(message)) {
                handleSendSuccess(temporaryId, message.messageId());
            } else {
                handleSendFailure(temporaryId, "发送失败");
            }
        }
    });
}

void MessageController::sendImageMessage(const QString& filePath, const QString& description)
{

}

void MessageController::sendVideoMessage(const QString& filePath, int duration, const QString& description)
{

}

void MessageController::sendFileMessage(const QString& filePath, const QString& fileName)
{

}

void MessageController::sendVoiceMessage(const QString& filePath, int duration)
{

}

void MessageController::deleteMessage(int messageId)
{
    if (deleteMessageFromDatabase(messageId)) {
        m_messagesModel->removeMessageById(messageId);
        emit messageDeleted(messageId);
    } else {
        emit errorOccurred("删除消息失败");
    }
}

void MessageController::recallMessage(int messageId)
{

}

void MessageController::forwardMessage(int messageId, int targetConversationId)
{

}

void MessageController::copyMessageContent(int messageId)
{

}

void MessageController::saveMessageMedia(int messageId, const QString& savePath)
{
    ChatMessage message = m_messagesModel->getMessageById(messageId);
    if (message.messageId() == 0 || !message.hasFile()) {
        emit errorOccurred("消息不存在或没有媒体文件");
        return;
    }

    QString sourcePath = message.filePath();
    if (sourcePath.isEmpty()) {
        // 如果本地没有文件，触发下载
        downloadMedia(messageId);
        return;
    }

    if (QFile::copy(sourcePath, savePath)) {
        qDebug() << "媒体文件已保存到:" << savePath;
    } else {
        emit errorOccurred("保存文件失败");
    }
}

void MessageController::loadRecentMessages(int limit)
{
    if (m_currentConversationId == -1) {
        return;
    }

    m_loading = true;
    m_currentOffset = 0;
    QVector<ChatMessage> messages = loadMessagesFromDatabase(limit, 0);

    m_messagesModel->clearAll();
    m_messagesModel->addMessages(messages);
    m_currentOffset = messages.size();

    // 检查是否还有更多消息
    int totalCount = getMessageCountFromDatabase();
    m_hasMoreMessages = m_currentOffset < totalCount;
    m_loading = false;
     emit hasMoreMessagesChanged();
    emit messagesLoaded(m_hasMoreMessages);
}

void MessageController::loadMoreMessages(int limit)
{
    if (m_loading || !m_hasMoreMessages || m_currentConversationId == 0) {
        return;
    }

    m_loading = true;

    QVector<ChatMessage> messages = loadMessagesFromDatabase(limit, m_currentOffset);

    // 插入到模型开头
    for (int i = messages.size() - 1; i >= 0; --i) {
        m_messagesModel->insertMessage(0, messages[i]);
    }
    m_currentOffset += messages.size();

    // 更新是否有更多消息
    int totalCount = getMessageCountFromDatabase();
    m_hasMoreMessages = m_currentOffset < totalCount;

    m_loading = false;
    emit hasMoreMessagesChanged();
    emit messagesLoaded(m_hasMoreMessages);
}

void MessageController::refreshMessages()
{
    loadRecentMessages();
}


void MessageController::deleteSelectedMessages(const QVector<int>& messageIds)
{
    for (int messageId : messageIds) {
        deleteMessage(messageId);
    }
}

void MessageController::forwardSelectedMessages(const QVector<int>& messageIds, int targetConversationId)
{
    for (int messageId : messageIds) {
        forwardMessage(messageId, targetConversationId);
    }
}

void MessageController::markAsRead(int messageId)
{
    qDebug() << "标记消息为已读:" << messageId;
}

void MessageController::toggleMessageFavorite(int messageId)
{

}

void MessageController::downloadMedia(int messageId)
{

}

void MessageController::cancelDownload(int messageId)
{
    if (m_downloadTimers.contains(messageId)) {
        m_downloadTimers[messageId]->stop();
        m_downloadTimers.remove(messageId);
        m_downloadProgress.remove(messageId);
    }
}


// 私有方法实现
bool MessageController::saveMessageToDatabase(ChatMessage& message)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    QJsonObject json = messageToJson(message);
    bool success = m_dataAccessContext->messageTable()->saveMessage(json);

    if (success) {

    }

    return success;
}

bool MessageController::updateMessageInDatabase(const ChatMessage& message)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    QJsonObject json = messageToJson(message);
    return m_dataAccessContext->messageTable()->updateMessage(json);
}

bool MessageController::deleteMessageFromDatabase(int messageId)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    return m_dataAccessContext->messageTable()->deleteMessage(messageId);
}

QVector<ChatMessage> MessageController::loadMessagesFromDatabase(int limit, int offset)
{
    QVector<ChatMessage> messages;

    if (!m_dataAccessContext->messageTable() || m_currentConversationId == 0) {
        return messages;
    }
    QJsonArray jsonArray = m_dataAccessContext->messageTable()->getMessages(m_currentConversationId, limit, offset);
    if (jsonArray.isEmpty()) {
        return messages;
    }

    QString senderName,avatar;

    for (const QJsonValue& value : std::as_const(jsonArray)) {
        QJsonObject json = value.toObject();
        qint64 send_id = json["sender_id"].toVariant().toLongLong();

        if(m_dataAccessContext->contactTable()->isContact(send_id)){
            senderName = m_dataAccessContext->contactTable()->getRemarkName(send_id);
        }else{
            senderName = m_dataAccessContext->userTable()->getNickname(send_id);
        }
        avatar = m_dataAccessContext->userTable()->getAvatarLocalPath(send_id);

        ChatMessage message = jsonToMessage(json, senderName, avatar);
        messages.append(message);
    }

    return messages;
}

int MessageController::getMessageCountFromDatabase()
{
    if (!m_dataAccessContext->messageTable() || m_currentConversationId == 0) {
        return 0;
    }

    return m_dataAccessContext->messageTable()->getMessageCount(m_currentConversationId);
}

ChatMessage MessageController::createMessage(MessageType type, const QString& content,
                                             const QString& filePath, qint64 fileSize,
                                             int duration, const QString& thumbnailPath)
{
    return ChatMessage();
}

QJsonObject MessageController::messageToJson(const ChatMessage& message)
{
    QJsonObject json;
    json["message_id"] = message.messageId();
    json["conversation_id"] = message.conversationId();
    json["sender_id"] = message.senderId();
    json["type"] = static_cast<int>(message.type());
    json["content"] = message.content();
    json["file_path"] = message.filePath();
    json["file_url"] = message.fileUrl();
    json["file_size"] = static_cast<qint64>(message.fileSize());
    json["duration"] = message.duration();
    json["thumbnail_path"] = message.thumbnailPath();
    json["msg_time"] = message.timestamp();

    return json;
}

ChatMessage MessageController::jsonToMessage(const QJsonObject& json, const QString& senderName,
                                             const QString& avatar)
{
    int messageId = json["message_id"].toVariant().toLongLong();
    int conversationId = json["conversation_id"].toVariant().toLongLong();
    int senderId = json["sender_id"].toVariant().toLongLong();
    MessageType type = static_cast<MessageType>(json["type"].toInt());
    QString content = json["content"].toString();
    QString filePath = json["file_path"].toString();
    QString fileUrl = json["file_url"].toString();
    qint64 fileSize = json["file_size"].toVariant().toLongLong();
    int duration = json["duration"].toInt();
    QString thumbnailPath = json["thumbnail_path"].toString();
    qint64 timestamp = json["msg_time"].toVariant().toLongLong();

    return ChatMessage(messageId, conversationId, senderId, type, content,
                       filePath, fileUrl, fileSize, duration, thumbnailPath,
                       timestamp, senderName, avatar);
}

qint64 MessageController::calculateFileSize(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() ? fileInfo.size() : 0;
}


void MessageController::asyncDownloadMedia(const ChatMessage& message)
{
    // 实现异步下载媒体文件的逻辑
    Q_UNUSED(message);
}

int MessageController::generateTemporaryId()
{
    return m_temporaryIdCounter++;
}

void MessageController::handleSendSuccess(int temporaryId, int actualId)
{
    auto it = m_temporaryMessages.find(temporaryId);
    if (it != m_temporaryMessages.end()) {
        ChatMessage message = it.value();
        m_temporaryMessages.remove(temporaryId);
        // .......

    }
}

void MessageController::handleSendFailure(int temporaryId, const QString& error)
{
    auto it = m_temporaryMessages.find(temporaryId);
    if (it != m_temporaryMessages.end()) {
        m_temporaryMessages.remove(temporaryId);
        // 从模型中移除失败的消息
        //  实现移除逻辑
    }
    emit messageSendFailed(temporaryId, error);
}
