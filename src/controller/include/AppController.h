#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "ConversationController.h"
#include "DatabaseInitializationController.h"
#include "MessageController.h"
#include "UserController.h"
/**
 * @brief 应用程序控制器：封装一组表访问对象。
 */
class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();
    UserController *userController() const { return m_userController; }
    ConversationController *conversationController() const { return m_conversationController; }
    MessageController *messageController() const { return m_messageController; }
    DatabaseInitializationController *databaseInitializationController() const { return m_databaseInitializationController; }

private:
    UserController *m_userController = nullptr;
    ConversationController *m_conversationController = nullptr;
    MessageController *m_messageController = nullptr;
    DatabaseInitializationController *m_databaseInitializationController = nullptr;
};

#endif // APPCONTROLLER_H
