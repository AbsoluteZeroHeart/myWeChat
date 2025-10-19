#include "ConversationsDelegate.h"
#include <QFontMetrics>
#include <QFileInfo>
#include <QtMath>
#include <QPainterPath>
#include <qDebug>
#include "FormatTime.h"
#include "AsyncThumbnailManager.h"


ConversationsDelegate::ConversationsDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void ConversationsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
    if (!index.isValid()) return;

    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 根据消息类型调用不同的绘制方法
    switch (message.type()) {
    case MessageType::TEXT:
        paintTextMessage(painter, option, message);
        break;
    case MessageType::IMAGE:
        paintImageMessage(painter, option, message);
        break;
    case MessageType::VIDEO:
        paintVideoMessage(painter, option, message);
        break;
    case MessageType::FILE:
        paintFileMessage(painter, option, message);
        break;
    case MessageType::VOICE:
        paintVoiceMessage(painter, option, message);
        break;
    }

    painter->restore();
}

QSize ConversationsDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    if (!index.isValid()) return QSize(100, 30);

    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    int width = option.rect.width();

    // 统一的边距设置（与paint函数保持一致）
    const int margin = 18;           // 边距
    const int bubblePadding = 10;    // 气泡内边距
    const int avatarSize = 38;       // 头像大小

    // 计算时间戳高度
    QFont timeFont = option.font;
    timeFont.setPointSizeF(7.5);
    QFontMetrics timeMetrics(timeFont);
    int timeHeight = timeMetrics.height();

    switch (message.type()) {
    case MessageType::TEXT: {
        // 计算文本内容所需尺寸
        int maxBubbleWidth = width * 0.6;
        QFont font = option.font;
        font.setPointSizeF(10.5);
        font.setFamily(QStringLiteral("微软雅黑"));
        QSize textSize = calculateTextSize(message.content(), font, maxBubbleWidth - 2 * bubblePadding);
        int bubbleHeight = textSize.height() + 2 * bubblePadding;
        int avatarAreaHeight = avatarSize + 2 * margin;
        int contentAreaHeight = bubbleHeight + timeHeight + 2*margin;
        return QSize(width, qMax(avatarAreaHeight, contentAreaHeight));
    }
    case MessageType::IMAGE:{
        QPixmap preview(message.extraData()["path"].toString());
        if(preview.isNull()) return QSize(0,0);
        QPixmap scaled = preview.scaled(200, 300,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        return QSize(width,  scaled.height() + timeHeight + 2*margin);
    }
    case MessageType::VIDEO:{
        QPixmap thumbnail(message.extraData()["thumbnailPath"].toString());
        if(thumbnail.isNull()){
            return QSize(width, 100 + 2*margin + timeHeight);
        } else {
            QPixmap scaled = thumbnail.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            return QSize(width, scaled.height() + timeHeight + 2*margin);
        }
    }
    case MessageType::FILE: {
        const int fileBubbleHeight = 95;
        int totalHeight = fileBubbleHeight + timeHeight + 2 * margin;
        int avatarAreaHeight = avatarSize + 2 * margin;
        return QSize(width, qMax(totalHeight, avatarAreaHeight));
    }
    case MessageType::VOICE: {
        // 语音消息高度计算
        const int voiceBubbleHeight = 40;
        int totalHeight = voiceBubbleHeight + timeHeight + 2 * margin;
        int avatarAreaHeight = avatarSize + 2 * margin;
        return QSize(width, qMax(totalHeight, avatarAreaHeight));
    }
    default:
        return QSize(100, 30 + 2 * margin);
    }
}

bool ConversationsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                      const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();

        QRect clickableRect = getClickableRect(option, message);
        if (clickableRect.contains(mouseEvent->pos())) {
            switch (message.type()) {
            case MessageType::IMAGE:{
                QString imgPath = message.extraData()["path"].toString();
                QString thumbnail = message.extraData()["thumbnailPath"].toString();

                if(QFileInfo(imgPath).exists()){
                    emit imageClicked(QPixmap(imgPath));
                }else{
                    QPixmap thumbnailPixmap = AsyncThumbnailManager::getWarningThumbnail(thumbnail, "图片");
                    emit imageClicked(thumbnailPixmap);
                    break;
                }
            }
            case MessageType::VIDEO:{
                QString videoPath = message.extraData()["path"].toString();
                QString thumbnail = message.extraData()["thumbnailPath"].toString();

                if(QFileInfo(videoPath).exists()){
                    emit videoClicked(videoPath);
                }else {
                    QPixmap thumbnailPixmap = AsyncThumbnailManager::getWarningThumbnail(thumbnail, "视频");
                    emit imageClicked(thumbnailPixmap);
                }
                break;
            }
            case MessageType::FILE:
                emit fileClicked(message.extraData()["path"].toString());
                break;
            case MessageType::VOICE:
                emit voiceClicked(message.extraData()["path"].toString());
                break;
            default:
                break;
            }
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// 绘制文本信息
void ConversationsDelegate::paintTextMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                           const ChatMessage& message) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 基本参数设置（与sizeHint保持一致）
    const int avatarSize = 38;
    const int margin = 10;
    const int bubblePadding = 10;
    const int maxBubbleWidth = option.rect.width() * 0.6;
    const int timeSpacing = 10;
    bool isOwnMessage = message.isOwn();

    // 计算头像位置绘制头像
    QRect avatarRect;
    if (isOwnMessage) {
        avatarRect = QRect(option.rect.right() - margin - avatarSize,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    } else {
        avatarRect = QRect(option.rect.left() + margin,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    }
    paintAvatar(painter, avatarRect, message);

    // 计算文本尺寸
    QFont contentFont = option.font;
    contentFont.setPointSizeF(10.5);
    contentFont.setFamily(QStringLiteral("微软雅黑"));
    QFontMetrics contentMetrics(contentFont);
    QRect textRect = contentMetrics.boundingRect(QRect(0, 0, maxBubbleWidth - 2 * bubblePadding, 10000),
                                                 Qt::TextWordWrap, message.content());
    // 计算气泡尺寸
    int bubbleWidth = qMin(textRect.width() + 2 * bubblePadding, maxBubbleWidth);
    int bubbleHeight = textRect.height() + 2 * bubblePadding;

    // 计算气泡位置
    QRect bubbleRect;
    QRect contentRect;

    if (isOwnMessage) {
        bubbleRect = QRect(avatarRect.left() - margin - bubbleWidth,avatarRect.top(),bubbleWidth, bubbleHeight);
        contentRect = bubbleRect.adjusted(bubblePadding, bubblePadding, -bubblePadding, -bubblePadding);
    } else {
        bubbleRect = QRect(avatarRect.right() + margin,avatarRect.top(),bubbleWidth, bubbleHeight);
        contentRect = bubbleRect.adjusted(bubblePadding, bubblePadding, -bubblePadding, -bubblePadding);
    }

    // 绘制气泡
    painter->setPen(Qt::NoPen);
    if (isOwnMessage) {
        painter->setBrush(QColor(149, 236, 105));
    } else {
        painter->setBrush(Qt::white);
    }
    painter->drawRoundedRect(bubbleRect, 5, 5);

    // 绘制气泡小三角
    QPolygon triangle;
    if (isOwnMessage) {
        triangle << QPoint(bubbleRect.right(), bubbleRect.top() + 20)
        << QPoint(bubbleRect.right() + 6, bubbleRect.top() + 15)
        << QPoint(bubbleRect.right(), bubbleRect.top() + 10);
    } else {
        triangle << QPoint(bubbleRect.left(), bubbleRect.top() + 20)
        << QPoint(bubbleRect.left() - 6, bubbleRect.top() + 15)
        << QPoint(bubbleRect.left(), bubbleRect.top() + 10);
    }
    painter->drawPolygon(triangle);

    // 绘制消息内容
    painter->setPen(Qt::black);
    painter->setFont(contentFont);
    painter->drawText(contentRect, Qt::AlignLeft | Qt::TextWordWrap, message.content());

    // 绘制时间戳
    QRect timeRect(bubbleRect.left(), bubbleRect.bottom() + timeSpacing,
                   bubbleRect.width(), 0);
    paintTime(painter,timeRect,option,message,isOwnMessage);

    painter->restore();
}

// 绘制图片信息
void ConversationsDelegate::paintImageMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                            const ChatMessage& message) const {
    QPixmap preview(message.extraData()["path"].toString());
    if(preview.isNull()) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    int avatarSize = 38;
    const int margin = 10;
    bool isOwnMessage = message.isOwn();

    QRect avatarRect;
    if(isOwnMessage){
        avatarRect = QRect(option.rect.right()-margin-avatarSize,
                           option.rect.top()+margin,
                           avatarSize,avatarSize);
    } else {
        avatarRect = QRect(option.rect.left()+margin,
                           option.rect.top()+margin,
                           avatarSize,avatarSize);
    }
    paintAvatar(painter, avatarRect, message);

    // 绘制缩略图
    QRect imageRect;
    QPixmap scaled = preview.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if(isOwnMessage){
        imageRect = QRect(avatarRect.left()-scaled.width() - margin,
                            avatarRect.top(),
                            scaled.width(),scaled.height());
    }else{
        imageRect = QRect(avatarRect.right() + margin,
                            avatarRect.top(),
                            scaled.width(),scaled.height());
    }
    painter->drawPixmap(imageRect, scaled);

    // 时间
    QRect timeRect (imageRect.left(),imageRect.bottom() + margin,
                    imageRect.width(), 0);
    paintTime(painter, timeRect, option, message, isOwnMessage);

    painter->restore();
}

// 绘制视频消息
void ConversationsDelegate::paintVideoMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                            const ChatMessage& message) const {

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const int margin = 10;
    const int avatarSize = 38;
    bool isOwnMessage = message.isOwn();

    QRect avatarRect;
    if(isOwnMessage){
        avatarRect = QRect(option.rect.right()-margin-avatarSize,
                           option.rect.top()+margin,
                           avatarSize, avatarSize);
    }else{
        avatarRect = QRect(option.rect.left()+margin,
                           option.rect.top()+margin,
                           avatarSize, avatarSize);
    }
    paintAvatar(painter, avatarRect, message);

    QPixmap thumbnail (message.extraData()["thumbnailPath"].toString());
    QRect videoRect;
    if (thumbnail.isNull()) {
        // 没有缩略图时绘制默认视频图标
        if(isOwnMessage){
            videoRect = QRect(avatarRect.left()-margin-100,
                              avatarRect.top(),100,100);
        }else{
            videoRect = QRect(avatarRect.right()+margin,
                              avatarRect.top(),100,100);
        }
        painter->fillRect(videoRect, Qt::darkGray);
        QFont videoFont = option.font;
        videoFont.setFamily("微软雅黑");
        videoFont.setPointSize(12);
        painter->setFont(videoFont);
        painter->setPen(Qt::white);
        painter->drawText(videoRect,Qt::AlignCenter, "📹 视频");
    } else {
        // 绘制缩略图
        QPixmap scaled = thumbnail.scaled(200,300,
                                          Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if(isOwnMessage){
            videoRect = QRect(avatarRect.left()-margin-scaled.width(),
                              avatarRect.top(), scaled.width(),scaled.height());
        }else {
            videoRect = QRect(avatarRect.right()+margin, avatarRect.top(),
                              scaled.width(), scaled.height());
        }
        painter->drawPixmap(videoRect, scaled);

        // 在缩略图中绘制视频播放标识
        QRect play(videoRect.left() + (videoRect.width() - 50)/2,
                    videoRect.top() + (videoRect.height()-50)/2,
                        50, 50);

        painter->save();
        painter->setBrush(QColor(0, 0, 0, 180)); // 半透明黑色背景
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(play);

        // 绘制播放三角形
        painter->setBrush(Qt::white);
        QPolygon triangle;
        triangle << QPoint(play.center().x() - 6, play.center().y() - 12)
                 << QPoint(play.center().x() - 6, play.center().y() + 12)
                 << QPoint(play.center().x() + 12, play.center().y());
        painter->drawPolygon(triangle);

        painter->restore();
    }

    // 时间
    QRect timeRect(videoRect.left(), videoRect.bottom()+margin, videoRect.width(), 0);
    paintTime(painter, timeRect, option, message, isOwnMessage);

    painter->restore();
}

// 绘制文件消息
void ConversationsDelegate::paintFileMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                           const ChatMessage& msg) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 基本参数设置
    const int avatarSize = 38;
    const int margin = 10;
    const int bubblePadding = 12;
    const int fileBubbleWidth = 240; // 固定宽度，类似微信
    const int fileBubbleHeight = 95; // 固定高度
    const int iconWidth = 29;
    const int iconHeight = 40;

    bool isOwnMessage = msg.isOwn();

    // 计算头像位置
    QRect avatarRect;
    if (isOwnMessage) {
        avatarRect = QRect(option.rect.right() - margin - avatarSize,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    } else {
        avatarRect = QRect(option.rect.left() + margin,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    }
    paintAvatar(painter, avatarRect, msg);

    // 计算文件气泡位置
    QRect fileBubbleRect;
    if (isOwnMessage) {
        fileBubbleRect = QRect(avatarRect.left() - margin - fileBubbleWidth,
                               avatarRect.top(),
                               fileBubbleWidth, fileBubbleHeight);
    } else {
        fileBubbleRect = QRect(avatarRect.right() + margin,
                               avatarRect.top(),
                               fileBubbleWidth, fileBubbleHeight);
    }

    // 绘制文件气泡背景
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(fileBubbleRect, 5, 5);

    // 绘制气泡小三角
    QPolygon triangle;
    if (isOwnMessage) {
        triangle << QPoint(fileBubbleRect.right(), fileBubbleRect.top() + 20)
        << QPoint(fileBubbleRect.right() + 6, fileBubbleRect.top() + 15)
        << QPoint(fileBubbleRect.right(), fileBubbleRect.top() + 10);
    } else {
        triangle << QPoint(fileBubbleRect.left(), fileBubbleRect.top() + 20)
        << QPoint(fileBubbleRect.left() - 6, fileBubbleRect.top() + 15)
        << QPoint(fileBubbleRect.left(), fileBubbleRect.top() + 10);
    }
    painter->drawPolygon(triangle);

    // 文件图标区域
    QRect iconRect(fileBubbleRect.right() - bubblePadding - iconWidth,
                   fileBubbleRect.top() + bubblePadding,
                   iconWidth, iconHeight);

    // 绘制文件类型图标
    QString fileName = msg.extraData()["name"].toString();
    QString fileExtension = getFileExtension(fileName).toLower();
    paintFileIcon(painter, iconRect, fileExtension);

    // 文本区域
    QRect textRect(fileBubbleRect.left() + bubblePadding,
                   fileBubbleRect.top() + bubblePadding,
                   fileBubbleWidth - iconWidth - 3*bubblePadding,
                   fileBubbleHeight - 2 * bubblePadding);

    // 绘制文件名
    painter->setPen(Qt::black);
    QFont fileNameFont = option.font;
    fileNameFont.setPointSizeF(10.2);
    fileNameFont.setFamily("微软雅黑");
    painter->setFont(fileNameFont);

    // 文件名省略处理
    QString displayName = fileName;
    QFontMetrics nameMetrics(fileNameFont);
    if (nameMetrics.horizontalAdvance(fileName) > textRect.width()) {
        displayName = nameMetrics.elidedText(fileName, Qt::ElideRight, textRect.width());
    }
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, displayName);

    // 绘制文件大小
    qint64 fileSize = msg.extraData()["size"].toLongLong();
    QString sizeStr = formatFileSize(fileSize);

    QFont sizeFont = option.font;
    sizeFont.setPointSizeF(9);
    painter->setFont(sizeFont);
    painter->setPen(QColor(150, 150, 150));

    QRect sizeRect = textRect.adjusted(0, nameMetrics.height() + bubblePadding, 0, 0);
    painter->drawText(sizeRect, Qt::AlignLeft | Qt::AlignTop, sizeStr);

    // 绘制底部横线和"微信电脑版"
    QFont wechatFont = option.font;
    wechatFont.setPointSizeF(8.5);
    wechatFont.setFamily("微软雅黑");
    painter->setFont(wechatFont);
    painter->setPen(QColor(200, 200, 200));

    // 横线
    QLine dividerLine(fileBubbleRect.left()+bubblePadding, fileBubbleRect.bottom() - 25,
                      fileBubbleRect.right()-bubblePadding, fileBubbleRect.bottom() - 25);
    painter->drawLine(dividerLine);

    // "微信电脑版"文字
    painter->setPen(QColor(150, 150, 150));
    painter->drawText(QRect(textRect.left(), fileBubbleRect.bottom() - 20,
                            textRect.width(), 15),
                      Qt::AlignLeft | Qt::AlignVCenter, "微信电脑版");

    // 绘制时间戳
    QRect timeRect = QRect(fileBubbleRect.left(), fileBubbleRect.bottom() + margin,
                         fileBubbleRect.width(), 0);
    paintTime(painter, timeRect, option, msg, isOwnMessage);

    painter->restore();
}

// 绘制语音消息
void ConversationsDelegate::paintVoiceMessage(QPainter* painter, const QStyleOptionViewItem& option,
                                            const ChatMessage& msg) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 基本参数设置
    const int avatarSize = 38;
    const int margin = 10;
    const int bubblePadding = 12;
    const int minVoiceBubbleWidth = 90;  // 最小宽度
    const int maxVoiceBubbleWidth = 200; // 最大宽度
    const int voiceBubbleHeight = 40;    // 固定高度
    const int playButtonSize = 24;       // 播放按钮大小
    const int waveformHeight = 16;       // 波形高度

    bool isOwnMessage = msg.isOwn();
    int duration = msg.extraData()["duration"].toInt(); // 秒数

    // 计算头像位置
    QRect avatarRect;
    if (isOwnMessage) {
        avatarRect = QRect(option.rect.right() - margin - avatarSize,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    } else {
        avatarRect = QRect(option.rect.left() + margin,
                           option.rect.top() + margin,
                           avatarSize, avatarSize);
    }
    paintAvatar(painter, avatarRect, msg);

    // 根据时长计算气泡宽度
    int voiceBubbleWidth = qMin(maxVoiceBubbleWidth,
                                minVoiceBubbleWidth + duration * 3);

    // 计算语音气泡位置
    QRect voiceBubbleRect;
    if (isOwnMessage) {
        voiceBubbleRect = QRect(avatarRect.left() - margin - voiceBubbleWidth,
                                avatarRect.top() + (avatarSize - voiceBubbleHeight) / 2,
                                voiceBubbleWidth, voiceBubbleHeight);
    } else {
        voiceBubbleRect = QRect(avatarRect.right() + margin,
                                avatarRect.top() + (avatarSize - voiceBubbleHeight) / 2,
                                voiceBubbleWidth, voiceBubbleHeight);
    }

    // 绘制语音气泡背景（微信样式：自己绿色，对方白色）
    painter->setPen(Qt::NoPen);
    if (isOwnMessage) {
        painter->setBrush(QColor(0x07, 0xC1, 0x60)); // 微信绿色
    } else {
        painter->setBrush(Qt::white);
    }
    painter->drawRoundedRect(voiceBubbleRect, 5, 5);

    // 绘制气泡小三角
    QPolygon triangle;
    if (isOwnMessage) {
        triangle << QPoint(voiceBubbleRect.right(), voiceBubbleRect.top() + 22)
        << QPoint(voiceBubbleRect.right() + 6, voiceBubbleRect.top() + 17)
        << QPoint(voiceBubbleRect.right(), voiceBubbleRect.top() + 12);
        painter->setBrush(QColor(0x07, 0xC1, 0x60));
    } else {
        triangle << QPoint(voiceBubbleRect.left(), voiceBubbleRect.top() + 22)
        << QPoint(voiceBubbleRect.left() - 6, voiceBubbleRect.top() + 17)
        << QPoint(voiceBubbleRect.left(), voiceBubbleRect.top() + 12);
        painter->setBrush(Qt::white);
    }
    painter->drawPolygon(triangle);

    // 绘制播放按钮和波形和
    paintPlayButtonAndWaveform(painter, voiceBubbleRect, isOwnMessage);
    paintDurationText(painter, voiceBubbleRect, duration, isOwnMessage);

    // 绘制时间戳
    QRect timeRect = QRect(voiceBubbleRect.left(), voiceBubbleRect.bottom() + margin,
                           voiceBubbleRect.width(), 0);
    paintTime(painter, timeRect, option, msg, isOwnMessage);

    painter->restore();
}

// 绘制文件图标
void ConversationsDelegate::paintFileIcon(QPainter* painter, const QRect& fileRect,
                                        const QString& extension) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QString typeText;
    bool unknownType = false;
    // 设置图标颜色基于文件类型
    QColor iconColor;
    if (extension == "pdf") {
        iconColor = QColor(230, 67, 64);
        typeText = "PDF";
    } else if (extension == "doc" || extension == "docx") {
        iconColor = QColor(44, 86, 154);
        typeText = "W";
    } else if (extension == "xls" || extension == "xlsx") {
        iconColor = QColor(32, 115, 70);
        typeText = "X";
    } else if (extension == "ppt" || extension == "pptx") {
        iconColor = QColor(242, 97, 63);
        typeText = "P";
    } else if (extension == "txt") {
        iconColor = QColor(250, 157, 59);
        typeText = "txt";
    } else {
        iconColor = QColor(237, 237, 237);
        typeText = "*";
        unknownType = true;

    }

    // 计算缺角大小
    int foldSize = qMin(fileRect.width(), fileRect.height()) * 0.3;

    // 创建缺角矩形路径（右上角缺失）
    QPainterPath filePath;
    filePath.moveTo(fileRect.topLeft()); // A点：左上角
    filePath.lineTo(fileRect.topRight().x() - foldSize, fileRect.top()); // B点：右上角向左偏移
    filePath.lineTo(fileRect.topRight().x(), fileRect.top() + foldSize); // C点：右上角向下偏移
    filePath.lineTo(fileRect.bottomRight()); // D点：右下角
    filePath.lineTo(fileRect.bottomLeft()); // E点：左下角
    filePath.closeSubpath(); // 回到A点

    // 绘制文件主体（填充iconColor）
    painter->setPen(QPen(iconColor, 1));
    painter->setBrush(iconColor);
    painter->drawPath(filePath);

    //绘制折角
    QColor foldedColor;
    if(unknownType){
        foldedColor = iconColor.lighter(80);
    }else{
        foldedColor = iconColor.lighter(140);
    }
    painter->setPen(foldedColor);
    painter->setBrush(foldedColor);

    QPainterPath foldedPath;
    foldedPath.moveTo(filePath.elementAt(1));
    foldedPath.lineTo(filePath.elementAt(2));
    foldedPath.lineTo(filePath.elementAt(1).x, filePath.elementAt(2).y);
    foldedPath.closeSubpath();
    painter->drawPath(foldedPath);

    // 文件类型文字
    if(unknownType){
        painter->setPen(QColor(86, 106, 148));

    }else {
        painter->setPen(Qt::white);
    }

    QFont iconFont = painter->font();

    // 根据文字长度调整字体大小
    if (typeText.length() <= 2) {
        iconFont.setPointSize(12);
    } else {
        iconFont.setPointSize(8);
    }

    QFontMetrics metrics (iconFont);
    painter->setFont(iconFont);

    // 调整文字区域
    QRect textRect = fileRect;
    textRect.setBottom(textRect.bottom() - 3);
    textRect.setTop(textRect.bottom()-metrics.height()-3);

    painter->drawText(textRect, Qt::AlignHCenter, typeText);

    painter->restore();
}

// 绘制语音时长文本
void ConversationsDelegate::paintDurationText(QPainter* painter, const QRect& bubbleRect,
                                            int duration, bool isOwnMessage) const {
    const int playButtonSize = 24;
    QString durationStr = QString("%1\"") .arg(duration);
    painter->setFont(QFont("微软雅黑", 9));

    if (isOwnMessage) {
        painter->setPen(Qt::white);
        painter->drawText(bubbleRect.adjusted(8, 0, -playButtonSize-20, 0),
                          Qt::AlignLeft | Qt::AlignVCenter, durationStr);
    } else {
        painter->setPen(QColor(100, 100, 100));
        painter->drawText(bubbleRect.adjusted(playButtonSize+20, 0, -8, 0),
                          Qt::AlignRight | Qt::AlignVCenter, durationStr);
    }
}

// 绘制语音播放按钮和波形
void ConversationsDelegate::paintPlayButtonAndWaveform(QPainter* painter,
                                                     const QRect& bubbleRect, bool isOwnMessage) const {
    const int playButtonSize = 24;
    const int waveformHeight = 16;
    const int waveformWidth = 60;

    // 计算播放按钮位置
    QRect playButtonRect;
    if (isOwnMessage) {
        playButtonRect = QRect(bubbleRect.right() - playButtonSize - 8,
                               bubbleRect.center().y() - playButtonSize/2,
                               playButtonSize, playButtonSize);
    } else {
        playButtonRect = QRect(bubbleRect.left() + 8,
                               bubbleRect.center().y() - playButtonSize/2,
                               playButtonSize, playButtonSize);
    }

    // 绘制播放按钮（圆形背景）
    painter->setPen(Qt::NoPen);
    if (isOwnMessage) {
        painter->setBrush(Qt::white);
    } else {
        painter->setBrush(QColor(0x07, 0xC1, 0x60)); // 微信绿色
    }
    painter->drawEllipse(playButtonRect);

    // 绘制播放图标（三角形）
    painter->setBrush(isOwnMessage ? QColor(0x07, 0xC1, 0x60) : Qt::white);
    QPolygon playIcon;
    int iconSize = 8;
    if (isOwnMessage) {
        playIcon << QPoint(playButtonRect.center().x() - 2, playButtonRect.center().y() - iconSize/2)
        << QPoint(playButtonRect.center().x() - 2, playButtonRect.center().y() + iconSize/2)
        << QPoint(playButtonRect.center().x() + iconSize/2, playButtonRect.center().y());
    } else {
        playIcon << QPoint(playButtonRect.center().x() + 2, playButtonRect.center().y() - iconSize/2)
        << QPoint(playButtonRect.center().x() + 2, playButtonRect.center().y() + iconSize/2)
        << QPoint(playButtonRect.center().x() - iconSize/2, playButtonRect.center().y());
    }
    painter->drawPolygon(playIcon);

    // 绘制波形（模拟微信样式）
    QRect waveformRect;
    if (isOwnMessage) {
        waveformRect = QRect(playButtonRect.left() - waveformWidth - 5,
                             bubbleRect.center().y() - waveformHeight/2,
                             waveformWidth, waveformHeight);
    } else {
        waveformRect = QRect(playButtonRect.right() + 5,
                             bubbleRect.center().y() - waveformHeight/2,
                             waveformWidth, waveformHeight);
    }

    paintVoiceWaveform(painter, waveformRect, isOwnMessage);
}

// 绘制语音波形
void ConversationsDelegate::paintVoiceWaveform(QPainter* painter, const QRect& rect,
                                             bool isOwnMessage) const {
    // 微信风格的波形：几条不同高度的竖线
    const int barCount = 4;
    const int barWidth = 3;
    const int gap = 2;
    int totalWidth = barCount * barWidth + (barCount - 1) * gap;
    int startX = rect.left() + (rect.width() - totalWidth) / 2;
    int centerY = rect.center().y();

    // 波形的相对高度（模拟）
    int heights[4] = {8, 12, 16, 12};

    painter->setPen(Qt::NoPen);
    for (int i = 0; i < barCount; ++i) {
        int barHeight = heights[i];
        QRect barRect(startX + i * (barWidth + gap),
                      centerY - barHeight/2,
                      barWidth, barHeight);

        if (isOwnMessage) {
            painter->setBrush(Qt::white);
        } else {
            painter->setBrush(QColor(0x07, 0xC1, 0x60));
        }
        painter->drawRect(barRect);
    }
}

//绘制头像
void ConversationsDelegate::paintAvatar(QPainter* painter, const QRect& avatarRect,
                                      const ChatMessage& message) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const int radius = 5;
    QImage avatarImg;
    if(!message.avatar().isEmpty() && avatarImg.load(message.avatar())){
        // 加载头像图片并绘制圆角效果
        QPixmap pix = QPixmap::fromImage(avatarImg);
        QPixmap rounded(avatarRect.width(), avatarRect.height());
        rounded.fill(Qt::transparent);

        {
            QPainter p(&rounded);
            p.setRenderHint(QPainter::Antialiasing, true);
            QPainterPath path;
            path.addRoundedRect(QRectF(0, 0, avatarRect.width(), avatarRect.height()), radius, radius);
            p.setClipPath(path);
            p.drawPixmap(0, 0, pix.scaled(avatarRect.width(), avatarRect.height(),
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation));
        }
        painter->drawPixmap(avatarRect, rounded);
    }
    else{
        // 绘制默认头像（带用户首字母）
        QPainterPath path;
        path.addRoundedRect(QRectF(avatarRect), radius, radius);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(210, 210, 210));
        painter->drawPath(path);

        QFont iniFont = painter->font();
        iniFont.setBold(true);
        iniFont.setPointSize(15);
        painter->setFont(iniFont);
        painter->setPen(QColor(100, 100, 100));
        painter->drawText(avatarRect, Qt::AlignCenter, message.sender().left(1));
    }

    painter->restore();
}

// 绘制时间
void ConversationsDelegate::paintTime(QPainter *painter, const QRect &Rect, const QStyleOptionViewItem& option,
                                    ChatMessage msg, bool isOwnMessage)const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,true);

    // 绘制时间戳
    QString timeText = FormatTime(msg.timestamp());
    QFont timeFont = option.font;
    timeFont.setPointSizeF(7.5);
    painter->setFont(timeFont);
    QFontMetrics timeMetrics(timeFont);
    int timeHeight = timeMetrics.height();
    painter->setPen(QColor(150, 150, 150));
    QRect timeRect (Rect.left(),Rect.top(),Rect.width(),timeHeight);

    if (isOwnMessage) {
        painter->drawText(timeRect, Qt::AlignRight | Qt::AlignTop, timeText);
    } else {
        painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignTop, timeText);
    }

    painter->restore();
}

// 计算文本在指定最大宽度下的尺寸（支持自动换行）
QSize ConversationsDelegate::calculateTextSize(const QString& text, const QFont& font,
                                             int maxWidth) const {
    if (text.isEmpty()) return QSize(0, 0);

    QFontMetrics metrics(font);
    QRect textRect = metrics.boundingRect(
        QRect(0, 0, maxWidth, 0),
        Qt::TextWordWrap | Qt::AlignLeft,
        text
        );
    return textRect.size();
}

// 获取文件扩展名
QString ConversationsDelegate::getFileExtension(const QString& fileName) const {
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex != -1 && dotIndex < fileName.length() - 1) {
        return fileName.mid(dotIndex + 1);
    }
    return "";
}

// 格式化文件大小
QString ConversationsDelegate::formatFileSize(qint64 bytes) const {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / (double)GB, 0, 'f', 1);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / (double)MB, 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / (double)KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

// 获取消息中可点击区域
QRect ConversationsDelegate::getClickableRect(const QStyleOptionViewItem& option,
                                            const ChatMessage& msg) const {
    const int avatarSize =38;
    const int margin =10;
    bool isOwnMessage =msg.isOwn();

    switch (msg.type()) {
    case MessageType::IMAGE:{
        QPixmap preview(msg.extraData()["path"].toString());
        if(preview.isNull()) return QRect();
        QPixmap scaled = preview.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if(isOwnMessage){
            return QRect(option.rect.right() - avatarSize - scaled.width()- 2*margin,
                         option.rect.top() + margin, scaled.width(), scaled.height());
        }else{
            return QRect(option.rect.left() + avatarSize + 2*margin,
                         option.rect.top() + margin, scaled.width(), scaled.height());
        }
    }
    case MessageType::VIDEO:{
        QRect videoRect;
        QPixmap thumbnail (msg.extraData()["thumbnailPath"].toString());

        int width, height;
        if(thumbnail.isNull()){
            width = 100; height = 100;
        }else{
            QPixmap scaled = thumbnail.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            width = scaled.width(); height = scaled.height();
        }

        if(isOwnMessage){
            videoRect = QRect(option.rect.right() - avatarSize - 2*margin - width,
                              option.rect.top() + margin, width, height);
        }else{
            videoRect = QRect(option.rect.left() + avatarSize + 2*margin,
                              option.rect.top() + margin, width, height);
        }
        return videoRect;
    }
    case MessageType::FILE:{
        const int fileBubbleWidth = 240;
        const int fileBubbleHeight = 95;
        QRect fileBubbleRect;
        if (isOwnMessage) {
            fileBubbleRect = QRect(option.rect.right() - 2*margin - avatarSize - fileBubbleWidth,
                                   option.rect.top()+margin, fileBubbleWidth, fileBubbleHeight);
        } else {
            fileBubbleRect = QRect(option.rect.left() + 2*margin + avatarSize,
                                   option.rect.top() + margin, fileBubbleWidth, fileBubbleHeight);
        }
        return fileBubbleRect;
    }
    case MessageType::VOICE:{
        const int minVoiceBubbleWidth = 90;  // 最小宽度
        const int maxVoiceBubbleWidth = 200; // 最大宽度
        const int voiceBubbleHeight = 40;    // 固定高度
        int duration = msg.extraData()["duration"].toInt(); // 秒数
        int voiceBubbleWidth = qMin(maxVoiceBubbleWidth,
                                    minVoiceBubbleWidth + duration * 3);
        QRect voiceBubbleRect;
        if (isOwnMessage) {
            voiceBubbleRect = QRect(option.rect.right()- avatarSize - 2*margin - voiceBubbleWidth,
                                    option.rect.top() + margin + (avatarSize - voiceBubbleHeight) / 2,
                                    voiceBubbleWidth, voiceBubbleHeight);
        } else {
            voiceBubbleRect = QRect(option.rect.left() + avatarSize + 2*margin,
                                    option.rect.top()+ margin + (avatarSize - voiceBubbleHeight) / 2,
                                    voiceBubbleWidth, voiceBubbleHeight);
        }
        return voiceBubbleRect;
    }
    default:
        return QRect();
    }
}
