#ifndef APPINITIALIZE_H
#define APPINITIALIZE_H

#include <QObject>


class SplashDialog;
class WeChatWidget;
class DatabaseInitializationController;

class AppInitialize : public QObject
{
    Q_OBJECT

public:
    AppInitialize(DatabaseInitializationController* initController,
                  QObject *parent = nullptr);

    bool initialize();

signals:
    void isInited();

private slots:
    void onDatabaseInitializationProgress(int progress, const QString& message);
    void onDatabaseInitializationFinished(bool success, const QString& errorMessage);
    void onDatabaseReady();

private:
    SplashDialog* m_splashDialog;
    DatabaseInitializationController* m_initController;  // 新增成员
};

#endif // APPINITIALIZE_H
