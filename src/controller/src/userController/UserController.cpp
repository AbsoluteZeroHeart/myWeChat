#include "UserController.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include "DataAccessContext.h"

UserController::UserController(QObject *parent)
    : QObject(parent)
    , m_dataAccessContext(nullptr)
    , m_currentUserId(-1)
{
    m_dataAccessContext = new DataAccessContext(this);
    if (m_dataAccessContext) {
        m_currentUserId = m_dataAccessContext->userTable()->getCurrentUserId();
    }
}

UserController::~UserController()
{
}

bool UserController::login(const QString& account, const QString& password)
{
    // 暂时不实现网络请求验证账号密码
    // 模拟登录成功
    User user = m_dataAccessContext->userTable()->getUserByAccount(account);
    if (!user.isValid()) {
        qWarning() << "Login failed: user not found -" << account;
        return false;
    }
    
    // 设置为当前用户
    user.isCurrent = true;
    if (m_dataAccessContext->userTable()->saveCurrentUser(user)) {
        m_currentUserId = user.userId;
        emit userLoggedIn(user);
        emit currentUserChanged(user);
        qInfo() << "User logged in:" << account;
        return true;
    }
    
    return false;
}

bool UserController::logout()
{    
    if (m_dataAccessContext->userTable()->clearCurrentUser()) {
        qint64 oldUserId = m_currentUserId;
        m_currentUserId = -1;
        emit userLoggedOut();
        qInfo() << "User logged out, previous user ID:" << oldUserId;
        return true;
    }
    
    return false;
}

bool UserController::isLoggedIn() const
{
    return m_currentUserId > 0;
}

User UserController::getCurrentUser()
{    
    return m_dataAccessContext->userTable()->getCurrentUser();
}

qint64 UserController::getCurrentUserId()
{
    return m_dataAccessContext->userTable()->getCurrentUserId();
}

bool UserController::updateCurrentUser(const User& user)
{    
    User userToUpdate = user;
    userToUpdate.isCurrent = true;
    
    if (m_dataAccessContext->userTable()->updateCurrentUser(userToUpdate)) {
        m_currentUserId = userToUpdate.userId;
        emit userProfileUpdated(userToUpdate);
        emit currentUserChanged(userToUpdate);
        return true;
    }
    
    return false;
}

bool UserController::updateCurrentUserProfile(const User& profileUpdates)
{    
    User currentUser = getCurrentUser();
    if (!currentUser.isValid()) return false;
    
    User updatedUser = mergeUserData(currentUser, profileUpdates);
    return updateCurrentUser(updatedUser);
}

bool UserController::clearCurrentUser()
{
    return logout();
}

bool UserController::saveUser(const User& user)
{
    if (!validateUserData(user)) {
        qWarning() << "Save user failed: invalid user data";
        return false;
    }
    
    if (m_dataAccessContext->userTable()->saveUser(user)) {
        emit userAdded(user);
        emit userDataChanged();
        return true;
    }
    
    return false;
}

bool UserController::updateUser(const User& user)
{    
    if (!validateUserData(user)) {
        qWarning() << "Update user failed: invalid user data";
        return false;
    }
    
    if (m_dataAccessContext->userTable()->updateUser(user)) {
        // 如果是当前用户，发送信号
        if (user.userId == m_currentUserId) {
            emit userProfileUpdated(user);
            emit currentUserChanged(user);
        } else {
            emit userDataChanged();
        }
        return true;
    }
    
    return false;
}

bool UserController::deleteUser(qint64 userId)
{    
    if (m_dataAccessContext->userTable()->deleteUser(userId)) {
        // 如果是当前用户，更新状态
        if (userId == m_currentUserId) {
            m_currentUserId = -1;
            emit userLoggedOut();
        }
        emit userDeleted(userId);
        emit userDataChanged();
        return true;
    }
    
    return false;
}

User UserController::getUser(qint64 userId)
{    
    return m_dataAccessContext->userTable()->getUser(userId);
}

User UserController::getUserByAccount(const QString& account)
{    
    return m_dataAccessContext->userTable()->getUserByAccount(account);
}

QList<User> UserController::getAllUsers()
{    
    return m_dataAccessContext->userTable()->getAllUsers();
}

bool UserController::userExists(qint64 userId)
{    
    return m_dataAccessContext->userTable()->userExists(userId);
}

QString UserController::getNickname(qint64 userId)
{    
    return m_dataAccessContext->userTable()->getNickname(userId);
}

QString UserController::getAvatarLocalPath(qint64 userId)
{    
    return m_dataAccessContext->userTable()->getAvatarLocalPath(userId);
}

bool UserController::updateNickname(const QString& nickname)
{
    if (!isLoggedIn()) return false;
    
    User updates;
    updates.nickname = nickname;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateAvatar(const QString& avatarUrl, const QString& localPath)
{
    if (!isLoggedIn()) return false;
    
    User updates;
    updates.avatar = avatarUrl;
    if (!localPath.isEmpty()) {
        updates.avatarLocalPath = localPath;
    } else {
        updates.avatarLocalPath = generateTempAvatarPath(avatarUrl);
    }
    return updateCurrentUserProfile(updates);
}

bool UserController::updateSignature(const QString& signature)
{
    if (!isLoggedIn()) return false;
    
    User updates;
    updates.signature = signature;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateGender(int gender)
{
    if (!isLoggedIn()) return false;
    
    User updates;
    updates.gender = gender;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateRegion(const QString& region)
{
    if (!isLoggedIn()) return false;
    
    User updates;
    updates.region = region;
    return updateCurrentUserProfile(updates);
}

bool UserController::batchSaveUsers(const QList<User>& users)
{    
    bool allSuccess = true;
    for (const User& user : users) {
        if (!saveUser(user)) {
            allSuccess = false;
            qWarning() << "Batch save failed for user:" << user.userId;
        }
    }
    
    if (allSuccess) {
        emit userDataChanged();
    }
    
    return allSuccess;
}

bool UserController::batchUpdateUsers(const QList<User>& users)
{    
    bool allSuccess = true;
    for (const User& user : users) {
        if (!updateUser(user)) {
            allSuccess = false;
            qWarning() << "Batch update failed for user:" << user.userId;
        }
    }
    
    return allSuccess;
}

QList<User> UserController::searchUsersByNickname(const QString& keyword)
{
    QList<User> result;    
    QList<User> allUsers = getAllUsers();
    for (const User& user : allUsers) {
        if (user.nickname.contains(keyword, Qt::CaseInsensitive)) {
            result.append(user);
        }
    }
    
    return result;
}

QList<User> UserController::searchUsersByAccount(const QString& keyword)
{
    QList<User> result;    
    QList<User> allUsers = getAllUsers();
    for (const User& user : allUsers) {
        if (user.account.contains(keyword, Qt::CaseInsensitive)) {
            result.append(user);
        }
    }
    
    return result;
}

QList<User> UserController::getUsersByGender(int gender)
{
    QList<User> result;
    QList<User> allUsers = getAllUsers();
    for (const User& user : allUsers) {
        if (user.gender == gender) {
            result.append(user);
        }
    }
    
    return result;
}

QList<User> UserController::getUsersByRegion(const QString& region)
{
    QList<User> result;
    
    QList<User> allUsers = getAllUsers();
    for (const User& user : allUsers) {
        if (user.region == region) {
            result.append(user);
        }
    }
    
    return result;
}

int UserController::getTotalUserCount()
{    
    return getAllUsers().size();
}

int UserController::getGenderStatistics(int gender)
{    
    return getUsersByGender(gender).size();
}

QJsonObject UserController::getUserStatistics()
{
    QJsonObject stats;
    
    int total = getTotalUserCount();
    int male = getGenderStatistics(1);   // 假设1代表男性
    int female = getGenderStatistics(2); // 假设2代表女性
    int unknown = getGenderStatistics(0); // 假设0代表未知
    
    stats["total_users"] = total;
    stats["male_users"] = male;
    stats["female_users"] = female;
    stats["unknown_gender"] = unknown;
    stats["male_percentage"] = total > 0 ? (male * 100.0 / total) : 0;
    stats["female_percentage"] = total > 0 ? (female * 100.0 / total) : 0;
    
    // 地区统计
    QJsonObject regionStats;
    QList<User> allUsers = getAllUsers();
    for (const User& user : allUsers) {
        if (!user.region.isEmpty()) {
            regionStats[user.region] = regionStats[user.region].toInt(0) + 1;
        }
    }
    stats["region_statistics"] = regionStats;
    
    return stats;
}

bool UserController::validateUserData(const User& user)
{
    // 检查必需字段
    if (user.userId <= 0 || user.account.isEmpty()) {
        return false;
    }
    
    // 检查账号是否重复（排除当前用户）
    User existingUser = getUserByAccount(user.account);
    if (existingUser.isValid()) {
        if (existingUser.userId != user.userId) {
            qWarning() << "Account already exists:" << user.account;
            return false;
        }
    }
    
    return true;
}

bool UserController::isAccountExist(const QString& account)
{    
    User user = getUserByAccount(account);
    return user.isValid();
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

QString UserController::generateTempAvatarPath(const QString& avatarUrl)
{
    if (avatarUrl.isEmpty()) return "";
    
    // 从URL中提取文件名
    QString fileName = QFileInfo(avatarUrl).fileName();
    if (fileName.isEmpty()) {
        fileName = QString::number(QDateTime::currentSecsSinceEpoch());
    }
    
    // 生成临时路径
    QString tempDir = QDir::tempPath() + "/avatar_cache/";
    QDir().mkpath(tempDir);
    
    return tempDir + fileName;
}
