#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QList>
#include <QJsonArray>
#include "models/Contact.h"

// 联系人表数据访问类，负责处理联系人相关的数据库操作
 class ContactTable : public QObject {
    Q_OBJECT

public:
     explicit ContactTable(QObject *parent = nullptr);
     ~ContactTable() override;

public slots:

     bool init();

    void getCurrentUser(int reqId);//获取当前登录用户

    // 异步槽函数：均携带reqId参数，用于Controller区分并发请求的结果对应关系
    void saveContact(int reqId, Contact contact);    // 保存联系人（新增）
    void updateContact(int reqId, Contact contact);  // 更新联系人信息
    void deleteContact(int reqId, qint64 userId);    // 根据用户ID删除联系人


    void getAllContacts(int reqId);                  // 获取所有联系人
    void getContact(int reqId, qint64 userId);       // 根据用户ID获取单个联系人
    void searchContacts(int reqId, const QString &keyword);    // 根据关键词搜索联系人
    void getStarredContacts(int reqId);              // 获取所有星标联系人

    void setContactStarred(int reqId, qint64 userId, bool starred); // 设置联系人是否为星标
    void setContactBlocked(int reqId, qint64 userId, bool blocked); // 设置联系人是否被屏蔽


signals:

    // 操作结果信号：均携带reqId，与对应槽函数的请求ID关联，用于返回结果
    void contactSaved(int reqId, bool ok, QString reason);      // 联系人保存结果（成功/失败，失败原因）
    void contactUpdated(int reqId, bool ok, QString reason);    // 联系人更新结果
    void contactDeleted(int reqId, bool ok, QString reason);    // 联系人删除结果

    void allContactsLoaded(int reqId, QList<Contact> contacts);      // 所有联系人加载完成（返回联系人列表）
    void contactLoaded(int reqId, Contact contact);                  // 单个联系人加载完成（返回联系人对象）
    void searchContactsResult(int reqId, QList<Contact> contacts);   // 搜索联系人结果（返回符合条件的联系人列表）
    void starredContactsLoaded(int reqId, QList<Contact> contacts);  // 星标联系人加载完成

    void contactStarredSet(int reqId, bool ok);    // 星标状态设置结果
    void contactBlockedSet(int reqId, bool ok);    // 屏蔽状态设置结果
    void dbError(int reqId, QString error);        // 数据库错误信号（reqId为-1时表示初始化或通用错误）


private:
    // 数据库连接智能指针，管理数据库连接生命周期
    QSharedPointer<QSqlDatabase> m_database;

    // 内部辅助函数
    QString buildSearchCondition(const QString &keyword) const;  // 根据搜索关键词构建SQL查询条件
    QString tagsToString(const QJsonArray &tags) const;          // 将QJsonArray类型的标签转换为字符串（用于数据库存储）

};
