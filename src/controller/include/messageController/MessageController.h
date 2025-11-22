#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QObject>
#include <QAtomicInteger>  // 线程安全计数器
#include <QHash>           // 哈希表存储键值对
#include <QTimer>
#include <QMap>            // 有序映射表
#include "DatabaseManager.h"
#include "ChatMessagesModel.h"
#include "Message.h"
#include "MediaItem.h"
#include "User.h"

class ImageProcessor;
class FileCopyProcessor;
class VideoProcessor;

/**
 * @class MessageController
 * @brief 消息控制器，管理聊天消息核心操作（发送、接收、加载等）
 */
class MessageController : public QObject
{
    Q_OBJECT


public:
    explicit MessageController(DatabaseManager* dbManager, QObject* parent = nullptr); // 构造函数
    ~MessageController(); // 析构函数

    // 获取属性值
    ChatMessagesModel* messagesModel() const { return m_messagesModel; }
    qint64 currentConversationId() const { return m_currentConversationId; }

    // 异步操作：会话管理、消息发送/处理等
    void setCurrentConversationId(qint64 conversationId); // 设置当前会话
    void setCurrentUser(int reqId, User user);            // 设置当前用户

    void sendTextMessage(const QString& content);         // 发送文本消息
    void sendImageMessage(const qint64 conversationId,
                          const QString &originalImagePath,
                          const QString &thumbnailPath,
                          bool success);                 // 发送图片消息

    void sendVideoMessage(qint64 conversationId,
                          const QString &originalPath,
                          const QString &thumbnailPath,
                          bool success);                 // 发送视频消息

    void sendFileMessage(const qint64 conversationId,
                         bool success,
                         const QString &sourcePath,
                         const QString &targetPath,
                         const QString &errorMessage);    // 发送文件消息

    void sendVoiceMessage(const QString& filePath, int duration);           // 发送语音消息

    void preprocessImageBeforeSend(QStringList pathList);
    void preprocessFileBeforeSend(QStringList fileList);
    void preprocessVideoBeforeSend(QStringList fileList);

    void loadRecentMessages(int limit = 30);      // 加载最近消息
    void loadMoreMessages(int limit = 20);        // 加载更多历史消息
    void getMediaItems(qint64 conversationId);    // 获取会话中所有媒体项

    // UI操作
    Q_INVOKABLE void handleCopy(const Message &message); // 处理消息复制
    Q_INVOKABLE void handleZoom();        // 处理图片/视频缩放
    Q_INVOKABLE void handleTranslate();   // 处理消息翻译
    Q_INVOKABLE void handleSearch();      // 处理消息搜索
    Q_INVOKABLE void handleForward();     // 处理消息转发
    Q_INVOKABLE void handleFavorite();    // 处理消息收藏
    Q_INVOKABLE void handleRemind();      // 处理消息提醒
    Q_INVOKABLE void handleMultiSelect(); // 处理消息多选
    Q_INVOKABLE void handleQuote();       // 处理消息引用
    Q_INVOKABLE void handleDelete(const Message &message); // 处理消息删除

signals:
    // 操作结果信号
    void messageSaved();
    void messageDeleted(bool success, const QString& error = QString()); // 消息删除结果
    void messagesLoaded(const QList<Message>& messages, bool hasMore);   // 消息列表加载结果
    void mediaItemsLoaded(const QList<MediaItem>& items);                // 媒体项加载结果



private slots:
    // 数据库操作结果处理
    void onMessageSaved(int reqId, bool ok, QString reason);
    void onMessageDeleted(int reqId, bool success, const QString& error); // 消息删除结果
    void onMessagesLoaded(int reqId, const QList<Message>& messages);     // 消息列表加载结果
    void onMediaItemsLoaded(int reqId, const QList<MediaItem>& items);    // 媒体项加载结果
    void onDbError(int reqId, const QString& error);                      // 数据库错误处理

private:
    int generateReqId();   // 生成唯一请求ID
    void connectSignals(); // 连接信号槽
    Message createMessage(qint64 conversationId,
                          MessageType type,
                          const QString& content = QString(),
                          const QString& filePath = QString(),
                          qint64 fileSize = 0,
                          int duration = 0,
                          const QString& thumbnailPath = QString()); // 创建消息对象


private:
    DatabaseManager* dbManager; // 数据库管理器
    MessageTable* messageTable; // 消息表操作接口
    ContactTable* contactTable; // 联系人表操作接口
    UserTable* userTable;       // 用户表操作接口
    ChatMessagesModel* m_messagesModel; // 消息模型（UI展示用）
    QAtomicInteger<int> reqIdCounter;   // 请求ID计数器（线程安全）

    qint64 m_currentConversationId; // 当前会话ID
    User currentUser;  // 当前登录用户
    bool loading;      // 加载状态标记
    int currentOffset; // 消息加载偏移量（分页用）
    bool isSearchMode; // 是否搜索模式

    QHash<int, QString> pendingOperations;

    ImageProcessor *imageProcessor;
    FileCopyProcessor *fileCopyProcessor;
    VideoProcessor *videoProcessor;
};

#endif // MESSAGECONTROLLER_H
