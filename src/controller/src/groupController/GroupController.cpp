#include "GroupController.h"
#include "DataAccessContext.h"
#include <QDebug>

GroupController::GroupController(QObject *parent)
    : QObject(parent)
    , m_dataAccessContext(new DataAccessContext(this))
{
}

GroupController::~GroupController()
{
}

bool GroupController::createGroup(const Group& group)
{
    if (!group.isValid()) {
        qWarning() << "Invalid group data";
        return false;
    }

    if (m_dataAccessContext->groupTable()->saveGroup(group)) {
        emit groupCreated(group);
        emit groupsChanged();
        return true;
    }
    
    return false;
}

bool GroupController::updateGroup(const Group& group)
{
    if (!group.isValid()) {
        qWarning() << "Invalid group data";
        return false;
    }

    if (m_dataAccessContext->groupTable()->updateGroup(group)) {
        emit groupUpdated(group);
        emit groupsChanged();
        return true;
    }
    
    return false;
}

bool GroupController::deleteGroup(qint64 groupId)
{
    if (m_dataAccessContext->groupTable()->deleteGroup(groupId)) {
        emit groupDeleted(groupId);
        emit groupsChanged();
        return true;
    }
    
    return false;
}

Group GroupController::getGroup(qint64 groupId)
{
    return m_dataAccessContext->groupTable()->getGroup(groupId);
}

QList<Group> GroupController::getAllGroups()
{
    return m_dataAccessContext->groupTable()->getAllGroups();
}

QList<Group> GroupController::searchGroups(const QString& keyword)
{
    return m_dataAccessContext->groupTable()->searchGroups(keyword);
}

bool GroupController::updateGroupAnnouncement(qint64 groupId, const QString& announcement)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    if (m_dataAccessContext->groupTable()->updateGroupAnnouncement(groupId, announcement)) {
        group.announcement = announcement;
        emit groupAnnouncementChanged(groupId, announcement);
        emit groupUpdated(group);
        return true;
    }
    
    return false;
}

bool GroupController::updateGroupAvatar(qint64 groupId, const QString& avatarUrl, const QString& localPath)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    if (m_dataAccessContext->groupTable()->updateGroupAvatar(groupId, avatarUrl, localPath)) {
        group.updateAvatar(avatarUrl, localPath);
        emit groupAvatarChanged(groupId, avatarUrl);
        emit groupUpdated(group);
        return true;
    }
    
    return false;
}

bool GroupController::updateGroupMemberCount(qint64 groupId, int memberCount)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    if (m_dataAccessContext->groupTable()->updateGroupMemberCount(groupId, memberCount)) {
        group.memberCount = memberCount;
        emit groupMemberCountChanged(groupId, memberCount);
        emit groupUpdated(group);
        return true;
    }
    
    return false;
}

bool GroupController::updateGroupNote(qint64 groupId, const QString& groupNote)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    if (m_dataAccessContext->groupTable()->updateGroupNote(groupId, groupNote)) {
        group.groupNote = groupNote;
        emit groupUpdated(group);
        return true;
    }
    
    return false;
}

QString GroupController::getGroupName(qint64 groupId)
{
    Group group = getGroup(groupId);
    return group.isValid() ? group.getDisplayName() : "";
}

QString GroupController::getLocalAvatarPath(qint64 groupId)
{
    return m_dataAccessContext->groupTable()->getLocalAvatarPath(groupId);
}

bool GroupController::incrementMemberCount(qint64 groupId)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    int newCount = group.memberCount + 1;
    return updateGroupMemberCount(groupId, newCount);
}

bool GroupController::decrementMemberCount(qint64 groupId)
{
    Group group = getGroup(groupId);
    if (!group.isValid()) {
        return false;
    }

    int newCount = qMax(0, group.memberCount - 1);
    return updateGroupMemberCount(groupId, newCount);
}