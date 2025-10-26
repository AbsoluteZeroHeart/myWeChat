#include "MediaCacheTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <stdexcept>

MediaCacheTable::MediaCacheTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool MediaCacheTable::saveMediaCache(const QJsonObject& mediaInfo)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO media_cache ("
                  "file_path, file_type, original_url, file_size, "
                  "access_count, last_access_time, created_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(mediaInfo.value("file_path").toString());
    query.addBindValue(mediaInfo.value("file_type").toInt());
    query.addBindValue(mediaInfo.value("original_url").toString());

    // 修复：使用 toInt() 而不是 toVariant()
    query.addBindValue(mediaInfo.value("file_size").toInt());

    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    query.addBindValue(mediaInfo.value("access_count").toInt(0));

    // 修复：使用 toInt() 并提供默认值
    qint64 lastAccessTime = mediaInfo.value("last_access_time").toInt(currentTime);
    qint64 createdTime = mediaInfo.value("created_time").toInt(currentTime);

    query.addBindValue(lastAccessTime);
    query.addBindValue(createdTime);

    if (!query.exec()) {
        qWarning() << "Save media cache failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MediaCacheTable::updateMediaCache(const QJsonObject& mediaInfo)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE media_cache SET "
                  "file_type = ?, original_url = ?, file_size = ?, "
                  "access_count = ?, last_access_time = ?, created_time = ? "
                  "WHERE file_path = ?");

    query.addBindValue(mediaInfo.value("file_type").toInt());
    query.addBindValue(mediaInfo.value("original_url").toString());

    // 修复：使用 toInt() 而不是 toVariant()
    query.addBindValue(mediaInfo.value("file_size").toInt());
    query.addBindValue(mediaInfo.value("access_count").toInt(0));

    // 修复：使用 toInt() 而不是 toVariant()
    query.addBindValue(mediaInfo.value("last_access_time").toInt());
    query.addBindValue(mediaInfo.value("created_time").toInt());
    query.addBindValue(mediaInfo.value("file_path").toString());

    if (!query.exec()) {
        qWarning() << "Update media cache failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool MediaCacheTable::deleteMediaCache(const QString& filePath)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM media_cache WHERE file_path = ?");
    query.addBindValue(filePath);

    if (!query.exec()) {
        qWarning() << "Delete media cache failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QJsonObject MediaCacheTable::getMediaCache(const QString& filePath)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM media_cache WHERE file_path = ?");
    query.addBindValue(filePath);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return mediaFromQuery(query);
}

QJsonObject MediaCacheTable::getMediaCacheByUrl(const QString& originalUrl)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM media_cache WHERE original_url = ?");
    query.addBindValue(originalUrl);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return mediaFromQuery(query);
}

bool MediaCacheTable::clearMediaCache()
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query("DELETE FROM media_cache", m_database);

    if (!query.exec()) {
        qWarning() << "Clear media cache failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MediaCacheTable::clearExpiredMediaCache(qint64 expireTime)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM media_cache WHERE last_access_time < ?");
    query.addBindValue(expireTime);

    if (!query.exec()) {
        qWarning() << "Clear expired media cache failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool MediaCacheTable::updateMediaAccess(const QString& filePath)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE media_cache SET "
                  "access_count = access_count + 1, "
                  "last_access_time = ? "
                  "WHERE file_path = ?");

    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(filePath);

    if (!query.exec()) {
        qWarning() << "Update media access failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QJsonArray MediaCacheTable::getLeastAccessedFiles(int limit)
{
    QJsonArray files;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return files;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM media_cache "
                  "ORDER BY access_count ASC, last_access_time ASC "
                  "LIMIT ?");
    query.addBindValue(limit);

    if (!query.exec()) {
        qWarning() << "Get least accessed files failed:" << query.lastError().text();
        return files;
    }

    while (query.next()) {
        files.append(mediaFromQuery(query));
    }

    return files;
}

QJsonArray MediaCacheTable::getMediaCacheByType(int fileType)
{
    QJsonArray files;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return files;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM media_cache WHERE file_type = ? ORDER BY last_access_time DESC");
    query.addBindValue(fileType);

    if (!query.exec()) {
        qWarning() << "Get media cache by type failed:" << query.lastError().text();
        return files;
    }

    while (query.next()) {
        files.append(mediaFromQuery(query));
    }

    return files;
}

qint64 MediaCacheTable::getTotalCacheSize()
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return -1;
    }

    QSqlQuery query("SELECT SUM(file_size) FROM media_cache", m_database);

    if (!query.exec() || !query.next()) {
        qWarning() << "Get total cache size failed:" << query.lastError().text();
        return -1;
    }

    return query.value(0).toLongLong();
}

bool MediaCacheTable::cleanupCache(qint64 maxSize)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    // 获取当前总大小
    qint64 currentSize = getTotalCacheSize();
    if (currentSize <= maxSize) {
        return true; // 无需清理
    }

    // 开始事务
    if (!m_database.transaction()) {
        qWarning() << "Begin transaction failed";
        return false;
    }

    try {
        // 获取需要删除的文件列表（按访问频率和最后访问时间排序）
        QSqlQuery query(m_database);
        query.prepare("SELECT file_path, file_size FROM media_cache "
                      "ORDER BY access_count ASC, last_access_time ASC");

        if (!query.exec()) {
            throw std::runtime_error("Query files for cleanup failed");
        }

        qint64 sizeToDelete = 0;
        QStringList filesToDelete;

        while (query.next() && (currentSize - sizeToDelete) > maxSize) {
            QString filePath = query.value("file_path").toString();
            qint64 fileSize = query.value("file_size").toLongLong();

            filesToDelete.append(filePath);
            sizeToDelete += fileSize;
        }

        // 删除选中的文件记录
        for (const QString& filePath : filesToDelete) {
            QSqlQuery deleteQuery(m_database);
            deleteQuery.prepare("DELETE FROM media_cache WHERE file_path = ?");
            deleteQuery.addBindValue(filePath);

            if (!deleteQuery.exec()) {
                throw std::runtime_error("Delete cache record failed");
            }
        }

        // 提交事务
        if (!m_database.commit()) {
            throw std::runtime_error("Commit transaction failed");
        }

        qDebug() << "Cache cleanup completed. Deleted" << filesToDelete.size()
                 << "files, freed" << sizeToDelete << "bytes";
        return true;

    } catch (const std::exception& e) {
        m_database.rollback();
        qWarning() << "Cache cleanup failed:" << e.what();
        return false;
    }
}

// 私有辅助方法
QJsonObject MediaCacheTable::mediaFromQuery(const QSqlQuery& query)
{
    QJsonObject media;

    media["cache_id"] = query.value("cache_id").toLongLong();
    media["file_path"] = query.value("file_path").toString();
    media["file_type"] = query.value("file_type").toInt();
    media["original_url"] = query.value("original_url").toString();
    media["file_size"] = query.value("file_size").toLongLong();
    media["access_count"] = query.value("access_count").toInt();
    media["last_access_time"] = query.value("last_access_time").toLongLong();
    media["created_time"] = query.value("created_time").toLongLong();

    return media;
}
