#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QThread>

namespace Ui {
class TestWidget;
}

class GenerationWorker;
class AppController;
class MessageController;

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestWidget(AppController *appController, QWidget *parent = nullptr);
    ~TestWidget();

private slots:
    void on_pushButton1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    // 处理工作线程信号
    void onProgressChanged(int current, int total, const QString& message, int type);
    void onNonFriendsGenerated(bool success, const QString& message);
    void onFriendsGenerated(bool success, const QString& message);
    void onErrorOccurred(const QString& error);

    // 工作线程完成
    void onWorkerFinished();

private:
    Ui::TestWidget *ui;

    // 工作线程和工作者
    QThread* m_workerThread = nullptr;
    GenerationWorker* m_worker = nullptr;

    MessageController *m_messageController;

    // 状态标志
    bool m_isGenerating = false;

    // 更新UI状态
    void setGeneratingState(bool generating);

    // 创建新的工作线程
    void createWorkerThread();
};

#endif // TESTWIDGET_H
