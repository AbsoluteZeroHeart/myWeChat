#ifndef CONTACTCONTROLLER_H
#define CONTACTCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "Contact.h"

class DataAccessContext;

class ContactController : public QObject
{
    Q_OBJECT

public:
    explicit ContactController(QObject *parent = nullptr);
    ~ContactController();

    // 联系人管理
    bool addContact(const Contact& contact);
    bool updateContact(const Contact& contact);
    bool deleteContact(qint64 userId);
    Contact getContact(qint64 userId);
    QList<Contact> getAllContacts();
    bool isContact(qint64 userId);
    QList<Contact> searchContacts(const QString& keyword);
    
    // 联系人状态管理
    bool setContactStarred(qint64 userId, bool starred);
    bool setContactBlocked(qint64 userId, bool blocked);
    QList<Contact> getStarredContacts();
    
    // 便捷方法
    QString getRemarkName(qint64 userId);
    bool updateRemarkName(qint64 userId, const QString& remarkName);
    bool updateDescription(qint64 userId, const QString& description);
    bool addTag(qint64 userId, const QString& tag);
    bool removeTag(qint64 userId, const QString& tag);
    bool updateLastContactTime(qint64 userId);

signals:
    void contactAdded(const Contact& contact);
    void contactUpdated(const Contact& contact);
    void contactDeleted(qint64 userId);
    void contactStarredChanged(qint64 userId, bool starred);
    void contactBlockedChanged(qint64 userId, bool blocked);
    void contactsChanged();

private:
    DataAccessContext* m_dataAccessContext;
};

#endif // CONTACTCONTROLLER_H