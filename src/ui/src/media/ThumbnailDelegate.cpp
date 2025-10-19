#include "ThumbnailDelegate.h"
#include "ThumbnailPreviewModel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QFontMetrics>
#include <QPainterPath>

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
    m_cornerRadius(5),
    m_videoIndicatorSize(20)
{
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect rect = option.rect;
    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isHovered = option.state & QStyle::State_MouseOver;

    // 绘制背景
    QColor backgroundColor = isSelected ? QColor(220, 240, 255) :
                                 isHovered ? QColor(240, 240, 240) : Qt::white;

    painter->fillRect(rect, backgroundColor);

    // 获取数据
    QPixmap thumbnail = index.data(ThumbnailPreviewModel::ThumbnailPixmapRole).value<QPixmap>();
    QString mediaType = index.data(ThumbnailPreviewModel::MediaTypeRole).toString();
    QSize thumbSize = index.data(ThumbnailPreviewModel::ItemSizeRole).toSize();

    QPoint thumbPos(rect.x() + (rect.width() - thumbSize.width()) / 2,
                    rect.y() + 5);

    QRect thumbRect(thumbPos, thumbSize);

    // 绘制缩略图背景和边框
    QColor borderColor = isSelected ? QColor(100, 150, 255) : QColor(200, 200, 200);
    painter->setPen(QPen(borderColor, isSelected ? 2 : 1));
    painter->setBrush(Qt::white);

    // 绘制圆角矩形背景
    QPainterPath backgroundPath = getRoundedRectPath(thumbRect, m_cornerRadius);
    painter->drawPath(backgroundPath);
    painter->fillPath(backgroundPath, Qt::white);

    // 绘制缩略图
    if (!thumbnail.isNull()) {
        // 在缩略图矩形内创建裁剪区域
        painter->save();
        QPainterPath clipPath = getRoundedRectPath(thumbRect.adjusted(1, 1, -1, -1), m_cornerRadius - 1);
        painter->setClipPath(clipPath);

        // 计算缩略图在矩形内的居中位置
        QRect pixmapRect;
        pixmapRect.setSize(thumbnail.size().scaled(thumbRect.size() - QSize(2, 2), Qt::KeepAspectRatio));
        pixmapRect.moveCenter(thumbRect.center());

        painter->drawPixmap(pixmapRect, thumbnail);
        painter->restore();
    } else {
        // 绘制加载中的提示
        painter->setPen(Qt::gray);
        painter->drawText(thumbRect, Qt::AlignCenter, "Loading...");
    }

    // 如果是视频，绘制视频标识
    if (mediaType == "video") {
        drawVideoIndicator(painter, thumbRect);
    }

    // 绘制文件名（可选）
    QString fileName = index.data(Qt::DisplayRole).toString();
    if (!fileName.isEmpty()) {
        QRect textRect(rect.x(), thumbRect.bottom(), rect.width(), 20);
        painter->setPen(Qt::black);

        // 文本省略处理
        QFontMetrics metrics(painter->font());
        QString elidedText = metrics.elidedText(fileName, Qt::ElideMiddle, textRect.width());
        painter->drawText(textRect, Qt::AlignCenter, elidedText);
    }

    // 绘制选中状态边框
    if (isSelected) {
        painter->setPen(QPen(QColor(100, 150, 255), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(backgroundPath);
    }
}

void ThumbnailDelegate::drawVideoIndicator(QPainter *painter, const QRect &rect) const
{
    // 在缩略图中心绘制视频播放标识
    QRect videoRect(rect.center().x() - m_videoIndicatorSize/2,
                    rect.center().y() - m_videoIndicatorSize/2,
                    m_videoIndicatorSize, m_videoIndicatorSize);

    painter->save();
    painter->setBrush(QColor(0, 0, 0, 180)); // 半透明黑色背景
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(videoRect);

    // 绘制播放三角形
    painter->setBrush(Qt::white);
    QPolygon triangle;
    triangle << QPoint(videoRect.center().x() - 2, videoRect.center().y() - 5)
             << QPoint(videoRect.center().x() - 2, videoRect.center().y() + 5)
             << QPoint(videoRect.center().x() + 6, videoRect.center().y());
    painter->drawPolygon(triangle);

    painter->restore();
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QSize itemSize = index.data(ThumbnailPreviewModel::ItemSizeRole).toSize();
    return QSize(itemSize.width() + 20, itemSize.height() + 25); // 增加边距和文本空间
}

bool ThumbnailDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 发射点击信号
            QString thumbnailPath = index.data(ThumbnailPreviewModel::ThumbnailPathRole).toString();
            QString sourceMediaPath = index.data(ThumbnailPreviewModel::SourceMediaPathRole).toString();
            QString mediaType = index.data(ThumbnailPreviewModel::MediaTypeRole).toString();

            emit itemClicked(thumbnailPath, sourceMediaPath, mediaType);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QPainterPath ThumbnailDelegate::getRoundedRectPath(const QRect &rect, int radius) const
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

