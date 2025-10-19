#ifndef ASYNCTHUMBNAILMANAGER_H
#define ASYNCTHUMBNAILMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QStyle>

// 使用正确的头文件包含方式
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

#ifdef QT_MULTIMEDIA_LIB
#include <QMediaPlayer>
#include <QVideoFrame>
#include <QVideoSink>
#endif

struct ThumbnailTask {
    QString sourcePath;
    QString thumbnailName;
    QString fileType;
    QSize size;
    bool isVideo;
};

class AsyncThumbnailManager : public QObject
{
    Q_OBJECT

public:
    explicit AsyncThumbnailManager(QObject *parent = nullptr);
    ~AsyncThumbnailManager();

    // 设置缩略图存储路径
    void setThumbnailStoragePath(const QString& path);

    // 异步生成图片缩略图
    void generateImageThumbnailAsync(const QString& imagePath,
                                     const QString& thumbnailName = "",
                                     const QSize& size = QSize(200, 150));

    // 异步生成视频缩略图
    void generateVideoThumbnailAsync(const QString& videoPath,
                                     const QString& thumbnailName = "",
                                     const QSize& size = QSize(200, 150));

    // 异步获取带有过期提示的缩略图
    void getExpiredThumbnailAsync(const QString& thumbnailName,
                                  const QString& fileType = "图片",
                                  bool checkSourceExists = false,
                                  const QString& sourcePath = "");

    // 获取警告缩略图
    static QPixmap getWarningThumbnail(const QString& thumbnailName, const QString &fileType);

    // 检查缩略图是否存在
    bool thumbnailExists(const QString& thumbnailName);

    // 获取缩略图完整路径
    QString getThumbnailPath(const QString& thumbnailName);

    // 设置默认缩略图尺寸
    void setDefaultThumbnailSize(const QSize& size);

    // 取消所有任务
    void cancelAllTasks();

    // 获取正在进行的任务数量
    int activeTaskCount() const;

signals:
    // 缩略图生成完成信号
    void imageThumbnailGenerated(const QString& thumbnailPath, bool success);
    void videoThumbnailGenerated(const QString& thumbnailPath, bool success);

    // 过期缩略图获取完成信号
    void expiredThumbnailReady(const QPixmap& thumbnail, const QString& thumbnailName);

    // 错误信号
    void errorOccurred(const QString& errorMessage);

private slots:
    void onImageThumbnailFinished();
    void onVideoThumbnailFinished();
    void onExpiredThumbnailFinished();

private:
    QString m_thumbnailStoragePath;
    QSize m_defaultThumbnailSize;

    // 异步任务管理
    QFutureWatcher<QPair<QString, bool>>* m_imageWatcher;
    QFutureWatcher<QPair<QString, bool>>* m_videoWatcher;
    QFutureWatcher<QPair<QPixmap, QString>>* m_expiredWatcher;

    // 任务队列
    QList<ThumbnailTask> m_pendingTasks;

    // 生成缩略图文件名
    QString generateThumbnailName(const QString& originalPath, const QString& customName);

    // 实际的生成函数（在后台线程中运行）
    QPair<QString, bool> generateImageThumbnailImpl(const ThumbnailTask& task);
    QPair<QString, bool> generateVideoThumbnailImpl(const ThumbnailTask& task);
    QPair<QPixmap, QString> getExpiredThumbnailImpl(const QString& thumbnailName,
                                                    const QString& fileType,
                                                    bool checkSourceExists,
                                                    const QString& sourcePath);

    // 保存缩略图
    bool saveThumbnail(const QImage& image, const QString& thumbnailPath);

    // 创建过期提示的缩略图
    static QPixmap createExpiredThumbnail(const QPixmap& baseThumbnail, const QString& fileType);
    static QPixmap createDefaultExpiredThumbnail(const QSize& size, const QString& fileType);
};

#endif // ASYNCTHUMBNAILMANAGER_H
