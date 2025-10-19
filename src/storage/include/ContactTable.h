#ifndef CONTACTTABLE_H
#define CONTACTTABLE_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>

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
    QJsonArray searchContacts(const QString& keyword);
    bool clearContacts();
    
    // 联系人状态管理
    bool setContactStarred(qint64 userId, bool starred);
    bool setContactBlocked(qint64 userId, bool blocked);
    bool updateLastContactTime(qint64 userId);
    QJsonArray getStarredContacts();
    QJsonArray getRecentContacts(int limit = 20);

private:
    QSqlDatabase m_database;
};

#endif // CONTACTTABLE_H