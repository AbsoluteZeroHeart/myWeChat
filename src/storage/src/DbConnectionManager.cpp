#include "DbConnectionManager.h"
#include "DatabaseInitializer.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDebug>

// 自动管理线程数据库连接
QThreadStorage<QSharedPointer<QSqlDatabase>> DbConnectionManager::s_connections;

QString DbConnectionManager::makeThreadConnectionName()
{
    auto id = reinterpret_cast<quintptr>(QThread::currentThreadId());
    return QString("thread_%1").arg(id);
}

QSharedPointer<QSqlDatabase> DbConnectionManager::connectionForCurrentThread()
{
    if (!s_connections.hasLocalData()) {
        // 创建新连接
        QString connName = makeThreadConnectionName();
        auto db = QSharedPointer<QSqlDatabase>::create(
            QSqlDatabase::addDatabase("QSQLITE", connName)
            );
        db->setDatabaseName(DatabaseInitializer::databasePath());
        if (!db->open()) {
            qCritical() << "Failed to open database for thread" << connName;
            return nullptr;
        }

        DatabaseInitializer::applyPragmas(*db);
        s_connections.setLocalData(db);

        qDebug() << "Created database connection for thread:" << connName;
    }
    return s_connections.localData();
}

