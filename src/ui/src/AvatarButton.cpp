#include "avatarButton.h"

AvatarButton::AvatarButton(QWidget* parent)
    : QPushButton(parent)
{
    setFixedSize(m_buttonSize, m_buttonSize);

    // 设置鼠标悬停时显示手型光标
    setCursor(Qt::PointingHandCursor);

    QString style = QString(
                        "QPushButton {"
                        "   border: 1px solid rgb(220,220,220);"
                        "   border-radius:6px;"
                        "   background-color: transparent;"
                        "   padding: 0px;"
                        "}");

    setStyleSheet(style);
    setAvatar(":/a/image/avatar.jpg");
}

void AvatarButton::setAvatar(const QString& imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        int radius = pixmap.size().width()/8;
        QPixmap roundedPixmap = getRoundedPixmap(pixmap, pixmap.size().width(), radius);
        setIcon(QIcon(roundedPixmap));
        setIconSize(QSize(m_buttonSize, m_buttonSize));
    } else {
        // 如果图片加载失败，清除图标
        setIcon(QIcon());
    }
}

QPixmap AvatarButton::getRoundedPixmap(const QPixmap& pixmap, int size, int radius) const
{
    // 创建圆角矩形头像
    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);

    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
    QPainterPath path;
    path.addRoundedRect(QRectF(0,0, size,size), radius,radius);
    painter.setClipPath(path);

    painter.drawPixmap(0,0, pixmap.scaled(size,size,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));

    return rounded;
}

void AvatarButton::setButtonSize(int size)
{
    m_buttonSize = size;
    setIconSize(QSize(m_buttonSize, m_buttonSize));
    setFixedSize(m_buttonSize, m_buttonSize);
}

