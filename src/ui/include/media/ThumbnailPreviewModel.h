#ifndef THUMBNAILPREVIEWMODEL_H
#define THUMBNAILPREVIEWMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QSize>

// 媒体项数据结构
struct MediaItem {
    QString thumbnailPath;    // 缩略图路径
    QString sourceMediaPath;  // 源媒体路径
    QString mediaType;        // 媒体类型: "image" 或 "video"
    QPixmap thumbnail;        // 加载的缩略图
    bool thumbnailLoaded;     // 缩略图是否已加载

    MediaItem(const QString& thumbPath = "",
              const QString& sourcePath = "",
              const QString& type = "image")
        : thumbnailPath(thumbPath)
        , sourceMediaPath(sourcePath)
        , mediaType(type)
        , thumbnailLoaded(false)
    {}
};

class ThumbnailPreviewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MediaRoles {
        ThumbnailPathRole = Qt::UserRole + 1,
        SourceMediaPathRole,
        MediaTypeRole,
        ThumbnailPixmapRole,
        ItemSizeRole
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

    // 缩略图管理
    void loadThumbnail(int index);
    void loadAllThumbnails();
    void setThumbnailSize(const QSize& size);
    QSize getThumbnailSize() const;

    // 获取媒体项信息
    MediaItem getMediaItem(int index) const;
    QString getSourceMediaPath(int index) const;
    QString getThumbnailPath(int index) const;

protected:
    QHash<int, QByteArray> roleNames() const override;

signals:
    // 当缩略图加载完成时发射
    void thumbnailLoaded(int index);
    // 当媒体项需要被展示时发射（由委托触发）
    void mediaItemSelected(const QString& thumbnailPath, const QString& sourceMediaPath, const QString& mediaType);

private:
    QList<MediaItem> m_mediaItems;
    QSize m_thumbnailSize;

    void loadThumbnailForItem(MediaItem& item) const;
};

#endif // THUMBNAILPREVIEWMODEL_H
