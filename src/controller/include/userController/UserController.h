#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H
// 头文件防重复包含

#include <QObject>
#include <QAtomicInteger>  // 线程安全计数器
#include <QHash>           // 哈希表存储键值对
#include <QJsonObject>     // JSON对象，用于统计数据
#include "DatabaseManager.h"  // 数据库管理器
#include "User.h"             // 用户数据结构

/**
 * @class UserController
 * @brief 用户控制器，管理用户信息的增删改查、状态更新等核心操作
 */
class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(DatabaseManager* dbManager, QObject* parent = nullptr); // 构造函数
    ~UserController(); // 析构函数

    // 异步操作方法
    void getCurrentUser();   // 获取当前用户信息
    void updateCurrentUser(const User& user); // 更新当前用户信息
    void updateCurrentUserProfile(const User& profileUpdates); // 更新当前用户资料
    void clearCurrentUser(); // 清除当前用户信息

    void saveUser(const User& user);   // 保存用户
    void updateUser(const User& user); // 更新用户信息
    void deleteUser(qint64 userId);    // 删除用户
    void getUser(qint64 userId);       // 获取指定用户
    void getAllUsers();                // 获取所有用户

    void updateNickname(const QString& nickname); // 更新昵称
    void updateAvatar(const QString& avatarUrl, const QString& localPath = QString()); // 更新头像
    void updateSignature(const QString& signature); // 更新个性签名
    void updateGender(int gender);            // 更新性别
    void updateRegion(const QString& region); // 更新地区

signals:
    // 操作结果信号
    void currentUserLoaded(int reqId, const User& user); // 当前用户加载结果
    void currentUserIdLoaded(int reqId, qint64 userId);  // 当前用户ID加载结果
    void currentUserUpdated(int reqId, bool success, const QString& error = QString()); // 当前用户更新结果
    void currentUserCleared(int reqId, bool success, const QString& error = QString()); // 当前用户清除结果
    void userSaved(int reqId, bool success, const QString& error = QString());   // 用户保存结果
    void userUpdated(int reqId, bool success, const QString& error = QString()); // 用户更新结果
    void userDeleted(int reqId, bool success, const QString& error = QString()); // 用户删除结果
    void userLoaded(int reqId, const User& user);            // 单个用户加载结果
    void allUsersLoaded(int reqId, const QList<User>& users);// 所有用户加载结果


    // 状态变更信号（用于UI更新）
    void userProfileUpdated(const User& user); // 用户资料更新通知
    void userAdded(const User& user); // 用户添加通知
    void userDeleted(qint64 userId);  // 用户删除通知
    void userDataChanged();           // 用户数据变更通知
    void userLoggedOut();             // 用户登出通知

private slots:
    // 数据库操作结果槽函数
    void onCurrentUserSaved(int reqId, bool success, const QString& error); // 当前用户保存结果处理
    void onCurrentUserLoaded(int reqId, const User& user);             // 当前用户加载结果处理
    void onUserSaved(int reqId, bool success, const QString& error);   // 用户保存结果处理
    void onUserUpdated(int reqId, bool success, const QString& error); // 用户更新结果处理
    void onUserDeleted(int reqId, bool success, const QString& error); // 用户删除结果处理
    void onUserLoaded(int reqId, const User& user);                    // 单个用户加载结果处理
    void onAllUsersLoaded(int reqId, const QList<User>& users);        // 所有用户加载结果处理
    void onDbError(int reqId, const QString& error);                   // 数据库错误处理

private:
    int generateReqId();   // 生成唯一请求ID
    void connectSignals(); // 连接信号槽
    User mergeUserData(const User& target, const User& source); // 合并用户数据

private:
    DatabaseManager* m_dbManager;            // 数据库管理器
    UserTable* m_userTable;                  // 用户表操作接口
    QAtomicInteger<int> m_reqIdCounter;      // 请求ID计数器（线程安全）
    User currentUser;                         // 当前登录用户
    QHash<int, QString> m_pendingOperations; // 待处理操作（reqId->操作类型）
};

#endif // USERCONTROLLER_H
