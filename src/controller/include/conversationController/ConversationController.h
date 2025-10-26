#ifndef CONVERSATIONCONTROLLER_H
#define CONVERSATIONCONTROLLER_H

#include <QObject>
#include <QDateTime>
#include "ChatListModel.h"


class DataAccessContext;   

class ConversationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ChatListModel* chatListModel READ chatListModel NOTIFY chatListModelChanged)

public:
    explicit ConversationController(QObject *parent = nullptr);
    ~ConversationController();

    // 获取聊天列表模型
    ChatListModel* chatListModel() const;

    // 加载会话列表
    Q_INVOKABLE void loadConversations();

    // 创建新会话
    Q_INVOKABLE bool createSingleChat(qint64 userId);
    Q_INVOKABLE bool createGroupChat(qint64 groupId);

    // 更新会话最后消息
    Q_INVOKABLE void updateLastMessage(qint64 conversationId, const QString &message);

    // 更新未读计数
    Q_INVOKABLE void updateUnreadCount(qint64 conversationId, int count);
    Q_INVOKABLE void incrementUnreadCount(qint64 conversationId);
    Q_INVOKABLE void clearUnreadCount(qint64 conversationId);
    Q_INVOKABLE void toggleReadStatus(qint64 conversationId);


    // 切换置顶会话
    Q_INVOKABLE void toggleTopStatus(qint64 conversationId);

    // 删除会话
    Q_INVOKABLE void deleteConversation(qint64 conversationId);

    // 获取会话信息
    Q_INVOKABLE QVariantMap getConversationInfo(qint64 conversationId);

    // 搜索会话
    Q_INVOKABLE QVariantList searchConversations(const QString &keyword);

public slots:
    void handleToggleTop(qint64 conversationId);
    void handleMarkAsUnread(qint64 conversationId);
    void handleToggleMute(qint64 conversationId);
    void handleOpenInWindow(qint64 conversationId);
    void handleDelete(qint64 conversationId);


signals:
    void chatListModelChanged();
    void conversationLoaded();
    void conversationCreated(qint64 conversationId);
    void conversationUpdated(qint64 conversationId);
    void conversationDeleted(qint64 conversationId);
    void openConversationInWindow(qint64 conversationId);
    void errorOccurred(const QString &errorMessage);

private slots:
    void onNewMessageReceived(qint64 conversationId, const QString &message, qint64 timestamp);

private:
    // 从数据库加载会话到模型
    void loadConversationsFromDatabase();

    // 更新数据库中的会话信息
    bool updateConversationInDatabase(const ConversationInfo &conversationInfo);

    // 创建会话信息对象
    ConversationInfo createConversationInfo(const QJsonObject &dbData);

    // 设置会话标题
    QString getConversationTitle(int type, qint64 targetId);

    // 设置会话头像
    QString getConversationAvatar(int type, qint64 targetId);

private:
    ChatListModel* m_chatListModel;
    DataAccessContext * m_dataAccessContext;
};

#endif // CONVERSATIONCONTROLLER_H
