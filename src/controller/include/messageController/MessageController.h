#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QHash>
#include "ChatMessage.h"
#include "ChatMessagesModel.h"

class DataAccessContext;
class MessageController : public QObject
{
    Q_OBJECT

public:
    explicit MessageController(QObject *parent = nullptr);
    ~MessageController();

    // 属性访问器
    ChatMessagesModel* messagesModel() const;
    int currentConversationId() const;
    int currentUserId() const;
    bool loading() const;
    bool hasMoreMessages() const;

public slots:
    // 会话管理
    void setCurrentConversationId(int conversationId);
    void setCurrentUserId(int userId);

    // 消息发送
    void sendTextMessage(const QString& content);
    void sendImageMessage(const QString& filePath, const QString& description = "");
    void sendVideoMessage(const QString& filePath, int duration, const QString& description = "");
    void sendFileMessage(const QString& filePath, const QString& fileName);
    void sendVoiceMessage(const QString& filePath, int duration);

    // 消息操作
    void deleteMessage(int messageId);
    void recallMessage(int messageId);
    void forwardMessage(int messageId, int targetConversationId);
    void copyMessageContent(int messageId);
    void saveMessageMedia(int messageId, const QString& savePath);

    // 消息查询和加载
    void loadRecentMessages(int limit = 50);
    void loadMoreMessages(int limit = 20);
    void refreshMessages();

    // 批量操作
    void deleteSelectedMessages(const QVector<int>& messageIds);
    void forwardSelectedMessages(const QVector<int>& messageIds, int targetConversationId);

    // 消息状态更新
    void markAsRead(int messageId);
    void toggleMessageFavorite(int messageId);

    // 媒体处理
    void downloadMedia(int messageId);
    void cancelDownload(int messageId);

signals:
    void messagesModelChanged();
    void currentConversationIdChanged();
    void currentUserIdChanged();
    void hasMoreMessagesChanged();

    // 操作结果信号
    void messageSent(int messageId);
    void messageSendFailed(int temporaryId, const QString& error);
    void messageDeleted(int messageId);
    void messageRecalled(int messageId);
    void messagesLoaded(bool hasMore);
    void errorOccurred(const QString& error);
    void mediaDownloadProgress(int messageId, int progress);
    void mediaDownloadFinished(int messageId, const QString& filePath);


private:
    // 数据库操作
    bool saveMessageToDatabase(ChatMessage& message);
    bool updateMessageInDatabase(const ChatMessage& message);
    bool deleteMessageFromDatabase(int messageId);
    QVector<ChatMessage> loadMessagesFromDatabase(int limit, int offset);
    int getMessageCountFromDatabase();

    // 消息处理
    ChatMessage createMessage(MessageType type, const QString& content = "",
                              const QString& filePath = "", qint64 fileSize = 0,
                              int duration = 0, const QString& thumbnailPath = "");
    QJsonObject messageToJson(const ChatMessage& message);
    ChatMessage jsonToMessage(const QJsonObject& json, const QString& senderName = "",
                              const QString& avatar = "");

    // 文件处理
    qint64 calculateFileSize(const QString& filePath);
     void asyncDownloadMedia(const ChatMessage& message);

    // 临时消息管理
    int generateTemporaryId();
    void handleSendSuccess(int temporaryId, int actualId);
    void handleSendFailure(int temporaryId, const QString& error);

private:
    ChatMessagesModel* m_messagesModel;
    DataAccessContext* m_dataAccessContext;
    int m_currentConversationId;
    int m_currentUserId;
    bool m_loading;
    bool m_hasMoreMessages;
    int m_currentOffset;

    // 临时消息管理
    QHash<int, ChatMessage> m_temporaryMessages;
    int m_temporaryIdCounter;

    // 下载管理
    QHash<int, QTimer*> m_downloadTimers;
    QHash<int, int> m_downloadProgress;

    // 搜索状态
    QString m_currentSearchKeyword;
    bool m_isSearchMode;
};

#endif // MESSAGECONTROLLER_H
