#ifndef CHATLISTVIEW_H
#define CHATLISTVIEW_H

#include <QTimer>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QAction>
#include "CustomListView.h"

class ChatListView: public CustomListView
{
    Q_OBJECT

public:

    explicit ChatListView(QWidget *parent = nullptr);
    ~ChatListView();

signals:
    // 会话列表菜单信号
    void conversationToggleTop(qint64 conversationId);
    void conversationMarkAsUnread(qint64 conversationId);
    void conversationToggleMute(qint64 conversationId);
    void conversationOpenInWindow(qint64 conversationId);
    void conversationDelete(qint64 conversationId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void createConversationContextMenu();
    void showDeleteConfirmationDialog();

    // 会话列表菜单
    QMenu *m_conversationMenu;
    QAction *m_toggleTopAction;
    QAction *m_markAsUnreadAction;
    QAction *m_toggleMuteAction;
    QAction *m_openInWindowAction;
    QAction *m_deleteAction;
    qint64 m_currentConversationId;

};

#endif //CUSTOMLISTVIEW_H
