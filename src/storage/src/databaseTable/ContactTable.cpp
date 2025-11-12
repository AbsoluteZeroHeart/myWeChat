#include "ContactTable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include "DbConnectionManager.h"
#include <QDebug>

ContactTable::ContactTable(QObject *parent)
    : QObject(parent)
{
}

ContactTable::~ContactTable()
{
}

void ContactTable::init()
{
    m_database = DbConnectionManager::connectionForCurrentThread();
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        QString errorText = m_database ? m_database->lastError().text() : "Failed to get database connection";
        emit dbError(-1, QString("Open DB failed: %1").arg(errorText));
        return;
    }
}

QString ContactTable::tagsToString(const QJsonArray &tags) const {
    return QString::fromUtf8(QJsonDocument(tags).toJson(QJsonDocument::Compact));
}

void ContactTable::saveContact(int reqId, Contact contact)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit contactSaved(reqId, false, "Database is not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("INSERT INTO contacts ("
                  "user_id, remark_name, description, tags, phone_note, "
                  "email_note, source, is_starred, is_blocked, add_time"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    QString tagsString = tagsToString(contact.tags);

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

    if (!query.exec()) {
        emit contactSaved(reqId, false, query.lastError().text());
        return;
    }

    emit contactSaved(reqId, true, QString());
}

void ContactTable::updateContact(int reqId, Contact contact)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit contactUpdated(reqId, false, "Database not open");
        return;
    }
    if (!contact.isValid()) {
        emit contactUpdated(reqId, false, "Invalid contact");
        return;
    }

    QStringList updateFields;
    QVariantList bindValues;

    if (!contact.remarkName.isNull()) { updateFields << "remark_name = ?"; bindValues << contact.remarkName; }
    if (!contact.description.isNull())  { updateFields << "description = ?";  bindValues << contact.description; }
    if (!contact.phoneNote.isNull())    { updateFields << "phone_note = ?";   bindValues << contact.phoneNote; }
    if (!contact.emailNote.isNull())    { updateFields << "email_note = ?";   bindValues << contact.emailNote; }
    if (!contact.source.isNull())       { updateFields << "source = ?";       bindValues << contact.source; }

    updateFields << "is_starred = ?"; bindValues << (contact.isStarred ? 1 : 0);
    updateFields << "is_blocked = ?"; bindValues << (contact.isBlocked ? 1 : 0);

    if (contact.addTime > 0) { updateFields << "add_time = ?"; bindValues << contact.addTime; }

    if (!contact.tags.isEmpty()) {
        updateFields << "tags = ?";
        bindValues << tagsToString(contact.tags);
    }

    if (updateFields.isEmpty()) {
        emit contactUpdated(reqId, false, "No fields to update");
        return;
    }

    QString sql = "UPDATE contacts SET " + updateFields.join(", ") + " WHERE user_id = ?";
    bindValues << contact.userId;

    QSqlQuery query(*m_database);
    query.prepare(sql);
    for (const QVariant &v : std::as_const(bindValues)) query.addBindValue(v);

    if (!query.exec()) {
        emit contactUpdated(reqId, false, query.lastError().text());
        return;
    }
    bool ok = query.numRowsAffected() > 0;
    emit contactUpdated(reqId, ok, ok ? QString() : "No rows affected");
}

void ContactTable::deleteContact(int reqId, qint64 userId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit contactDeleted(reqId, false, "Database not open");
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("DELETE FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec()) {
        emit contactDeleted(reqId, false, query.lastError().text());
        return;
    }

    emit contactDeleted(reqId, query.numRowsAffected() > 0, QString());
}

void ContactTable::getAllContacts(int reqId)
{
    QList<Contact> contacts;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit allContactsLoaded(reqId, contacts);
        return;
    }

    QSqlQuery query(*m_database);
    if (!query.exec("SELECT * FROM contacts ORDER BY remark_name")) {
        emit dbError(reqId, query.lastError().text());
        emit allContactsLoaded(reqId, contacts);
        return;
    }

    while (query.next()){
        Contact contact = Contact(query);

        QSqlQuery q(*m_database);
        q.prepare("SELECT * FROM users WHERE user_id = ?");
        q.addBindValue(contact.userId);

        if (!q.exec() || !q.next()) {
            continue; // 跳过这个联系人，继续下一个
        }
        contact.user = User(q);
        contacts.append(contact);
    }
    emit allContactsLoaded(reqId, contacts);
}

void ContactTable::getContact(int reqId, qint64 userId)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit contactLoaded(reqId, Contact());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("SELECT * FROM contacts WHERE user_id = ?");
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        emit contactLoaded(reqId, Contact());
        return;
    }
    Contact contact = Contact(query);

    QSqlQuery q(*m_database);
    q.prepare("SELECT * FROM users WHERE user_id = ?");
    q.addBindValue(contact.userId);

    if (!q.exec() || !q.next()) {
        emit contactLoaded(reqId, Contact());
        return;
    }
    contact.user = User(q);

    emit contactLoaded(reqId, contact);
}

void ContactTable::searchContacts(int reqId, const QString &keyword)
{
    QList<Contact> contacts;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit searchContactsResult(reqId, contacts);
        return;
    }

    QString condition = buildSearchCondition(keyword);
    QString sql = QString("SELECT * FROM contacts WHERE %1 ORDER BY remark_name").arg(condition);

    QSqlQuery query(*m_database);
    if (!query.prepare(sql)) {
        emit dbError(reqId, query.lastError().text());
        emit searchContactsResult(reqId, contacts);
        return;
    }

    QString likePattern = QString("%%1%").arg(keyword);
    query.addBindValue(likePattern); // remark_name
    query.addBindValue(likePattern); // description
    query.addBindValue(likePattern); // phone_note
    query.addBindValue(likePattern); // email_note
    query.addBindValue(likePattern); // source
    query.addBindValue(likePattern); // tags

    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit searchContactsResult(reqId, contacts);
        return;
    }

    while (query.next()) {
        Contact contact = Contact(query);

        QSqlQuery q(*m_database);
        q.prepare("SELECT * FROM users WHERE user_id = ?");
        q.addBindValue(contact.userId);

        if (!q.exec() || !q.next()) {
            continue; // 跳过这个联系人，继续下一个
        }
        contact.user = User(q);
        contacts.append(contact);
    }
    emit searchContactsResult(reqId, contacts);
}

void ContactTable::setContactStarred(int reqId, qint64 userId, bool starred)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit contactStarredSet(reqId, false);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE contacts SET is_starred = ? WHERE user_id = ?");
    query.addBindValue(starred ? 1 : 0);
    query.addBindValue(userId);
    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit contactStarredSet(reqId, false);
        return;
    }
    emit contactStarredSet(reqId, query.numRowsAffected() > 0);
}

void ContactTable::setContactBlocked(int reqId, qint64 userId, bool blocked)
{
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit contactBlockedSet(reqId, false);
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("UPDATE contacts SET is_blocked = ? WHERE user_id = ?");
    query.addBindValue(blocked ? 1 : 0);
    query.addBindValue(userId);
    if (!query.exec()) {
        emit dbError(reqId, query.lastError().text());
        emit contactBlockedSet(reqId, false);
        return;
    }
    emit contactBlockedSet(reqId, query.numRowsAffected() > 0);
}

void ContactTable::getStarredContacts(int reqId)
{
    QList<Contact> contacts;
    if (!m_database || !m_database->isValid() || !m_database->isOpen()) {
        emit dbError(reqId, "Database not open");
        emit starredContactsLoaded(reqId, contacts);
        return;
    }

    QSqlQuery query(*m_database);
    if (!query.exec("SELECT * FROM contacts WHERE is_starred = 1 ORDER BY remark_name")) {
        emit dbError(reqId, query.lastError().text());
        emit starredContactsLoaded(reqId, contacts);
        return;
    }

    while (query.next()) {
        Contact contact = Contact(query);

        QSqlQuery q(*m_database);
        q.prepare("SELECT * FROM users WHERE user_id = ?");
        q.addBindValue(contact.userId);

        if (!q.exec() || !q.next()) {
            continue; // 跳过这个联系人，继续下一个
        }
        contact.user = User(q);
        contacts.append(contact);
    }
    emit starredContactsLoaded(reqId, contacts);
}

QString ContactTable::buildSearchCondition(const QString &keyword) const
{
    Q_UNUSED(keyword);
    return "remark_name LIKE ? OR description LIKE ? OR phone_note LIKE ? OR email_note LIKE ? OR source LIKE ? OR tags LIKE ?";
}
