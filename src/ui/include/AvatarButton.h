#ifndef AVATARBUTTON_H
#define AVATARBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

class AvatarButton : public QPushButton
{
    Q_OBJECT

public:

    // 构造函数
    explicit AvatarButton(QWidget* parent = nullptr);

    // 设置头像图片
    void setAvatar(const QString& imagePath);

    void setButtonSize(int size);

private:

    // 按钮尺寸
    int m_buttonSize = 40;

    // 创建圆角头像
    QPixmap getRoundedPixmap(const QPixmap& pixmap, int size, int radius) const;
};

#endif // AVATARBUTTON_H
