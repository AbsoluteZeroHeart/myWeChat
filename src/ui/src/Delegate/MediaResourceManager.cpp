// MediaResourceManager.cpp
#include "MediaResourceManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

MediaResourceManager* MediaResourceManager::instance()
{
    static MediaResourceManager instance;
    return &instance;
}

MediaResourceManager::MediaResourceManager(QObject* parent)
    : QObject(parent)
    , m_cache(500) // 缓存500个资源
{
    m_threadPool.setMaxThreadCount(4);
    
    // 定时清理旧资源
    QTimer* cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() {
        cleanupOldResources(3600000);
    });
    cleanupTimer->start(300000);
}

MediaResourceManager::~MediaResourceManager()
{
    m_threadPool.waitForDone();
}

QPixmap MediaResourceManager::getMedia(const QString& resourcePath, const QSize& size, 
                                      MediaType type, int radius)
{
    if(resourcePath.isEmpty()) {
        return QPixmap();
    }
    
    QString cacheKey = generateCacheKey(resourcePath, size, type, radius);
    
    QMutexLocker locker(&m_mutex);
    
    // 查找缓存
    MediaCacheItem* cachedItem = m_cache.object(cacheKey);
    if(cachedItem) {
        // 更新访问时间
        cachedItem->lastAccessTime = QDateTime::currentMSecsSinceEpoch();
        cachedItem->accessCount++;
        return cachedItem->media;
    }
    
    // 检查是否正在加载
    if(!m_loadingMap.contains(cacheKey)) {
        m_loadingMap[cacheKey] = true;
        
        // 启动异步加载
        MediaLoadTask* task = new MediaLoadTask(resourcePath, size, type, radius, this);
        m_threadPool.start(task);
    }
    
    // 返回空图片，异步加载完成后会发送信号
    return QPixmap();
}

void MediaResourceManager::preloadMedia(const QString& resourcePath, const QSize& size,
                                       MediaType type, int radius)
{
    if(resourcePath.isEmpty()) return;
    
    QString cacheKey = generateCacheKey(resourcePath, size, type, radius);
    
    QMutexLocker locker(&m_mutex);
    
    // 如果已经在缓存或正在加载，则跳过
    if(m_cache.contains(cacheKey) || m_loadingMap.contains(cacheKey)) {
        return;
    }
    
    m_loadingMap[cacheKey] = true;
    MediaLoadTask* task = new MediaLoadTask(resourcePath, size, type, radius, this);
    m_threadPool.start(task);
}

void MediaResourceManager::onMediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type)
{
    QString cacheKey;
    
    {
        QMutexLocker locker(&m_mutex);
        
        // 使用 QMutableMapIterator 安全地删除元素
        QMutableMapIterator<QString, bool> it(m_loadingMap);
        while (it.hasNext()) {
            it.next();
            if (it.key().startsWith(resourcePath)) {
                cacheKey = it.key();
                it.remove();  // 安全删除当前元素
                break;
            }
        }
        
        if(!cacheKey.isEmpty() && !media.isNull()) {
            // 添加到缓存
            MediaCacheItem* item = new MediaCacheItem{
                media,
                QDateTime::currentMSecsSinceEpoch(),
                1,
                type
            };
            m_cache.insert(cacheKey, item);
        }
    }
    
    // 通知资源加载完成
    if(!media.isNull()) {
        emit mediaLoaded(resourcePath, media, type);
    }
}

QPixmap MediaResourceManager::processMedia(const QImage& image, const QSize& size, 
                                          MediaType type, int radius)
{
    switch(type) {
    case MediaType::Avatar:
        return processAvatar(image, size, radius);
    case MediaType::ImageThumb:
        return processImageThumb(image, size);
    case MediaType::VideoThumb:
        return processVideoThumb(image, size);
    case MediaType::FileIcon:
        return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    default:
        return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QPixmap MediaResourceManager::processAvatar(const QImage& image, const QSize& size, int radius)
{
    QImage scaledImage = image.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    QPixmap result(size);
    result.fill(Qt::transparent);
    
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 创建圆角路径
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
    painter.setClipPath(path);
    
    // 居中绘制图片
    int x = (scaledImage.width() - size.width()) / 2;
    int y = (scaledImage.height() - size.height()) / 2;
    painter.drawImage(0, 0, scaledImage, x, y, size.width(), size.height());
    
    return result;
}

QPixmap MediaResourceManager::processImageThumb(const QImage& image, const QSize& size)
{
    // 图片缩略图：保持比例缩放，无圆角
    return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap MediaResourceManager::processVideoThumb(const QImage& image, const QSize& size)
{
    // 视频缩略图：保持比例缩放，无圆角
    return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QString MediaResourceManager::generateCacheKey(const QString& path, const QSize& size, 
                                              MediaType type, int radius) const
{
    return QString("%1_%2x%3_%4_%5")
           .arg(path)
           .arg(size.width())
           .arg(size.height())
           .arg(static_cast<int>(type))
           .arg(radius);
}

void MediaResourceManager::clearCache()
{
    QMutexLocker locker(&m_mutex);
    m_cache.clear();
}

void MediaResourceManager::setCacheSize(int maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_cache.setMaxCost(maxSize);
}

void MediaResourceManager::cleanupOldResources(qint64 maxAgeMs)
{
    QMutexLocker locker(&m_mutex);
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QList<QString> keysToRemove;
    
    // 收集需要清理的key
    for(const QString& key : m_cache.keys()) {
        MediaCacheItem* item = m_cache.object(key);
        if(item && (now - item->lastAccessTime > maxAgeMs)) {
            keysToRemove.append(key);
        }
    }
    
    // 移除过期项
    for(const QString& key : keysToRemove) {
        m_cache.remove(key);
    }
}
