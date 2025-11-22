#include <QStyledItemDelegate>
#include "ThumbnailResourceManager.h"

class ChatListDelegate : public QStyledItemDelegate 
{
    Q_OBJECT

public:
    ChatListDelegate(QObject *parent = nullptr);
    ~ChatListDelegate();
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, 
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
    void onMediaLoaded(const QString& resourcePath, const QPixmap& media, MediaType type);

private:
    void drawDefaultAvatar(QPainter *painter, const QRect &avatarRect, 
                          const QString &name, int radius) const;
};
