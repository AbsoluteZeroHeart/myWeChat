#include "CurrentUserInfoDialog.h"
#include "ThumbnailResourceManager.h"
#include "ui_CurrentUserInfoDialog.h"
#include <QPainter>
#include <QPainterPath>

CurrentUserInfoDialog::CurrentUserInfoDialog(QWidget *parent)
    : ClickClosePopup(parent)
    , ui(new Ui::CurrentUserInfoDialog)
{
    ui->setupUi(this);
    avatarLabel = ui->avatarLabel;
    account = ui->account;
    region = ui->region;
    nickname = ui->nickname;
}

CurrentUserInfoDialog::~CurrentUserInfoDialog()
{
    delete ui;
}

void CurrentUserInfoDialog::setCurrentUser(const Contact &user)
{
    currentUser = user;
    ThumbnailResourceManager* thumbnailManager = ThumbnailResourceManager::instance();

    connect(thumbnailManager, &ThumbnailResourceManager::mediaLoaded, this,
        [this, thumbnailManager](const QString& resourcePath, const QPixmap& media, MediaType type){

        QPixmap avatar = thumbnailManager->getThumbnail(currentUser.user.avatarLocalPath,
                                                QSize(500, 500), MediaType::Avatar,0);
        avatarLabel ->setPixmap(avatar);
    });

    QPixmap avatar = thumbnailManager->getThumbnail(currentUser.user.avatarLocalPath,
                                            QSize(500, 500), MediaType::Avatar,0);
    avatarLabel ->setPixmap(avatar);
    account->setText(currentUser.user.account);
    nickname->setText(currentUser.user.nickname);
    region->setText(currentUser.user.region);
}


void CurrentUserInfoDialog::on_switchMessageInterfaceToolButton_clicked()
{
    emit switchMessageInterfaceToolButton(currentUser);
}

