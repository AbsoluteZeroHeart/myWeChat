#include "ImageProcessingTask.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

ImageProcessingTask::ImageProcessingTask(const qint64 conversationId,
                                         const QString &sourceImagePath,
                                         const QString &imageSavePath,
                                         const QString &thumbnailSavePath,
                                         const QSize &thumbnailSize,
                                         int priority)
    : m_conversationId(conversationId)
    , m_sourceImagePath(sourceImagePath)
    , m_imageSavePath(imageSavePath)
    , m_thumbnailSavePath(thumbnailSavePath)
    , m_thumbnailSize(thumbnailSize)
    , m_priority(priority)
{
    setAutoDelete(true);
}

void ImageProcessingTask::run()
{
    try {
        // 1. 检查源文件是否存在
        if (!QFile::exists(m_sourceImagePath)) {
            m_errorMessage = "源图片文件不存在";
            emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
            return;
        }

        // 2. 加载原图
        QImage sourceImage(m_sourceImagePath);
        if (sourceImage.isNull()) {
            m_errorMessage = "无法加载图片文件";
            emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
            return;
        }

        // 3. 生成唯一文件名
        QFileInfo sourceFileInfo(m_sourceImagePath);
        QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
        QString randomSuffix = QString::number(QRandomGenerator::global()->generate() % 10000);
        QString fileName = QString("%1_%2_%3.%4")
                               .arg(sourceFileInfo.baseName(),
                                    timestamp,
                                    randomSuffix,
                                    sourceFileInfo.suffix());

        QString thumbFileName = QString("%1_%2_%3_thumb.%4")
                                    .arg(sourceFileInfo.baseName(),
                                         timestamp,
                                         randomSuffix,
                                         sourceFileInfo.suffix());

        // 4. 保存原图到目标路径
        QString originalDestPath = m_imageSavePath + "/" + fileName;
        if (!QFile::copy(m_sourceImagePath, originalDestPath)) {
            m_errorMessage = "复制原图失败";
            emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
            return;
        }

        // 5. 生成并保存缩略图
        QImage thumbnail = sourceImage.scaled(m_thumbnailSize,
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation);

        if (thumbnail.isNull()) {
            m_errorMessage = "生成缩略图失败";
            QFile::remove(originalDestPath); // 清理已复制的原图
            emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
            return;
        }

        QString thumbnailDestPath = m_thumbnailSavePath + "/" + thumbFileName;
        if (!thumbnail.save(thumbnailDestPath)) {
            m_errorMessage = "保存缩略图失败";
            QFile::remove(originalDestPath); // 清理已复制的原图
            emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
            return;
        }

        qDebug() << "图片处理成功 - 原图:" << originalDestPath
                 << "缩略图:" << thumbnailDestPath;

        emit finished(m_conversationId, true, m_sourceImagePath, originalDestPath, thumbnailDestPath, "");

    } catch (const std::exception &e) {
        m_errorMessage = QString("处理异常: %1").arg(e.what());
        emit finished(m_conversationId, false, m_sourceImagePath, "", "", m_errorMessage);
    }
}
