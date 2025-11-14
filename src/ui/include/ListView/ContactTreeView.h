#ifndef CONTACTTREEVIEW_H
#define CONTACTTREEVIEW_H

#include <QTreeView>
#include "Contact.h"

class ContactTreeModel;

class ContactTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit ContactTreeView(QWidget *parent = nullptr);

    Contact getSelectedContact() const;
    Contact getContactFromIndex(const QModelIndex &index) const;

    void setContactModel(ContactTreeModel *model);

signals:
    void sendMessage(const Contact &contact);
    void starFriend(const Contact &contact);
    void removeFriend(const Contact &contact);



protected:
    // 重写鼠标事件处理
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    // 重写选择事件
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    ContactTreeModel *m_contactModel;

    // 处理节点点击
    void handleParentNodeClick(const QModelIndex &index);
    void createContactMenu();
    void showDeleteConfirmationDialog();


    QMenu *contactMenu;
    QAction* sendMessageAction;
    QAction* starFriendAction;
    QAction *removeFriendAction;


};

#endif // CONTACTTREEVIEW_H
