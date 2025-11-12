#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QThreadPool>
#include <QString>
#include <QSize>

class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(QObject *parent = nullptr);
    ~VideoProcessor();

    void processVideo(const qint64 conversationId, const QString &sourceVideoPath, int priority = 0);
    void processVideos(const qint64 conversationId, const QStringList &sourceVideoPaths, int priority = 0);

    void setVideoSavePath(const QString &path);
    void setThumbnailSavePath(const QString &path);
    void setThumbnailSize(const QSize &size);
    void setMaxThreadCount(int count);
    void cancelAllTasks();

signals:
    void videoProcessed(qint64 conversationId,
                        const QString &originalPath,
                        const QString &thumbnailPath,
                        bool success);

    void videoProcessingFailed(qint64 conversationId,
                               const QString &sourcePath,
                               const QString &errorMessage);

private slots:
    void onTaskFinished(qint64 conversationId,
                        bool success,
                        const QString &sourcePath,
                        const QString &originalPath,
                        const QString &thumbnailPath,
                        const QString &errorMessage);

private:
    QThreadPool m_threadPool;
    QString m_videoSavePath;
    QString m_thumbnailSavePath;
    QSize m_thumbnailSize;
};

#endif // VIDEOPROCESSOR_H
