#ifndef CONTACTTABLE_H
#define CONTACTTABLE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include "Contact.h"

class ContactTable : public QObject
{
    Q_OBJECT

public:
    explicit ContactTable(QSqlDatabase database, QObject *parent = nullptr);

    // 联系人管理
    bool saveContact(const Contact& contact);
    bool updateContact(const Contact& contact);
    bool deleteContact(qint64 userId);
    QList<Contact> getAllContacts();
    Contact getContact(qint64 userId);
    QString getRemarkName(qint64 userId);
    bool isContact(qint64 userId);
    QList<Contact> searchContacts(const QString& keyword);
    
    // 联系人状态管理
    bool setContactStarred(qint64 userId, bool starred);
    bool setContactBlocked(qint64 userId, bool blocked);
    QList<Contact> getStarredContacts();

private:
    QSqlDatabase m_database;
    
    QString buildSearchCondition(const QString& keyword);
};

#endif // CONTACTTABLE_H
