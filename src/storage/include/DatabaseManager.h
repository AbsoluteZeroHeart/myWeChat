#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* getInstance();

    // 初始化相关
    void initializeDatabaseAsync();
    void cancelInitialization();
    bool isDatabaseInitialized() const;

    // 数据库连接
    QSqlDatabase getDatabase();

    // 数据库维护
    bool vacuumDatabase();
    bool backupDatabase(const QString& backupPath);
    qint64 getDatabaseSize();

signals:
    void initializationStarted();
    void initializationProgress(int progress, const QString& message);
    void initializationFinished(bool success, const QString& errorMessage = "");
    void databaseReady();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    static DatabaseManager* m_instance;
    static QMutex m_mutex;

    // 初始化相关方法
    QString getDatabasePath();
    bool ensureDatabaseDirectory();
    bool initializeDatabase();
    bool createTables();
    bool createIndexes();

    // 数据库连接
    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_isInitialized;
    bool m_initializationCancelled;
    QFutureWatcher<bool>* m_futureWatcher;
};

#endif // DATABASEMANAGER_H
