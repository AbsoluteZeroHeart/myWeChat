#include "UserController.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include "UserTable.h"

UserController::UserController(DatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_userTable(nullptr)
    , m_reqIdCounter(0)
{
    if (m_dbManager) {
        m_userTable = m_dbManager->userTable();
        connectSignals();

        // 异步获取当前用户
        getCurrentUser();
    } else {
        qWarning() << "DatabaseManager is null in UserController constructor";
    }
}

UserController::~UserController()
{}

int UserController::generateReqId()
{
    return m_reqIdCounter.fetchAndAddAcquire(1);
}

void UserController::connectSignals()
{
    if (!m_userTable) {
        qWarning() << "UserTable is null, cannot connect signals";
        return;
    }

    // 连接UserTable信号
    connect(m_userTable, &UserTable::currentUserSaved, this, &UserController::onCurrentUserSaved);
    connect(m_userTable, &UserTable::currentUserLoaded, this, &UserController::onCurrentUserLoaded);
    connect(m_userTable, &UserTable::userSaved, this, &UserController::onUserSaved);
    connect(m_userTable, &UserTable::userDeleted, this, &UserController::onUserDeleted);
    connect(m_userTable, &UserTable::userLoaded, this, &UserController::onUserLoaded);
    connect(m_userTable, &UserTable::allUsersLoaded, this, &UserController::onAllUsersLoaded);
    connect(m_userTable, &UserTable::dbError, this, &UserController::onDbError);
}

void UserController::getCurrentUser()
{
    if (!m_userTable) {
        emit currentUserLoaded(-1, User());
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "getCurrentUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId));
}


void UserController::updateCurrentUser(const User& user)
{
    if (!m_userTable) {
        emit currentUserUpdated(-1, false, "User table not available");
        return;
    }

    User userToUpdate = user;
    userToUpdate.isCurrent = true;

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "updateCurrentUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(User, userToUpdate));
}

void UserController::updateCurrentUserProfile(const User& profileUpdates)
{
}

void UserController::clearCurrentUser()
{
    if (!m_userTable) {
        emit currentUserCleared(-1, false, "User table not available");
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "clearCurrentUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId));
}

void UserController::saveUser(const User& user)
{
    if (!m_userTable) {
        emit userSaved(-1, false, "User table not available");
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "saveUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(User, user));
}

void UserController::updateUser(const User& user)
{
    if (!m_userTable) {
        emit userUpdated(-1, false, "User table not available");
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "updateUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(User, user));
}

void UserController::deleteUser(qint64 userId)
{
    if (!m_userTable) {
        emit userDeleted(-1, false, "User table not available");
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "deleteUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, userId));
}

void UserController::getUser(qint64 userId)
{
    if (!m_userTable) {
        emit userLoaded(-1, User());
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "getUser",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId),
                              Q_ARG(qint64, userId));
}

void UserController::getAllUsers()
{
    if (!m_userTable) {
        emit allUsersLoaded(-1, QList<User>());
        return;
    }

    int reqId = generateReqId();
    QMetaObject::invokeMethod(m_userTable, "getAllUsers",
                              Qt::QueuedConnection,
                              Q_ARG(int, reqId));
}

void UserController::updateNickname(const QString& nickname)
{
    User updates;
    updates.nickname = nickname;
    updateCurrentUserProfile(updates);
}

void UserController::updateAvatar(const QString& avatarUrl, const QString& localPath)
{
}

void UserController::updateSignature(const QString& signature)
{
    User updates;
    updates.signature = signature;
    updateCurrentUserProfile(updates);
}

void UserController::updateGender(int gender)
{
    User updates;
    updates.gender = gender;
    updateCurrentUserProfile(updates);
}

void UserController::updateRegion(const QString& region)
{
    User updates;
    updates.region = region;
    updateCurrentUserProfile(updates);
}

// 数据库操作结果处理槽函数
void UserController::onCurrentUserSaved(int reqId, bool success, const QString& error)
{
    emit currentUserUpdated(reqId, success, error);

    if (success) {
        emit userDataChanged();
    }
}

void UserController::onCurrentUserLoaded(int reqId, const User& user)
{
    if (user.isValid()) {
        currentUser = user;
        emit currentUserLoaded(reqId, user);
    } else {
        emit currentUserLoaded(reqId, User());
    }
}


void UserController::onUserSaved(int reqId, bool success, const QString& error)
{
    emit userSaved(reqId, success, error);

    if (success) {
        emit userDataChanged();
    }
}

void UserController::onUserUpdated(int reqId, bool success, const QString& error)
{
}

void UserController::onUserDeleted(int reqId, bool success, const QString& error)
{
    emit userDeleted(reqId, success, error);

    if (success) {
        emit userDataChanged();
    }
}

void UserController::onUserLoaded(int reqId, const User& user)
{
    emit userLoaded(reqId, user);
}

void UserController::onAllUsersLoaded(int reqId, const QList<User>& users)
{
    QString operation = m_pendingOperations.take(reqId);

    emit allUsersLoaded(reqId, users);
}

void UserController::onDbError(int reqId, const QString& error)
{
    qWarning() << "Database error in request" << reqId << ":" << error;
}

User UserController::mergeUserData(const User& target, const User& source)
{
    User result = target;

    // 只更新非空的字段
    if (!source.account.isEmpty()) result.account = source.account;
    if (!source.nickname.isEmpty()) result.nickname = source.nickname;
    if (!source.avatar.isEmpty()) result.avatar = source.avatar;
    if (!source.avatarLocalPath.isEmpty()) result.avatarLocalPath = source.avatarLocalPath;
    if (source.gender != 0) result.gender = source.gender;
    if (!source.region.isEmpty()) result.region = source.region;
    if (!source.signature.isEmpty()) result.signature = source.signature;

    return result;
}
