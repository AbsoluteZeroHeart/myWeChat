#ifndef FILECOPYTASK_H
#define FILECOPYTASK_H

#include <QRunnable>
#include <QString>
#include <QObject>

class FileCopyTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    FileCopyTask(const qint64 taskId,
                 const QString &sourceFilePath,
                 const QString &targetDir,
                 int priority = 0);

    void run() override;

signals:
    void finished(const qint64 taskId, bool success, const QString &sourcePath,
                  const QString &targetPath, const QString &errorMessage);

private:
    qint64 m_taskId;
    QString m_sourceFilePath;
    QString m_targetDir;
    int m_priority;
    QString m_errorMessage;

    // 生成唯一文件名
    QString extracted(QString &baseName, QString &suffix, QString &timestamp,
                      QString &randomSuffix);
    QString generateUniqueFileName(const QString &originalFileName);
};

#endif // FILECOPYTASK_H
