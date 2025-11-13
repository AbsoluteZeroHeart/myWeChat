#ifndef CONVERSATIONCONTROLLER_H
#define CONVERSATIONCONTROLLER_H
// 头文件防重复包含

#include <QObject>
#include <QAtomicInteger>     // 线程安全计数器
#include <QHash>              // 哈希表存储键值对
#include "Contact.h"
#include "DatabaseManager.h"  // 数据库管理器
#include "ChatListModel.h"    // 会话列表模型
#include "Conversation.h"     // 会话数据结构

/**
 * @class ConversationController
 * @brief 会话控制器，管理聊天会话核心操作（加载、创建、更新、删除等）
 */
class ConversationController : public QObject
{
    Q_OBJECT

public:
    explicit ConversationController(DatabaseManager* dbManager, QObject* parent = nullptr);
    ~ConversationController();

    // 获取会话列表模型（UI展示用）
    ChatListModel* chatListModel() const { return m_chatListModel; }

    // 异步操作：会话管理相关
    void loadConversations(int reqId);    // 加载所有会话
    void createSingleChat(Contact contact); // 创建单聊会话
    void createGroupChat(qint64 groupId); // 创建群聊会话

    void clearUnreadCount(qint64 conversationId);      // 清空未读消息数

    void deleteConversation(qint64 conversationId);    // 删除会话
    void handleToggleTop(qint64 conversationId);       // 处理置顶切换
    void handleToggleMute(qint64 conversationId);      // 处理静音切换
    void handleOpenInWindow(qint64 conversationId);    // 处理在新窗口打开会话
    void handleDelete(qint64 conversationId);          // 处理会话删除
    void handltoggleReadStatus(qint64 conversationId); // 切换已读状态


    void setCurrentConversationId(qint64 conversationId);


signals:
    // 操作结果信号
    void conversationCreated(int reqId, qint64 conversationId, bool success, const QString& error = QString()); // 会话创建结果
    void conversationInfoLoaded(int reqId, const QVariantMap& conversationInfo);         // 会话详情加载结果

    // 状态变更信号
    void errorOccurred(const QString& error); // 错误发生
    void conversationLoaded(QString functionCaller);

private slots:
    // 数据库操作结果处理
    void onAllConversationsLoaded(int reqId, const QList<Conversation>& conversations); // 所有会话加载完成
    void onConversationSaved(int reqId, bool success, const QString& error);   // 会话保存结果
    void onConversationUpdated(int reqId, bool success, const QString& error); // 会话更新结果
    void onConversationDeleted(int reqId, bool success, const qint64& conversationId); // 会话删除结果
    void onTopStatusToggled(int reqId, qint64 conversationId);                 // 处理顶置返回结果
    void onDbError(int reqId, const QString& error);                           // 数据库错误处理

private:
    int generateReqId();   // 生成唯一请求ID
    void connectSignals(); // 连接信号槽

    // 当前会话管理


private:
    DatabaseManager* m_dbManager;           // 数据库管理器
    ConversationTable* m_conversationTable; // 会话表操作接口
    ChatListModel* m_chatListModel;         // 会话列表模型（UI展示用）
    qint64 m_currentConversationId = -1;    // 当前选中会话ID
    QAtomicInteger<int> m_reqIdCounter;     // 请求ID计数器（线程安全）
    QHash<int, QString> m_pendingOperations;// 待处理操作（reqId->操作类型）
};

#endif // CONVERSATIONCONTROLLER_H
