#ifndef FILECOPYPROCESSOR_H
#define FILECOPYPROCESSOR_H

#include <QObject>
#include <QThreadPool>
#include <QString>
#include <QStringList>
#include <QSize>

class FileCopyTask;

class FileCopyProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileCopyProcessor(QObject *parent = nullptr);
    ~FileCopyProcessor();

    // 单个文件复制
    void copyFile(const qint64 taskId, const QString &sourceFilePath, const QString &targetDir, int priority = 0);

    // 批量文件复制
    void copyFiles(const qint64 taskId, const QStringList &sourceFilePaths, const QString &targetDir = QString(), int priority = 0);

    // 设置最大线程数
    void setMaxThreadCount(int count);

    // 取消所有任务
    void cancelAllTasks();

    // 获取默认保存路径
    static QString getDefaultSavePath();

signals:
    // 文件复制完成信号
    void fileCopied(const qint64 taskId, bool success, const QString &sourcePath,
                    const QString &targetPath, const QString &errorMessage);

private slots:
    void onTaskFinished(const qint64 taskId, bool success, const QString &sourcePath,
                        const QString &targetPath, const QString &errorMessage);

private:
    QThreadPool m_threadPool;
};

#endif // FILECOPYPROCESSOR_H
