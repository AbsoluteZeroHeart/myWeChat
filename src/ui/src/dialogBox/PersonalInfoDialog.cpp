#include "personalinfodialog.h"
#include "ui_personalinfodialog.h"
#include <QPainter>
#include <QPainterPath>

PersonalInfoDialog::PersonalInfoDialog(QWidget *parent)
    : ClickClosePopup(parent)
    , ui(new Ui::PersonalInfoDialog)
{
    ui->setupUi(this);
}

PersonalInfoDialog::~PersonalInfoDialog()
{
    delete ui;
}
