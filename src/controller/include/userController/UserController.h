#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "User.h"

class DataAccessContext;

class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(QObject *parent = nullptr);
    ~UserController();

    // 用户认证
    bool login(const QString& account, const QString& password);
    bool logout();
    bool isLoggedIn() const;

    // 当前用户操作
    User getCurrentUser();
    qint64 getCurrentUserId();
    bool updateCurrentUser(const User& user);
    bool updateCurrentUserProfile(const User& profileUpdates);
    bool clearCurrentUser();

    // 用户管理
    bool saveUser(const User& user);
    bool updateUser(const User& user);
    bool deleteUser(qint64 userId);
    User getUser(qint64 userId);
    User getUserByAccount(const QString& account);
    QList<User> getAllUsers();
    bool userExists(qint64 userId);

    // 便捷方法
    QString getNickname(qint64 userId);
    QString getAvatarLocalPath(qint64 userId);
    bool updateNickname(const QString& nickname);
    bool updateAvatar(const QString& avatarUrl, const QString& localPath = "");
    bool updateSignature(const QString& signature);
    bool updateGender(int gender);
    bool updateRegion(const QString& region);

    // 批量操作
    bool batchSaveUsers(const QList<User>& users);
    bool batchUpdateUsers(const QList<User>& users);

    // 搜索和统计
    QList<User> searchUsersByNickname(const QString& keyword);
    QList<User> searchUsersByAccount(const QString& keyword);
    QList<User> getUsersByGender(int gender);
    QList<User> getUsersByRegion(const QString& region);
    int getTotalUserCount();
    int getGenderStatistics(int gender);
    QJsonObject getUserStatistics();
    bool isAccountExist(const QString& account);

signals:
    void userLoggedIn(const User& user);
    void userLoggedOut();
    void userProfileUpdated(const User& user);
    void currentUserChanged(const User& user);
    void userAdded(const User& user);
    void userDeleted(qint64 userId);
    void userDataChanged();

private:
    DataAccessContext* m_dataAccessContext;
    qint64 m_currentUserId;

    bool validateUserData(const User& user);
    User mergeUserData(const User& target, const User& source);
    QString generateTempAvatarPath(const QString& avatarUrl);
};

#endif // USERCONTROLLER_H