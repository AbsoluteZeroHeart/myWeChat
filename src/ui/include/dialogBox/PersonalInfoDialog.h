#ifndef PERSONALINFODIALOG_H
#define PERSONALINFODIALOG_H

#include "ClickClosePopup.h"

namespace Ui {
class PersonalInfoDialog;
}

class PersonalInfoDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit PersonalInfoDialog(QWidget *parent = nullptr);
    ~PersonalInfoDialog();


protected:


private:
    Ui::PersonalInfoDialog *ui;
};

#endif // PERSONALINFODIALOG_H
