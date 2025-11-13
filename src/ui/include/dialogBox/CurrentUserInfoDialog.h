#ifndef CURRENTUSERINFODIALOG_H
#define CURRENTUSERINFODIALOG_H

#include "ClickClosePopup.h"
#include "User.h"
#include <QLabel>

namespace Ui {
class CurrentUserInfoDialog;
}
class ImgLabel;

class CurrentUserInfoDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit CurrentUserInfoDialog(QWidget *parent = nullptr);
    ~CurrentUserInfoDialog();
    void setCurrentUser(const User &user);

    ImgLabel *avatarLabel;
    QLabel *account;
    QLabel *region;
    QLabel *nickname;

signals:
    void switchMessageInterfaceToolButton();

protected:


private slots:
    void on_switchMessageInterfaceToolButton_clicked();

private:
    Ui::CurrentUserInfoDialog *ui;
    User currentUser;

};

#endif // CURRENTUSERINFODIALOG_H
