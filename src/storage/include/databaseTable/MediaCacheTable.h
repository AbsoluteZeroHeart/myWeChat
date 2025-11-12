#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include "models/MediaCache.h"

class MediaCacheTable : public QObject {
    Q_OBJECT
public:
    explicit MediaCacheTable(QObject *parent = nullptr);
    ~MediaCacheTable() override;


public slots:
    void init();

    void saveMediaCache(int reqId, MediaCache mediaCache);
    void updateMediaCache(int reqId, MediaCache mediaCache);
    void deleteMediaCache(int reqId, const QString &filePath);

    void getMediaCache(int reqId, const QString &filePath);
    void getMediaCacheByUrl(int reqId, const QString &originalUrl);

    void clearMediaCache(int reqId);
    void clearExpiredMediaCache(int reqId, qint64 expireTime);

    void updateMediaAccess(int reqId, const QString &filePath);

    void getLeastAccessedFiles(int reqId, int limit);
    void getMediaCacheByType(int reqId, int fileType);
    void getTotalCacheSize(int reqId);

    void cleanupCache(int reqId, qint64 maxSize);

signals:
    // 存储/修改/删除结果
    void mediaCacheSaved(int reqId, bool ok, QString reason);
    void mediaCacheUpdated(int reqId, bool ok, QString reason);
    void mediaCacheDeleted(int reqId, bool ok, QString reason);

    // 查询结果
    void mediaCacheLoaded(int reqId, MediaCache media);
    void mediaCacheByUrlLoaded(int reqId, MediaCache media);

    void mediaCacheCleared(int reqId, bool ok, QString reason);
    void mediaCacheExpiredCleared(int reqId, bool ok, QString reason);

    void mediaAccessUpdated(int reqId, bool ok);

    void leastAccessedFilesLoaded(int reqId, QList<MediaCache> files);
    void mediaCacheByTypeLoaded(int reqId, QList<MediaCache> files);
    void totalCacheSizeLoaded(int reqId, qint64 size);

    void cacheCleanupDone(int reqId, bool ok, QString reason);

    // 通用错误（reqId 可为 -1 表示 init/全局错误）
    void dbError(int reqId, QString error);

private:
    QSharedPointer<QSqlDatabase> m_database;

    // 私有辅助
    MediaCache mediaFromQuery(const QSqlQuery &query) const;
};
