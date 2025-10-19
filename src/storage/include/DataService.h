#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QMutex>
#include "UserTable.h"
#include "ContactTable.h"
#include "GroupTable.h"
#include "GroupMemberTable.h"
#include "ConversationTable.h"
#include "MessageTable.h"
#include "MediaCacheTable.h"

class DataService : public QObject
{
    Q_OBJECT

public:
    static DataService* getInstance();
    
    // 获取各个表的操作接口
    UserTable* userTable();
    ContactTable* contactTable();
    GroupTable* groupTable();
    GroupMemberTable* groupMemberTable();
    ConversationTable* conversationTable();
    MessageTable* messageTable();
    MediaCacheTable* mediaCacheTable();
    
    // 初始化
    void initialize();
    bool isInitialized() const;

private:
    explicit DataService(QObject *parent = nullptr);
    ~DataService();
    
    static DataService* m_instance;
    static QMutex m_mutex;
    
    bool m_initialized;
    UserTable* m_userTable;
    ContactTable* m_contactTable;
    GroupTable* m_groupTable;
    GroupMemberTable* m_groupMemberTable;
    ConversationTable* m_conversationTable;
    MessageTable* m_messageTable;
    MediaCacheTable* m_mediaCacheTable;
};

#endif // DATASERVICE_H
