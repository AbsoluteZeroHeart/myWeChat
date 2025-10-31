#include "ThumbnailPreviewModel.h"
#include <QDebug>
#include <QFileInfo>

ThumbnailPreviewModel::ThumbnailPreviewModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ThumbnailPreviewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_mediaItems.count();
}

QVariant ThumbnailPreviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_mediaItems.count())
        return QVariant();

    const MediaItem &item = m_mediaItems.at(index.row());

    switch (role) {
    case FullMediaRole:
        return QVariant::fromValue(item);

    case Qt::DisplayRole:
        return QFileInfo(item.sourceMediaPath).fileName();

    case ThumbnailPathRole:
        return item.thumbnailPath;

    case MessageIdRole:
        return item.messageId;

    case SourceMediaPathRole:
        return item.sourceMediaPath;

    case MediaTypeRole:
        return item.mediaType;

    default:
        return QVariant();
    }
}

bool ThumbnailPreviewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_mediaItems.count())
        return false;

    MediaItem &item = m_mediaItems[index.row()];

    switch (role) {
    case ThumbnailPathRole:
        item.thumbnailPath = value.toString();

    case SourceMediaPathRole:
        item.sourceMediaPath = value.toString();

    case MediaTypeRole:
        item.mediaType = value.toString();

    default:
        return false;
    }
}

void ThumbnailPreviewModel::addMediaItem(const QString& thumbnailPath, const QString& sourceMediaPath, const QString& mediaType)
{
    addMediaItem(MediaItem(thumbnailPath, sourceMediaPath, mediaType));
}

void ThumbnailPreviewModel::addMediaItem(const MediaItem& item)
{
    beginInsertRows(QModelIndex(), m_mediaItems.count(), m_mediaItems.count());
    m_mediaItems.append(item);
    endInsertRows();
}

void ThumbnailPreviewModel::clearAllMediaItems()
{
    beginResetModel();
    m_mediaItems.clear();
    endResetModel();
}

void ThumbnailPreviewModel::setMediaItems(const QList<MediaItem>& items)
{
    beginResetModel();
    m_mediaItems.clear();
    m_mediaItems = items;
    endResetModel();
}


QHash<int, QByteArray> ThumbnailPreviewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[ThumbnailPathRole] = "thumbnailPath";
    roles[SourceMediaPathRole] = "sourceMediaPath";
    roles[MediaTypeRole] = "mediaType";
    roles[MessageIdRole] = "messageId";
    return roles;
}

int ThumbnailPreviewModel::rowFromMessageId(qint64 messageId) const
{
    for (int i = 0; i < m_mediaItems.count(); ++i) {
        if (m_mediaItems.at(i).messageId == messageId) {
            return i;
        }
    }
    return -1; // 未找到
}

QModelIndex ThumbnailPreviewModel::indexFromMessageId(qint64 messageId) const
{
    int row = rowFromMessageId(messageId);
    if (row >= 0) {
        return createIndex(row, 0);
    }
    return QModelIndex();
}
