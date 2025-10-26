#ifndef MEDIACACHETABLE_H
#define MEDIACACHETABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

class MediaCacheTable : public QObject
{
    Q_OBJECT

public:
    explicit MediaCacheTable(QSqlDatabase database, QObject *parent = nullptr);
    
    // 媒体缓存管理
    bool saveMediaCache(const QJsonObject& mediaInfo);
    bool updateMediaCache(const QJsonObject& mediaInfo);
    bool deleteMediaCache(const QString& filePath);
    QJsonObject getMediaCache(const QString& filePath);
    QJsonObject getMediaCacheByUrl(const QString& originalUrl);
    bool clearMediaCache();
    bool clearExpiredMediaCache(qint64 expireTime);
    
    // 缓存访问管理
    bool updateMediaAccess(const QString& filePath);
    QJsonArray getLeastAccessedFiles(int limit = 100);
    QJsonArray getMediaCacheByType(int fileType);
    qint64 getTotalCacheSize();
    bool cleanupCache(qint64 maxSize);

private:
    QSqlDatabase m_database;
    QJsonObject mediaFromQuery(const QSqlQuery& query);

};

#endif // MEDIACACHETABLE_H
