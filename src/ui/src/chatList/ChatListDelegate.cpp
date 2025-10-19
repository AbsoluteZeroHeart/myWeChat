#include "ChatListDelegate.h"
#include "Chatlistmodel.h"
#include <QPainter>
#include <QDateTime>
#include <QPainterPath>
#include <QSize>
#include "formatTime.h"

ChatListDelegate::ChatListDelegate(QObject *parent) :QStyledItemDelegate(parent) {}

void ChatListDelegate::paint(QPainter *painter, const QStyleOptionViewItem
                                                    &option, const QModelIndex &index )const
{
    painter->save();

    const int topBottomMargin = 15;
    const int leftRightMargin = 12;
    const int avatarSize = 40;
    const int spacing = 12;

    QRect rect = option.rect;

    //背景：选中灰色，悬停浅灰
    if(option.state & QStyle::State_Selected){
        painter->fillRect(rect, QColor(226,226,226));
    }else if(option.state & QStyle::State_MouseOver){
        painter->fillRect(rect, QColor(238,238,238));
    }

    //从模型获取数据
    QString name = index.data(TitleRole).toString();
    QString lastMsg = index.data(LastMsgRole).toString();
    QDateTime lastTime = index.data(LastTimeRole).toDateTime();
    int unreadCount = index.data(UnreadCountRole).toInt();
    QString avatar = index.data(AvatarRole).toString();

    //布局计算
    QRect avatarRect(rect.left()+leftRightMargin, rect.center().y()-avatarSize/2, avatarSize, avatarSize);

    QString timeText = FormatTime(lastTime);
    QFont timeFont = option.font;
    timeFont.setPointSizeF(7.5);
    timeFont.setFamily(QStringLiteral("微软雅黑"));
    QFontMetrics timeFm(timeFont);
    int timeW = timeFm.horizontalAdvance(timeText);
    QRect timeRect(rect.right()-leftRightMargin-timeW, rect.top()+topBottomMargin, timeW, timeFm.height());

    int left = avatarRect.right()+spacing;
    int right = timeRect.left()-spacing;
    QRect nameRect(left, rect.top()+topBottomMargin, right-left, option.fontMetrics.height());
    QRect msgRect(left,rect.bottom()-topBottomMargin-option.fontMetrics.height(),right-left,option.fontMetrics.height());

    //画-name
    QFont nameFont = option.font;
    nameFont.setPointSizeF(10.5);
    nameFont.setFamily(QStringLiteral("微软雅黑"));
    painter->setFont(nameFont);
    painter->setPen(option.palette.text().color());
    QString elidedName = QFontMetrics(nameFont).elidedText(name,Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, Qt::AlignLeft|Qt::AlignVCenter, elidedName);

    //画-lastMsg
    QFont msgFont = option.font;
    msgFont.setPointSizeF(8);
    msgFont.setFamily(QStringLiteral("微软雅黑"));
    painter->setFont(msgFont);
    painter->setPen(QColor(150,150,150));
    QString elidedMsg = QFontMetrics(msgFont).elidedText(lastMsg, Qt::ElideRight, msgRect.width());
    painter->drawText(msgRect, Qt::AlignLeft|Qt::AlignVCenter,elidedMsg);

    // 画-lastTime
    painter->setFont(timeFont);
    painter->setPen(QColor(150,150,150));
    painter->drawText(timeRect, Qt::AlignLeft|Qt::AlignVCenter,timeText);

    //画-头像
    painter->setRenderHint(QPainter::Antialiasing,true);
    const int radius = 5;//圆角
    QImage avatarImg;
    if(!avatar.isEmpty() && avatarImg.load(avatar)){
        //使用Qpixmap绘制圆角图
        QPixmap pix = QPixmap::fromImage(avatarImg);
        QPixmap rounded(avatarSize,avatarSize);
        rounded.fill(Qt::transparent);
        {
            QPainter p(&rounded);
            p.setRenderHint(QPainter::Antialiasing,true);
            QPainterPath path;
            path.addRoundedRect(QRectF(0,0, avatarSize,avatarSize), radius,radius);
            p.setClipPath(path);
            p.drawPixmap(0,0, pix.scaled(avatarSize,avatarSize,
                                           Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation));
        }
        painter->drawPixmap(avatarRect,rounded);
    }
    else{
        //加载失败：绘制圆角背景并首字母
        QPainterPath path;
        path.addRoundedRect(QRectF(avatarRect), radius, radius);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(210,210,210));
        painter->drawPath(path);

        QFont iniFont = option.font;
        iniFont.setBold(true);
        iniFont.setPointSize(15);
        painter->setFont(iniFont);
        painter->setPen(QColor(100,100,100));
        painter->drawText(avatarRect, Qt::AlignCenter, name.left(1));
    }

    //画未读
    if(unreadCount>0){
        QString badgetText = (unreadCount>99)? QStringLiteral("99+") : QString::number(unreadCount);
        QFont badgetFont = option.font;
        badgetFont.setBold(true);
        badgetFont.setPointSizeF(7.5);

        QFontMetrics bf(badgetFont);
        int textW = bf.horizontalAdvance(badgetText);
        int textH = bf.height();

        //内边距
        const int hPadding = 3;
        const int vpadding = 2;

        int badgeW = textW + hPadding*2;
        int badgeH = textH + vpadding*2;

        if(badgeW<badgeH)badgeW = badgeH;

        int bx = avatarRect.right()-badgeW/2;
        int by = avatarRect.top()-badgeH/4;

        QRect badgeRect(bx,by, badgeW, badgeH);

        //pill背景
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(249,81,81));
        painter->drawRoundedRect(badgeRect, badgeH/2.0, badgeH/2.0);

        //绘制文字
        painter->setFont(badgetFont);
        painter->setPen(Qt::white);
        painter->drawText(badgeRect, Qt::AlignCenter, badgetText);
    }
    painter->restore();
}

QSize ChatListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index)const {
    Q_UNUSED(index)
    return QSize(option.rect.width(), 69);
}
