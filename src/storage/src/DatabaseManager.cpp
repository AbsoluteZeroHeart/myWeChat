#include "DatabaseManager.h"
#include "DatabaseSchema.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrentRun>
#include <QFileInfo>
#include <QSqlError>

// 静态成员初始化
DatabaseManager* DatabaseManager::m_instance = nullptr;
QMutex DatabaseManager::m_mutex;

DatabaseManager* DatabaseManager::getInstance() {
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)
    , m_initializationCancelled(false)
    , m_futureWatcher(nullptr)
{
    m_databasePath = getDatabasePath();
}

DatabaseManager::~DatabaseManager() {
    if (m_futureWatcher) {
        m_futureWatcher->waitForFinished();
        delete m_futureWatcher;
    }
    
    if (m_database.isOpen()) {
        m_database.close();
    }
}

QString DatabaseManager::getDatabasePath() {
    QString dataLocation;
    
#ifdef Q_OS_WIN
    dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#elif defined(Q_OS_MAC)
    dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_LINUX)
    dataLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share";
#else
    dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
    
    QDir dir(dataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return dir.filePath("wechat_clone.db");
}

bool DatabaseManager::ensureDatabaseDirectory() {
    QFileInfo fileInfo(m_databasePath);
    QDir dir = fileInfo.dir();
    return dir.mkpath(".");
}

void DatabaseManager::initializeDatabaseAsync() {
    if (m_isInitialized) {
        emit databaseReady();
        return;
    }
    
    if (m_futureWatcher && m_futureWatcher->isRunning()) {
        qWarning() << "Database initialization is already in progress";
        return;
    }
    
    m_initializationCancelled = false;
    
    QFuture<bool> future = QtConcurrent::run( &DatabaseManager::initializeDatabase, this);
    
    m_futureWatcher = new QFutureWatcher<bool>(this);
    connect(m_futureWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
        bool success = m_futureWatcher->result();
        
        if (success) {
            m_isInitialized = true;
            emit initializationFinished(true);
            emit databaseReady();
            qDebug() << "Database initialized successfully";
        } else {
            emit initializationFinished(false, "Database initialization failed");
            qCritical() << "Database initialization failed";
        }
        
        m_futureWatcher->deleteLater();
        m_futureWatcher = nullptr;
    });
    
    m_futureWatcher->setFuture(future);
    emit initializationStarted();
}

bool DatabaseManager::initializeDatabase() {
    if (!ensureDatabaseDirectory()) {
        qCritical() << "Failed to create database directory";
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        QString connectionName = QString("async_connection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

        if (QSqlDatabase::contains(connectionName)) {
            m_database = QSqlDatabase::database(connectionName);
        } else {
            m_database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            m_database.setDatabaseName(m_databasePath);
        }
    }

    if (!m_database.open()) {
        qCritical() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }

    // 设置数据库参数
    QSqlQuery pragmaQuery(m_database);
    QStringList pragmaStatements = {
        "PRAGMA foreign_keys = ON",
        "PRAGMA journal_mode = WAL",
        "PRAGMA synchronous = NORMAL",
        "PRAGMA cache_size = -64000",
        "PRAGMA temp_store = MEMORY"
    };

    for (const QString& pragma : pragmaStatements) {
        if (!pragmaQuery.exec(pragma)) {
            qWarning() << "Failed to set PRAGMA:" << pragma << "-" << pragmaQuery.lastError().text();
        }
    }

    emit initializationProgress(30, "创建数据库表...");
    if (m_initializationCancelled) return false;

    if (!createTables()) {
        return false;
    }

    emit initializationProgress(60, "创建索引...");
    if (m_initializationCancelled) return false;

    if (!createIndexes()) {
        return false;
    }

    emit initializationProgress(100, "数据库已就绪");
    return true;
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_database);

    if (!m_database.transaction()) {
        qCritical() << "Failed to start transaction:" << m_database.lastError().text();
        return false;
    }

    try {
        QStringList createTableQueries = {
            DatabaseSchema::getCreateTableCurrentUser(),
            DatabaseSchema::getCreateTableContacts(),
            DatabaseSchema::getCreateTableGroups(),
            DatabaseSchema::getCreateTableGroupMembers(),
            DatabaseSchema::getCreateTableConversations(),
            DatabaseSchema::getCreateTableMessages(),
            DatabaseSchema::getCreateTableMediaCache()
        };

        for (const QString& sql : createTableQueries) {
            if (!query.exec(sql)) {
                throw std::runtime_error(
                    QString("Failed to create table: %1\nError: %2")
                        .arg(sql.left(50), query.lastError().text())
                        .toStdString()
                );
            }
        }

        if (!m_database.commit()) {
            throw std::runtime_error("Failed to commit transaction");
        }

        return true;

    } catch (const std::exception& e) {
        m_database.rollback();
        qCritical() << "Database table creation failed:" << e.what();
        return false;
    }
}

bool DatabaseManager::createIndexes() {
    QSqlQuery query(m_database);
    QString indexesSql = DatabaseSchema::getCreateIndexes();

    QStringList indexQueries = indexesSql.split(';', Qt::SkipEmptyParts);
    for (const QString& sql : std::as_const(indexQueries)) {
        QString trimmedSql = sql.trimmed();
        if (!trimmedSql.isEmpty() && !query.exec(trimmedSql)) {
            qWarning() << "Failed to create index:" << query.lastError().text();
        }
    }

    return true;
}

bool DatabaseManager::isDatabaseInitialized() const {
    return m_isInitialized;
}

QSqlDatabase DatabaseManager::getDatabase() {
    QMutexLocker locker(&m_mutex);

    if (!m_isInitialized) {
        qWarning() << "Database is not initialized yet";
        return QSqlDatabase();
    }

    if (QSqlDatabase::contains("main_connection")) {
        return QSqlDatabase::database("main_connection");
    } else {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
        db.setDatabaseName(m_databasePath);
        if (!db.open()) {
            qCritical() << "Failed to open main database connection";
        }
        return db;
    }
}

void DatabaseManager::cancelInitialization() {
    m_initializationCancelled = true;
    if (m_futureWatcher && m_futureWatcher->isRunning()) {
        m_futureWatcher->cancel();
    }
}

bool DatabaseManager::vacuumDatabase() {
    QSqlDatabase db = getDatabase();
    if (!db.isValid() || !db.isOpen()) return false;

    QSqlQuery query(db);
    return query.exec("VACUUM");
}

bool DatabaseManager::backupDatabase(const QString& backupPath) {
    if (!m_isInitialized) return false;
    
    QFile::remove(backupPath);
    return QFile::copy(m_databasePath, backupPath);
}

qint64 DatabaseManager::getDatabaseSize() {
    QFileInfo fileInfo(m_databasePath);
    return fileInfo.size();
}
