#include "DatabaseManager.h"
#include "DatabaseSchema.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

std::atomic<bool> DatabaseManager::s_initialized{false};
QMutex DatabaseManager::s_initMutex;

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    m_dbPath = databasePath();

    // 在构造函数中自动检查数据库状态
    if (!s_initialized.load()) {
        QMutexLocker lock(&s_initMutex);
        if (!s_initialized.load()) {
            if (databaseFileExists()) {
                s_initialized.store(true);
                qDebug() << "Database already exists and is valid, marked as initialized";
            }
        }
    }
}

DatabaseManager::~DatabaseManager()
{
    if (QSqlDatabase::contains("main"))
        QSqlDatabase::database("main").close();
}

bool DatabaseManager::initializeDatabase()
{
    if (s_initialized.load()) {
        return true;
    }

    // 确保目录存在
    QDir dir(QFileInfo(m_dbPath).absolutePath());
    if (!dir.mkpath(".")) {
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (QSqlDatabase::contains("main"))
            QSqlDatabase::removeDatabase("main");

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "main");
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            return false;
        }
        setupConnection(db);

        bool ok = createTables() && createIndexes();
        db.close();
        if (!ok) {
            return false;
        }
    }

    s_initialized.store(true);
    qDebug() << "Database initialized successfully at" << m_dbPath;
    return true;
}

bool DatabaseManager::databaseFileExists()
{
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
                     "/wechat_clone.db";
    QFileInfo dbFile(dbPath);
    return dbFile.exists() && dbFile.size() > 0;
}

QSqlDatabase DatabaseManager::databaseForThread()
{
    if (!s_initialized.load()) {
        qWarning() << "Database not initialized yet";
        return QSqlDatabase();
    }

    QString connName = QString("thread_%1")
                           .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::database(connName);
        if (db.isOpen()) return db;
        QSqlDatabase::removeDatabase(connName);
    }

    QMutexLocker lock(&m_mutex);   // 保护 addDatabase
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qCritical() << "Failed to open db for thread" << connName;
        QSqlDatabase::removeDatabase(connName);
        return QSqlDatabase();
    }
    setupConnection(db);
    return db;
}

QString DatabaseManager::databasePath() const
{
    QString loc = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(loc);
    dir.mkpath(".");
    return dir.absoluteFilePath("wechat_clone.db");
}

void DatabaseManager::setupConnection(QSqlDatabase &db)
{
    const QStringList pragmas = {
        "PRAGMA foreign_keys = ON",
        "PRAGMA journal_mode = WAL",
        "PRAGMA synchronous = NORMAL",
        "PRAGMA cache_size = -64000",
        "PRAGMA temp_store = MEMORY"
    };
    for (const QString &p : pragmas) {
        QSqlQuery q(db);
        if (!q.exec(p))
            qWarning() << "Failed to set pragma" << p << q.lastError().text();
    }
}

bool DatabaseManager::createTables()
{
    QSqlDatabase db = QSqlDatabase::database("main");
    if (!db.transaction()) return false;
    QSqlQuery q(db);
    const QStringList sqls = {
        DatabaseSchema::getCreateTableCurrentUser(),
        DatabaseSchema::getCreateTableContacts(),
        DatabaseSchema::getCreateTableGroups(),
        DatabaseSchema::getCreateTableGroupMembers(),
        DatabaseSchema::getCreateTableConversations(),
        DatabaseSchema::getCreateTableMessages(),
        DatabaseSchema::getCreateTableMediaCache()
    };
    for (const QString &s : sqls) {
        if (!q.exec(s)) {
            db.rollback();
            qCritical() << "Create table failed:" << q.lastError().text();
            return false;
        }
    }
    return db.commit();
}

bool DatabaseManager::createIndexes()
{
    QSqlDatabase db = QSqlDatabase::database("main");
    QSqlQuery q(db);
    const QStringList idx = DatabaseSchema::getCreateIndexes().split(';', Qt::SkipEmptyParts);
    for (const QString& s : idx) {
        if (!s.trimmed().isEmpty() && !q.exec(s)) {
            qWarning() << "Create index failed:" << q.lastError().text();
        }
    }
    return true;
}
