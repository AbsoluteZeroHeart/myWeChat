#include "ThumbnailPreviewModel.h"
#include <QDebug>
#include <QFileInfo>

ThumbnailPreviewModel::ThumbnailPreviewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_thumbnailSize(50, 50) // 默认缩略图大小
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
    case Qt::DisplayRole:
        return QFileInfo(item.sourceMediaPath).fileName();

    case ThumbnailPathRole:
        return item.thumbnailPath;

    case SourceMediaPathRole:
        return item.sourceMediaPath;

    case MediaTypeRole:
        return item.mediaType;

    case ThumbnailPixmapRole:
        // 如果缩略图尚未加载，尝试加载
        if (!item.thumbnailLoaded && !item.thumbnailPath.isEmpty()) {
            const_cast<ThumbnailPreviewModel*>(this)->loadThumbnail(index.row());
        }
        return item.thumbnail;

    case ItemSizeRole:
        return m_thumbnailSize;

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
    case ThumbnailPixmapRole:
        item.thumbnail = value.value<QPixmap>();
        item.thumbnailLoaded = true;
        emit dataChanged(index, index, {role});
        emit thumbnailLoaded(index.row());
        return true;

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
    m_mediaItems = items;
    endResetModel();
}

void ThumbnailPreviewModel::loadThumbnail(int index)
{
    if (index < 0 || index >= m_mediaItems.count())
        return;

    MediaItem &item = m_mediaItems[index];
    if (item.thumbnailLoaded || item.thumbnailPath.isEmpty())
        return;

    loadThumbnailForItem(item);

    // 更新模型数据
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {ThumbnailPixmapRole});
    emit thumbnailLoaded(index);
}

void ThumbnailPreviewModel::loadAllThumbnails()
{
    for (int i = 0; i < m_mediaItems.count(); ++i) {
        loadThumbnail(i);
    }
}

void ThumbnailPreviewModel::setThumbnailSize(const QSize& size)
{
    if (m_thumbnailSize != size) {
        m_thumbnailSize = size;
        // 通知视图尺寸变化，需要重新布局
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_mediaItems.count() - 1, 0),
                         {ItemSizeRole});
    }
}

QSize ThumbnailPreviewModel::getThumbnailSize() const
{
    return m_thumbnailSize;
}

MediaItem ThumbnailPreviewModel::getMediaItem(int index) const
{
    if (index >= 0 && index < m_mediaItems.count()) {
        return m_mediaItems.at(index);
    }
    return MediaItem();
}

QString ThumbnailPreviewModel::getSourceMediaPath(int index) const
{
    if (index >= 0 && index < m_mediaItems.count()) {
        return m_mediaItems.at(index).sourceMediaPath;
    }
    return QString();
}

QString ThumbnailPreviewModel::getThumbnailPath(int index) const
{
    if (index >= 0 && index < m_mediaItems.count()) {
        return m_mediaItems.at(index).thumbnailPath;
    }
    return QString();
}

QHash<int, QByteArray> ThumbnailPreviewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[ThumbnailPathRole] = "thumbnailPath";
    roles[SourceMediaPathRole] = "sourceMediaPath";
    roles[MediaTypeRole] = "mediaType";
    roles[ThumbnailPixmapRole] = "thumbnailPixmap";
    roles[ItemSizeRole] = "itemSize";
    return roles;
}

void ThumbnailPreviewModel::loadThumbnailForItem(MediaItem& item) const
{
    if (item.thumbnailLoaded || item.thumbnailPath.isEmpty())
        return;

    QPixmap pixmap(item.thumbnailPath);
    if (!pixmap.isNull()) {
        // 缩放缩略图到指定大小
        if (!m_thumbnailSize.isEmpty() && pixmap.size() != m_thumbnailSize) {
            pixmap = pixmap.scaled(m_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        item.thumbnail = pixmap;
        item.thumbnailLoaded = true;
    }
}
