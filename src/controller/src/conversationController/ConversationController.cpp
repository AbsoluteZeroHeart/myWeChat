#include "ConversationController.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include "ConversationTable.h"

ConversationController::ConversationController(DatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_conversationTable(nullptr)
    , m_chatListModel(new ChatListModel(this))
    , m_reqIdCounter(0)
{
    if (m_dbManager) {
        m_conversationTable = m_dbManager->conversationTable();
        connectSignals();
    } else {
        qWarning() << "DatabaseManager is null in ConversationController constructor";
    }
}

ConversationController::~ConversationController()
{}

int ConversationController::generateReqId()
{
    return m_reqIdCounter.fetchAndAddAcquire(1);
}

void ConversationController::connectSignals()
{
    if (!m_conversationTable) {
        qWarning() << "ConversationTable is null, cannot connect signals";
        return;
    }

    // 连接ConversationTable信号
    connect(m_conversationTable, &ConversationTable::allConversationsLoaded,
            this, &ConversationController::onAllConversationsLoaded);
    connect(m_conversationTable, &ConversationTable::conversationSaved,
            this, &ConversationController::onConversationSaved);
    connect(m_conversationTable, &ConversationTable::conversationUpdated,
            this, &ConversationController::onConversationUpdated);
    connect(m_conversationTable, &ConversationTable::conversationDeleted,
            this, &ConversationController::onConversationDeleted);
    connect(m_conversationTable, &ConversationTable::dbError,
            this, &ConversationController::onDbError);
    connect(m_conversationTable, &ConversationTable::topStatusToggled,
            this, &ConversationController::onTopStatusToggled);
}

void ConversationController::loadConversations(int reqId)
{
    if (!m_conversationTable) {
        emit errorOccurred("Conversation table not available");
        return;
    }

    QMetaObject::invokeMethod(m_conversationTable, "getAllConversations",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId));
}

void ConversationController::createSingleChat(Contact contact)
{
    Conversation conversation;
    conversation.userId = contact.userId;
    conversation.type = 0;
    conversation.title = contact.remarkName;
    conversation.avatar = contact.user.avatar;
    conversation.avatarLocalPath = contact.user.avatarLocalPath;

    int reqId = generateReqId();
    m_pendingOperations[reqId] = "createSingleChat";

    QMetaObject::invokeMethod(m_conversationTable, "saveConversation",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(Conversation,conversation));

}

void ConversationController::createGroupChat(qint64 groupId)
{
}

void ConversationController::clearUnreadCount(qint64 conversationId)
{
    int reqId = generateReqId();

    // 预先更新模型（乐观更新）
    if (m_chatListModel) {
        m_chatListModel->updateUnreadCount(conversationId, 0);
    }
    QMetaObject::invokeMethod(m_conversationTable, "setUnreadCount",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, conversationId),
                              Q_ARG(int, 0));
}

void ConversationController::handltoggleReadStatus(qint64 conversationId)
{
    if (!m_conversationTable) {
        return;
    }

    // 异步获取当前会话信息
    int reqId = generateReqId();
    int currentUnread = 0;
    if (m_chatListModel) {
        Conversation currentConv = m_chatListModel->getConversation(conversationId);
        if (currentConv.isValid()) {
            currentUnread = (currentConv.unreadCount>0)? 0:1;
        }
        m_chatListModel->updateUnreadCount(conversationId, currentUnread);
    }
    QMetaObject::invokeMethod(m_conversationTable, "setUnreadCount",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, conversationId),
                              Q_ARG(int, currentUnread));
}

void ConversationController::handleToggleTop(qint64 conversationId)
{
    if (!m_conversationTable) {
        return;
    }

    // 异步获取当前会话信息
    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_conversationTable, "toggleTopStatus",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, conversationId));
}

void ConversationController::deleteConversation(qint64 conversationId)
{
    if (!m_conversationTable) {
        return;
    }
    // 异步删除会话
    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_conversationTable, "deleteConversation",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, conversationId));
}

void ConversationController::handleToggleMute(qint64 conversationId)
{
    qDebug() << "Toggle mute for conversation:" << conversationId;
}

void ConversationController::handleOpenInWindow(qint64 conversationId)
{
    qDebug() << "Open conversation in window:" << conversationId;
}

void ConversationController::handleDelete(qint64 conversationId)
{
    deleteConversation(conversationId);
}

// 数据库操作结果处理槽函数
void ConversationController::onAllConversationsLoaded(int reqId, const QList<Conversation>& conversations)
{
    m_chatListModel->clearAll();
    for (const Conversation& conv : std::as_const(conversations)) {
        m_chatListModel->addConversation(conv);
    }
    QString functionCaller = m_pendingOperations[reqId];
    m_pendingOperations.remove(reqId);

    emit conversationLoaded(functionCaller);
}

void ConversationController::onConversationSaved(int reqId, bool success, const QString& error)
{
    if(!success)qDebug()<<"创建会话失败"<<error;
    loadConversations(reqId);
}

void ConversationController::onConversationUpdated(int reqId, bool success, const QString& error)
{
}

void ConversationController::onConversationDeleted(int reqId, bool success, const qint64& conversationId)
{
    if (success) {
        m_chatListModel->removeConversation(conversationId);
    } else {
        emit errorOccurred("删除会话失败");
    }
}

void ConversationController::onTopStatusToggled(int reqId, qint64 conversationId)
{
    loadConversations(0);
}

void ConversationController::onDbError(int reqId, const QString& error)
{
    qWarning() << "Database error in request" << reqId << ":" << error;

    // 发射错误信号
    emit errorOccurred(error);
}



// 当前会话相关操作
void ConversationController::setCurrentConversationId(qint64 conversationId)
{
    if (m_currentConversationId != conversationId) {
        m_currentConversationId = conversationId;
    }
}
