#include "DataService.h"
#include "DatabaseManager.h"

// 静态成员初始化
DataService* DataService::m_instance = nullptr;
QMutex DataService::m_mutex;

DataService* DataService::getInstance() {
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = new DataService();
    }
    return m_instance;
}

DataService::DataService(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_userTable(nullptr)
    , m_contactTable(nullptr)
    , m_groupTable(nullptr)
    , m_groupMemberTable(nullptr)
    , m_conversationTable(nullptr)
    , m_messageTable(nullptr)
    , m_mediaCacheTable(nullptr)
{
}

DataService::~DataService() {
    delete m_userTable;
    delete m_contactTable;
    delete m_groupTable;
    delete m_groupMemberTable;
    delete m_conversationTable;
    delete m_messageTable;
    delete m_mediaCacheTable;
}

void DataService::initialize() {
    if (m_initialized) return;
    
    QSqlDatabase db = DatabaseManager::getInstance()->getDatabase();
    if (!db.isValid()) return;
    
    m_userTable = new UserTable(db);
    m_contactTable = new ContactTable(db);
    m_groupTable = new GroupTable(db);
    m_groupMemberTable = new GroupMemberTable(db);
    m_conversationTable = new ConversationTable(db);
    m_messageTable = new MessageTable(db);
    m_mediaCacheTable = new MediaCacheTable(db);
    
    m_initialized = true;
}

bool DataService::isInitialized() const {
    return m_initialized;
}

UserTable* DataService::userTable() {
    return m_userTable;
}

ContactTable* DataService::contactTable() {
    return m_contactTable;
}

GroupTable* DataService::groupTable() {
    return m_groupTable;
}

GroupMemberTable* DataService::groupMemberTable() {
    return m_groupMemberTable;
}

ConversationTable* DataService::conversationTable() {
    return m_conversationTable;
}

MessageTable* DataService::messageTable() {
    return m_messageTable;
}

MediaCacheTable* DataService::mediaCacheTable() {
    return m_mediaCacheTable;
}