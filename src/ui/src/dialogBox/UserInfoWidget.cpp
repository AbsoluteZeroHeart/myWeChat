#include "UserInfoWidget.h"
#include "ui_UserInfoWidget.h"

UserInfoWidget::UserInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserInfoWidget)
{
    ui->setupUi(this);
}

UserInfoWidget::~UserInfoWidget()
{
    delete ui;
}
