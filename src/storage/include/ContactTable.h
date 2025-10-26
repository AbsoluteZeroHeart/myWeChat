#ifndef CONTACTTABLE_H
#define CONTACTTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDateTime>
#include <QDebug>

class ContactTable : public QObject
{
    Q_OBJECT

public:
    explicit ContactTable(QSqlDatabase database, QObject *parent = nullptr);

    // 联系人管理
    bool saveContact(const QJsonObject& contact);
    bool updateContact(const QJsonObject& contact);
    bool deleteContact(qint64 userId);
    QJsonArray getAllContacts();
    QJsonObject getContact(qint64 userId);
    QString getRemarkName(qint64 userId);
    bool isContact(qint64 userId);
    QJsonArray searchContacts(const QString& keyword);

    // 联系人状态管理
    bool setContactStarred(qint64 userId, bool starred);
    bool setContactBlocked(qint64 userId, bool blocked);
    QJsonArray getStarredContacts();

private:
    QSqlDatabase m_database;

    // 辅助方法
    QJsonObject contactFromQuery(const QSqlQuery& query);
    QString buildSearchCondition(const QString& keyword);
};

#endif // CONTACTTABLE_H
