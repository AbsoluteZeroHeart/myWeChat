#ifndef CURRENTUSERINFODIALOG_H
#define CURRENTUSERINFODIALOG_H

#include "ClickClosePopup.h"
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
    ImgLabel *avatarLabel;
    QLabel *account;
    QLabel *region;
    QLabel *nickname;

protected:


private:
    Ui::CurrentUserInfoDialog *ui;

};

#endif // CURRENTUSERINFODIALOG_H
