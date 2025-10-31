#include "ContactTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>

ContactTable::ContactTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool ContactTable::saveContact(const Contact& contact)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO contacts ("
                  "user_id, remark_name, description, tags, phone_note, "
                  "email_note, source, is_starred, is_blocked, add_time, last_contact_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    // 处理 tags 字段
    QString tagsString = QJsonDocument(contact.tags).toJson(QJsonDocument::Compact);

    query.addBindValue(contact.userId);
    query.addBindValue(contact.remarkName);
    query.addBindValue(contact.description);
    query.addBindValue(tagsString);
    query.addBindValue(contact.phoneNote);
    query.addBindValue(contact.emailNote);
    query.addBindValue(contact.source);
    query.addBindValue(contact.isStarred ? 1 : 0);
    query.addBindValue(contact.isBlocked ? 1 : 0);
    query.addBindValue(contact.addTime);
    query.addBindValue(contact.lastContactTime);

    if (!query.exec()) {
        qWarning() << "Save contact failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool ContactTable::updateContact(const Contact& contact)
{
    if (!m_database.isOpen()) return false;
    if (!contact.isValid()) return false;

    QStringList updateFields;
    QVariantList bindValues;

    // 动态构建更新字段
    if (!contact.remarkName.isNull()) {
        updateFields << "remark_name = ?";
        bindValues << contact.remarkName;
    }
    if (!contact.description.isNull()) {
        updateFields << "description = ?";
        bindValues << contact.description;
    }
    if (!contact.phoneNote.isNull()) {
        updateFields << "phone_note = ?";
        bindValues << contact.phoneNote;
    }
    if (!contact.emailNote.isNull()) {
        updateFields << "email_note = ?";
        bindValues << contact.emailNote;
    }
    if (!contact.source.isNull()) {
        updateFields << "source = ?";
        bindValues << contact.source;
    }
    
    updateFields << "is_starred = ?";
    bindValues << (contact.isStarred ? 1 : 0);
    
    updateFields << "is_blocked = ?";
    bindValues << (contact.isBlocked ? 1 : 0);

    if (contact.addTime > 0) {
        updateFields << "add_time = ?";
        bindValues << contact.addTime;
    }
    if (contact.lastContactTime > 0) {
        updateFields << "last_contact_time = ?";
        bindValues << contact.lastContactTime;
    }

    // 特殊处理 tags 字段
    if (!contact.tags.isEmpty()) {
        updateFields << "tags = ?";
        QString tagsString = QJsonDocument(contact.tags).toJson(QJsonDocument::Compact);
        bindValues << tagsString;
    }

    if (updateFields.isEmpty()) return false;

    QString sql = "UPDATE contacts SET " + updateFields.join(", ") + " WHERE user_id = ?";
    bindValues << contact.userId;

    QSqlQuery query(m_database);
    query.prepare(sql);
    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    return query.exec() && query.numRowsAffected() > 0;
}

bool ContactTable::deleteContact(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Delete contact failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Contact> ContactTable::getAllContacts()
{
    QList<Contact> contacts;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return contacts;
    }

    QSqlQuery query("SELECT * FROM contacts ORDER BY remark_name", m_database);

    while (query.next()) {
        contacts.append(Contact(query));
    }

    return contacts;
}

Contact ContactTable::getContact(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return Contact();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        return Contact();
    }

    return Contact(query);
}

QString ContactTable::getRemarkName(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open (getRemarkName)";
        return "";
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT remark_name FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);
    if (!query.exec()) {
        qWarning() << "Failed to get remark name: " << query.lastError().text();
        return "";
    }

    if (query.next()) {
        return query.value("remark_name").toString();
    } else {
        qWarning() << "No contact found for user ID: " << userId;
        return "";
    }
}

bool ContactTable::isContact(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open (isContact)";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM contacts WHERE user_id = ? LIMIT 1");
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Failed to check contact existence: " << query.lastError().text();
        return false;
    }

    return query.next();
}

QList<Contact> ContactTable::searchContacts(const QString& keyword)
{
    QList<Contact> contacts;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return contacts;
    }

    QString condition = buildSearchCondition(keyword);
    QString sql = QString("SELECT * FROM contacts WHERE %1 ORDER BY remark_name").arg(condition);

    QSqlQuery query(m_database);
    if (!query.prepare(sql)) {
        qWarning() << "Prepare search query failed:" << query.lastError().text();
        return contacts;
    }

    // 绑定搜索参数
    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(likePattern); // remark_name
    query.addBindValue(likePattern); // description
    query.addBindValue(likePattern); // phone_note
    query.addBindValue(likePattern); // email_note
    query.addBindValue(likePattern); // source
    query.addBindValue(likePattern); // tags

    if (!query.exec()) {
        qWarning() << "Search contacts failed:" << query.lastError().text();
        return contacts;
    }

    while (query.next()) {
        contacts.append(Contact(query));
    }

    return contacts;
}

bool ContactTable::setContactStarred(qint64 userId, bool starred)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE contacts SET is_starred = ? WHERE user_id = ?");
    query.addBindValue(starred ? 1 : 0);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Set contact starred failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ContactTable::setContactBlocked(qint64 userId, bool blocked)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE contacts SET is_blocked = ? WHERE user_id = ?");
    query.addBindValue(blocked ? 1 : 0);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "Set contact blocked failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Contact> ContactTable::getStarredContacts()
{
    QList<Contact> contacts;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return contacts;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contacts WHERE is_starred = 1 ORDER BY remark_name");

    if (!query.exec()) {
        qWarning() << "Get starred contacts failed:" << query.lastError().text();
        return contacts;
    }

    while (query.next()) {
        contacts.append(Contact(query));
    }

    return contacts;
}

QString ContactTable::buildSearchCondition(const QString& keyword)
{
    return "remark_name LIKE ? OR "
           "description LIKE ? OR "
           "phone_note LIKE ? OR "
           "email_note LIKE ? OR "
           "source LIKE ? OR "
           "tags LIKE ?";
}