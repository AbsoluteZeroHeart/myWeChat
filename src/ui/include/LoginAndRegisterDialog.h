#ifndef LOGINANDREGISTERDIALOG_H
#define LOGINANDREGISTERDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QPixmap>


namespace Ui { class LoginAndRegisterDialog; }
class LoginAndRegisterController;


class LoginAndRegisterDialog : public QDialog
{
    Q_OBJECT

    
public:
    explicit LoginAndRegisterDialog(LoginAndRegisterController *loginAndRegisterController, QWidget *parent = nullptr);
    ~LoginAndRegisterDialog() override;


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;


private slots:
    void on_closeToolButton_clicked();
    void on_SMSLoginButton_clicked();
    void on_passwordLoginButton_clicked();
    void on_registerButton_clicked();
    void on_wechatNumButton_clicked();
    void on_registerButton_2_clicked();


    void on_LoginOrRegister_clicked();

    void on_loginButton1_clicked();

    void on_loginButton2_clicked();

private:
    Ui::LoginAndRegisterDialog *ui;
    QPixmap background;
    bool m_isDragging;
    QPoint m_dragStartPosition;

};


#endif // LOGINANDREGISTERDIALOG_H
