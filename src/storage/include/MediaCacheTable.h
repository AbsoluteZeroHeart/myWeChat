#ifndef MEDIACACHETABLE_H
#define MEDIACACHETABLE_H

#include <QtSql/QSqlDatabase>
#include <QObject>
#include <QList>
#include "MediaCache.h"

class MediaCacheTable : public QObject
{
    Q_OBJECT

public:
    explicit MediaCacheTable(QSqlDatabase database, QObject *parent = nullptr);
    
    bool saveMediaCache(const MediaCache& mediaCache);
    bool updateMediaCache(const MediaCache& mediaCache);
    bool deleteMediaCache(const QString& filePath);
    MediaCache getMediaCache(const QString& filePath);
    MediaCache getMediaCacheByUrl(const QString& originalUrl);
    bool clearMediaCache();
    bool clearExpiredMediaCache(qint64 expireTime);
    bool updateMediaAccess(const QString& filePath);
    QList<MediaCache> getLeastAccessedFiles(int limit);
    QList<MediaCache> getMediaCacheByType(int fileType);
    qint64 getTotalCacheSize();
    bool cleanupCache(qint64 maxSize);



private:
    QSqlDatabase m_database;
    
    MediaCache mediaFromQuery(const QSqlQuery& query);
};

#endif // MEDIACACHETABLE_H
