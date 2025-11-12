#include "DatabaseManager.h"
#include <QDebug>
#include "UserTable.h"
#include "ContactTable.h"
#include "GroupTable.h"
#include "GroupMemberTable.h"
#include "ConversationTable.h"
#include "MessageTable.h"
#include "MediaCacheTable.h"

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{
    m_dbThread = new QThread(this);

    m_userTable = new UserTable();
    m_contactTable = new ContactTable();
    m_groupTable = new GroupTable();
    m_groupMemberTable = new GroupMemberTable();
    m_conversationTable = new ConversationTable();
    m_messageTable = new MessageTable();
    m_mediaCacheTable = new MediaCacheTable();

    m_userTable->moveToThread(m_dbThread);
    m_contactTable->moveToThread(m_dbThread);
    m_groupTable->moveToThread(m_dbThread);
    m_groupMemberTable->moveToThread(m_dbThread);
    m_conversationTable->moveToThread(m_dbThread);
    m_messageTable->moveToThread(m_dbThread);
    m_mediaCacheTable->moveToThread(m_dbThread);

    connect(m_dbThread, &QThread::started, m_userTable, &UserTable::init);
    connect(m_dbThread, &QThread::started, m_contactTable, &ContactTable::init);
    connect(m_dbThread, &QThread::started, m_groupTable, &GroupTable::init);
    connect(m_dbThread, &QThread::started, m_groupMemberTable, &GroupMemberTable::init);
    connect(m_dbThread, &QThread::started, m_conversationTable, &ConversationTable::init);
    connect(m_dbThread, &QThread::started, m_messageTable, &MessageTable::init);
    connect(m_dbThread, &QThread::started, m_mediaCacheTable, &MediaCacheTable::init);
}

DatabaseManager::~DatabaseManager()
{
    stop();
}

void DatabaseManager::start()
{
    if (!m_dbThread->isRunning()) {
        m_dbThread->start();
    }
}

void DatabaseManager::stop()
{
    if (!m_dbThread) return;
    if (m_dbThread->isRunning()) {
        // 请求线程退出，并等待结束
        m_dbThread->quit();
        m_dbThread->wait(3000); // 可调整超时
    }
}










