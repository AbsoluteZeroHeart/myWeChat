#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>

class SplashDialog;

class MainApplication : public QApplication
{
    Q_OBJECT
    
public:
    MainApplication(int &argc, char **argv);
    bool initialize();
    
private slots:
    void onDatabaseInitializationStarted();
    void onDatabaseInitializationProgress(int progress, const QString& message);
    void onDatabaseInitializationFinished(bool success, const QString& errorMessage);
    void onDatabaseReady();
    void showMainWindow();
    
private:
    SplashDialog *m_splashDialog;
};

#endif // MAINAPPLICATION_H
