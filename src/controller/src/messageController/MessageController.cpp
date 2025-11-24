#include "MessageController.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include "MessageTable.h"
#include "UserTable.h"
#include <QMimeData>
#include "ImageProcessor.h"
#include "FileCopyProcessor.h"
#include <QDir>
#include <QStandardPaths>
#include "VideoProcessor.h"


MessageController::MessageController(DatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , dbManager(dbManager)
    , messageTable(nullptr)
    , contactTable(nullptr)
    , userTable(nullptr)
    , m_messagesModel(new ChatMessagesModel(this))
    , m_currentConversationId(-1)
    , currentUser(User())
    , loading(false)
    , currentOffset(0)
    , isSearchMode(false)
    , reqIdCounter(0)

    , imageProcessor(new ImageProcessor(this))
    , fileCopyProcessor(new FileCopyProcessor(this))
    , videoProcessor(new VideoProcessor(this))
{
    if (dbManager) {
        messageTable = dbManager->messageTable();
        contactTable = dbManager->contactTable();
        userTable = dbManager->userTable();
        connectSignals();
    } else {
        qWarning() << "DatabaseManager is null in MessageController constructor";
    }
    // 异步设置当前用户
    if (userTable) {
        int requestId = generateReqId();
        QMetaObject::invokeMethod(userTable, "getCurrentUser",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, requestId));
    }
}

MessageController::~MessageController()
{

}

int MessageController::generateReqId()
{
    return reqIdCounter.fetchAndAddAcquire(1);
}

void MessageController::connectSignals()
{
    if (!messageTable) {
        qWarning() << "MessageTable is null, cannot connect signals";
        return;
    }

    // 连接MessageTable信号
    connect(messageTable, &MessageTable::messageSaved, this, &MessageController::onMessageSaved);
    connect(messageTable, &MessageTable::messageDeleted, this, &MessageController::onMessageDeleted);
    connect(messageTable, &MessageTable::messagesLoaded, this, &MessageController::onMessagesLoaded);
    connect(messageTable, &MessageTable::mediaItemsLoaded, this, &MessageController::onMediaItemsLoaded);
    connect(messageTable, &MessageTable::dbError, this, &MessageController::onDbError);

    if(userTable){
        connect(userTable, &UserTable::currentUserLoaded, this, &MessageController::setCurrentUser);
    }

    if(imageProcessor){
        connect(imageProcessor, &ImageProcessor::imageProcessed, this, &MessageController::sendImageMessage);
    }

    if(fileCopyProcessor){
        connect(fileCopyProcessor, &FileCopyProcessor::fileCopied, this, &MessageController::sendFileMessage);
    }

    if(videoProcessor){
        connect(videoProcessor, &VideoProcessor::videoProcessed, this, &MessageController::sendVideoMessage);
    }
}

void MessageController::setCurrentConversationId(qint64 conversationId)
{
    if (m_currentConversationId != conversationId) {
        m_currentConversationId = conversationId;
        m_messagesModel->setConversationId(conversationId);
        currentOffset = 0;
        isSearchMode = false;

        loadRecentMessages();
    }
}

void MessageController::setCurrentUser(int reqId, User user)
{
    if (currentUser.userId != user.userId && user.userId != -1) {
        currentUser = user;
        m_messagesModel->setCurrentUserId(currentUser.userId);
    }
}


// 文本发送
void MessageController::sendTextMessage(const QString& content)
{
    if (content.trimmed().isEmpty() || m_currentConversationId == -1) {
        return;
    }

    Message message = createMessage(m_currentConversationId, MessageType::TEXT, content);

    // 先添加到模型（立即显示）
    m_messagesModel->addMessage(message);
    currentOffset += 1;

    // 异步保存到数据库
    int reqId = generateReqId();
    QMetaObject::invokeMethod(messageTable, "saveMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Message, message));
}


// 图片发送
void MessageController::preprocessImageBeforeSend(QStringList pathLis)
{
    imageProcessor->processImages(m_currentConversationId, pathLis);
}
void MessageController::sendImageMessage(const qint64 conversationId,
                                         const QString &originalImagePath,
                                         const QString &thumbnailPath,
                                         bool success)
{
    if(!success) return;

    QFileInfo fileInfo(originalImagePath);

    qint64 fileSize = fileInfo.size();
    Message message = createMessage(conversationId,
                                    MessageType::IMAGE,
                                    "【图片】"+fileInfo.fileName(),
                                    originalImagePath,
                                    fileSize,
                                    0,
                                    thumbnailPath);

    if(conversationId==m_currentConversationId){
        m_messagesModel->addMessage(message);
        currentOffset += 1;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(messageTable, "saveMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Message, message));
}


// 视频发送
void MessageController::preprocessVideoBeforeSend(QStringList fileList)
{
    videoProcessor->processVideos(m_currentConversationId, fileList);
}
void MessageController::sendVideoMessage(qint64 conversationId,
                                         const QString &originalPath,
                                         const QString &thumbnailPath,
                                         bool success)
{
    QFileInfo fileInfo(originalPath);
    qint64 fileSize = fileInfo.size();
    Message message = createMessage(conversationId,
                                    MessageType::VIDEO,
                                    "【视频】"+fileInfo.fileName(),
                                    originalPath,
                                    fileSize,
                                    0,
                                    thumbnailPath);


    if(conversationId==m_currentConversationId){
        m_messagesModel->addMessage(message);
        currentOffset += 1;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(messageTable, "saveMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Message, message));
}


// 文件发送
void MessageController::preprocessFileBeforeSend(QStringList fileList)
{
    fileCopyProcessor->copyFiles(m_currentConversationId, fileList);
}
void MessageController::sendFileMessage(const qint64 conversationId,
                                        bool success,
                                        const QString &sourcePath,
                                        const QString &targetPath,
                                        const QString &errorMessage)
{
    QFileInfo fileInfo(targetPath);
    qint64 fileSize = fileInfo.size();
    Message message = createMessage(conversationId, MessageType::FILE, "【文件】"+fileInfo.fileName(), targetPath, fileSize);

    if(conversationId==m_currentConversationId){
        m_messagesModel->addMessage(message);
        currentOffset += 1;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(messageTable, "saveMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Message, message));
}


void MessageController::sendVoiceMessage(const QString& filePath, int duration)
{
    if (filePath.isEmpty() || m_currentConversationId == -1) {
        return;
    }

    qint64 fileSize = 0;
    Message message = createMessage(m_currentConversationId, MessageType::VOICE, "语音消息", filePath, fileSize, duration);

    m_messagesModel->addMessage(message);

    int reqId = generateReqId();

    QMetaObject::invokeMethod(messageTable, "saveMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Message, message));
}


// 异步查询加载操作
void MessageController::loadRecentMessages(int limit)
{
    if (m_currentConversationId == -1 || !messageTable) {
        return;
    }

    loading = true;
    currentOffset = 0;

    int reqId = generateReqId();
    pendingOperations.insert(reqId, "loadRecentMessages");

    QMetaObject::invokeMethod(messageTable, "getMessages",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, m_currentConversationId),
                              Q_ARG(int, limit),
                              Q_ARG(int, 0));
}

void MessageController::loadMoreMessages(int limit)
{
    if (loading || m_currentConversationId == -1 || !messageTable) {
        return;
    }

    loading = true;

    int reqId = generateReqId();
    pendingOperations.insert(reqId, "loadMoreMessages");

    QMetaObject::invokeMethod(messageTable, "getMessages",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, m_currentConversationId),
                              Q_ARG(int, limit),
                              Q_ARG(int, currentOffset));
}

void MessageController::getMediaItems(qint64 conversationId)
{
    if (!messageTable) {
        emit mediaItemsLoaded(QList<MediaItem>());
        return;
    }

    int reqId = generateReqId();

    QMetaObject::invokeMethod(messageTable, "getMediaItems",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, conversationId));
}


// 控制UI 操作
void MessageController::handleCopy(const Message &message)
{
    if(message.isText()){
        QApplication::clipboard()->setText(message.content);
    }
    else if(message.isMedia() || message.isFile() || message.isImage())
    {
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(message.filePath));

        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(urls);
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

void MessageController::handleZoom()
{
    // 实现缩放逻辑
}

void MessageController::handleTranslate()
{
    // 实现翻译逻辑
}

void MessageController::handleSearch()
{
    // 实现搜索逻辑
}

void MessageController::handleForward()
{
    // 实现转发逻辑
}

void MessageController::handleFavorite()
{
    // 实现收藏逻辑
}

void MessageController::handleRemind()
{
    // 实现提醒逻辑
}

void MessageController::handleMultiSelect()
{
    // 实现多选逻辑
}

void MessageController::handleQuote()
{
    // 实现引用逻辑
}

void MessageController::handleDelete(const Message &message)
{
    if (!messageTable) {
        return;
    }

    // 先从模型和文件系统移除
    m_messagesModel->removeMessageById(message.messageId);
    QFile file(message.filePath);
    QFile thumbnail(message.thumbnailPath);

    file.remove();
    thumbnail.remove();

    // 异步从数据库删除
    int reqId = generateReqId();

    QMetaObject::invokeMethod(messageTable, "deleteMessage",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, message.messageId));
}


// 处理数据库异步信号
void MessageController::onMessageSaved(int reqId, bool ok, QString reason)
{
    if(!ok){
        qDebug()<<reason;
    }
    else {
        emit messageSaved();
        loadRecentMessages();
    }
}

void MessageController::onMessageDeleted(int reqId, bool success, const QString& error)
{
    emit messageDeleted(success, error);
}

void MessageController::onMessagesLoaded(int reqId, const QVector<Message>& messages)
{
    QString operation = pendingOperations.take(reqId);
    loading = false;

    if (operation == "loadRecentMessages") {
        // 加载最近消息
        m_messagesModel->clearAll();
        m_messagesModel->addMessages(messages);
        currentOffset = messages.count();
    } else if (operation == "loadMoreMessages") {
        // 加载更多历史消息
        for(int i = messages.count()-1; i >= 0; i--){
            m_messagesModel->insertMessage(0, messages[i]);
        }
        currentOffset += messages.count();
    }
}

void MessageController::onMediaItemsLoaded(int reqId, const QList<MediaItem>& items)
{
    emit mediaItemsLoaded(items);
}

void MessageController::onDbError(int reqId, const QString& error)
{
    qWarning() << "Database error in request" << reqId << ":" << error;


}

// 私有辅助方法
Message MessageController::createMessage(qint64 conversationId,
                                         MessageType type,
                                         const QString& content,
                                         const QString& filePath,
                                         qint64 fileSize,
                                         int duration,
                                         const QString& thumbnailPath)
{
    Message message;
    message.messageId = QDateTime::currentMSecsSinceEpoch();
    message.conversationId = conversationId;

    message.senderId = currentUser.userId;
    message.senderName = currentUser.nickname;
    message.avatar = currentUser.avatarLocalPath;

    message.type = type;
    message.content = content;
    message.filePath = filePath;
    message.fileSize = fileSize;
    message.duration = duration;
    message.thumbnailPath = thumbnailPath;
    message.timestamp = QDateTime::currentSecsSinceEpoch();

    return message;
}

