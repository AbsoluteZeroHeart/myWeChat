#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QListView>
#include <QTimer>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QAction>
#include "Message.h"

class CustomListView: public QListView
{
    Q_OBJECT

public:

    enum ListType {
        ConversationList,  // 会话列表
        MessageList,       // 消息列表
        DefaultList        // 默认列表
    };

    explicit CustomListView(QWidget *parent = nullptr);
    ~CustomListView();
    void setMarginRight(int value);

    // 设置列表类型，用于决定显示哪种右键菜单
    void setListType(ListType type) { m_listType = type; }
    ListType listType() const { return m_listType; }

    // 显示消息列表菜单
    void execMessageListMenu(const QPoint& globalPos, const Message &message);

signals:
    // 会话列表菜单信号
    void conversationToggleTop(qint64 conversationId);
    void conversationMarkAsUnread(qint64 conversationId);
    void conversationToggleMute(qint64 conversationId);
    void conversationOpenInWindow(qint64 conversationId);
    void conversationDelete(qint64 conversationId);

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
    // void messagePin();
    void messageDelete(const Message & message);


    // 鼠标滚动加载更多消息
    void loadmoreMsg(int count);


protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event)override;
    void leaveEvent(QEvent *event)override;
    bool eventFilter(QObject *watched, QEvent *event)override;
    void wheelEvent(QWheelEvent *event)override;
    void resizeEvent(QResizeEvent *event)override;
    void scrollContentsBy(int dx, int dy)override;


private slots:
    void fadeOutOverlay();



private:
    void showOverlayScrollBar();
    void startHideTimer();
    void updateScrollBarPosition();
    void applyScrollBarStyle();
    void onScrollTimerTimeout();

    // 新滚动条显示相关
    QScrollBar *nativeVsb;
    QScrollBar *newVsb;
    QGraphicsOpacityEffect *opacity;
    QPropertyAnimation *fadeAnim;
    QTimer *hideTimer;
    int overlayWidth = 8;
    int marginRight = 2;

    //信号同步锁
    bool isSyncing;

    // 自定义鼠标滚动，精细分帧平滑滚动相关
    int scrollSpeed = 40;         // 滚动一“卡擦”的像素，初始速度
    QTimer *scrollTimer;          // 分帧滚动的定时器
    int remainingScroll;          // 剩余滚动量
    const int scrollStep = 5;     // 每帧滚动的像素
    const int frameInterval = 10; // 每帧间隔（定时器间隔）

private:
    void createConversationContextMenu();
    void createMessageContextMenu();
    void showDeleteConfirmationDialog();
    void showContextMenu(const QPoint &pos);

    ListType m_listType;

    // 会话列表菜单
    QMenu *m_conversationMenu;
    QAction *m_toggleTopAction;
    QAction *m_markAsUnreadAction;
    QAction *m_toggleMuteAction;
    QAction *m_openInWindowAction;

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

    // 菜单公用
    QAction *m_deleteAction;

    // 用做会话列表时
    qint64 m_currentConversationId;

    // 用作消息列表时
    Message currentMsg;

};

#endif //CUSTOMLISTVIEW_H
