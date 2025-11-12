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
#include <QFileInfo>
#include <QPointer>

enum class MediaType {
    Avatar,        // 头像
    ImageThumb,    // 图片缩略图
    VideoThumb,    // 视频缩略图
    FileIcon,      // 文件图标
    OriginalImage  // 新增：原图片
};

class MediaResourceManager : public QObject
{
    Q_OBJECT

public:
    static MediaResourceManager* instance();

    static void cleanup() {

        if (MediaResourceManager* instance = MediaResourceManager::instance()) {
            // 等待所有任务完成
            instance->m_threadPool.waitForDone();
        }
    }

    // 获取资源 - 对于ImageThumb和VideoThumb，resourcePath是原路径，iconPath是图标路径
    QPixmap getMedia(const QString& resourcePath, const QSize& size = QSize(),
                     MediaType type = MediaType::Avatar, int radius = 5,
                     const QString& iconPath = QString());


    // 预加载资源到缓存
    void preloadMedia(const QString& resourcePath, const QSize& size = QSize(),
                      MediaType type = MediaType::Avatar, int radius = 5,
                      const QString& iconPath = QString());

    // 清理缓存
    void clearCache();
    void setCacheSize(int maxSize);

    // 清理长时间未使用的资源
    void cleanupOldResources(qint64 maxAgeMs = 3600000);

    static QPixmap processMedia(const QImage& image, const QSize& size,
                                MediaType type, int radius, bool isOriginalExists = true);

    // 处理缩略图的特殊逻辑
    static QPixmap processThumbnail(const QString& resourcePath, const QString& iconPath,
                                    const QSize& size, MediaType type);

    // 处理原图片加载
    static QPixmap processOriginalImage(const QImage& image);

    // 取消正在进行的加载任务
    void cancelLoading(const QString& cacheKey);

    static QPixmap getWarningThumbnail(const QString& thumbnailPath,const QString &mediaType, const QSize &size = QSize(200,300));


signals:
    void mediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type);
    void mediaLoadFailed(const QString& resourcePath, MediaType type);

private slots:
    void onMediaLoaded(const QString& cacheKey, const QString& resourcePath,
                       const QPixmap& media, MediaType type, bool success);

private:
    MediaResourceManager(QObject* parent = nullptr);
    ~MediaResourceManager();

    struct MediaCacheItem {
        QPixmap media;
        qint64 lastAccessTime;
        int accessCount;
        MediaType type;
        qint64 fileSize;
    };

    QPixmap getPixmap(const QSize &size);
    static QPixmap processAvatar(const QImage& image, const QSize& size, int radius);
    static QPixmap processImageThumb(const QImage& image, const QSize& size, bool isOriginalExists);
    static QPixmap processVideoThumb(const QImage& image, const QSize& size, bool isOriginalExists);

    // 创建默认缩略图
    static QPixmap createDefaultThumbnail(const QSize& size, MediaType type, const QString& text = QString());

    static QPixmap createDefaultExpiredThumbnail(const QSize& size, const QString& mediaType);
    static QPixmap createExpiredThumbnail(const QPixmap& baseThumbnail, const QString& mediaType, const QSize size = QSize(200,300));

    // 添加播放按钮到视频缩略图
    static void addPlayButton(QPixmap& pixmap);
    // 添加文字标识
    static void addTextIndicator(QPixmap& pixmap, const QString& text);

    QString generateCacheKey(const QString& path, const QSize& size, MediaType type, int radius, const QString& iconPath = QString()) const;

    // 估算图片的缓存成本
    int estimateCacheCost(const QPixmap& pixmap) const;

    QCache<QString, MediaCacheItem> m_cache;
    QMap<QString, bool> m_loadingMap;  // key: cacheKey, value: 是否正在加载
    QMutex m_mutex;
    QThreadPool m_threadPool;
};

class MediaLoadTask : public QRunnable
{
public:
    MediaLoadTask(const QString& path, const QSize& size, MediaType type,
                  int radius, const QString& cacheKey, MediaResourceManager* manager,
                  const QString& iconPath = QString())
        : m_path(path), m_size(size), m_type(type), m_radius(radius),
        m_cacheKey(cacheKey), m_manager(manager), m_iconPath(iconPath) {}

    void run() override {

        bool success = false;
        QPixmap result;

        if (m_type == MediaType::ImageThumb || m_type == MediaType::VideoThumb) {
            result = MediaResourceManager::processThumbnail(m_path, m_iconPath, m_size, m_type);
            success = !result.isNull();
        } else {
            QImage image(m_path);
            QFileInfo fileInfo(m_path);
            if(fileInfo.exists() && fileInfo.isReadable() && !image.isNull()) {
                result = MediaResourceManager::processMedia(image, m_size, m_type, m_radius, true);
                success = !result.isNull();
            }
        }

        QMetaObject::invokeMethod(m_manager, "onMediaLoaded",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, m_cacheKey),
                                  Q_ARG(QString, m_path),
                                  Q_ARG(QPixmap, result),
                                  Q_ARG(MediaType, m_type),
                                  Q_ARG(bool, success));
    }

private:
    QString m_path;
    QSize m_size;
    MediaType m_type;
    int m_radius;
    QString m_cacheKey;
    MediaResourceManager* m_manager;
    QString m_iconPath;
};

#endif // MEDIARESOURCEMANAGER_H
