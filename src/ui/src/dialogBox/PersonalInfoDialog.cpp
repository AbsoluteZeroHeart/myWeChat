#include "personalinfodialog.h"
#include "ui_personalinfodialog.h"
#include <QPainter>
#include <QPainterPath>

PersonalInfoDialog::PersonalInfoDialog(QWidget *parent)
    : ClickClosePopup(parent)
    , ui(new Ui::PersonalInfoDialog)
{
    ui->setupUi(this);
    avatarLabel = ui->avatarLabel;
    account = ui->account;
    region = ui->region;
    nickname = ui->nickname;
}

PersonalInfoDialog::~PersonalInfoDialog()
{
    delete ui;
}
