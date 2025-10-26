#include "DataAccessContext.h"


DataAccessContext::DataAccessContext(QObject *parent)
    : QObject(parent)
{
    databaseManager = new DatabaseManager(this);
    m_db = databaseManager->databaseForThread();

    m_userTable = new UserTable(m_db, this);
    m_contactTable = new ContactTable(m_db, this);
    m_groupTable = new GroupTable(m_db, this);
    m_groupMemberTable = new GroupMemberTable(m_db, this);
    m_conversationTable = new ConversationTable(m_db, this);
    m_messageTable = new MessageTable(m_db, this);
    m_mediaCacheTable = new MediaCacheTable(m_db, this);
}

DataAccessContext::~DataAccessContext() = default;
