#include "MessageController.h"
#include "DataAccessContext.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QClipboard>

MessageController::MessageController(QObject *parent)
    : QObject(parent)
    , m_messagesModel(new ChatMessagesModel(this))
    , m_dataAccessContext(new DataAccessContext(this))
    , m_currentConversationId(-1)
    , m_currentUserId(-1)
    , m_loading(false)
    , m_hasMoreMessages(false)
    , m_currentOffset(0)
    , m_temporaryIdCounter(1000000)
    , m_isSearchMode(false)
{
}

MessageController::~MessageController()
{
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

qint64 MessageController::currentConversationId() const
{
    return m_currentConversationId;
}

qint64 MessageController::currentUserId() const
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

QList<MediaItem> MessageController::getMediaItems(qint64 conversationId)
{
    return m_dataAccessContext->messageTable()->getMediaItems(conversationId);
}

void MessageController::setCurrentConversationId(qint64 conversationId)
{
    if (m_currentConversationId != conversationId) {
        m_currentConversationId = conversationId;
        m_messagesModel->setConversationId(conversationId);
        m_currentOffset = 0;
        m_isSearchMode = false;

        loadRecentMessages();
        setCurrentUserId(m_dataAccessContext->userTable()->getCurrentUserId());
    }
}

void MessageController::setCurrentUserId(qint64 userId)
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
        return;
    }

    Message message = createMessage(MessageType::TEXT, content);
    qint64 temporaryId = generateTemporaryId();
    m_temporaryMessages[temporaryId] = message;

    // 先添加到模型（立即显示）
    m_messagesModel->addMessage(message);

    QTimer::singleShot(0, [this, temporaryId]() {
        auto it = m_temporaryMessages.find(temporaryId);
        if (it != m_temporaryMessages.end()) {
            Message& message = it.value();
            if (saveMessageToDatabase(message)) {
                handleSendSuccess(temporaryId, message.messageId);
            } else {
                handleSendFailure(temporaryId, "发送失败");
            }
        }
    });
}

void MessageController::sendImageMessage(const QString& filePath, const QString& description)
{
    // 实现图片消息发送
    Q_UNUSED(filePath);
    Q_UNUSED(description);
}

void MessageController::sendVideoMessage(const QString& filePath, int duration, const QString& description)
{
    // 实现视频消息发送
    Q_UNUSED(filePath);
    Q_UNUSED(duration);
    Q_UNUSED(description);
}

void MessageController::sendFileMessage(const QString& filePath, const QString& fileName)
{
    // 实现文件消息发送
    Q_UNUSED(filePath);
    Q_UNUSED(fileName);
}

void MessageController::sendVoiceMessage(const QString& filePath, int duration)
{
    // 实现语音消息发送
    Q_UNUSED(filePath);
    Q_UNUSED(duration);
}


void MessageController::recallMessage(qint64 messageId)
{
    // 实现消息撤回
    Q_UNUSED(messageId);
}

void MessageController::saveMessageMedia(qint64 messageId, const QString& savePath)
{
    Message message = m_messagesModel->getMessageById(messageId);
    if (!message.isValid() || !message.hasFile()) {
        return;
    }

    QString sourcePath = message.filePath;
    if (sourcePath.isEmpty()) {
        downloadMedia(messageId);
        return;
    }

    if (QFile::copy(sourcePath, savePath)) {
        qDebug() << "媒体文件已保存到:" << savePath;
    }
}

void MessageController::handleCopy(const Message &message)
{
    if(message.isText()){
        QApplication::clipboard()->setText(message.content);
    }
    else if(message.isImage()){
        QImage image(message.filePath);
        if(!image.isNull()) {
            QApplication::clipboard()->setImage(image);
            qDebug() << "图片复制成功";
        }
    }

}
void MessageController::handleZoom(){}
void MessageController::handleTranslate(){}
void MessageController::handleSearch(){}
void MessageController::handleForward(){}
void MessageController::handleFavorite(){}
void MessageController::handleRemind(){}
void MessageController::handleMultiSelect(){}
void MessageController::handleQuote(){}
void MessageController::handleDelete(const Message & message)
{
    if (deleteMessageFromDatabase(message.messageId)) {
        m_messagesModel->removeMessageById(message.messageId);
    }
}

void MessageController::loadRecentMessages(int limit)
{
    if (m_currentConversationId == -1) {
        return;
    }

    m_loading = true;
    m_currentOffset = 0;
    QVector<Message> messages = loadMessagesFromDatabase(limit, 0);

    m_messagesModel->clearAll();
    m_messagesModel->addMessages(messages);
    m_currentOffset = messages.size();

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

    QVector<Message> messages = loadMessagesFromDatabase(limit, m_currentOffset);

    for (int i = messages.size() - 1; i >= 0; --i) {
        m_messagesModel->insertMessage(0, messages[i]);
    }
    m_currentOffset += messages.size();

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

void MessageController::toggleMessageFavorite(qint64 messageId)
{
    // 实现消息收藏
    Q_UNUSED(messageId);
}

void MessageController::downloadMedia(qint64 messageId)
{
    // 实现媒体下载
    Q_UNUSED(messageId);
}

void MessageController::cancelDownload(qint64 messageId)
{
    if (m_downloadTimers.contains(messageId)) {
        m_downloadTimers[messageId]->stop();
        m_downloadTimers.remove(messageId);
        m_downloadProgress.remove(messageId);
    }
}

// 私有方法实现
bool MessageController::saveMessageToDatabase(Message& message)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    bool success = m_dataAccessContext->messageTable()->saveMessage(message);

    if (success) {
        // 发送成功处理
    }

    return success;
}

bool MessageController::updateMessageInDatabase(const Message& message)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    return m_dataAccessContext->messageTable()->updateMessage(message);
}

bool MessageController::deleteMessageFromDatabase(qint64 messageId)
{
    if (!m_dataAccessContext->messageTable()) {
        return false;
    }

    return m_dataAccessContext->messageTable()->deleteMessage(messageId);
}

QVector<Message> MessageController::loadMessagesFromDatabase(int limit, int offset)
{
    QVector<Message> messages;

    if (!m_dataAccessContext->messageTable() || m_currentConversationId == 0) {
        return messages;
    }

    messages = m_dataAccessContext->messageTable()->getMessages(m_currentConversationId, limit, offset);
    if (messages.isEmpty()) {
        return messages;
    }

    // 为消息添加发送者信息
    for (Message& message : messages) {
        qint64 senderId = message.senderId;

        if (m_dataAccessContext->contactTable()->isContact(senderId)) {
            message.senderName = m_dataAccessContext->contactTable()->getRemarkName(senderId);
        } else {
            message.senderName = m_dataAccessContext->userTable()->getNickname(senderId);
        }
        message.avatar = m_dataAccessContext->userTable()->getAvatarLocalPath(senderId);
    }
    std::reverse(messages.begin(), messages.end());

    return messages;
}

int MessageController::getMessageCountFromDatabase()
{
    if (!m_dataAccessContext->messageTable() || m_currentConversationId == 0) {
        return 0;
    }

    return m_dataAccessContext->messageTable()->getMessageCount(m_currentConversationId);
}

Message MessageController::createMessage(MessageType type, const QString& content,
                                         const QString& filePath, qint64 fileSize,
                                         int duration, const QString& thumbnailPath)
{
    Message message;
    message.messageId = generateTemporaryId();
    message.conversationId = m_currentConversationId;
    message.senderId = m_currentUserId;
    message.type = type;
    message.content = content;
    message.filePath = filePath;
    message.fileSize = fileSize;
    message.duration = duration;
    message.thumbnailPath = thumbnailPath;
    message.timestamp = QDateTime::currentSecsSinceEpoch();

    // 设置发送者信息
    message.senderName = m_dataAccessContext->userTable()->getNickname(m_currentUserId);
    message.avatar = m_dataAccessContext->userTable()->getAvatarLocalPath(m_currentUserId);

    return message;
}

qint64 MessageController::calculateFileSize(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() ? fileInfo.size() : 0;
}

void MessageController::asyncDownloadMedia(const Message& message)
{
    // 实现异步下载媒体文件的逻辑
    Q_UNUSED(message);
}

qint64 MessageController::generateTemporaryId()
{
    return m_temporaryIdCounter++;
}

void MessageController::handleSendSuccess(qint64 temporaryId, qint64 actualId)
{
    auto it = m_temporaryMessages.find(temporaryId);
    if (it != m_temporaryMessages.end()) {
        Message message = it.value();
        m_temporaryMessages.remove(temporaryId);
        // 更新消息ID并通知成功
        message.messageId = actualId;
        m_messagesModel->updateMessage(message);
    }
}

void MessageController::handleSendFailure(qint64 temporaryId, const QString& error)
{
    auto it = m_temporaryMessages.find(temporaryId);
    if (it != m_temporaryMessages.end()) {
        m_temporaryMessages.remove(temporaryId);
        // 从模型中移除失败的消息
        m_messagesModel->removeMessageById(temporaryId);
    }
}
