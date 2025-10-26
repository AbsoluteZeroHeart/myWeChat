#include "MainApplication.h"
#include "DatabaseInitializationController.h"
#include "SplashDialog.h"
#include "WeChatWidget.h"
#include <QMessageBox>
#include <QTimer>

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_splashDialog(nullptr)
    , m_initController(nullptr)
{
}

bool MainApplication::initialize()
{
    // 创建并显示启动界面
    m_splashDialog = new SplashDialog();
    m_splashDialog->show();

    // 创建数据库初始化控制器
    m_initController = new DatabaseInitializationController(this);

    connect(m_initController, &DatabaseInitializationController::initializationProgress,
            this, &MainApplication::onDatabaseInitializationProgress);
    connect(m_initController, &DatabaseInitializationController::initializationFinished,
            this, &MainApplication::onDatabaseInitializationFinished);
    connect(m_initController, &DatabaseInitializationController::databaseReady,
            this, &MainApplication::onDatabaseReady);

    // 启动数据库初始化
    QTimer::singleShot(200, m_initController, &DatabaseInitializationController::initialize);

    return true;
}

void MainApplication::onDatabaseInitializationProgress(int progress, const QString& message)
{
    qDebug() << "Database initialization progress:" << progress << "% -" << message;
    if (m_splashDialog) {
        m_splashDialog->setProgress(progress, message);
    }
}

void MainApplication::onDatabaseInitializationFinished(bool success, const QString& errorMessage)
{
    if (success) {
        if (m_splashDialog) {
            m_splashDialog->setProgress(100, "数据初始化完成!");
        }
    } else {
        if (m_splashDialog) {
            m_splashDialog->setProgress(0, "数据初始化失败!");
        }
        QMessageBox::critical(nullptr, "Database Error",
                              "Failed to initialize database: " + errorMessage);
        quit();
    }
}

void MainApplication::onDatabaseReady()
{
    if (m_splashDialog) {
        m_splashDialog->setProgress(100, "应用程序准备就绪!");
        QTimer::singleShot(500, this, &MainApplication::showMainWindow);
    } else {
        showMainWindow();
    }
}

void MainApplication::showMainWindow()
{
    // 隐藏启动界面
    if (m_splashDialog) {
        m_splashDialog->hide();
        m_splashDialog->deleteLater();
        m_splashDialog = nullptr;
    }

    // 清理初始化控制器
    if (m_initController) {
        m_initController->deleteLater();
        m_initController = nullptr;
    }
    // 创建并显示主窗口
    WeChatWidget* mainWindow = new WeChatWidget();
    mainWindow->show();
}
