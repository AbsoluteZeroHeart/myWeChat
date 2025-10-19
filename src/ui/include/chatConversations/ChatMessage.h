#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QSize>
#include "messageType.h"

class ChatMessage {
public:
    ChatMessage() :
        m_type(MessageType::TEXT),
        m_content(""),
        m_sender(""),
        m_timestamp(QDateTime()),
        m_extraData(QVariantMap())
    {}

    ChatMessage(MessageType type,
                const QString& content,
                const QString& sender,
                const QString &avatar,
                const QDateTime& timestamp,
                const QVariantMap& extraData = QVariantMap()):
        m_type(type),
        m_content(content),
        m_sender(sender),
        m_avatar(avatar),
        m_timestamp(timestamp),
        m_extraData(extraData)
    {}

    // Getters
    MessageType type() const { return m_type; }
    QString content() const { return m_content; }
    QString sender() const { return m_sender; }
    QDateTime timestamp() const { return m_timestamp; }
    QVariantMap extraData() const { return m_extraData; }
    bool isOwn() const {return sender() == "用户A"; }//currentUserId()
    QString avatar() const {return m_avatar;}

    // Setters
    void setContent(const QString& content) { m_content = content; }
    void setExtraData(const QVariantMap& data) { m_extraData = data; }

private:
    MessageType m_type;
    QString m_content;
    QString m_sender;
    QString m_avatar;
    QDateTime m_timestamp;
    QVariantMap m_extraData; // 存储文件路径、图片尺寸、语音时长等
};

#endif // CHATMESSAGE_H
