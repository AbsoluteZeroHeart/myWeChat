#include "moredialog.h"
#include "ui_moredialog.h"

MoreDialog::MoreDialog(QWidget *parent)
    : ClickClosePopup(parent)
    , ui(new Ui::MoreDialog)
{
    ui->setupUi(this);
}

MoreDialog::~MoreDialog()
{
    delete ui;
}
