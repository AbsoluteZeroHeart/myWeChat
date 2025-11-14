#include "MediaResourceManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QFontMetrics>
#include <QIcon>
#include <QStyle>

MediaResourceManager* MediaResourceManager::instance()
{
    static MediaResourceManager instance;
    return &instance;
}

MediaResourceManager::MediaResourceManager(QObject* parent)
    : QObject(parent)
    , m_cache(200*1024)
{
    int idealThreadCount = QThread::idealThreadCount();
    m_threadPool.setMaxThreadCount(qMax(2, idealThreadCount));


    QTimer* cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() {
        cleanupOldResources(600000);
    });
    cleanupTimer->start(600000);
}

MediaResourceManager::~MediaResourceManager()
{
    m_threadPool.waitForDone();
}

QPixmap MediaResourceManager::getMedia(const QString& resourcePath, const QSize& size,
                                       MediaType type, int radius, const QString& iconPath)
{
    if(resourcePath.isEmpty()) {
        return QPixmap();
    }

    QString cacheKey = generateCacheKey(resourcePath, size, type, radius, iconPath);

    QMutexLocker locker(&m_mutex);

    // 查找缓存
    MediaCacheItem* cachedItem = m_cache.object(cacheKey);
    if(cachedItem) {
        cachedItem->lastAccessTime = QDateTime::currentMSecsSinceEpoch();
        cachedItem->accessCount++;
        return cachedItem->media;
    }

    // 不在缓存中，且没有正在加载，启动异步加载
    if(!m_loadingMap.contains(cacheKey)) {
        m_loadingMap[cacheKey] = true;

        // 启动异步加载
        MediaLoadTask* task = new MediaLoadTask(resourcePath, size, type, radius, cacheKey, this, iconPath);
        m_threadPool.start(task);
    }

    if(type!=MediaType::Avatar) return getPixmap(size);

    return QPixmap();
}

QPixmap MediaResourceManager::getPixmap(const QSize &size)
{
    QPixmap pixmap(100,100);
    pixmap.fill(QColor(255, 255, 255)); // 浅灰色背景

    QPainter painter(&pixmap);
    painter.setPen(Qt::darkGray);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "加载中...");

    return pixmap;
}

void MediaResourceManager::preloadMedia(const QString& resourcePath, const QSize& size,
                                        MediaType type, int radius, const QString& iconPath)
{
    if(resourcePath.isEmpty()) return;

    QString cacheKey = generateCacheKey(resourcePath, size, type, radius, iconPath);

    QMutexLocker locker(&m_mutex);

    if(m_cache.contains(cacheKey) || m_loadingMap.contains(cacheKey)) {
        return;
    }

    // 检查文件是否存在
    QFileInfo fileInfo(type == MediaType::ImageThumb || type == MediaType::VideoThumb ? iconPath : resourcePath);
    if(!fileInfo.exists() || !fileInfo.isReadable()) {
        return;
    }

    m_loadingMap[cacheKey] = true;
    MediaLoadTask* task = new MediaLoadTask(resourcePath, size, type, radius, cacheKey, this, iconPath);
    m_threadPool.start(task);
}

QPixmap MediaResourceManager::processThumbnail(const QString& resourcePath, const QString& iconPath,
                                               const QSize& size, MediaType type)
{
    QFileInfo originalFile(resourcePath);
    bool originalExists = originalFile.exists()&&originalFile.isReadable();

    if (originalExists) {
        QImage iconImage(iconPath);
        if (iconImage.isNull()) {
            return createDefaultThumbnail(size, type);
        } else {
            return processMedia(iconImage, size, type, 0, originalExists);
        }
    } else {
        return getWarningThumbnail(iconPath,
                                                     type == MediaType::ImageThumb ? "image" : "video");
    }
}

// 处理原图片
QPixmap MediaResourceManager::processOriginalImage(const QImage& image, QSize size)
{
    return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap MediaResourceManager::createDefaultThumbnail(const QSize& size, MediaType type, const QString& text)
{
    QPixmap pixmap(size);
    pixmap.fill(QColor(50, 50, 50)); // 灰色背景

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (type == MediaType::VideoThumb) {
        addPlayButton(pixmap);
    } else if (type == MediaType::ImageThumb) {
        QString displayText = text.isEmpty() ? "图片" : text;
        addTextIndicator(pixmap, displayText);
    }

    return pixmap;
}

void MediaResourceManager::addPlayButton(QPixmap& pixmap)
{
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制播放三角形
    int triangleSize = qMin(pixmap.width(), pixmap.height()) / 4;
    QPointF center(pixmap.width() / 2.0, pixmap.height() / 2.0);

    QPolygonF triangle;
    triangle << QPointF(center.x() - triangleSize/2, center.y() - triangleSize/2)
             << QPointF(center.x() - triangleSize/2, center.y() + triangleSize/2)
             << QPointF(center.x() + triangleSize/2, center.y());

    painter.setBrush(QColor(255, 255, 255, 255));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(triangle);
}

void MediaResourceManager::addTextIndicator(QPixmap& pixmap, const QString& text)
{
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
}

QPixmap MediaResourceManager::processMedia(const QImage& image, const QSize& size,
                                           MediaType type, int radius, bool isOriginalExists)
{
    switch(type) {
    case MediaType::Avatar:
        return processAvatar(image, size, radius);
    case MediaType::ImageThumb:
        return processImageThumb(image, size, isOriginalExists);
    case MediaType::VideoThumb:
        return processVideoThumb(image, size, isOriginalExists);
    case MediaType::FileIcon:
        return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    case MediaType::OriginalImage:  // 原图片处理
        return processOriginalImage(image, size);
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

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
    painter.setClipPath(path);

    int x = (scaledImage.width() - size.width()) / 2;
    int y = (scaledImage.height() - size.height()) / 2;
    painter.drawImage(0, 0, scaledImage, x, y, size.width(), size.height());

    return result;
}

QPixmap MediaResourceManager::processImageThumb(const QImage& image, const QSize& size, bool isOriginalExists)
{
    // 如果size为空，返回原图
    if (size.isEmpty()) {
        return QPixmap::fromImage(image);
    }

    QPixmap result = QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    if (isOriginalExists && image.isNull()) {
        addTextIndicator(result, "图片");
    }

    return result;
}

QPixmap MediaResourceManager::processVideoThumb(const QImage& image, const QSize& size, bool isOriginalExists)
{
    // 如果size为空，返回原图
    if (size.isEmpty()) {
        return QPixmap::fromImage(image);
    }

    QPixmap result = QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 添加播放按钮
    addPlayButton(result);

    return result;
}

QString MediaResourceManager::generateCacheKey(const QString& path, const QSize& size,
                                               MediaType type, int radius, const QString& iconPath) const
{
    QString baseKey = QString("%1_%2_%3_%4")
    .arg(path)
        .arg(static_cast<int>(type))
        .arg(radius);

    // 如果size为空，表示原图，在缓存键中特殊标记
    if (size.isEmpty()) {
        baseKey += "_original";
    } else {
        baseKey += QString("_%1x%2").arg(size.width()).arg(size.height());
    }

    if (type == MediaType::ImageThumb || type == MediaType::VideoThumb) {
        baseKey += "_" + iconPath;
    }

    return baseKey;
}

// 估算缓存成本
int MediaResourceManager::estimateCacheCost(const QPixmap& pixmap) const
{
    if (pixmap.isNull()) {
        return 1;
    }

    // 估算图片占用的内存大小（字节）
    int cost = pixmap.width() * pixmap.height() * pixmap.depth() / 8;

    // 转换为KB，并确保最小成本为1
    cost = qMax(1, cost / 1024);

    return cost;
}

void MediaResourceManager::onMediaLoaded(const QString& cacheKey, const QString& resourcePath,
                                         const QPixmap& media, MediaType type, bool success)
{
    {
        QMutexLocker locker(&m_mutex);
        m_loadingMap.remove(cacheKey);
        if(success && !media.isNull()) {
            MediaCacheItem* item = new MediaCacheItem{
                media,
                QDateTime::currentMSecsSinceEpoch(),
                1,
                type,
                estimateCacheCost(media)  // 使用估算的成本
            };
            m_cache.insert(cacheKey, item, item->fileSize);
        }
    }

    if(success && !media.isNull()) {
        emit mediaLoaded(resourcePath, media, type);
    } else {
        emit mediaLoadFailed(resourcePath, type);
    }
}

void MediaResourceManager::cancelLoading(const QString& cacheKey)
{
    QMutexLocker locker(&m_mutex);
    m_loadingMap.remove(cacheKey);
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

    const QList<QString>& keys = m_cache.keys();
    for (const QString& key : keys) {
        MediaCacheItem* item = m_cache.object(key);
        if (item && (now - item->lastAccessTime > maxAgeMs)) {
            keysToRemove.append(key);
        }
    }

    for(const QString& key : keysToRemove) {
        m_cache.remove(key);
    }
}

QPixmap MediaResourceManager::getWarningThumbnail(const QString& thumbnailPath,const QString &mediaType, const QSize &size)
{
    if(!QFileInfo::exists(thumbnailPath)){
        return createDefaultExpiredThumbnail({100,100}, mediaType);
    }
    return  createExpiredThumbnail(QPixmap(thumbnailPath), mediaType, size);
}

QPixmap MediaResourceManager::createDefaultExpiredThumbnail(const QSize& size, const QString& mediaType)
{
    QPixmap resultPixmap(size);
    resultPixmap.fill(QColor(80, 80, 80));

    QPainter painter(&resultPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制半透明遮罩
    painter.fillRect(resultPixmap.rect(), QColor(0, 0, 0, 160));

    // 计算图标大小和位置
    int iconSize = qMin(size.width(), size.height()) / 3;
    QRect iconRect((size.width() - iconSize) / 2,
                   (size.height() - iconSize) / 3,
                   iconSize, iconSize);

    // 绘制警告图标
    QStyle* style = QApplication::style();
    QIcon warningIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);
    warningIcon.paint(&painter, iconRect);

    // 绘制文字
    QFont font = painter.font();
    font.setPointSize(qMax(10, iconSize / 5));
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::red);

    QString text = QString("%1已过期或被清除").arg(mediaType=="video"? "视频":"图片");
    QRect textRect(0, iconRect.bottom() + 10,
                   size.width(), iconSize / 2);

    painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
    painter.end();

    return resultPixmap;
}

QPixmap MediaResourceManager::createExpiredThumbnail(const QPixmap& baseThumbnail, const QString& mediaType, const QSize size)
{
    if(baseThumbnail.isNull())return QPixmap();
    QPixmap resultPixmap = baseThumbnail.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&resultPixmap);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // 绘制半透明遮罩
    painter.fillRect(resultPixmap.rect(), QColor(0, 0, 0, 160));

    // 计算图标大小和位置
    int iconSize = qMin(resultPixmap.width(), resultPixmap.height()) / 3;
    QRect iconRect((resultPixmap.width() - iconSize) / 2,
                   (resultPixmap.height() - iconSize) / 3,
                   iconSize, iconSize);

    // 绘制警告图标
    QStyle* style = QApplication::style();
    QIcon warningIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);
    warningIcon.paint(&painter, iconRect);

    // 绘制文字
    QFont font = painter.font();
    font.setPointSize(qMax(10, iconSize / 5));
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QString text = QString("%1已过期或被清除").arg(mediaType=="video"? "视频":"图片");
    QRect textRect(0, iconRect.bottom() + 10,
                   resultPixmap.width(), iconSize / 2);

    painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
    painter.end();

    return resultPixmap;
}
