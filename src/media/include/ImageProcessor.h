#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QThreadPool>
#include <QSet>
#include <QMutex>

class ImageProcessingTask;

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor();

    // 处理单张图片
    void processImage(const qint64 conversationId, const QString &sourceImagePath, int priority = 0);

    // 批量处理图片
    void processImages(const qint64 conversationId, const QStringList &sourceImagePaths, int priority = 0);

    void setImageSavePath(const QString &path);
    void setThumbnailSavePath(const QString &path);
    void setThumbnailSize(const QSize &size);
    void setMaxThreadCount(int count);
    void cancelAllTasks();

signals:
    void imageProcessed(const qint64 conversationId,
                        const QString &originalImagePath,
                        const QString &thumbnailPath,
                        bool success);

    void imageProcessingFailed(const qint64 conversationId,
                               const QString &imagePath,
                               const QString &errorMessage);

private slots:
    void onTaskFinished(const qint64 conversationId,
                        bool success,
                        const QString &sourcePath,
                        const QString &originalPath,
                        const QString &thumbnailPath,
                        const QString &errorMessage);

private:
    QString m_imageSavePath;
    QString m_thumbnailSavePath;
    QSize m_thumbnailSize;
    QThreadPool m_threadPool;

};

#endif // IMAGEPROCESSOR_H
