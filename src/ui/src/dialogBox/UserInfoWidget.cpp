#include "UserInfoWidget.h"
#include "MediaResourceManager.h"
#include "ui_UserInfoWidget.h"

UserInfoWidget::UserInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserInfoWidget)
{
    ui->setupUi(this);

    avatarLabel = ui->avatarLabel;
    remarkNameLable = ui->remarkNameLable;
    nicknameLable = ui->nicknameLable;
    accountLable = ui->accountLable;
    regionLable = ui->regionLable;
    remarkNameButton = ui->remarkNameButton;
    tagButton = ui->tagButton;
    signatureLabel = ui->signatureLabel;
    mutualgroupLable = ui->mutualgroupLable;
    sourceLabel = ui->sourceLabel;
}

UserInfoWidget::~UserInfoWidget()
{
    delete ui;
}


void UserInfoWidget::setSelectedContact(const Contact &contact)
{
    m_contact = contact;

    MediaResourceManager* mediaManager = MediaResourceManager::instance();
    connect(mediaManager, &MediaResourceManager::mediaLoaded, this,
        [this, mediaManager](const QString& resourcePath, const QPixmap& media, MediaType type){
        QPixmap avatar = mediaManager->getMedia(m_contact.user.avatarLocalPath,
                                                QSize(500, 500), MediaType::Avatar,0);
        avatarLabel ->setPixmap(avatar);
    });
    QPixmap avatar = mediaManager->getMedia(m_contact.user.avatarLocalPath,
                                            QSize(500, 500), MediaType::Avatar,0);
    avatarLabel ->setPixmap(avatar);

    remarkNameLable->setText(m_contact.remarkName);
    nicknameLable->setText(m_contact.user.nickname);
    accountLable->setText(m_contact.user.account);
    regionLable->setText(m_contact.user.region);
    remarkNameButton->setText(m_contact.remarkName);
    tagButton->setText(m_contact.getTagsString());
    signatureLabel->setText(m_contact.user.signature);
    mutualgroupLable->setText("9");// 忘记实现了，随便写个数字，有空再搞。
    sourceLabel->setText(m_contact.source);
}

void UserInfoWidget::on_switchMessageInterfaceToolButton_clicked()
{
    emit switchtoMessageInterface(m_contact);
}

