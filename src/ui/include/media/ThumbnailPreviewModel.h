#ifndef THUMBNAILPREVIEWMODEL_H
#define THUMBNAILPREVIEWMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QSize>
#include "MediaItem.h"


class ThumbnailPreviewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MediaRoles {
        ThumbnailPathRole = Qt::UserRole + 1,
        SourceMediaPathRole,
        MediaTypeRole,
        MessageIdRole,
        FullMediaRole,
    };

    explicit ThumbnailPreviewModel(QObject *parent = nullptr);

    // 基本的模型接口
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // 媒体项管理
    void addMediaItem(const QString& thumbnailPath, const QString& sourceMediaPath, const QString& mediaType = "image");
    void addMediaItem(const MediaItem& item);
    void clearAllMediaItems();
    void setMediaItems(const QList<MediaItem>& items);

    QModelIndex indexFromMessageId(qint64 messageId) const;
    int rowFromMessageId(qint64 messageId) const;


protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<MediaItem> m_mediaItems;

};

#endif // THUMBNAILPREVIEWMODEL_H
