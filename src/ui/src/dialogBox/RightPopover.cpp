#include "rightpopover.h"
#include "ui_rightpopover.h"
#include<QDebug>
RightPopover::RightPopover(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RightPopover)
{
    ui->setupUi(this);
}

RightPopover::~RightPopover()
{
    delete ui;
}


void RightPopover::on_pushButton_4_clicked()
{
    qDebug()<<"点击按钮";
}

