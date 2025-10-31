#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include "Message.h"
#include "ChatMessagesModel.h"
#include "MediaItem.h"

class DataAccessContext;

/**
 * @brief 消息控制器类，负责处理所有消息相关的业务逻辑
 *
 * 包括消息的发送、接收、删除、转发、复制等操作，以及消息数据的加载和管理
 */
class MessageController : public QObject
{
    Q_OBJECT

public:
    explicit MessageController(QObject *parent = nullptr);
    ~MessageController();

    // 属性访问方法
    ChatMessagesModel* messagesModel() const;
    qint64 currentConversationId() const;
    qint64 currentUserId() const;
    bool loading() const;
    bool hasMoreMessages() const;

    // 获取某会话的所有媒体
    QList<MediaItem> getMediaItems(qint64 conversationId);

public slots:
    // 会话管理
    void setCurrentConversationId(qint64 conversationId);
    void setCurrentUserId(qint64 userId);

    // 消息发送功能
    void sendTextMessage(const QString& content);
    void sendImageMessage(const QString& filePath, const QString& description = "");
    void sendVideoMessage(const QString& filePath, int duration, const QString& description = "");
    void sendFileMessage(const QString& filePath, const QString& fileName = "");
    void sendVoiceMessage(const QString& filePath, int duration);

    // 消息操作功能
    void recallMessage(qint64 messageId);
    void saveMessageMedia(qint64 messageId, const QString& savePath);
    void handleCopy(const Message &message);
    void handleZoom();
    void handleTranslate();
    void handleSearch();
    void handleForward();
    void handleFavorite();
    void handleRemind();
    void handleMultiSelect();
    void handleQuote();
    void handleDelete(const Message & message);

    // 消息加载功能
    void loadRecentMessages(int limit = 10);
    void loadMoreMessages(int limit = 10);
    void refreshMessages();

    // 消息状态管理
    void toggleMessageFavorite(qint64 messageId);
    void downloadMedia(qint64 messageId);
    void cancelDownload(qint64 messageId);

signals:
    // 状态变化信号
    void currentUserIdChanged();
    void hasMoreMessagesChanged();
    void messagesLoaded(bool hasMore);



private:
    // 核心组件
    ChatMessagesModel* m_messagesModel;      ///< 消息数据模型
    DataAccessContext* m_dataAccessContext;  ///< 数据访问上下文

    // 会话状态
    qint64 m_currentConversationId;          ///< 当前会话ID
    qint64 m_currentUserId;                  ///< 当前用户ID
    bool m_loading;                          ///< 是否正在加载消息
    bool m_hasMoreMessages;                  ///< 是否还有更多消息可加载
    int m_currentOffset;                     ///< 当前消息加载偏移量

    // 临时消息管理
    qint64 m_temporaryIdCounter;             ///< 临时ID计数器
    bool m_isSearchMode;                     ///< 是否为搜索模式
    QMap<qint64, Message> m_temporaryMessages; ///< 临时消息存储

    // 下载管理
    QMap<qint64, QTimer*> m_downloadTimers;  ///< 下载计时器
    QMap<qint64, int> m_downloadProgress;    ///< 下载进度

private:
    // 数据库操作
    bool saveMessageToDatabase(Message& message);
    bool updateMessageInDatabase(const Message& message);
    bool deleteMessageFromDatabase(qint64 messageId);
    QVector<Message> loadMessagesFromDatabase(int limit, int offset);
    int getMessageCountFromDatabase();

    // 消息创建和工具方法
    Message createMessage(MessageType type, const QString& content = "",
                          const QString& filePath = "", qint64 fileSize = 0,
                          int duration = 0, const QString& thumbnailPath = "");
    void asyncDownloadMedia(const Message& message);
    qint64 generateTemporaryId();
    void handleSendSuccess(qint64 temporaryId, qint64 actualId);
    void handleSendFailure(qint64 temporaryId, const QString& error);
    qint64 calculateFileSize(const QString& filePath);
};

#endif // MESSAGECONTROLLER_H
