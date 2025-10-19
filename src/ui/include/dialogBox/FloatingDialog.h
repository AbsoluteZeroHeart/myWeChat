#ifndef FLOATINGDIALOG_H
#define FLOATINGDIALOG_H

#include "ClickClosePopup.h"

namespace Ui {
class FloatingDialog;
}

class FloatingDialog : public ClickClosePopup
{
    Q_OBJECT

public:
    explicit FloatingDialog(QWidget *parent = nullptr);
    ~FloatingDialog();

private:
    Ui::FloatingDialog *ui;
};

#endif // FLOATINGDIALOG_H
