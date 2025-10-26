#include "customlistview.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include "ConversationTypes.h"

CustomListView::CustomListView(QWidget *parent)
    : QListView(parent), isSyncing(false), remainingScroll(0),
    m_listType(ConversationList), m_currentConversationId(0)
{
    // 基本行为
    setSelectionMode(QAbstractItemView::SingleSelection);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    // 设置平滑滚动
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 让视图不占用右侧滚动条布局空间（用新的滚动条）
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 取得native scrollbar(视图实际内部仍使用它进行实际滚动）
    nativeVsb = QListView::verticalScrollBar();
    if (!nativeVsb) nativeVsb = new QScrollBar(Qt::Vertical, this);
    nativeVsb->hide();

    // 创建overly scrollbar （放在viewport上）
    newVsb = new QScrollBar(Qt::Vertical, this->viewport());
    newVsb->setFixedWidth(overlayWidth);
    newVsb->setCursor(Qt::ArrowCursor);

    updateScrollBarPosition();
    applyScrollBarStyle();

    // 设置透明度
    opacity = new QGraphicsOpacityEffect(newVsb);
    opacity->setOpacity(0.0);
    newVsb->setGraphicsEffect(opacity);

    // 设置透明动画，淡进淡出
    fadeAnim = new QPropertyAnimation(opacity, "opacity", this);
    fadeAnim->setDuration(200);
    fadeAnim->setEasingCurve(QEasingCurve::InOutQuad);

    // 隐藏延时
    hideTimer = new QTimer(this);
    hideTimer->setInterval(600);
    hideTimer->setSingleShot(true);
    connect(hideTimer, &QTimer::timeout, this, &CustomListView::fadeOutOverlay);

    // 确保viewport的event/leave/滚轮事件也被捕获
    viewport()->installEventFilter(this);

    //当动画结束并且透明的为0时隐藏overlay
    connect(fadeAnim, &QPropertyAnimation::finished, this, [this](){
        if(opacity->opacity()<= 0.001 && newVsb){
            newVsb->hide();
        }
    });

    // native->overlay(视图更新overlay）
    connect(nativeVsb, &QScrollBar::valueChanged, this, [this](int value){
        if(!isSyncing){
            isSyncing = true;
            newVsb->setValue(value);
            isSyncing = false;
        }
    });
    connect(nativeVsb, &QScrollBar::rangeChanged, this, [this](int min, int max){
        newVsb->setRange(min, max);
        newVsb->setPageStep(nativeVsb->pageStep());
        updateScrollBarPosition();
        if(min == max){
            newVsb->hide();
        }else if(newVsb->isHidden()&&(max-min > 0)){
            showOverlayScrollBar();
            startHideTimer();
        }
    });
    connect(nativeVsb, &QScrollBar::sliderMoved, this, [this](int position){
        if(!isSyncing){
            isSyncing = true;
            newVsb->setSliderPosition(position);
            isSyncing = true;
        }
    });

    // voerlay->native(用户拖动overlay时控制真实滚动）
    connect(newVsb, &QScrollBar::valueChanged, this, [this](int value){
        if(!isSyncing){
            isSyncing = true;
            nativeVsb->setValue(value);
            isSyncing = false;
        }
    });
    connect(newVsb, &QScrollBar::sliderMoved, this, [this](int position){
        if(!isSyncing){
            isSyncing = true;
            nativeVsb->setSliderPosition(position);
            isSyncing = false;
        }
    });

    // 初始同步状态
    newVsb->setRange(nativeVsb->minimum(), nativeVsb->maximum());
    newVsb->setPageStep(nativeVsb->pageStep());
    newVsb->setValue(nativeVsb->value());

    // 默认overlay隐藏
    newVsb->hide();

    // 初始化分帧滚动定时器
    scrollTimer = new QTimer(this);
    scrollTimer->setInterval(frameInterval);
    scrollTimer->setSingleShot(false);
    connect(scrollTimer, &QTimer::timeout, this, &CustomListView::onScrollTimerTimeout);

    createConversationContextMenu();
    createMessageContextMenu();
}

CustomListView::~CustomListView()
{
    delete m_conversationMenu;
    delete m_messageMenu;
    delete hideTimer;
    delete scrollTimer;
}

// 更新样式
void CustomListView::applyScrollBarStyle()
{
    if(!newVsb) return;
    newVsb->setStyleSheet(QString(
        "QScrollBar:vertical{"
        "background: transparent;"
        "margin: 0px;"
        "border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical{"
        "background: rgba(150,150,150,220);"
        "border-radius: 4px;"
        "min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover{"
        "background: rgba(140,140,140,220);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "subcontrol-origin: margin;"
        "background: transparent;"
        "height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "background: transparent;"
        "}"
        ));
}

void CustomListView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    updateScrollBarPosition();
}

void CustomListView::updateScrollBarPosition()
{
    if(!newVsb || !viewport()) return;
    QRect vpRect = viewport()->rect();
    int x = vpRect.right() - overlayWidth - marginRight;
    int y = vpRect.top();
    int w = overlayWidth;
    int h = vpRect.height();

    newVsb->setGeometry(x,y,w,h);
    newVsb->raise();
}

void CustomListView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if(index.isValid()){
        if(selectionModel()->isSelected(index)){
            selectionModel()->select(index, QItemSelectionModel::Deselect);
            emit clicked(index);
            return;
        }
    }
    // 点击时显示滚动条
    showOverlayScrollBar();
    QListView::mousePressEvent(event);
}

void CustomListView::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    showOverlayScrollBar();
    hideTimer->stop();
    QListView::enterEvent(event);
}

void CustomListView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    startHideTimer();
    QListView::leaveEvent(event);
}

bool CustomListView::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == viewport()){
        if(event->type() == QEvent::Enter){
            showOverlayScrollBar();
            hideTimer->stop();
        }else if(event->type() == QEvent::Leave){
            startHideTimer();
        }else if(event->type() == QEvent::Wheel){
            showOverlayScrollBar();
        }
    }
    return QListView::eventFilter(watched, event);
}

void CustomListView::wheelEvent(QWheelEvent *event)
{
    showOverlayScrollBar();
    int totalScrollPixel = 0;
    const qreal ANGLE_TO_PIXEL = 0.17;

    if(event->pixelDelta().y() != 0){
        //触控板：直接使用像素增量
        totalScrollPixel = event->pixelDelta().y();
    }
    else{
        // 鼠标滚轮：角度转像素
        const qreal angle = event->angleDelta().y();
        const qreal scrollPixel = angle * ANGLE_TO_PIXEL;

        // 滚动越多速度越快
        qreal mul = qMin(qAbs(remainingScroll)/scrollSpeed, 2);
        qreal scrollSpeed_1 = scrollSpeed + mul*scrollSpeed;

        const qreal finalScrollPixel = scrollPixel * (scrollSpeed_1/20.0);
        totalScrollPixel = qRound(finalScrollPixel);
    }
    // 添加到滚动量
    remainingScroll += -totalScrollPixel;
    if(!scrollTimer->isActive()) scrollTimer->start();

    event->accept();
}

void CustomListView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    updateScrollBarPosition();

    updateGeometries();

    // 如果有模型，触发数据改变信号来刷新所有可见项
    if (model()) {
        // 获取可见区域的首尾索引
        QModelIndex firstVisible = indexAt(viewport()->rect().topLeft());
        QModelIndex lastVisible = indexAt(viewport()->rect().bottomLeft());

        if (firstVisible.isValid() && lastVisible.isValid()) {
            // 触发可见区域的数据改变信号
            emit model()->dataChanged(firstVisible, lastVisible);
        }
    }
}

void CustomListView::showOverlayScrollBar()
{
    if(!newVsb) return;
    hideTimer->stop();
    if(!newVsb->isVisible()) newVsb->show();

    // 如果已经完全不透明，不需要启动动画
    if(qFuzzyCompare(opacity->opacity(), 1.0)) return;

    fadeAnim->stop();
    fadeAnim->setStartValue(opacity->opacity());
    fadeAnim->setEndValue(1.0);
    fadeAnim->start();
}

void CustomListView::startHideTimer()
{
    if(!newVsb) return;
    hideTimer->start();
}

void CustomListView::fadeOutOverlay()
{
    if(!newVsb) return;

    fadeAnim->stop();
    fadeAnim->setStartValue(opacity->opacity());
    fadeAnim->setEndValue(0.0);
    fadeAnim->start();
}

// 定时器触发的小幅度滚动函数
void CustomListView::onScrollTimerTimeout()
{
    //剩余滚动量为0，停止定时器
    if(remainingScroll == 0){
        scrollTimer->stop();
        return;
    }
    // 滚动量存的越多速度越快
    int mul = qAbs(remainingScroll)/scrollSpeed;
    int step = scrollStep + mul*scrollStep;

    // 保持滚动方向（剩余量为正--下滚， 负--上滚）
    if(remainingScroll<0) step = -step;

    // 更新原生滚动条的vlue（驱动视图滚动）
    if(nativeVsb){
        int newVlue = nativeVsb->value() + step;
        newVlue = qMax(nativeVsb->minimum(), qMin(nativeVsb->maximum(),newVlue));
        nativeVsb->setValue(newVlue);
    }

    // 减少剩余滚动量
    remainingScroll -= step;
    if(qAbs(remainingScroll) < scrollStep) remainingScroll = 0;
}


void CustomListView::setMarginRight(int value)
{
    marginRight = value;
}


void CustomListView::createConversationContextMenu()
{
    m_conversationMenu = new QMenu(this);

    // 设置菜单样式
    m_conversationMenu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 4px;
            font-family: "Microsoft YaHei";
            font-size: 14px;
        }
        QMenu::item {
            height: 32px;
            padding: 0px 12px;
            border-radius: 4px;
            margin: 2px;
            color: #333333;
        }
        QMenu::item:selected:!disabled {
            background-color: #4CAF50;
            color: white;
        }
        QMenu::item[class="delete"] {
            color: #ff4444;
        }
        QMenu::item[class="delete"]:selected:!disabled {
            background-color: #ff4444;
            color: white;
        }
        QMenu::separator {
            height: 1px;
            background: #e0e0e0;
            margin: 4px 8px;
        }
    )");

    m_toggleTopAction = new QAction("置顶", this);
    m_markAsUnreadAction = new QAction("标为未读", this);
    m_toggleMuteAction = new QAction("消息免打扰", this);
    m_openInWindowAction = new QAction("独立窗口显示", this);
    m_deleteAction = new QAction("删除", this);

    // 为删除项设置特殊类名
    m_deleteAction->setProperty("class", "delete");

    m_conversationMenu->addAction(m_toggleTopAction);
    m_conversationMenu->addAction(m_markAsUnreadAction);
    m_conversationMenu->addAction(m_toggleMuteAction);
    m_conversationMenu->addSeparator();
    m_conversationMenu->addAction(m_openInWindowAction);
    m_conversationMenu->addSeparator();
    m_conversationMenu->addAction(m_deleteAction);

    // 连接信号
    connect(m_toggleTopAction, &QAction::triggered, this, [this]() {
        emit conversationToggleTop(m_currentConversationId);
    });
    connect(m_markAsUnreadAction, &QAction::triggered, this, [this]() {
        emit conversationMarkAsUnread(m_currentConversationId);
    });
    connect(m_toggleMuteAction, &QAction::triggered, this, [this]() {
        emit conversationToggleMute(m_currentConversationId);
    });
    connect(m_openInWindowAction, &QAction::triggered, this, [this]() {
        emit conversationOpenInWindow(m_currentConversationId);
    });
    connect(m_deleteAction, &QAction::triggered, this, [this]() {
        emit conversationDelete(m_currentConversationId);
    });
}

void CustomListView::createMessageContextMenu()
{
    m_messageMenu = new QMenu(this);

    // 设置菜单样式
    m_messageMenu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 4px;
            font-family: "Microsoft YaHei";
            font-size: 14px;
        }
        QMenu::item {
            height: 32px;
            padding: 0px 12px;
            border-radius: 4px;
            margin: 2px;
            color: #333333;
        }
        QMenu::item:selected:!disabled {
            background-color: #4CAF50;
            color: white;
        }
        QMenu::item[class="delete"] {
            color: #ff4444;
        }
        QMenu::item[class="delete"]:selected:!disabled {
            background-color: #ff4444;
            color: white;
        }
        QMenu::separator {
            height: 1px;
            background: #e0e0e0;
            margin: 4px 8px;
        }
    )");

    m_copyAction = new QAction("复制", this);
    m_zoomAction = new QAction("放大查看", this);
    m_translateAction = new QAction("翻译", this);
    m_searchAction = new QAction("搜索", this);
    m_forwardAction = new QAction("转发", this);
    m_favoriteAction = new QAction("收藏", this);
    m_remindAction = new QAction("提醒", this);
    m_multiSelectAction = new QAction("多选", this);
    m_quoteAction = new QAction("引用", this);
    m_pinAction = new QAction("置顶", this);
    m_deleteAction = new QAction("删除", this);

    // 为删除项设置特殊类名
    m_deleteAction->setProperty("class", "delete");

    m_messageMenu->addAction(m_copyAction);
    m_messageMenu->addAction(m_zoomAction);
    m_messageMenu->addAction(m_translateAction);
    m_messageMenu->addAction(m_searchAction);
    m_messageMenu->addSeparator();
    m_messageMenu->addAction(m_forwardAction);
    m_messageMenu->addAction(m_favoriteAction);
    m_messageMenu->addAction(m_remindAction);
    m_messageMenu->addSeparator();
    m_messageMenu->addAction(m_multiSelectAction);
    m_messageMenu->addAction(m_quoteAction);
    m_messageMenu->addAction(m_pinAction);
    m_messageMenu->addSeparator();
    m_messageMenu->addAction(m_deleteAction);

    // 连接信号
    connect(m_copyAction, &QAction::triggered, this, &CustomListView::messageCopy);
    connect(m_zoomAction, &QAction::triggered, this, &CustomListView::messageZoom);
    connect(m_translateAction, &QAction::triggered, this, &CustomListView::messageTranslate);
    connect(m_searchAction, &QAction::triggered, this, &CustomListView::messageSearch);
    connect(m_forwardAction, &QAction::triggered, this, &CustomListView::messageForward);
    connect(m_favoriteAction, &QAction::triggered, this, &CustomListView::messageFavorite);
    connect(m_remindAction, &QAction::triggered, this, &CustomListView::messageRemind);
    connect(m_multiSelectAction, &QAction::triggered, this, &CustomListView::messageMultiSelect);
    connect(m_quoteAction, &QAction::triggered, this, &CustomListView::messageQuote);
    connect(m_pinAction, &QAction::triggered, this, &CustomListView::messagePin);
    connect(m_deleteAction, &QAction::triggered, this, &CustomListView::messageDelete);
}

void CustomListView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return;
    }

    if (m_listType == ConversationList) {
        // 会话列表：
        m_currentConversationId = index.data(ConversationIdRole).toLongLong();
        if (m_currentConversationId != 0) {

            bool isTop = index.data(IsTopRole).toBool();
            m_toggleTopAction->setText(isTop ? "取消置顶" : "置顶");

            int unreadCount = index.data(UnreadCountRole).toInt();
            m_markAsUnreadAction->setText(unreadCount > 0 ? "标记为已读" : "标记为未读");

            m_conversationMenu->exec(event->globalPos());
        }
    } else if (m_listType == MessageList) {
        // 消息列表：
        m_messageMenu->exec(event->globalPos());
    }
}
// 设置会话列表菜单信号连接的便捷方法
void CustomListView::setConversationMenuSignals(QObject *receiver, const char *toggleTopSlot,
                                                const char *markUnreadSlot, const char *toggleMuteSlot,
                                                const char *openWindowSlot, const char *deleteSlot)
{
    if (toggleTopSlot) connect(this, SIGNAL(conversationToggleTop(qint64)), receiver, toggleTopSlot);
    if (markUnreadSlot) connect(this, SIGNAL(conversationMarkAsUnread(qint64)), receiver, markUnreadSlot);
    if (toggleMuteSlot) connect(this, SIGNAL(conversationToggleMute(qint64)), receiver, toggleMuteSlot);
    if (openWindowSlot) connect(this, SIGNAL(conversationOpenInWindow(qint64)), receiver, openWindowSlot);
    if (deleteSlot) connect(this, SIGNAL(conversationDelete(qint64)), receiver, deleteSlot);
}

// 设置消息列表菜单信号连接的便捷方法
void CustomListView::setMessageMenuSignals(QObject *receiver, const char *copySlot,
                                           const char *zoomSlot, const char *translateSlot,
                                           const char *searchSlot, const char *forwardSlot,
                                           const char *favoriteSlot, const char *remindSlot,
                                           const char *multiSelectSlot, const char *quoteSlot,
                                           const char *pinSlot, const char *deleteSlot)
{
    if (copySlot) connect(this, SIGNAL(messageCopy()), receiver, copySlot);
    if (zoomSlot) connect(this, SIGNAL(messageZoom()), receiver, zoomSlot);
    if (translateSlot) connect(this, SIGNAL(messageTranslate()), receiver, translateSlot);
    if (searchSlot) connect(this, SIGNAL(messageSearch()), receiver, searchSlot);
    if (forwardSlot) connect(this, SIGNAL(messageForward()), receiver, forwardSlot);
    if (favoriteSlot) connect(this, SIGNAL(messageFavorite()), receiver, favoriteSlot);
    if (remindSlot) connect(this, SIGNAL(messageRemind()), receiver, remindSlot);
    if (multiSelectSlot) connect(this, SIGNAL(messageMultiSelect()), receiver, multiSelectSlot);
    if (quoteSlot) connect(this, SIGNAL(messageQuote()), receiver, quoteSlot);
    if (pinSlot) connect(this, SIGNAL(messagePin()), receiver, pinSlot);
    if (deleteSlot) connect(this, SIGNAL(messageDelete()), receiver, deleteSlot);
}






