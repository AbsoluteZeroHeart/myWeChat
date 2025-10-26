#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QSize>

enum class MessageType {
    TEXT = 0,
    IMAGE = 1,
    VIDEO = 2,
    FILE = 3,
    VOICE = 4
};

class ChatMessage {
public:
    // 构造函数
    ChatMessage() = default;

    ChatMessage(int messageId,
                int conversationId,
                int senderId,
                MessageType type,
                const QString& content = "",
                const QString& filePath = "",
                const QString& fileUrl = "",
                qint64 fileSize = 0,
                int duration = 0,
                const QString& thumbnailPath = "",
                const qint64& timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch()
,
                const QString& senderName = "",
                const QString& avatar = "")
        : m_messageId(messageId)
        , m_conversationId(conversationId)
        , m_senderId(senderId)
        , m_type(type)
        , m_content(content)
        , m_filePath(filePath)
        , m_fileUrl(fileUrl)
        , m_fileSize(fileSize)
        , m_duration(duration)
        , m_thumbnailPath(thumbnailPath)
        , m_timestamp(timestamp)
        , m_senderName(senderName)
        , m_avatar(avatar)
    {}

    // Getters
    int messageId() const { return m_messageId; }
    int conversationId() const { return m_conversationId; }
    int senderId() const { return m_senderId; }
    MessageType type() const { return m_type; }
    QString content() const { return m_content; }
    QString filePath() const { return m_filePath; }
    QString fileUrl() const { return m_fileUrl; }
    qint64 fileSize() const { return m_fileSize; }
    int duration() const { return m_duration; }
    QString thumbnailPath() const { return m_thumbnailPath; }
    qint64 timestamp() const { return m_timestamp; }
    QString senderName() const { return m_senderName; }
    QString avatar() const { return m_avatar; }

    // 便捷方法
    bool isText() const { return m_type == MessageType::TEXT; }
    bool isImage() const { return m_type == MessageType::IMAGE; }
    bool isVideo() const { return m_type == MessageType::VIDEO; }
    bool isFile() const { return m_type == MessageType::FILE; }
    bool isVoice() const { return m_type == MessageType::VOICE; }
    bool isMedia() const { return m_type != MessageType::TEXT; }
    bool hasFile() const { return !m_filePath.isEmpty() || !m_fileUrl.isEmpty(); }
    bool hasThumbnail() const { return !m_thumbnailPath.isEmpty(); }

    // 文件大小格式化显示
    QString formattedFileSize() const {
        if (m_fileSize == 0) return "0 B";

        static const QStringList units = {"B", "KB", "MB", "GB"};
        double size = m_fileSize;
        int unitIndex = 0;

        while (size >= 1024.0 && unitIndex < units.size() - 1) {
            size /= 1024.0;
            unitIndex++;
        }

        return QString("%1 %2").arg(size, 0, 'f', unitIndex > 0 ? 1 : 0).arg(units[unitIndex]);
    }

    // 时长格式化显示（用于音视频）
    QString formattedDuration() const {
        if (m_duration <= 0) return "00:00";

        int minutes = m_duration / 60;
        int seconds = m_duration % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    // Setters
    void setContent(const QString& content) { m_content = content; }
    void setFilePath(const QString& path) { m_filePath = path; }
    void setFileUrl(const QString& url) { m_fileUrl = url; }
    void setFileSize(qint64 size) { m_fileSize = size; }
    void setDuration(int duration) { m_duration = duration; }
    void setThumbnailPath(const QString& path) { m_thumbnailPath = path; }
    void setSenderName(const QString& name) { m_senderName = name; }
    void setAvatar(const QString& avatar) { m_avatar = avatar; }

private:
    // 数据库字段
    int m_messageId = 0;
    int m_conversationId = 0;
    int m_senderId = 0;
    MessageType m_type = MessageType::TEXT;
    QString m_content;
    QString m_filePath;
    QString m_fileUrl;
    qint64 m_fileSize = 0;
    int m_duration = 0;
    QString m_thumbnailPath;
    qint64 m_timestamp;

    QString m_senderName;
    QString m_avatar;
};

#endif // CHATMESSAGE_H
