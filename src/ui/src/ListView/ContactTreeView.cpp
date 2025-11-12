#include "ContactTreeView.h"
#include "ContactTreeModel.h"
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QDebug>

ContactTreeView::ContactTreeView(QWidget *parent)
    : QTreeView(parent)
    , m_contactModel(nullptr)
{
    // 配置视图
    setHeaderHidden(true);
    setSelectionMode(SingleSelection);
    setAlternatingRowColors(false);
    setRootIsDecorated(true);
    setUniformRowHeights(false);
    setEditTriggers(QTreeView::NoEditTriggers);

    // 展开所有节点
    expandAll();

}


void ContactTreeView::setContactModel(ContactTreeModel *model)
{
    m_contactModel = model;
    setModel(model);
}

Contact ContactTreeView::getSelectedContact() const
{
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        return getContactFromIndex(selectedIndexes.first());
    }
    return Contact();
}

Contact ContactTreeView::getContactFromIndex(const QModelIndex &index) const
{
    if (!index.isValid()) return Contact();

    QVariant contactData = index.data(Qt::UserRole);
    if (contactData.isValid() && contactData.canConvert<Contact>()) {
        return contactData.value<Contact>();
    }
    return Contact();
}

void ContactTreeView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());

    if (m_contactModel && m_contactModel->isParentNode(index)) {
        // 对于父节点
        handleParentNodeClick(index);
        event->accept(); // 处理事件，不进行选择
        return;

    } else {
        // 对于其他节点，正常处理
        QTreeView::mousePressEvent(event);
    }
}

void ContactTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());

    if (m_contactModel && m_contactModel->isParentNode(index)) {
        // 对于父节点，确保不会选中
        clearSelection();
        event->accept();
    } else {
        QTreeView::mouseReleaseEvent(event);
    }
}

void ContactTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());

    if (m_contactModel && m_contactModel->isParentNode(index)) {
        // 对于父节点，双击也不选中，但可以切换展开状态
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
        event->accept();
    } else {
        QTreeView::mouseDoubleClickEvent(event);
    }
}

void ContactTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // 首先检查是否有选择，并且选中的是父节点
    if (m_contactModel && !selected.isEmpty()) {
        QModelIndexList selectedIndexes = selected.indexes();
        if (!selectedIndexes.isEmpty()) {
            QModelIndex selectedIndex = selectedIndexes.first();
            if (m_contactModel->isParentNode(selectedIndex)) {
                // 清除对父节点的选择
                clearSelection();
                return; // 直接返回，不调用基类
            }
        }
    }

    // 正常处理选择变化
    QTreeView::selectionChanged(selected, deselected);
}

void ContactTreeView::handleParentNodeClick(const QModelIndex &index)
{
    // 切换展开状态
    if (isExpanded(index)) {
        collapse(index);
    } else {
        expand(index);
    }
}

