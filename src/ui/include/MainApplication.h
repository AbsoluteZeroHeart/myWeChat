#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>

class SplashDialog;
class WeChatWidget;
class DatabaseInitializationController;  // 前向声明

class MainApplication : public QApplication
{
    Q_OBJECT

public:
    MainApplication(int &argc, char **argv);
    bool initialize();

private slots:
    void onDatabaseInitializationProgress(int progress, const QString& message);
    void onDatabaseInitializationFinished(bool success, const QString& errorMessage);
    void onDatabaseReady();
    void showMainWindow();

private:
    SplashDialog* m_splashDialog;
    WeChatWidget* m_mainWindow;
    DatabaseInitializationController* m_initController;  // 新增成员
};

#endif // MAINAPPLICATION_H
