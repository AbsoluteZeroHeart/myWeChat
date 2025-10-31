#ifndef PERSONALINFODIALOG_H
#define PERSONALINFODIALOG_H

#include "ClickClosePopup.h"
#include <QLabel>

namespace Ui {
class PersonalInfoDialog;
}
class ImgLabel;

class PersonalInfoDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit PersonalInfoDialog(QWidget *parent = nullptr);
    ~PersonalInfoDialog();
    ImgLabel *avatarLabel;
    QLabel *account;
    QLabel *region;
    QLabel *nickname;

protected:


private:
    Ui::PersonalInfoDialog *ui;

};

#endif // PERSONALINFODIALOG_H
