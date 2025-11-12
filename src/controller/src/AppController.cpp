#include "AppController.h"

AppController::AppController(DatabaseManager *databaseManager, QObject *parent)
    : QObject(parent)
{
    m_userController = new UserController(databaseManager, this);
    m_conversationController = new ConversationController(databaseManager, this);
    m_messageController = new MessageController(databaseManager, this);
    m_contactController = new ContactController(databaseManager, this);
}

AppController::~AppController() = default;
