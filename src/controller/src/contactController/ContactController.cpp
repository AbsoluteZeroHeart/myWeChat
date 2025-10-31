#include "ContactController.h"
#include "DataAccessContext.h"
#include <QDebug>

ContactController::ContactController(QObject *parent)
    : QObject(parent)
    , m_dataAccessContext(new DataAccessContext(this))
{
}

ContactController::~ContactController()
{
}

bool ContactController::addContact(const Contact& contact)
{
    if (!contact.isValid()) {
        qWarning() << "Invalid contact data";
        return false;
    }

    if (m_dataAccessContext->contactTable()->saveContact(contact)) {
        emit contactAdded(contact);
        emit contactsChanged();
        return true;
    }
    
    return false;
}

bool ContactController::updateContact(const Contact& contact)
{
    if (!contact.isValid()) {
        qWarning() << "Invalid contact data";
        return false;
    }

    if (m_dataAccessContext->contactTable()->updateContact(contact)) {
        emit contactUpdated(contact);
        emit contactsChanged();
        return true;
    }
    
    return false;
}

bool ContactController::deleteContact(qint64 userId)
{
    if (m_dataAccessContext->contactTable()->deleteContact(userId)) {
        emit contactDeleted(userId);
        emit contactsChanged();
        return true;
    }
    
    return false;
}

Contact ContactController::getContact(qint64 userId)
{
    return m_dataAccessContext->contactTable()->getContact(userId);
}

QList<Contact> ContactController::getAllContacts()
{
    return m_dataAccessContext->contactTable()->getAllContacts();
}

bool ContactController::isContact(qint64 userId)
{
    return m_dataAccessContext->contactTable()->isContact(userId);
}

QList<Contact> ContactController::searchContacts(const QString& keyword)
{
    return m_dataAccessContext->contactTable()->searchContacts(keyword);
}

bool ContactController::setContactStarred(qint64 userId, bool starred)
{
    if (m_dataAccessContext->contactTable()->setContactStarred(userId, starred)) {
        emit contactStarredChanged(userId, starred);
        emit contactsChanged();
        return true;
    }
    
    return false;
}

bool ContactController::setContactBlocked(qint64 userId, bool blocked)
{
    if (m_dataAccessContext->contactTable()->setContactBlocked(userId, blocked)) {
        emit contactBlockedChanged(userId, blocked);
        emit contactsChanged();
        return true;
    }
    
    return false;
}

QList<Contact> ContactController::getStarredContacts()
{
    return m_dataAccessContext->contactTable()->getStarredContacts();
}

QString ContactController::getRemarkName(qint64 userId)
{
    return m_dataAccessContext->contactTable()->getRemarkName(userId);
}

bool ContactController::updateRemarkName(qint64 userId, const QString& remarkName)
{
    Contact contact = getContact(userId);
    if (!contact.isValid()) {
        return false;
    }
    
    contact.remarkName = remarkName;
    return updateContact(contact);
}

bool ContactController::updateDescription(qint64 userId, const QString& description)
{
    Contact contact = getContact(userId);
    if (!contact.isValid()) {
        return false;
    }
    
    contact.description = description;
    return updateContact(contact);
}

bool ContactController::addTag(qint64 userId, const QString& tag)
{
    Contact contact = getContact(userId);
    if (!contact.isValid()) {
        return false;
    }
    
    contact.addTag(tag);
    return updateContact(contact);
}

bool ContactController::removeTag(qint64 userId, const QString& tag)
{
    Contact contact = getContact(userId);
    if (!contact.isValid()) {
        return false;
    }
    
    contact.removeTag(tag);
    return updateContact(contact);
}

bool ContactController::updateLastContactTime(qint64 userId)
{
    Contact contact = getContact(userId);
    if (!contact.isValid()) {
        return false;
    }
    
    contact.updateLastContactTime();
    return updateContact(contact);
}