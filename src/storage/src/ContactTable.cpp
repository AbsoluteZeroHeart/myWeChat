#include "ContactTable.h"
#include <QJsonDocument>

ContactTable::ContactTable(QSqlDatabase database, QObject *parent)
    : QObject(parent), m_database(database)
{
}

bool ContactTable::saveContact(const QJsonObject& contact)
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

    qint64 userId = contact.value("user_id").toVariant().toLongLong();
    QString remarkName = contact.value("remark_name").toString();
    QString description = contact.value("description").toString();

    // 处理 tags 字段
    QJsonArray tagsArray = contact.value("tags").toArray();
    QString tagsString = QJsonDocument(tagsArray).toJson(QJsonDocument::Compact);

    QString phoneNote = contact.value("phone_note").toString();
    QString emailNote = contact.value("email_note").toString();
    QString source = contact.value("source").toString();
    int isStarred = contact.value("is_starred").toInt(0);
    int isBlocked = contact.value("is_blocked").toInt(0);

    qint64 currentTime = QDateTime::currentSecsSinceEpoch();

    qint64 addTime = currentTime;
    if (contact.contains("add_time") && !contact.value("add_time").isUndefined()) {
        addTime = contact.value("add_time").toVariant().toLongLong();
    }

    qint64 lastContactTime = currentTime;
    if (contact.contains("last_contact_time") && !contact.value("last_contact_time").isUndefined()) {
        lastContactTime = contact.value("last_contact_time").toVariant().toLongLong();
    }

    query.addBindValue(userId);
    query.addBindValue(remarkName);
    query.addBindValue(description);
    query.addBindValue(tagsString);
    query.addBindValue(phoneNote);
    query.addBindValue(emailNote);
    query.addBindValue(source);
    query.addBindValue(isStarred);
    query.addBindValue(isBlocked);
    query.addBindValue(addTime);
    query.addBindValue(lastContactTime);

    if (!query.exec()) {
        qWarning() << "Save contact failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 定义辅助宏
#define ADD_FIELD_IF_EXISTS(field, jsonField) \
if (contact.contains(#jsonField)) { \
        updateFields << #field " = ?"; \
        bindValues << contact.value(#jsonField).toVariant(); \
}

bool ContactTable::updateContact(const QJsonObject& contact)
{
    if (!m_database.isOpen()) return false;
    if (!contact.contains("user_id")) return false;

    qint64 userId = contact.value("user_id").toVariant().toLongLong();
    QStringList updateFields;
    QVariantList bindValues;

    // 使用宏添加字段
    ADD_FIELD_IF_EXISTS(remark_name, remark_name)
    ADD_FIELD_IF_EXISTS(description, description)
    ADD_FIELD_IF_EXISTS(phone_note, phone_note)
    ADD_FIELD_IF_EXISTS(email_note, email_note)
    ADD_FIELD_IF_EXISTS(source, source)
    ADD_FIELD_IF_EXISTS(is_starred, is_starred)
    ADD_FIELD_IF_EXISTS(is_blocked, is_blocked)
    ADD_FIELD_IF_EXISTS(add_time, add_time)
    ADD_FIELD_IF_EXISTS(last_contact_time, last_contact_time)

    // 特殊处理 tags 字段
    if (contact.contains("tags")) {
        updateFields << "tags = ?";
        QJsonArray tagsArray = contact.value("tags").toArray();
        QString tagsString = QJsonDocument(tagsArray).toJson(QJsonDocument::Compact);
        bindValues << tagsString;
    }

    if (updateFields.isEmpty()) return false;

    QString sql = "UPDATE contacts SET " + updateFields.join(", ") + " WHERE user_id = ?";
    bindValues << userId;

    QSqlQuery query(m_database);
    query.prepare(sql);
    for (const QVariant& value : std::as_const(bindValues)) {
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

QJsonArray ContactTable::getAllContacts()
{
    QJsonArray contacts;

    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return contacts;
    }

    QSqlQuery query("SELECT * FROM contacts ORDER BY remark_name", m_database);

    while (query.next()) {
        contacts.append(contactFromQuery(query));
    }

    return contacts;
}

QJsonObject ContactTable::getContact(qint64 userId)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database is not open";
        return QJsonObject();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        return QJsonObject();
    }

    return contactFromQuery(query);
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

QJsonArray ContactTable::searchContacts(const QString& keyword)
{
    QJsonArray contacts;

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
        contacts.append(contactFromQuery(query));
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


QJsonArray ContactTable::getStarredContacts()
{
    QJsonArray contacts;

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
        contacts.append(contactFromQuery(query));
    }

    return contacts;
}

// 私有辅助方法
QJsonObject ContactTable::contactFromQuery(const QSqlQuery& query)
{
    QJsonObject contact;

    contact["user_id"] = query.value("user_id").toLongLong();
    contact["remark_name"] = query.value("remark_name").toString();
    contact["description"] = query.value("description").toString();

    // 解析JSON格式的tags
    QString tagsJson = query.value("tags").toString();
    if (!tagsJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(tagsJson.toUtf8());
        if (doc.isArray()) {
            contact["tags"] = doc.array();
        } else {
            contact["tags"] = QJsonArray();
        }
    } else {
        contact["tags"] = QJsonArray();
    }

    contact["phone_note"] = query.value("phone_note").toString();
    contact["email_note"] = query.value("email_note").toString();
    contact["source"] = query.value("source").toString();
    contact["is_starred"] = query.value("is_starred").toInt();
    contact["is_blocked"] = query.value("is_blocked").toInt();
    contact["add_time"] = query.value("add_time").toLongLong();
    contact["last_contact_time"] = query.value("last_contact_time").toLongLong();

    return contact;
}

QString ContactTable::buildSearchCondition(const QString& keyword)
{
    // 在多个字段中搜索关键词
    return "remark_name LIKE ? OR "
           "description LIKE ? OR "
           "phone_note LIKE ? OR "
           "email_note LIKE ? OR "
           "source LIKE ? OR "
           "tags LIKE ?"; // tags是JSON，但也支持搜索
}
