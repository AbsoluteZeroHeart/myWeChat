#ifndef CHATMESSAGELISTVIEW_H
#define CHATMESSAGELISTVIEW_H

#include <QListView>
#include <QTimer>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QAction>
#include "Message.h"
#include "CustomListView.h"

class ChatMessageListView: public CustomListView
{
    Q_OBJECT

public:

    explicit ChatMessageListView(QWidget *parent = nullptr);
    ~ChatMessageListView();

    // 显示消息列表菜单
    void execMessageListMenu(const QPoint& globalPos, const Message &message);

signals:

    // 消息列表菜单信号
    void messageCopy(const Message & message);
    void messageZoom();
    void messageTranslate();
    void messageSearch();
    void messageForward();
    void messageFavorite();
    void messageRemind();
    void messageMultiSelect();
    void messageQuote();
    void messageDelete(const Message & message);

    // 鼠标滚动加载更多消息
    void loadmoreMsg(int count);

protected:
    void wheelEvent(QWheelEvent *event)override;

private:
    void createMessageContextMenu();
    void showDeleteConfirmationDialog();

    // 消息列表菜单
    QMenu *m_messageMenu;
    QAction *m_copyAction;
    QAction *m_zoomAction;
    QAction *m_translateAction;
    QAction *m_searchAction;
    QAction *m_forwardAction;
    QAction *m_favoriteAction;
    QAction *m_remindAction;
    QAction *m_multiSelectAction;
    QAction *m_quoteAction;
    QAction *m_deleteAction;

    Message currentMsg;

};

#endif //CUSTOMLISTVIEW_H
