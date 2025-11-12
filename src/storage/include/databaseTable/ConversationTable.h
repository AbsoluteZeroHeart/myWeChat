#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/Conversation.h"

// 会话表数据访问类，负责处理会话相关的数据库操作
class ConversationTable : public QObject {
    Q_OBJECT

public:
    explicit ConversationTable(QObject *parent = nullptr);
    ~ConversationTable() override;

public slots:
    void init();

    // 异步操作：均携带reqId，用于关联并发请求与结果
    void saveConversation(int reqId, const Conversation &conversation);           // 保存会话（新增）
    void updateConversationPartial(int reqId, const Conversation &conversation);  // 部分更新会话信息（非全量更新）
    void deleteConversation(int reqId, qint64 conversationId);   // 根据会话ID删除会话

    void getAllConversations(int reqId);                         // 获取所有会话
    void getConversation(int reqId, qint64 conversationId);      // 根据会话ID获取单个会话

    void setUnreadCount(int reqId, qint64 conversationId, int unreadCount);     // 设置会话未读消息数量
    void toggleTopStatus(int reqId, qint64 conversationId);  // 切换会话的置顶状态（置顶/取消置顶）


signals:
    // 操作结果信号：携带reqId，与对应请求关联
    void conversationSaved(int reqId, bool ok, QString reason);           // 会话保存结果（成功状态及失败原因）
    void conversationUpdated(int reqId, bool ok, QString reason);         // 会话更新结果（成功状态及失败原因）
    void conversationDeleted(int reqId, bool ok, qint64 conversationId);  // 会话删除结果（成功状态及被删除的会话ID）

    void allConversationsLoaded(int reqId, QList<Conversation> conversations);  // 所有会话加载完成（返回会话列表）
    void conversationLoaded(int reqId, Conversation conversation);              // 单个会话加载完成（返回会话对象）

    void topStatusToggled(int reqId, qint64 conversationId);  // 会话置顶状态切换结果（返回被操作的会话ID）
    void dbError(int reqId, QString error);                   // 数据库操作错误信号（错误信息）

private:
    QSharedPointer<QSqlDatabase> m_database;  // 数据库连接智能指针，管理数据库连接的生命周期
};
