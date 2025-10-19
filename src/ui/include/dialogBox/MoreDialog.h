#ifndef MOREDIALOG_H
#define MOREDIALOG_H

#include "ClickClosePopup.h"

namespace Ui {
class MoreDialog;
}

class MoreDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit MoreDialog(QWidget *parent = nullptr);
    ~MoreDialog();

private:
    Ui::MoreDialog *ui;
};

#endif // MOREDIALOG_H
