#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
class DataAccessContext;

class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(QObject *parent = nullptr);
    ~UserController();

    // 用户管理
    bool login(const QString& account, const QString& password); // 模拟登录
    bool logout();
    bool isLoggedIn() const;
    
    // 当前用户操作
    QJsonObject getCurrentUser();
    qint64 getCurrentUserId();
    bool updateCurrentUser(const QJsonObject& user);
    bool updateCurrentUserProfile(const QJsonObject& profileUpdates);
    bool clearCurrentUser();
    
    // 用户信息操作
    bool saveUser(const QJsonObject& user);
    bool updateUser(const QJsonObject& user);
    bool deleteUser(qint64 userId);
    QJsonObject getUser(qint64 userId);
    QJsonObject getUserByAccount(const QString& account);
    QJsonArray getAllUsers();
    bool userExists(qint64 userId);
    
    // 用户资料相关
    QString getNickname(qint64 userId);
    QString getAvatarLocalPath(qint64 userId);
    bool updateNickname(const QString& nickname);
    bool updateAvatar(const QString& avatarUrl, const QString& localPath = "");
    bool updateSignature(const QString& signature);
    bool updateGender(int gender);
    bool updateRegion(const QString& region);
    
    // 批量操作
    bool batchSaveUsers(const QJsonArray& users);
    bool batchUpdateUsers(const QJsonArray& users);
    
    // 搜索和过滤
    QJsonArray searchUsersByNickname(const QString& keyword);
    QJsonArray searchUsersByAccount(const QString& keyword);
    QJsonArray getUsersByGender(int gender);
    QJsonArray getUsersByRegion(const QString& region);
    
    // 统计信息
    int getTotalUserCount();
    int getGenderStatistics(int gender);
    QJsonObject getUserStatistics();
    
    // 数据验证
    bool validateUserData(const QJsonObject& user);
    bool isAccountExist(const QString& account);

signals:
    void userLoggedIn(const QJsonObject& user);
    void userLoggedOut();
    void userProfileUpdated(const QJsonObject& user);
    void userAdded(const QJsonObject& user);
    void userDeleted(qint64 userId);
    void currentUserChanged(const QJsonObject& user);
    void userDataChanged();

private:
    
    // 工具函数
    QJsonObject mergeJsonObjects(const QJsonObject& target, const QJsonObject& source);
    QString generateTempAvatarPath(const QString& avatarUrl);
    
    DataAccessContext* m_dataAccessContext;
    qint64 m_currentUserId;
};

#endif // USERCONTROLLER_H
