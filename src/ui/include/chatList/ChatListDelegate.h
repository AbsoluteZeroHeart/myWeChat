#ifndef CHATLISTDELEGATE_H
#define CHATLISTDELEGATE_H

#include <QStyledItemDelegate>

class ChatListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatListDelegate(QObject *parent = nullptr);

    //绘制列表项
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const override;

    //定义列表项大小
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index)const override;

private:


};

#endif // CHATLISTDELEGATE_H
