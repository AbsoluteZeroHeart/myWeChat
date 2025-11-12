#ifndef CONTACTITEMDELEGATE_H
#define CONTACTITEMDELEGATE_H

#include "ContactTreeModel.h"
#include <QStyledItemDelegate>
#include <QPainter>

class ContactItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ContactItemDelegate(ContactTreeModel *m_model, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:

    void drawDefaultAvatar(QPainter *painter, const QRect &avatarRect,
                           const QString &name, int radius) const;


    ContactTreeModel *m_model;
};

#endif // CONTACTITEMDELEGATE_H
