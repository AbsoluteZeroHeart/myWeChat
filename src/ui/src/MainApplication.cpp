#include "MainApplication.h"
#include "DatabaseManager.h"
#include "SplashDialog.h"
#include "WeChatWidget.h"
#include <QMessageBox>
#include <QTimer>

MainApplication::MainApplication(int &argc, char **argv) 
    : QApplication(argc, argv)
    , m_splashDialog(nullptr)
{
}

bool MainApplication::initialize()
{
    // 创建并显示启动界面
    m_splashDialog = new SplashDialog();
    m_splashDialog->show();
    
    // 获取数据库管理器实例
    DatabaseManager* dbManager = DatabaseManager::getInstance();
    
    // 连接信号
    connect(dbManager, &DatabaseManager::initializationStarted, 
            this, &MainApplication::onDatabaseInitializationStarted);
    connect(dbManager, &DatabaseManager::initializationProgress, 
            this, &MainApplication::onDatabaseInitializationProgress);
    connect(dbManager, &DatabaseManager::initializationFinished, 
            this, &MainApplication::onDatabaseInitializationFinished);
    connect(dbManager, &DatabaseManager::databaseReady, 
            this, &MainApplication::onDatabaseReady);
    
    // 延迟启动数据库初始化，确保界面已经显示
    QTimer::singleShot(200, dbManager, &DatabaseManager::initializeDatabaseAsync);
    
    return true;
}

void MainApplication::onDatabaseInitializationStarted()
{
    qDebug() << "Database initialization started...";
    if (m_splashDialog) {
        m_splashDialog->setProgress(0, "开始初始化数据...");
    }
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
        qDebug() << "Database initialization completed successfully";
        if (m_splashDialog) {
            m_splashDialog->setProgress(100, "数据初始化完成!");
        }
    } else {
        qCritical() << "Database initialization failed:" << errorMessage;
        if (m_splashDialog) {
            m_splashDialog->setProgress(0, "数据初始化失败!");
        }
        // 显示错误信息给用户
        QMessageBox::critical(nullptr, "Database Error", 
                            "Failed to initialize database: " + errorMessage);
        // 错误时退出应用
        quit();
    }
}

void MainApplication::onDatabaseReady()
{
    qDebug() << "Database is ready for use";
    
    // 延迟显示主窗口，让用户看到完成状态
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
    
    // 创建并显示主窗口
    WeChatWidget* mainWindow = new WeChatWidget();
    mainWindow->show();
    
}
