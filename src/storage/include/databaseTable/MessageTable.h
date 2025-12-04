#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/Message.h"
#include "models/MediaItem.h"

class MessageTable : public QObject {
    Q_OBJECT
public:
    explicit MessageTable(QObject *parent = nullptr);
    ~MessageTable() override;

    void init();

public slots:
    // 异步操作（带 reqId）
    void saveMessage(int reqId, Message message);
    void updateMessage(int reqId, Message message);
    void deleteMessage(int reqId, qint64 messageId);

    void getMessages(int reqId, qint64 conversationId, int limit, int offset);
    void getMessage(int reqId, qint64 messageId);
    void getLastMessage(int reqId, qint64 conversationId);

    void clearMessages(int reqId);
    void clearConversationMessages(int reqId, qint64 conversationId);

    void getMessagesByTimeRange(int reqId, qint64 conversationId, qint64 startTime, qint64 endTime);
    void getMessageCount(int reqId, qint64 conversationId);

    void getMediaItems(int reqId, qint64 conversationId);

signals:

    void messageSaved(int reqId, bool ok, QString reason);
    void messageUpdated(int reqId, bool ok, QString reason);
    void messageDeleted(int reqId, bool ok, QString reason);

    void messagesLoaded(int reqId, QVector<Message> messages);
    void messageLoaded(int reqId, Message message);
    void lastMessageLoaded(int reqId, Message message);

    void messagesCleared(int reqId, bool ok, QString reason);
    void conversationMessagesCleared(int reqId, bool ok, QString reason);

    void messagesByTimeRangeLoaded(int reqId, QList<Message> messages);
    void messageCountLoaded(int reqId, int count);

    void mediaItemsLoaded(int reqId, QList<MediaItem> items);

    // 通用错误
    void dbError(int reqId, QString error);

private:
    QSharedPointer<QSqlDatabase> m_database;

};
