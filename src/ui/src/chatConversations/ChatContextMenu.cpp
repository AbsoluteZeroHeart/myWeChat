#include "ChatContextMenu.h"
#include <QWidget>

ChatContextMenu::ChatContextMenu(QWidget* parent)
    : QMenu(parent)
    , m_copyAction(nullptr)
    , m_zoomAction(nullptr)
    , m_translateAction(nullptr)
    , m_searchAction(nullptr)
    , m_forwardAction(nullptr)
    , m_favoriteAction(nullptr)
    , m_remindAction(nullptr)
    , m_multiSelectAction(nullptr)
    , m_quoteAction(nullptr)
    , m_pinAction(nullptr)
    , m_deleteAction(nullptr)
{
    setupActions();
}

void ChatContextMenu::setupActions()
{
    // 创建各个动作
    m_copyAction = addAction(tr("复制"), this, &ChatContextMenu::onActionTriggered);
    m_zoomAction = addAction(tr("放大查看"), this, &ChatContextMenu::onActionTriggered);
    m_translateAction = addAction(tr("翻译"), this, &ChatContextMenu::onActionTriggered);
    m_searchAction = addAction(tr("搜索"), this, &ChatContextMenu::onActionTriggered);
    m_forwardAction = addAction(tr("转发"), this, &ChatContextMenu::onActionTriggered);
    m_favoriteAction = addAction(tr("收藏"), this, &ChatContextMenu::onActionTriggered);
    m_remindAction = addAction(tr("提醒"), this, &ChatContextMenu::onActionTriggered);
    m_multiSelectAction = addAction(tr("多选"), this, &ChatContextMenu::onActionTriggered);
    m_quoteAction = addAction(tr("引用"), this, &ChatContextMenu::onActionTriggered);
    m_pinAction = addAction(tr("置顶"), this, &ChatContextMenu::onActionTriggered);
    m_deleteAction = addAction(tr("删除"), this, &ChatContextMenu::onActionTriggered);

    // 为每个动作设置属性以便识别
    m_copyAction->setProperty("actionType", Copy);
    m_zoomAction->setProperty("actionType", Zoom);
    m_translateAction->setProperty("actionType", Translate);
    m_searchAction->setProperty("actionType", Search);
    m_forwardAction->setProperty("actionType", Forward);
    m_favoriteAction->setProperty("actionType", Favorite);
    m_remindAction->setProperty("actionType", Remind);
    m_multiSelectAction->setProperty("actionType", MultiSelect);
    m_quoteAction->setProperty("actionType", Quote);
    m_pinAction->setProperty("actionType", Pin);
    m_deleteAction->setProperty("actionType", Delete);

    // 可以设置快捷键（可选）
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_deleteAction->setShortcut(QKeySequence::Delete);
}

void ChatContextMenu::onActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    // 从动作属性中获取对应的 ActionType
    ActionType type = static_cast<ActionType>(action->property("actionType").toInt());

    // 发出信号，传递动作类型和当前数据
    emit actionTriggered(type, m_currentData);
}

// 可选：提供设置当前数据的方法
void ChatContextMenu::setCurrentData(const QVariant& data)
{
    m_currentData = data;
}

// 可选：重写 exec 方法以设置当前数据
QAction* ChatContextMenu::exec(const QPoint& pos, const QVariant& data)
{
    m_currentData = data;
    return QMenu::exec(pos);
}
