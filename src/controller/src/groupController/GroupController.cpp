#include "GroupController.h"
#include <QDebug>
#include "GroupTable.h"

GroupController::GroupController(DatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_groupTable(nullptr)
    , m_reqIdCounter(0)
{
    if (m_dbManager) {
        m_groupTable = m_dbManager->groupTable();
        connectSignals();
        connectAsyncSignals(); // 连接异步信号
    } else {
        qWarning() << "DatabaseManager is null in GroupController constructor";
    }
}

GroupController::~GroupController()
{
    disconnectSignals();
}

int GroupController::generateReqId()
{
    return m_reqIdCounter.fetchAndAddAcquire(1);
}

void GroupController::connectSignals()
{
    if (!m_groupTable) {
        qWarning() << "GroupTable is null, cannot connect signals";
        return;
    }

    // 连接GroupTable信号
    connect(m_groupTable, &GroupTable::groupSaved, this, &GroupController::onGroupSaved);
    connect(m_groupTable, &GroupTable::groupUpdated, this, &GroupController::onGroupUpdated);
    connect(m_groupTable, &GroupTable::groupDeleted, this, &GroupController::onGroupDeleted);
    connect(m_groupTable, &GroupTable::groupLoaded, this, &GroupController::onGroupLoaded);
    connect(m_groupTable, &GroupTable::allGroupsLoaded, this, &GroupController::onAllGroupsLoaded);
    connect(m_groupTable, &GroupTable::groupsSearched, this, &GroupController::onGroupsSearched);
    connect(m_groupTable, &GroupTable::groupAnnouncementUpdated, this, &GroupController::onGroupAnnouncementUpdated);
    connect(m_groupTable, &GroupTable::groupAvatarUpdated, this, &GroupController::onGroupAvatarUpdated);
    connect(m_groupTable, &GroupTable::groupMemberCountUpdated, this, &GroupController::onGroupMemberCountUpdated);
    connect(m_groupTable, &GroupTable::groupNoteUpdated, this, &GroupController::onGroupNoteUpdated);
    connect(m_groupTable, &GroupTable::localAvatarPathLoaded, this, &GroupController::onLocalAvatarPathLoaded);
    connect(m_groupTable, &GroupTable::dbError, this, &GroupController::onDbError);
}

void GroupController::connectAsyncSignals()
{
    if (!m_groupTable) return;

    // 直接连接到 GroupTable 的方法，使用异步连接
    connect(this, &GroupController::createGroupRequested,
            m_groupTable, &GroupTable::saveGroup, Qt::QueuedConnection);
    connect(this, &GroupController::updateGroupRequested,
            m_groupTable, &GroupTable::updateGroup, Qt::QueuedConnection);
    connect(this, &GroupController::deleteGroupRequested,
            m_groupTable, &GroupTable::deleteGroup, Qt::QueuedConnection);
    connect(this, &GroupController::getGroupRequested,
            m_groupTable, &GroupTable::getGroup, Qt::QueuedConnection);
    connect(this, &GroupController::getAllGroupsRequested,
            m_groupTable, &GroupTable::getAllGroups, Qt::QueuedConnection);
    connect(this, &GroupController::searchGroupsRequested,
            m_groupTable, &GroupTable::searchGroups, Qt::QueuedConnection);
    connect(this, &GroupController::updateGroupAnnouncementRequested,
            m_groupTable, &GroupTable::updateGroupAnnouncement, Qt::QueuedConnection);
    connect(this, &GroupController::updateGroupAvatarRequested,
            m_groupTable, &GroupTable::updateGroupAvatar, Qt::QueuedConnection);
    connect(this, &GroupController::updateGroupMemberCountRequested,
            m_groupTable, &GroupTable::updateGroupMemberCount, Qt::QueuedConnection);
    connect(this, &GroupController::updateGroupNoteRequested,
            m_groupTable, &GroupTable::updateGroupNote, Qt::QueuedConnection);
    connect(this, &GroupController::getLocalAvatarPathRequested,
            m_groupTable, &GroupTable::getLocalAvatarPath, Qt::QueuedConnection);
}

void GroupController::disconnectSignals()
{
    if (!m_groupTable) return;

    // 断开原有的信号连接
    disconnect(m_groupTable, &GroupTable::groupSaved, this, &GroupController::onGroupSaved);
    disconnect(m_groupTable, &GroupTable::groupUpdated, this, &GroupController::onGroupUpdated);
    disconnect(m_groupTable, &GroupTable::groupDeleted, this, &GroupController::onGroupDeleted);
    disconnect(m_groupTable, &GroupTable::groupLoaded, this, &GroupController::onGroupLoaded);
    disconnect(m_groupTable, &GroupTable::allGroupsLoaded, this, &GroupController::onAllGroupsLoaded);
    disconnect(m_groupTable, &GroupTable::groupsSearched, this, &GroupController::onGroupsSearched);
    disconnect(m_groupTable, &GroupTable::groupAnnouncementUpdated, this, &GroupController::onGroupAnnouncementUpdated);
    disconnect(m_groupTable, &GroupTable::groupAvatarUpdated, this, &GroupController::onGroupAvatarUpdated);
    disconnect(m_groupTable, &GroupTable::groupMemberCountUpdated, this, &GroupController::onGroupMemberCountUpdated);
    disconnect(m_groupTable, &GroupTable::groupNoteUpdated, this, &GroupController::onGroupNoteUpdated);
    disconnect(m_groupTable, &GroupTable::localAvatarPathLoaded, this, &GroupController::onLocalAvatarPathLoaded);
    disconnect(m_groupTable, &GroupTable::dbError, this, &GroupController::onDbError);

    // 断开异步信号连接
    disconnect(this, &GroupController::createGroupRequested,
               m_groupTable, &GroupTable::saveGroup);
    disconnect(this, &GroupController::updateGroupRequested,
               m_groupTable, &GroupTable::updateGroup);
    disconnect(this, &GroupController::deleteGroupRequested,
               m_groupTable, &GroupTable::deleteGroup);
    disconnect(this, &GroupController::getGroupRequested,
               m_groupTable, &GroupTable::getGroup);
    disconnect(this, &GroupController::getAllGroupsRequested,
               m_groupTable, &GroupTable::getAllGroups);
    disconnect(this, &GroupController::searchGroupsRequested,
               m_groupTable, &GroupTable::searchGroups);
    disconnect(this, &GroupController::updateGroupAnnouncementRequested,
               m_groupTable, &GroupTable::updateGroupAnnouncement);
    disconnect(this, &GroupController::updateGroupAvatarRequested,
               m_groupTable, &GroupTable::updateGroupAvatar);
    disconnect(this, &GroupController::updateGroupMemberCountRequested,
               m_groupTable, &GroupTable::updateGroupMemberCount);
    disconnect(this, &GroupController::updateGroupNoteRequested,
               m_groupTable, &GroupTable::updateGroupNote);
    disconnect(this, &GroupController::getLocalAvatarPathRequested,
               m_groupTable, &GroupTable::getLocalAvatarPath);
}

// 异步操作方法 - 只进行参数验证和发射信号
void GroupController::createGroup(const Group& group)
{
    if (!m_groupTable) {
        emit groupCreated(-1, false, "Group table not available");
        return;
    }

    if (!group.isValid()) {
        emit groupCreated(-1, false, "Invalid group data");
        return;
    }

    int reqId = generateReqId();
    m_pendingGroups.insert(reqId, group);
    emit createGroupRequested(reqId, group);
}

void GroupController::updateGroup(const Group& group)
{
    if (!m_groupTable) {
        emit groupUpdated(-1, false, "Group table not available");
        return;
    }

    if (!group.isValid()) {
        emit groupUpdated(-1, false, "Invalid group data");
        return;
    }

    int reqId = generateReqId();
    m_pendingGroups.insert(reqId, group);
    emit updateGroupRequested(reqId, group);
}

void GroupController::deleteGroup(qint64 groupId)
{
    if (!m_groupTable) {
        emit groupDeleted(-1, false, "Group table not available");
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit deleteGroupRequested(reqId, groupId);
}

void GroupController::getGroup(qint64 groupId)
{
    if (!m_groupTable) {
        emit groupLoaded(-1, Group());
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit getGroupRequested(reqId, groupId);
}

void GroupController::getAllGroups()
{
    if (!m_groupTable) {
        emit allGroupsLoaded(-1, QList<Group>());
        return;
    }

    int reqId = generateReqId();
    emit getAllGroupsRequested(reqId);
}

void GroupController::searchGroups(const QString& keyword)
{
    if (!m_groupTable) {
        emit groupsSearched(-1, QList<Group>());
        return;
    }

    int reqId = generateReqId();
    emit searchGroupsRequested(reqId, keyword);
}

void GroupController::updateGroupAnnouncement(qint64 groupId, const QString& announcement)
{
    if (!m_groupTable) {
        emit groupAnnouncementUpdated(-1, false);
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit updateGroupAnnouncementRequested(reqId, groupId, announcement);
}

void GroupController::updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath)
{
    if (!m_groupTable) {
        emit groupAvatarUpdated(-1, false);
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    m_pendingAvatarUpdates.insert(reqId, QPair<QString, QString>(avatarUrl, localPath));
    emit updateGroupAvatarRequested(reqId, groupId, avatarUrl, localPath);
}

void GroupController::updateGroupMemberCount(qint64 groupId, int memberCount)
{
    if (!m_groupTable) {
        emit groupMemberCountUpdated(-1, false);
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit updateGroupMemberCountRequested(reqId, groupId, memberCount);
}

void GroupController::updateGroupNote(qint64 groupId, const QString& groupNote)
{
    if (!m_groupTable) {
        emit groupNoteUpdated(-1, false);
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit updateGroupNoteRequested(reqId, groupId, groupNote);
}


void GroupController::getLocalAvatarPath(qint64 groupId)
{
    if (!m_groupTable) {
        emit localAvatarPathLoaded(-1, QString());
        return;
    }

    int reqId = generateReqId();
    m_pendingGroupIds.insert(reqId, groupId);
    emit getLocalAvatarPathRequested(reqId, groupId);
}




// 数据库操作结果处理槽函数
void GroupController::onGroupSaved(int reqId, bool success, const QString& error)
{
    Group group = m_pendingGroups.take(reqId);

    emit groupCreated(reqId, success, error);

    if (success) {
        emit groupCreated(group);
        emit groupsChanged();
    }
}

void GroupController::onGroupUpdated(int reqId, bool success, const QString& error)
{
    Group group = m_pendingGroups.take(reqId);

    emit groupUpdated(reqId, success, error);

    if (success) {
        emit groupUpdated(group);
        emit groupsChanged();
    }
}

void GroupController::onGroupDeleted(int reqId, bool success, const QString& error)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    emit groupDeleted(reqId, success, error);

    if (success) {
        emit groupDeleted(groupId);
        emit groupsChanged();
    }
}

void GroupController::onGroupLoaded(int reqId, const Group& group)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    // 检查是否有待处理的增量/减量操作
    // 这里可以根据需要添加逻辑来处理incrementMemberCount和decrementMemberCount

    emit groupLoaded(reqId, group);

    // 如果是通过getGroupName发起的请求，发射groupNameLoaded信号
    if (group.isValid()) {
        emit groupNameLoaded(reqId, group.getDisplayName());
    }
}

void GroupController::onAllGroupsLoaded(int reqId, const QList<Group>& groups)
{
    emit allGroupsLoaded(reqId, groups);
}

void GroupController::onGroupsSearched(int reqId, const QList<Group>& groups)
{
    emit groupsSearched(reqId, groups);
}

void GroupController::onGroupAnnouncementUpdated(int reqId, bool success)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    emit groupAnnouncementUpdated(reqId, success);

    if (success) {
        // 发射状态变更信号
        // 注意：这里我们不知道announcement的内容，所以只发射groupId
        // 如果需要announcement内容，需要在调用时保存
        emit groupAnnouncementChanged(groupId, "");
        emit groupsChanged();

        // 获取更新后的群组信息并发射groupUpdated信号
        getGroup(groupId);
    }
}

void GroupController::onGroupAvatarUpdated(int reqId, bool success)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);
    QPair<QString, QString> avatarInfo = m_pendingAvatarUpdates.take(reqId);

    emit groupAvatarUpdated(reqId, success);

    if (success) {
        emit groupAvatarChanged(groupId, avatarInfo.first);
        emit groupsChanged();

        // 获取更新后的群组信息并发射groupUpdated信号
        getGroup(groupId);
    }
}

void GroupController::onGroupMemberCountUpdated(int reqId, bool success)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    emit groupMemberCountUpdated(reqId, success);

    if (success) {
        // 注意：这里我们不知道确切的memberCount，所以发射-1表示未知
        // 如果需要确切值，需要在调用时保存
        emit groupMemberCountChanged(groupId, -1);
        emit groupsChanged();

        // 获取更新后的群组信息并发射groupUpdated信号
        getGroup(groupId);
    }
}

void GroupController::onGroupNoteUpdated(int reqId, bool success)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    emit groupNoteUpdated(reqId, success);

    if (success) {
        emit groupsChanged();

        // 获取更新后的群组信息并发射groupUpdated信号
        getGroup(groupId);
    }
}

void GroupController::onLocalAvatarPathLoaded(int reqId, const QString& localPath)
{
    qint64 groupId = m_pendingGroupIds.take(reqId);

    emit localAvatarPathLoaded(reqId, localPath);
}

void GroupController::onDbError(int reqId, const QString& error)
{
    qWarning() << "Database error in request" << reqId << ":" << error;

    // 清理pending数据
    m_pendingGroups.remove(reqId);
    m_pendingGroupIds.remove(reqId);
    m_pendingAvatarUpdates.remove(reqId);
}
