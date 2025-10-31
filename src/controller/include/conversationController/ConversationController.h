#ifndef CONVERSATIONCONTROLLER_H
#define CONVERSATIONCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "Conversation.h"
#include "ChatListModel.h"



class ChatListModel;
class DataAccessContext;

class ConversationController : public QObject
{
    Q_OBJECT

public:
    explicit ConversationController(QObject *parent = nullptr);
    ~ConversationController();

    ChatListModel* chatListModel() const;

    // 会话管理
    void loadConversations();
    bool createSingleChat(qint64 userId);
    bool createGroupChat(qint64 groupId);
    void deleteConversation(qint64 conversationId);

    // 会话状态管理
    void updateLastMessage(qint64 conversationId, const QString &message);
    void updateUnreadCount(qint64 conversationId, int count);
    void incrementUnreadCount(qint64 conversationId);
    void clearUnreadCount(qint64 conversationId);
    void toggleTopStatus(qint64 conversationId);

    // 查询方法
    Conversation getConversation(qint64 conversationId);
    QVariantMap getConversationInfo(qint64 conversationId);
    QVariantList searchConversations(const QString &keyword);

    // 便捷方法
    QString getConversationTitle(int type, qint64 targetId);
    QString getConversationAvatar(int type, qint64 targetId);

public slots:
    void onNewMessageReceived(qint64 conversationId, const QString &message, qint64 timestamp);
    void handleToggleTop(qint64 conversationId);
    void handltoggleReadStatus(qint64 conversationId);
    void handleToggleMute(qint64 conversationId);
    void handleOpenInWindow(qint64 conversationId);
    void handleDelete(qint64 conversationId);

signals:
    void conversationLoaded();
    void conversationCreated(qint64 conversationId);
    void conversationUpdated(qint64 conversationId);
    void conversationDeleted(qint64 conversationId);
    void errorOccurred(const QString& error);
    void openConversationInWindow(qint64 conversationId);

private:
    ChatListModel* m_chatListModel;
    DataAccessContext* m_dataAccessContext;

    void loadConversationsFromDatabase();
    bool updateConversationInDatabase(const Conversation& conversation);
};

#endif // CONVERSATIONCONTROLLER_H
