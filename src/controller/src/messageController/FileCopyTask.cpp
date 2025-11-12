#include "FileCopyTask.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

FileCopyTask::FileCopyTask(const qint64 taskId,
                           const QString &sourceFilePath,
                           const QString &targetDir,
                           int priority)
    : m_taskId(taskId)
    , m_sourceFilePath(sourceFilePath)
    , m_targetDir(targetDir)
    , m_priority(priority)
{
    setAutoDelete(true);
}

void FileCopyTask::run()
{
    try {
        // 1. 检查源文件是否存在
        if (!QFile::exists(m_sourceFilePath)) {
            m_errorMessage = "源文件不存在: " + m_sourceFilePath;
            emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
            return;
        }

        QFileInfo sourceFileInfo(m_sourceFilePath);

        // 2. 确保目标目录存在
        QDir targetDir(m_targetDir);
        if (!targetDir.exists()) {
            if (!targetDir.mkpath(".")) {
                m_errorMessage = "无法创建目标目录: " + m_targetDir;
                emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
                return;
            }
        }

        // 3. 生成目标文件路径
        QString targetFileName;
        QString targetFilePath;

        // 检查目标目录中是否已存在同名文件
        QString originalFileName = sourceFileInfo.fileName();

        targetFileName = originalFileName;
        targetFilePath = targetDir.absoluteFilePath(targetFileName);

        // 如果目标文件已存在，生成唯一文件名
        if (QFile::exists(targetFilePath)) {
            targetFileName = generateUniqueFileName(originalFileName);
            targetFilePath = targetDir.absoluteFilePath(targetFileName);
        }

        // 4. 执行文件复制
        QFile sourceFile(m_sourceFilePath);
        if (!sourceFile.copy(targetFilePath)) {
            m_errorMessage = QString("文件复制失败: %1 -> %2, 错误: %3")
                                 .arg(m_sourceFilePath, targetFilePath, sourceFile.errorString());
            emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
            return;
        }

        // 5. 验证复制结果
        QFile targetFile(targetFilePath);
        if (!targetFile.exists()) {
            m_errorMessage = "复制后目标文件不存在";
            emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
            return;
        }

        // 检查文件大小是否一致
        if (sourceFileInfo.size() != targetFile.size()) {
            m_errorMessage = QString("文件大小不一致，源文件: %1 bytes, 目标文件: %2 bytes")
                                 .arg(sourceFileInfo.size())
                                 .arg(targetFile.size());
            targetFile.remove(); // 清理不完整的文件
            emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
            return;
        }

        qDebug() << "文件复制成功 - 源文件:" << m_sourceFilePath
                 << "目标文件:" << targetFilePath
                 << "文件大小:" << targetFile.size() << "bytes";

        emit finished(m_taskId, true, m_sourceFilePath, targetFilePath, "");

    } catch (const std::exception &e) {
        m_errorMessage = QString("复制过程异常: %1").arg(e.what());
        emit finished(m_taskId, false, m_sourceFilePath, "", m_errorMessage);
    }
}

QString FileCopyTask::generateUniqueFileName(const QString &originalFileName)
{
    QFileInfo fileInfo(originalFileName);
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix();

    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString randomSuffix = QString::number(QRandomGenerator::global()->generate() % 10000);

    return QString("%1_%2_%3.%4")
        .arg(baseName, timestamp, randomSuffix, suffix);
}
