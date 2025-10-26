#ifndef DATAACCESSCONTEXT_H
#define DATAACCESSCONTEXT_H

#include <QObject>
#include <QtSql/QSqlDatabase>

#include "UserTable.h"
#include "ContactTable.h"
#include "GroupTable.h"
#include "GroupMemberTable.h"
#include "ConversationTable.h"
#include "MessageTable.h"
#include "MediaCacheTable.h"
#include "DatabaseManager.h"


/**
 * @brief 数据访问上下文：封装一组表访问对象。
 */
class DataAccessContext : public QObject
{
    Q_OBJECT
public:
    explicit DataAccessContext(QObject *parent = nullptr);
    ~DataAccessContext();

    UserTable *userTable() const { return m_userTable; }
    ContactTable *contactTable() const { return m_contactTable; }
    GroupTable *groupTable() const { return m_groupTable; }
    GroupMemberTable *groupMemberTable() const { return m_groupMemberTable; }
    ConversationTable *conversationTable() const { return m_conversationTable; }
    MessageTable *messageTable() const { return m_messageTable; }
    MediaCacheTable *mediaCacheTable() const { return m_mediaCacheTable; }

private:

    QSqlDatabase m_db;
    DatabaseManager *databaseManager;
    UserTable *m_userTable = nullptr;
    ContactTable *m_contactTable = nullptr;
    GroupTable *m_groupTable = nullptr;
    GroupMemberTable *m_groupMemberTable = nullptr;
    ConversationTable *m_conversationTable = nullptr;
    MessageTable *m_messageTable = nullptr;
    MediaCacheTable *m_mediaCacheTable = nullptr;
};

#endif // DATAACCESSCONTEXT_H
