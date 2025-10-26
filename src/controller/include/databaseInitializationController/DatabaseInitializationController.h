#ifndef DATABASEINITIALIZATIONCONTROLLER_H
#define DATABASEINITIALIZATIONCONTROLLER_H

#include <QObject>
#include <QTimer>
class DatabaseManager;

class DatabaseInitializationController : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseInitializationController(QObject *parent = nullptr);
    ~DatabaseInitializationController();

    void initialize();

signals:
    void initializationProgress(int progress, const QString& message);
    void initializationFinished(bool success, const QString& errorMessage = "");
    void databaseReady();

private slots:
    void performInitialization();
    void onDatabaseInitializationFinished(bool success, const QString& errorMessage);

private:
    void initializeDatabase();
    void loadNetworkData();
    void updateProgress(int progress, const QString& message);

    // 网络数据加载步骤
    void loadCurrentUserInfo();
    void loadContacts();
    void loadGroups();
    void loadGroupMembers();
    void completeInitialization();

private:
    bool m_initializationInProgress;
    int m_currentProgress;
    DatabaseManager *dbManager;
};

#endif // DATABASEINITIALIZATIONCONTROLLER_H
