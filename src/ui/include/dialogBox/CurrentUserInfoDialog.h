#ifndef CURRENTUSERINFODIALOG_H
#define CURRENTUSERINFODIALOG_H

#include "ClickClosePopup.h"
#include "Contact.h"
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
    void setCurrentUser(const Contact &user);

    ImgLabel *avatarLabel;
    QLabel *account;
    QLabel *region;
    QLabel *nickname;

signals:
    void switchMessageInterfaceToolButton(const Contact &contact);

protected:


private slots:
    void on_switchMessageInterfaceToolButton_clicked();

private:
    Ui::CurrentUserInfoDialog *ui;
    Contact currentUser;

};

#endif // CURRENTUSERINFODIALOG_H
