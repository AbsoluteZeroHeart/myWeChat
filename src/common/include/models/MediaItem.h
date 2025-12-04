#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <QString>
#include <QtGui/QPixmap>
#include <QtSql/QSqlQuery>

// 媒体项数据结构
struct MediaItem {
    QString thumbnailPath;    // 缩略图路径
    QString sourceMediaPath;  // 源媒体路径
    QString mediaType;        // 媒体类型
    qint64 messageId;         // 消息ID
    qint64 timestamp;         // 时间戳

    MediaItem(const QString& thumbPath = "",
              const QString& sourcePath = "",
              const QString& type = "image",
              qint64 msgId = 0,
              qint64 time = 0)
        : thumbnailPath(thumbPath)
        , sourceMediaPath(sourcePath)
        , mediaType(type)
        , messageId(msgId)
        , timestamp(time)
    {}

    static MediaItem fromSqlQuery(const QSqlQuery& query) {
        MediaItem media;

        // 从查询结果中获取字段值
        media.messageId = query.value("message_id").toLongLong();
        media.sourceMediaPath = query.value("file_path").toString();
        if (media.sourceMediaPath.isEmpty()) {
            media.sourceMediaPath = query.value("file_url").toString();
        }
        media.thumbnailPath = query.value("thumbnail_path").toString();
        media.timestamp = query.value("msg_time").toLongLong();

        // 根据消息类型设置媒体类型
        int messageType = query.value("type").toInt();
        if (messageType == 1) {
            media.mediaType = "image";
        } else if (messageType == 2) {
            media.mediaType = "video";
        } else {
            media.mediaType = "unknown";
        }

        return media;
    }

    // 检查是否为有效的媒体项
    bool isValid() const {
        return !sourceMediaPath.isEmpty() && mediaType != "unknown";
    }

};

Q_DECLARE_METATYPE(MediaItem)


#endif // MEDIAITEM_H
