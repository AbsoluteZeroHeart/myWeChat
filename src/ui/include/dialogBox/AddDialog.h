#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include "ClickClosePopup.h"

namespace Ui {
class AddDialog;
}

class AddDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit AddDialog(QWidget *parent = nullptr);
    ~AddDialog();

private:
    Ui::AddDialog *ui;
};

#endif // ADDDIALOG_H
