#ifndef THUMBNAILDELEGATE_H
#define THUMBNAILDELEGATE_H

#include <QStyledItemDelegate>
#include <QSize>
#include "MediaResourceManager.h"

class ThumbnailDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ThumbnailDelegate(QObject *parent = nullptr);

    // 重写基类函数
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    // 项点击信号
    void itemClicked(const QString &thumbnailPath, const QString &sourceMediaPath, const QString &mediaType);

private:
    // 绘制辅助函数
    void drawVideoIndicator(QPainter *painter, const QRect &rect) const;
    QPainterPath getRoundedRectPath(const QRect &rect, int radius) const;

private slots:
    void onMediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type);


private:
    int m_cornerRadius;            // 圆角半径
    int m_videoIndicatorSize;      // 视频指示器大小
    MediaResourceManager *mediaManager;

};

#endif // THUMBNAILDELEGATE_H
