#include "MediaCacheTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include "DbConnectionManager.h"

MediaCacheTable::MediaCacheTable(QObject *parent)
    : QObject(parent)
{
}

MediaCacheTable::~MediaCacheTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void MediaCacheTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

void MediaCacheTable::saveMediaCache(int reqId, MediaCache mediaCache)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaCacheSaved(reqId, false, "Database is not open");
        return;
    }
    if (!mediaCache.isValid()) {
        emit mediaCacheSaved(reqId, false, "Invalid media cache data");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("INSERT OR REPLACE INTO media_cache ("
                  "file_path, file_type, original_url, file_size, "
                  "access_count, last_access_time, created_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(mediaCache.filePath);
    query.addBindValue(mediaCache.fileType);
    query.addBindValue(mediaCache.originalUrl);
    query.addBindValue(mediaCache.fileSize);
    query.addBindValue(mediaCache.accessCount);
    query.addBindValue(mediaCache.lastAccessTime);
    query.addBindValue(mediaCache.createdTime);

    if (!query.exec()) {
        emit mediaCacheSaved(reqId, false, query.lastError().text());
        return;
    }

    emit mediaCacheSaved(reqId, true, QString());
}

void MediaCacheTable::updateMediaCache(int reqId, MediaCache mediaCache)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaCacheUpdated(reqId, false, "Database is not open");
        return;
    }
    if (!mediaCache.isValid()) {
        emit mediaCacheUpdated(reqId, false, "Invalid media cache data");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE media_cache SET "
                  "file_type = ?, original_url = ?, file_size = ?, "
                  "access_count = ?, last_access_time = ?, created_time = ? "
                  "WHERE file_path = ?");

    query.addBindValue(mediaCache.fileType);
    query.addBindValue(mediaCache.originalUrl);
    query.addBindValue(mediaCache.fileSize);
    query.addBindValue(mediaCache.accessCount);
    query.addBindValue(mediaCache.lastAccessTime);
    query.addBindValue(mediaCache.createdTime);
    query.addBindValue(mediaCache.filePath);

    if (!query.exec()) {
        emit mediaCacheUpdated(reqId, false, query.lastError().text());
        return;
    }

    emit mediaCacheUpdated(reqId, query.numRowsAffected() > 0, QString());
}

void MediaCacheTable::deleteMediaCache(int reqId, const QString &filePath)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaCacheDeleted(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM media_cache WHERE file_path = ?");
    query.addBindValue(filePath);

    if (!query.exec()) {
        emit mediaCacheDeleted(reqId, false, query.lastError().text());
        return;
    }

    emit mediaCacheDeleted(reqId, query.numRowsAffected() > 0, QString());
}

void MediaCacheTable::getMediaCache(int reqId, const QString &filePath)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit mediaCacheLoaded(reqId, MediaCache());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM media_cache WHERE file_path = ?");
    query.addBindValue(filePath);

    if (!query.exec() || !query.next()) {
        emit mediaCacheLoaded(reqId, MediaCache());
        return;
    }

    emit mediaCacheLoaded(reqId, mediaFromQuery(query));
}

void MediaCacheTable::getMediaCacheByUrl(int reqId, const QString &originalUrl)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit mediaCacheByUrlLoaded(reqId, MediaCache());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM media_cache WHERE original_url = ?");
    query.addBindValue(originalUrl);

    if (!query.exec() || !query.next()) {
        emit mediaCacheByUrlLoaded(reqId, MediaCache());
        return;
    }

    emit mediaCacheByUrlLoaded(reqId, mediaFromQuery(query));
}

void MediaCacheTable::clearMediaCache(int reqId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaCacheCleared(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query("DELETE FROM media_cache", *m_database);
    if (!query.exec()) {
        emit mediaCacheCleared(reqId, false, query.lastError().text());
        return;
    }

    emit mediaCacheCleared(reqId, true, QString());
}

void MediaCacheTable::clearExpiredMediaCache(int reqId, qint64 expireTime)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaCacheExpiredCleared(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM media_cache WHERE last_access_time < ?");
    query.addBindValue(expireTime);

    if (!query.exec()) {
        emit mediaCacheExpiredCleared(reqId, false, query.lastError().text());
        return;
    }

    emit mediaCacheExpiredCleared(reqId, true, QString());
}

void MediaCacheTable::updateMediaAccess(int reqId, const QString &filePath)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit mediaAccessUpdated(reqId, false);
        emit dbError(reqId, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE media_cache SET "
                  "access_count = access_count + 1, "
                  "last_access_time = ? "
                  "WHERE file_path = ?");

    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(filePath);

    if (!query.exec()) {
        emit mediaAccessUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit mediaAccessUpdated(reqId, query.numRowsAffected() > 0);
}

void MediaCacheTable::getLeastAccessedFiles(int reqId, int limit)
{
    QList<MediaCache> files;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit leastAccessedFilesLoaded(reqId, files);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM media_cache "
                  "ORDER BY access_count ASC, last_access_time ASC "
                  "LIMIT ?");
    query.addBindValue(limit);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit leastAccessedFilesLoaded(reqId, files);
        return;
    }

    while (query.next()) files.append(mediaFromQuery(query));
    emit leastAccessedFilesLoaded(reqId, files);
}

void MediaCacheTable::getMediaCacheByType(int reqId, int fileType)
{
    QList<MediaCache> files;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit mediaCacheByTypeLoaded(reqId, files);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM media_cache WHERE file_type = ? ORDER BY last_access_time DESC");
    query.addBindValue(fileType);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit mediaCacheByTypeLoaded(reqId, files);
        return;
    }

    while (query.next()) files.append(mediaFromQuery(query));
    emit mediaCacheByTypeLoaded(reqId, files);
}

void MediaCacheTable::getTotalCacheSize(int reqId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database is not open");
        emit totalCacheSizeLoaded(reqId, -1);
        return;
    }

    QSqlQuery query("SELECT SUM(file_size) FROM media_cache", *m_database);
    if (!query.exec() || !query.next()) {
        emit dbError(reqId, query.lastError().text());
        emit totalCacheSizeLoaded(reqId, -1);
        return;
    }

    emit totalCacheSizeLoaded(reqId, query.value(0).toLongLong());
}

void MediaCacheTable::cleanupCache(int reqId, qint64 maxSize)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit cacheCleanupDone(reqId, false, "Database is not open");
        return;
    }

    qint64 currentSize = -1;
    {
        QSqlQuery q(*m_database);
        if (!q.exec("SELECT SUM(file_size) FROM media_cache") || !q.next()) {
            emit cacheCleanupDone(reqId, false, "Failed to get total cache size");
            return;
        }
        currentSize = q.value(0).toLongLong();
    }

    if (currentSize <= maxSize) {
        emit cacheCleanupDone(reqId, true, QString("No cleanup needed"));
        return;
    }

    if (!m_database->transaction()) {
        emit cacheCleanupDone(reqId, false, "Begin transaction failed");
        return;
    }

    bool ok = true;
    QString reason;
    qint64 sizeToDelete = 0;
    QStringList filesToDelete;

    QSqlQuery query(*m_database);
    query.prepare("SELECT file_path, file_size FROM media_cache "
                  "ORDER BY access_count ASC, last_access_time ASC");

    if (!query.exec()) {
        emit cacheCleanupDone(reqId, false, query.lastError().text());
        m_database->rollback();
        return;
    }

    while (query.next() && (currentSize - sizeToDelete) > maxSize) {
        QString filePath = query.value("file_path").toString();
        qint64 fileSize = query.value("file_size").toLongLong();
        filesToDelete.append(filePath);
        sizeToDelete += fileSize;
    }

    for (const QString &filePath : std::as_const(filesToDelete)) {
        QSqlQuery delQ(*m_database);
        delQ.prepare("DELETE FROM media_cache WHERE file_path = ?");
        delQ.addBindValue(filePath);
        if (!delQ.exec()) {
            ok = false;
            reason = delQ.lastError().text();
            break;
        }
    }

    if (ok) {
        if (!m_database->commit()) {
            ok = false;
            reason = m_database->lastError().text();
        }
    } else {
        m_database->rollback();
    }

    if (ok) {
        qDebug() << "Cache cleanup completed. Deleted" << filesToDelete.size()
        << "files, freed" << sizeToDelete << "bytes";
        emit cacheCleanupDone(reqId, true, QString("Deleted %1 files, freed %2 bytes").arg(filesToDelete.size()).arg(sizeToDelete));
    } else {
        qWarning() << "Cache cleanup failed:" << reason;
        emit cacheCleanupDone(reqId, false, reason);
    }
}

MediaCache MediaCacheTable::mediaFromQuery(const QSqlQuery &query) const
{
    return MediaCache::fromSqlQuery(query);
}
