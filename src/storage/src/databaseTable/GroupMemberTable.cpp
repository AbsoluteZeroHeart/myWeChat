#include "GroupMemberTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "DbConnectionManager.h"

GroupMemberTable::GroupMemberTable(QObject *parent)
    : QObject(parent)
{}

GroupMemberTable::~GroupMemberTable()
{
    // 不需要手动关闭连接，智能指针会自动管理
}

void GroupMemberTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

void GroupMemberTable::saveGroupMember(int reqId, GroupMember member)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupMemberSaved(reqId, false, "Database is not open");
        return;
    }
    if (!member.isValid()) {
        emit groupMemberSaved(reqId, false, "Invalid member data");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("INSERT OR REPLACE INTO group_members ("
                  "group_id, user_id, nickname, role, join_time, is_contact"
                  ") VALUES (?, ?, ?, ?, ?, ?)");

    query.addBindValue(member.groupId);
    query.addBindValue(member.userId);
    query.addBindValue(member.nickname);
    query.addBindValue(member.role);
    query.addBindValue(member.joinTime);
    query.addBindValue(member.isContact ? 1 : 0);

    if (!query.exec()) {
        emit groupMemberSaved(reqId, false, query.lastError().text());
        return;
    }

    emit groupMemberSaved(reqId, true, QString());
}

void GroupMemberTable::updateGroupMember(int reqId, GroupMember member)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupMemberUpdated(reqId, false, "Database is not open");
        return;
    }
    if (!member.isValid()) {
        emit groupMemberUpdated(reqId, false, "Invalid member data");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE group_members SET "
                  "nickname = ?, role = ?, join_time = ?, is_contact = ? "
                  "WHERE group_id = ? AND user_id = ?");

    query.addBindValue(member.nickname);
    query.addBindValue(member.role);
    query.addBindValue(member.joinTime);
    query.addBindValue(member.isContact ? 1 : 0);
    query.addBindValue(member.groupId);
    query.addBindValue(member.userId);

    if (!query.exec()) {
        emit groupMemberUpdated(reqId, false, query.lastError().text());
        return;
    }

    emit groupMemberUpdated(reqId, query.numRowsAffected() > 0, QString());
}

void GroupMemberTable::deleteGroupMember(int reqId, qint64 groupId, qint64 userId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupMemberDeleted(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM group_members WHERE group_id = ? AND user_id = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        emit groupMemberDeleted(reqId, false, query.lastError().text());
        return;
    }

    emit groupMemberDeleted(reqId, query.numRowsAffected() > 0, QString());
}

void GroupMemberTable::deleteAllGroupMembers(int reqId, qint64 groupId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit groupMembersDeletedAll(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM group_members WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit groupMembersDeletedAll(reqId, false, query.lastError().text());
        return;
    }

    emit groupMembersDeletedAll(reqId, true, QString());
}

void GroupMemberTable::getGroupMembers(int reqId, qint64 groupId)
{
    QList<GroupMember> members;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit groupMembersLoaded(reqId, members);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? ORDER BY role DESC, nickname");
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit groupMembersLoaded(reqId, members);
        return;
    }

    while (query.next()) members.append(memberFromQuery(query));
    emit groupMembersLoaded(reqId, members);
}

void GroupMemberTable::getGroupMember(int reqId, qint64 groupId, qint64 userId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit groupMemberLoaded(reqId, GroupMember());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND user_id = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        emit groupMemberLoaded(reqId, GroupMember());
        return;
    }

    emit groupMemberLoaded(reqId, memberFromQuery(query));
}

void GroupMemberTable::searchGroupMembers(int reqId, qint64 groupId, const QString &keyword)
{
    QList<GroupMember> members;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit searchGroupMembersResult(reqId, members);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND "
                  "(nickname LIKE ? OR user_id IN (SELECT user_id FROM users WHERE username LIKE ?)) "
                  "ORDER BY role DESC, nickname");

    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(groupId);
    query.addBindValue(likePattern);
    query.addBindValue(likePattern);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit searchGroupMembersResult(reqId, members);
        return;
    }

    while (query.next()) members.append(memberFromQuery(query));
    emit searchGroupMembersResult(reqId, members);
}

void GroupMemberTable::syncGroupMembers(int reqId, qint64 groupId, QList<GroupMember> members)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit syncGroupMembersDone(reqId, false, "Database not open");
        return;
    }

    if (!m_database->transaction()) {
        emit syncGroupMembersDone(reqId, false, "Begin transaction failed");
        return;
    }

    bool ok = true;
    QString reason;

    // 删除旧成员
    QSqlQuery delQ(*m_database);
    delQ.prepare("DELETE FROM group_members WHERE group_id = ?");
    delQ.addBindValue(groupId);
    if (!delQ.exec()) {
        reason = delQ.lastError().text();
        ok = false;
    }

    // 插入新成员
    if (ok) {
        QSqlQuery insQ(*m_database);
        insQ.prepare("INSERT OR REPLACE INTO group_members ("
                     "group_id, user_id, nickname, role, join_time, is_contact"
                     ") VALUES (?, ?, ?, ?, ?, ?)");
        for (const GroupMember &gm : std::as_const(members)) {
            GroupMember gm2 = gm;
            gm2.groupId = groupId;
            insQ.addBindValue(gm2.groupId);
            insQ.addBindValue(gm2.userId);
            insQ.addBindValue(gm2.nickname);
            insQ.addBindValue(gm2.role);
            insQ.addBindValue(gm2.joinTime);
            insQ.addBindValue(gm2.isContact ? 1 : 0);
            if (!insQ.exec()) {
                reason = insQ.lastError().text();
                ok = false;
                break;
            }
            insQ.finish();
            insQ.prepare(insQ.lastQuery());
        }
    }

    if (ok) {
        if (!m_database->commit()) {
            reason = m_database->lastError().text();
            ok = false;
        }
    } else {
        m_database->rollback();
    }

    emit syncGroupMembersDone(reqId, ok, ok ? QString() : reason);
}

void GroupMemberTable::updateMemberRole(int reqId, qint64 groupId, qint64 userId, int role)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit memberRoleUpdated(reqId, false);
        emit dbError(reqId, "Database not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE group_members SET role = ? WHERE group_id = ? AND user_id = ?");
    query.addBindValue(role);
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        emit memberRoleUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit memberRoleUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupMemberTable::updateMemberNickname(int reqId, qint64 groupId, qint64 userId, const QString &nickname)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit memberNicknameUpdated(reqId, false);
        emit dbError(reqId, "Database not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE group_members SET nickname = ? WHERE group_id = ? AND user_id = ?");
    query.addBindValue(nickname);
    query.addBindValue(groupId);
    query.addBindValue(userId);

    if (!query.exec()) {
        emit memberNicknameUpdated(reqId, false);
        emit dbError(reqId, query.lastError().text());
        return;
    }

    emit memberNicknameUpdated(reqId, query.numRowsAffected() > 0);
}

void GroupMemberTable::refreshContactStatus(int reqId, qint64 userId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit refreshContactStatusDone(reqId, false);
        emit dbError(reqId, "Database not open");
        return;
    }

    // 检查 contacts 表中是否存在该用户
    QSqlQuery checkQ(*m_database);
    checkQ.prepare("SELECT COUNT(*) FROM contacts WHERE user_id = ?");
    checkQ.addBindValue(userId);

    if (!checkQ.exec() || !checkQ.next()) {
        emit refreshContactStatusDone(reqId, false);
        emit dbError(reqId, checkQ.lastError().text());
        return;
    }

    bool isContact = checkQ.value(0).toInt() > 0;

    QSqlQuery updateQ(*m_database);
    updateQ.prepare("UPDATE group_members SET is_contact = ? WHERE user_id = ?");
    updateQ.addBindValue(isContact ? 1 : 0);
    updateQ.addBindValue(userId);

    if (!updateQ.exec()) {
        emit refreshContactStatusDone(reqId, false);
        emit dbError(reqId, updateQ.lastError().text());
        return;
    }

    emit refreshContactStatusDone(reqId, true);
}

void GroupMemberTable::getGroupAdmins(int reqId, qint64 groupId)
{
    QList<GroupMember> admins;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit groupAdminsLoaded(reqId, admins);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM group_members WHERE group_id = ? AND role >= 1 ORDER BY role DESC, nickname");
    query.addBindValue(groupId);

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit groupAdminsLoaded(reqId, admins);
        return;
    }

    while (query.next()) admins.append(memberFromQuery(query));
    emit groupAdminsLoaded(reqId, admins);
}

void GroupMemberTable::getGroupMemberCount(int reqId, qint64 groupId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit groupMemberCountLoaded(reqId, -1);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        emit dbError(reqId, query.lastError().text());
        emit groupMemberCountLoaded(reqId, -1);
        return;
    }

    emit groupMemberCountLoaded(reqId, query.value(0).toInt());
}

GroupMember GroupMemberTable::memberFromQuery(const QSqlQuery &query) const
{
    return GroupMember::fromSqlQuery(query);
}
