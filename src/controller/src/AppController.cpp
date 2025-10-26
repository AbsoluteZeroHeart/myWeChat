#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    m_databaseInitializationController = new DatabaseInitializationController(this);
    m_userController = new UserController(this);
    m_conversationController = new ConversationController(this);
    m_messageController = new MessageController(this);
}

AppController::~AppController() = default;
