#include "ThumbnailManager.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>

ThumbnailManager::ThumbnailManager(QObject *parent)
    : QObject(parent)
    , m_defaultThumbnailSize(200, 150)
    , m_imageWatcher(new QFutureWatcher<QPair<QString, bool>>(this))
    , m_videoWatcher(new QFutureWatcher<QPair<QString, bool>>(this))
    , m_expiredWatcher(new QFutureWatcher<QPair<QPixmap, QString>>(this))
{
    // 设置默认缩略图存储路径
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cachePath.isEmpty()) {
        cachePath = QDir::currentPath() + "/thumbnails";
    } else {
        cachePath += "/thumbnails";
    }
    setThumbnailStoragePath(cachePath);

    // 连接信号槽
    connect(m_imageWatcher, &QFutureWatcher<QPair<QString, bool>>::finished,
            this, &ThumbnailManager::onImageThumbnailFinished);

    connect(m_videoWatcher, &QFutureWatcher<QPair<QString, bool>>::finished,
            this, &ThumbnailManager::onVideoThumbnailFinished);

    connect(m_expiredWatcher, &QFutureWatcher<QPair<QPixmap, QString>>::finished,
            this, &ThumbnailManager::onExpiredThumbnailFinished);
}

ThumbnailManager::~ThumbnailManager()
{
    cancelAllTasks();
}

void ThumbnailManager::setThumbnailStoragePath(const QString& path)
{
    m_thumbnailStoragePath = path;

    // 确保目录存在
    QDir dir;
    if (!dir.exists(m_thumbnailStoragePath)) {
        dir.mkpath(m_thumbnailStoragePath);
    }
}

void ThumbnailManager::generateImageThumbnailAsync(const QString& imagePath,
                                                        const QString& thumbnailName,
                                                        const QSize& size)
{
    ThumbnailTask task;
    task.sourcePath = imagePath;
    task.thumbnailName = thumbnailName.isEmpty() ? generateThumbnailName(imagePath, "") : thumbnailName;
    task.fileType = "图片";
    task.size = size.isValid() ? size : m_defaultThumbnailSize;
    task.isVideo = false;

    // 如果已经有任务在运行，添加到待处理队列
    if (m_imageWatcher->isRunning()) {
        m_pendingTasks.append(task);
        return;
    }

    // 启动异步任务 - 使用 lambda 表达式
    QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
        return generateImageThumbnailImpl(task);
    });
    m_imageWatcher->setFuture(future);
}

void ThumbnailManager::generateVideoThumbnailAsync(const QString& videoPath,
                                                        const QString& thumbnailName,
                                                        const QSize& size)
{
    ThumbnailTask task;
    task.sourcePath = videoPath;
    task.thumbnailName = thumbnailName.isEmpty() ? generateThumbnailName(videoPath, "") : thumbnailName;
    task.fileType = "视频";
    task.size = size.isValid() ? size : m_defaultThumbnailSize;
    task.isVideo = true;

    // 如果已经有任务在运行，添加到待处理队列
    if (m_videoWatcher->isRunning()) {
        m_pendingTasks.append(task);
        return;
    }

    // 启动异步任务 - 使用 lambda 表达式
    QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
        return generateVideoThumbnailImpl(task);
    });
    m_videoWatcher->setFuture(future);
}

void ThumbnailManager::getExpiredThumbnailAsync(const QString& thumbnailName,
                                                     const QString& mediaType,
                                                     bool checkSourceExists,
                                                     const QString& sourcePath)
{
    // 启动异步任务 - 使用 lambda 表达式
    QFuture<QPair<QPixmap, QString>> future = QtConcurrent::run([this, thumbnailName, mediaType, checkSourceExists, sourcePath]() {
        return getExpiredThumbnailImpl(thumbnailName, mediaType, checkSourceExists, sourcePath);
    });
    m_expiredWatcher->setFuture(future);
}

QPixmap ThumbnailManager::getWarningThumbnail(const QString& thumbnailPath,const QString &mediaType)
{
    if(!QFileInfo::exists(thumbnailPath)){
        return createDefaultExpiredThumbnail({100,100}, mediaType);
    }
    return  createExpiredThumbnail(QPixmap(thumbnailPath), mediaType);
}

bool ThumbnailManager::thumbnailExists(const QString& thumbnailName)
{
    QString thumbnailPath = getThumbnailPath(thumbnailName);
    return QFileInfo::exists(thumbnailPath);
}

QString ThumbnailManager::getThumbnailPath(const QString& thumbnailName)
{
    return m_thumbnailStoragePath + "/" + thumbnailName;
}

void ThumbnailManager::setDefaultThumbnailSize(const QSize& size)
{
    if (size.isValid()) {
        m_defaultThumbnailSize = size;
    }
}

void ThumbnailManager::cancelAllTasks()
{
    if (m_imageWatcher->isRunning()) {
        m_imageWatcher->cancel();
        m_imageWatcher->waitForFinished();
    }

    if (m_videoWatcher->isRunning()) {
        m_videoWatcher->cancel();
        m_videoWatcher->waitForFinished();
    }

    if (m_expiredWatcher->isRunning()) {
        m_expiredWatcher->cancel();
        m_expiredWatcher->waitForFinished();
    }

    m_pendingTasks.clear();
}

int ThumbnailManager::activeTaskCount() const
{
    int count = 0;
    if (m_imageWatcher->isRunning()) count++;
    if (m_videoWatcher->isRunning()) count++;
    if (m_expiredWatcher->isRunning()) count++;
    return count + m_pendingTasks.count();
}

QString ThumbnailManager::generateThumbnailName(const QString& originalPath, const QString& customName)
{
    if (!customName.isEmpty()) {
        return customName + ".png";
    }

    QFileInfo fileInfo(originalPath);
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();

    qint64 timestamp = fileInfo.lastModified().toMSecsSinceEpoch();
    QString uniqueName = QString("%1_%2_%3").arg(baseName).arg(extension).arg(timestamp);

    uint hash = qHash(uniqueName);
    return QString("thumb_%1.png").arg(hash);
}

QPair<QString, bool> ThumbnailManager::generateImageThumbnailImpl(const ThumbnailTask& task)
{
    QFileInfo fileInfo(task.sourcePath);
    if (!fileInfo.exists()) {
        return qMakePair(task.thumbnailName, false);
    }

    // 加载原始图片
    QImage originalImage(task.sourcePath);
    if (originalImage.isNull()) {
        return qMakePair(task.thumbnailName, false);
    }

    // 缩放图片
    QImage thumbnail = originalImage.scaled(task.size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 保存缩略图
    QString thumbnailPath = getThumbnailPath(task.thumbnailName);
    bool success = saveThumbnail(thumbnail, thumbnailPath);

    return qMakePair(task.thumbnailName, success);
}

QPair<QString, bool> ThumbnailManager::generateVideoThumbnailImpl(const ThumbnailTask& task)
{
#ifdef QT_MULTIMEDIA_LIB
    QFileInfo fileInfo(task.sourcePath);
    if (!fileInfo.exists()) {
        return qMakePair(task.thumbnailName, false);
    }

    // 使用Qt Multimedia提取视频缩略图
    // 注意：QMediaPlayer需要在主线程创建，所以这里使用同步方式
    // 在实际应用中，可能需要更复杂的异步处理

    QImage thumbnail;
    bool success = false;

    // 这里简化处理，实际应用中需要更完善的视频帧提取
    QImage tempImage(task.size, QImage::Format_RGB32);
    tempImage.fill(Qt::darkGray);

    // 在实际应用中，这里应该使用QMediaPlayer提取视频帧
    // 由于QMediaPlayer的线程限制，这里只是示例

    // 保存缩略图
    QString thumbnailPath = getThumbnailPath(task.thumbnailName);
    success = saveThumbnail(tempImage, thumbnailPath);

    return qMakePair(task.thumbnailName, success);
#else
    qWarning() << "Qt Multimedia is not available. Cannot generate video thumbnail.";
    return qMakePair(task.thumbnailName, false);
#endif
}

QPair<QPixmap, QString> ThumbnailManager::getExpiredThumbnailImpl(const QString& thumbnailName,
                                                                       const QString& mediaType,
                                                                       bool checkSourceExists,
                                                                       const QString& sourcePath)
{
    QString thumbnailPath = getThumbnailPath(thumbnailName);
    QPixmap baseThumbnail;

    // 检查缩略图是否存在
    bool thumbnailLoaded = false;
    if (QFileInfo::exists(thumbnailPath)) {
        baseThumbnail = QPixmap(thumbnailPath);
        thumbnailLoaded = !baseThumbnail.isNull();
    }

    // 检查源文件是否存在（如果需要）
    bool sourceExists = true;
    if (checkSourceExists && !sourcePath.isEmpty()) {
        sourceExists = QFileInfo::exists(sourcePath);
    }

    // 如果缩略图加载成功且源文件存在，返回原始缩略图
    if (thumbnailLoaded && sourceExists) {
        return qMakePair(baseThumbnail, thumbnailName);
    }

    // 如果缩略图加载成功但源文件不存在，添加过期提示
    if (thumbnailLoaded && !sourceExists) {
        QPixmap expiredThumbnail = createExpiredThumbnail(baseThumbnail, mediaType);
        return qMakePair(expiredThumbnail, thumbnailName);
    }

    // 如果缩略图加载失败，创建默认的过期缩略图
    QPixmap expiredThumbnail = createDefaultExpiredThumbnail(m_defaultThumbnailSize, mediaType);
    return qMakePair(expiredThumbnail, thumbnailName);
}

bool ThumbnailManager::saveThumbnail(const QImage& image, const QString& thumbnailPath)
{
    if (image.isNull()) {
        return false;
    }

    return image.save(thumbnailPath, "PNG");
}

QPixmap ThumbnailManager::createExpiredThumbnail(const QPixmap& baseThumbnail, const QString& mediaType)
{
    if(baseThumbnail.isNull())return QPixmap();
    QPixmap resultPixmap = baseThumbnail;
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

QPixmap ThumbnailManager::createDefaultExpiredThumbnail(const QSize& size, const QString& mediaType)
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

void ThumbnailManager::onImageThumbnailFinished()
{
    QPair<QString, bool> result = m_imageWatcher->result();
    emit imageThumbnailGenerated(result.first, result.second);

    // 处理下一个待处理任务
    if (!m_pendingTasks.isEmpty()) {
        ThumbnailTask task = m_pendingTasks.takeFirst();
        if (task.isVideo) {
            // 使用 lambda 表达式
            QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
                return generateVideoThumbnailImpl(task);
            });
            m_videoWatcher->setFuture(future);
        } else {
            // 使用 lambda 表达式
            QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
                return generateImageThumbnailImpl(task);
            });
            m_imageWatcher->setFuture(future);
        }
    }
}

void ThumbnailManager::onVideoThumbnailFinished()
{
    QPair<QString, bool> result = m_videoWatcher->result();
    emit videoThumbnailGenerated(result.first, result.second);

    // 处理下一个待处理任务
    if (!m_pendingTasks.isEmpty()) {
        ThumbnailTask task = m_pendingTasks.takeFirst();
        if (task.isVideo) {
            // 使用 lambda 表达式
            QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
                return generateVideoThumbnailImpl(task);
            });
            m_videoWatcher->setFuture(future);
        } else {
            // 使用 lambda 表达式
            QFuture<QPair<QString, bool>> future = QtConcurrent::run([this, task]() {
                return generateImageThumbnailImpl(task);
            });
            m_imageWatcher->setFuture(future);
        }
    }
}

void ThumbnailManager::onExpiredThumbnailFinished()
{
    QPair<QPixmap, QString> result = m_expiredWatcher->result();
    emit expiredThumbnailReady(result.first, result.second);
}
