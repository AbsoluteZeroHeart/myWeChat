#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QMutex>
#include <atomic>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initializeDatabase();
    bool isInitialized() const { return s_initialized.load(); }
    QSqlDatabase databaseForThread();

    // 检查数据库状态
    static bool databaseFileExists();

private:
    void setupConnection(QSqlDatabase &db);
    bool createTables();
    bool createIndexes();
    QString databasePath() const;

    QString m_dbPath;
    QMutex m_mutex;

    // 静态成员
    static std::atomic<bool> s_initialized;
    static QMutex s_initMutex;
};

#endif // DATABASEMANAGER_H
