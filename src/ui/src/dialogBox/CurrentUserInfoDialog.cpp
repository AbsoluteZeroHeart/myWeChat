#include "CurrentUserInfoDialog.h"
#include "MediaResourceManager.h"
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

void CurrentUserInfoDialog::setCurrentUser(const User &user)
{
    currentUser = user;
    MediaResourceManager* mediaManager = MediaResourceManager::instance();

    connect(mediaManager, &MediaResourceManager::mediaLoaded, this,
        [this, mediaManager](const QString& resourcePath, const QPixmap& media, MediaType type){

        QPixmap avatar = mediaManager->getMedia(currentUser.avatarLocalPath,
                                                QSize(500, 500), MediaType::Avatar,0);
        avatarLabel ->setPixmap(avatar);
    });

    QPixmap avatar = mediaManager->getMedia(currentUser.avatarLocalPath,
                                            QSize(500, 500), MediaType::Avatar,0);
    avatarLabel ->setPixmap(avatar);
    account->setText(currentUser.account);
    nickname->setText(currentUser.nickname);
    region->setText(currentUser.region);
}


void CurrentUserInfoDialog::on_switchMessageInterfaceToolButton_clicked()
{
    emit switchMessageInterfaceToolButton();
}

