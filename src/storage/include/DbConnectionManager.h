#pragma once

#include <QSqlDatabase>
#include <QThreadStorage>
#include <QMutex>

class DbConnectionManager {
public:
    static QSharedPointer<QSqlDatabase> connectionForCurrentThread();

private:
    static QThreadStorage<QSharedPointer<QSqlDatabase>> s_connections;
    static QString makeThreadConnectionName();
};
