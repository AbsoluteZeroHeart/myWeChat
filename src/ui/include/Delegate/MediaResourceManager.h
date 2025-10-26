#ifndef MEDIARESOURCEMANAGER_H
#define MEDIARESOURCEMANAGER_H

#include <QObject>
#include <QCache>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QDateTime>
#include <QMutex>
#include <QThreadPool>
#include <QRunnable>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

enum class MediaType {
    Avatar,        // 头像
    ImageThumb,    // 图片缩略图
    VideoThumb,    // 视频缩略图
    FileIcon       // 文件图标
};

class MediaResourceManager : public QObject
{
    Q_OBJECT

public:
    static MediaResourceManager* instance();
    
    // 获取资源，如果缓存中有则直接返回，否则异步加载
    QPixmap getMedia(const QString& resourcePath, const QSize& size, 
                     MediaType type = MediaType::Avatar, int radius = 5);
    
    // 预加载资源到缓存
    void preloadMedia(const QString& resourcePath, const QSize& size,
                      MediaType type = MediaType::Avatar, int radius = 5);
    
    // 清理缓存
    void clearCache();
    void setCacheSize(int maxSize);
    
    // 清理长时间未使用的资源
    void cleanupOldResources(qint64 maxAgeMs = 3600000);

    static QPixmap processMedia(const QImage& image, const QSize& size,
                                MediaType type, int radius);

signals:
    void mediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type);

private slots:
    void onMediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type);

private:
    MediaResourceManager(QObject* parent = nullptr);
    ~MediaResourceManager();
    
    struct MediaCacheItem {
        QPixmap media;
        qint64 lastAccessTime;
        int accessCount;
        MediaType type;
    };
    
    static QPixmap processAvatar(const QImage& image, const QSize& size, int radius);
    static QPixmap processImageThumb(const QImage& image, const QSize& size);
    static QPixmap processVideoThumb(const QImage& image, const QSize& size);
    
    QString generateCacheKey(const QString& path, const QSize& size, MediaType type, int radius) const;
    
    QCache<QString, MediaCacheItem> m_cache;
    QMap<QString, bool> m_loadingMap;
    QMutex m_mutex;
    QThreadPool m_threadPool;
};

// 异步加载任务
class MediaLoadTask : public QRunnable
{
public:
    MediaLoadTask(const QString& path, const QSize& size, MediaType type, 
                  int radius, MediaResourceManager* manager)
        : m_path(path), m_size(size), m_type(type), m_radius(radius), m_manager(manager) {}
        
    void run() override {
        QImage image(m_path);
        QPixmap result;
        
        if(!image.isNull()) {
            result = MediaResourceManager::processMedia(image, m_size, m_type, m_radius);
        }
        
        QMetaObject::invokeMethod(m_manager, "onMediaLoaded", 
                                Qt::QueuedConnection,
                                Q_ARG(QString, m_path),
                                Q_ARG(QPixmap, result),
                                Q_ARG(MediaType, m_type));
    }
    
private:
    QString m_path;
    QSize m_size;
    MediaType m_type;
    int m_radius;
    MediaResourceManager* m_manager;
};

#endif // MEDIARESOURCEMANAGER_H
