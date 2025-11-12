#ifndef IMAGEPROCESSINGTASK_H
#define IMAGEPROCESSINGTASK_H

#include <QRunnable>
#include <QString>
#include <QSize>
#include <QImage>
#include <QObject>

class ImageProcessingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ImageProcessingTask(const qint64 conversationId,
                        const QString &sourceImagePath,
                        const QString &imageSavePath,
                        const QString &thumbnailSavePath,
                        const QSize &thumbnailSize,
                        int priority = 0);

    void run() override;

signals:
    void finished(const qint64 conversationId,
                  bool success,
                  const QString &sourcePath,
                  const QString &originalPath,
                  const QString &thumbnailPath,
                  const QString &errorMessage);

private:
    qint64 m_conversationId;
    QString m_sourceImagePath;
    QString m_imageSavePath;
    QString m_thumbnailSavePath;
    QSize m_thumbnailSize;
    int m_priority;

    QString m_errorMessage;
};

#endif // IMAGEPROCESSINGTASK_H
