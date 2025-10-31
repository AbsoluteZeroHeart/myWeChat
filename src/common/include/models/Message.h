#ifndef MESSAGE_H
#define MESSAGE_H

#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDateTime>

// 消息类型枚举
enum class MessageType {
    TEXT = 0,
    IMAGE = 1,
    VIDEO = 2,
    FILE = 3,
    VOICE = 4
};

struct Message {
    qint64 messageId = 0;
    qint64 conversationId = 0;
    qint64 senderId = 0;
    MessageType type = MessageType::TEXT;
    QString content;
    QString filePath;
    QString fileUrl;
    qint64 fileSize = 0;
    int duration = 0;
    QString thumbnailPath;
    qint64 timestamp = 0;

    // 发送者信息（非数据库字段，用于显示）
    QString senderName;
    QString avatar;

    Message() = default;
    
    explicit Message(const QSqlQuery& query) {
        messageId = query.value("message_id").toLongLong();
        conversationId = query.value("conversation_id").toLongLong();
        senderId = query.value("sender_id").toLongLong();
        type = static_cast<MessageType>(query.value("type").toInt());
        content = query.value("content").toString();
        filePath = query.value("file_path").toString();
        fileUrl = query.value("file_url").toString();
        fileSize = query.value("file_size").toLongLong();
        duration = query.value("duration").toInt();
        thumbnailPath = query.value("thumbnail_path").toString();
        timestamp = query.value("msg_time").toLongLong();
    }

    QJsonObject toJson() const {
        return {
            {"message_id", QString::number(messageId)},
            {"conversation_id", QString::number(conversationId)},
            {"sender_id", QString::number(senderId)},
            {"type", static_cast<int>(type)},
            {"content", content},
            {"file_path", filePath},
            {"file_url", fileUrl},
            {"file_size", fileSize},
            {"duration", duration},
            {"thumbnail_path", thumbnailPath},
            {"msg_time", timestamp}
        };
    }

    static Message fromJson(const QJsonObject& json) {
        Message msg;
        msg.messageId = json["message_id"].toString().toLongLong();
        msg.conversationId = json["conversation_id"].toString().toLongLong();
        msg.senderId = json["sender_id"].toString().toLongLong();
        msg.type = static_cast<MessageType>(json["type"].toInt());
        msg.content = json["content"].toString();
        msg.filePath = json["file_path"].toString();
        msg.fileUrl = json["file_url"].toString();
        msg.fileSize = json["file_size"].toVariant().toLongLong();
        msg.duration = json["duration"].toInt();
        msg.thumbnailPath = json["thumbnail_path"].toString();
        msg.timestamp = json["msg_time"].toVariant().toLongLong();
        return msg;
    }

    bool isValid() const {
        return messageId > 0 && conversationId > 0 && senderId > 0;
    }

    // 便捷方法
    bool isText() const { return type == MessageType::TEXT; }
    bool isImage() const { return type == MessageType::IMAGE; }
    bool isVideo() const { return type == MessageType::VIDEO; }
    bool isFile() const { return type == MessageType::FILE; }
    bool isVoice() const { return type == MessageType::VOICE; }
    bool isMedia() const { return isImage() || isVideo() || isVoice(); }
    bool hasFile() const { return !filePath.isEmpty() || !fileUrl.isEmpty(); }
    bool hasThumbnail() const { return !thumbnailPath.isEmpty(); }

    QString formattedFileSize() const {
        if (fileSize == 0) return "0 B";
        
        static const QStringList units = {"B", "KB", "MB", "GB"};
        double size = fileSize;
        int unitIndex = 0;
        
        while (size >= 1024 && unitIndex < units.size() - 1) {
            size /= 1024;
            unitIndex++;
        }
        
        return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
    }

    QString formattedDuration() const {
        if (duration <= 0) return "0:00";
        
        int minutes = duration / 60;
        int seconds = duration % 60;
        return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
    }

    bool isOwn(qint64 currentUserId) const {
        return senderId == currentUserId;
    }
};

#endif // MESSAGE_H
