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
    // 这里应该是网络请求验证账号密码
    // 模拟登录成功，实际项目中需要调用网络接口
    QJsonObject user = m_dataAccessContext->userTable()->getUserByAccount(account);
    if (user.isEmpty()) {
        qWarning() << "Login failed: user not found -" << account;
        return false;
    }
    
    // 设置为当前用户
    user["is_current"] = 1;
    if (m_dataAccessContext->userTable()->saveCurrentUser(user)) {
        m_currentUserId = user["user_id"].toVariant().toLongLong();
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

QJsonObject UserController::getCurrentUser()
{    
    return m_dataAccessContext->userTable()->getCurrentUser();
}

qint64 UserController::getCurrentUserId()
{
    return m_dataAccessContext->userTable()->getCurrentUserId();
}

bool UserController::updateCurrentUser(const QJsonObject& user)
{    
    QJsonObject userToUpdate = user;
    userToUpdate["is_current"] = 1;
    
    if (m_dataAccessContext->userTable()->updateCurrentUser(userToUpdate)) {
        m_currentUserId = userToUpdate["user_id"].toVariant().toLongLong();
        emit userProfileUpdated(userToUpdate);
        emit currentUserChanged(userToUpdate);
        return true;
    }
    
    return false;
}

bool UserController::updateCurrentUserProfile(const QJsonObject& profileUpdates)
{    
    QJsonObject currentUser = getCurrentUser();
    if (currentUser.isEmpty()) return false;
    
    QJsonObject updatedUser = mergeJsonObjects(currentUser, profileUpdates);
    return updateCurrentUser(updatedUser);
}

bool UserController::clearCurrentUser()
{
    return logout();
}

bool UserController::saveUser(const QJsonObject& user)
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

bool UserController::updateUser(const QJsonObject& user)
{    
    if (!validateUserData(user)) {
        qWarning() << "Update user failed: invalid user data";
        return false;
    }
    
    if (m_dataAccessContext->userTable()->updateUser(user)) {
        // 如果是当前用户，发送信号
        if (user["user_id"].toVariant().toLongLong() == m_currentUserId) {
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

QJsonObject UserController::getUser(qint64 userId)
{    
    return m_dataAccessContext->userTable()->getUser(userId);
}

QJsonObject UserController::getUserByAccount(const QString& account)
{    
    return m_dataAccessContext->userTable()->getUserByAccount(account);
}

QJsonArray UserController::getAllUsers()
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
    
    QJsonObject updates;
    updates["nickname"] = nickname;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateAvatar(const QString& avatarUrl, const QString& localPath)
{
    if (!isLoggedIn()) return false;
    
    QJsonObject updates;
    updates["avatar"] = avatarUrl;
    if (!localPath.isEmpty()) {
        updates["avatar_local_path"] = localPath;
    } else {
        updates["avatar_local_path"] = generateTempAvatarPath(avatarUrl);
    }
    return updateCurrentUserProfile(updates);
}

bool UserController::updateSignature(const QString& signature)
{
    if (!isLoggedIn()) return false;
    
    QJsonObject updates;
    updates["signature"] = signature;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateGender(int gender)
{
    if (!isLoggedIn()) return false;
    
    QJsonObject updates;
    updates["gender"] = gender;
    return updateCurrentUserProfile(updates);
}

bool UserController::updateRegion(const QString& region)
{
    if (!isLoggedIn()) return false;
    
    QJsonObject updates;
    updates["region"] = region;
    return updateCurrentUserProfile(updates);
}

bool UserController::batchSaveUsers(const QJsonArray& users)
{    
    bool allSuccess = true;
    for (const QJsonValue& value : users) {
        if (!saveUser(value.toObject())) {
            allSuccess = false;
            qWarning() << "Batch save failed for user:" << value.toObject()["user_id"].toVariant().toLongLong();
        }
    }
    
    if (allSuccess) {
        emit userDataChanged();
    }
    
    return allSuccess;
}

bool UserController::batchUpdateUsers(const QJsonArray& users)
{    
    bool allSuccess = true;
    for (const QJsonValue& value : users) {
        if (!updateUser(value.toObject())) {
            allSuccess = false;
            qWarning() << "Batch update failed for user:" << value.toObject()["user_id"].toVariant().toLongLong();
        }
    }
    
    return allSuccess;
}

QJsonArray UserController::searchUsersByNickname(const QString& keyword)
{
    QJsonArray result;    
    QJsonArray allUsers = getAllUsers();
    for (const QJsonValue& value : allUsers) {
        QJsonObject user = value.toObject();
        QString nickname = user["nickname"].toString();
        if (nickname.contains(keyword, Qt::CaseInsensitive)) {
            result.append(user);
        }
    }
    
    return result;
}

QJsonArray UserController::searchUsersByAccount(const QString& keyword)
{
    QJsonArray result;    
    QJsonArray allUsers = getAllUsers();
    for (const QJsonValue& value : allUsers) {
        QJsonObject user = value.toObject();
        QString account = user["account"].toString();
        if (account.contains(keyword, Qt::CaseInsensitive)) {
            result.append(user);
        }
    }
    
    return result;
}

QJsonArray UserController::getUsersByGender(int gender)
{
    QJsonArray result;
    QJsonArray allUsers = getAllUsers();
    for (const QJsonValue& value : allUsers) {
        QJsonObject user = value.toObject();
        if (user["gender"].toInt() == gender) {
            result.append(user);
        }
    }
    
    return result;
}

QJsonArray UserController::getUsersByRegion(const QString& region)
{
    QJsonArray result;
    
    QJsonArray allUsers = getAllUsers();
    for (const QJsonValue& value : allUsers) {
        QJsonObject user = value.toObject();
        if (user["region"].toString() == region) {
            result.append(user);
        }
    }
    
    return result;
}

int UserController::getTotalUserCount()
{    
    QJsonArray allUsers = getAllUsers();
    return allUsers.size();
}

int UserController::getGenderStatistics(int gender)
{    
    QJsonArray genderUsers = getUsersByGender(gender);
    return genderUsers.size();
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
    QJsonArray allUsers = getAllUsers();
    for (const QJsonValue& value : allUsers) {
        QJsonObject user = value.toObject();
        QString region = user["region"].toString();
        if (!region.isEmpty()) {
            regionStats[region] = regionStats[region].toInt(0) + 1;
        }
    }
    stats["region_statistics"] = regionStats;
    
    return stats;
}

bool UserController::validateUserData(const QJsonObject& user)
{
    // 检查必需字段
    if (!user.contains("user_id") || !user.contains("account")) {
        return false;
    }
    
    qint64 userId = user["user_id"].toVariant().toLongLong();
    QString account = user["account"].toString();
    
    if (userId <= 0 || account.isEmpty()) {
        return false;
    }
    
    // 检查账号是否重复（排除当前用户）
    QJsonObject existingUser = getUserByAccount(account);
    if (!existingUser.isEmpty()) {
        qint64 existingUserId = existingUser["user_id"].toVariant().toLongLong();
        if (existingUserId != userId) {
            qWarning() << "Account already exists:" << account;
            return false;
        }
    }
    
    return true;
}

bool UserController::isAccountExist(const QString& account)
{    
    QJsonObject user = getUserByAccount(account);
    return !user.isEmpty();
}

QJsonObject UserController::mergeJsonObjects(const QJsonObject& target, const QJsonObject& source)
{
    QJsonObject result = target;
    
    for (auto it = source.begin(); it != source.end(); ++it) {
        result[it.key()] = it.value();
    }
    
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
