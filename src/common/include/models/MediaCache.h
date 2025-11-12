#ifndef MEDIACACHE_H
#define MEDIACACHE_H

#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QDateTime>

struct MediaCache {
    qint64 cacheId = 0;
    QString filePath;
    int fileType = 0;           // 文件类型：0-图片，1-视频，2-音频，3-文件等
    QString originalUrl;
    qint64 fileSize = 0;        // 文件大小（字节）
    int accessCount = 0;        // 访问次数
    qint64 lastAccessTime = 0;  // 最后访问时间（时间戳）
    qint64 createdTime = 0;     // 创建时间（时间戳）

    MediaCache() = default;
    
    static MediaCache fromSqlQuery(const QSqlQuery& query) {
        MediaCache cache;
        cache.cacheId = query.value("cache_id").toLongLong();
        cache.filePath = query.value("file_path").toString();
        cache.fileType = query.value("file_type").toInt();
        cache.originalUrl = query.value("original_url").toString();
        cache.fileSize = query.value("file_size").toLongLong();
        cache.accessCount = query.value("access_count").toInt();
        cache.lastAccessTime = query.value("last_access_time").toLongLong();
        cache.createdTime = query.value("created_time").toLongLong();
        return cache;
    }

    QJsonObject toJson() const {
        return {
            {"cache_id", QString::number(cacheId)},
            {"file_path", filePath},
            {"file_type", fileType},
            {"original_url", originalUrl},
            {"file_size", fileSize},
            {"access_count", accessCount},
            {"last_access_time", lastAccessTime},
            {"created_time", createdTime}
        };
    }

    static MediaCache fromJson(const QJsonObject& json) {
        MediaCache cache;
        cache.cacheId = json["cache_id"].toString().toLongLong();
        cache.filePath = json["file_path"].toString();
        cache.fileType = json["file_type"].toInt();
        cache.originalUrl = json["original_url"].toString();
        cache.fileSize = json["file_size"].toVariant().toLongLong();
        cache.accessCount = json["access_count"].toInt();
        cache.lastAccessTime = json["last_access_time"].toVariant().toLongLong();
        cache.createdTime = json["created_time"].toVariant().toLongLong();
        return cache;
    }

    bool isValid() const {
        return !filePath.isEmpty() && fileSize >= 0;
    }

    // 便捷方法
    bool isImage() const { return fileType == 0; }
    bool isVideo() const { return fileType == 1; }
    bool isAudio() const { return fileType == 2; }
    bool isFile() const { return fileType == 3; }
    
    QString getFileSizeHumanReadable() const {
        if (fileSize < 1024) {
            return QString("%1 B").arg(fileSize);
        } else if (fileSize < 1024 * 1024) {
            return QString("%1 KB").arg(fileSize / 1024.0, 0, 'f', 1);
        } else if (fileSize < 1024 * 1024 * 1024) {
            return QString("%1 MB").arg(fileSize / (1024.0 * 1024.0), 0, 'f', 1);
        } else {
            return QString("%1 GB").arg(fileSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
        }
    }

    bool isExpired(qint64 expireThreshold) const {
        return lastAccessTime < expireThreshold;
    }

    void updateAccessTime() {
        accessCount++;
        lastAccessTime = QDateTime::currentSecsSinceEpoch();
    }
};

Q_DECLARE_METATYPE(MediaCache)

#endif // MEDIACACHE_H
