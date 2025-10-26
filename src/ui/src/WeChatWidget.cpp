#include "WeChatWidget.h"
#include "ui_WeChatWidget.h"
#include "rightpopover.h"
#include "addDialog.h"
#include "moredialog.h"
#include "floatingdialog.h"
#include "personalinfodialog.h"
#include "MediaDialog.h"
#include "imglabel.h"
#include "ChatListDelegate.h"
#include "ChatMessageDelegate.h"
#include "AppController.h"
#include <QSplitter>
#include <QFrame>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QStandardPaths>


WeChatWidget::WeChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WeChatWidget)
    // 弹窗
    , addDialog(nullptr)
    , moreDialog(nullptr)
    , rightPopover(nullptr)
    , floatingDialog(nullptr)
    , mediaDialog(nullptr)
    , personalInfoDialog(nullptr)

    , appController(new AppController(this))

    , chatListDelegate(new ChatListDelegate(this))
    , chatMessageDelegate(new ChatMessageDelegate(this))

    // 自定义窗口相关
    , m_isOnTop(false)
    , m_titleBarHeight(70)
    , m_isMaximized(false)
    , m_isDragging(false)
    , m_isDraggingMax(false)
    , m_currentEdge(None)
    , m_isResizing(false)
    , m_borderWidth(5)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    // 初始化聊天列表
    chatListView = ui->chatListView;
    chatListView->setListType(CustomListView::ConversationList);
    chatListView->setModel(appController->conversationController()->chatListModel());
    // 连接会话列表菜单信号
    chatListView->setConversationMenuSignals(
        appController->conversationController(),
        SLOT(handleToggleTop(qint64)),
        SLOT(handleMarkAsUnread(qint64)),
        SLOT(handleToggleMute(qint64)),
        SLOT(handleOpenInWindow(qint64)),
        SLOT(handleDelete(qint64))
        );
    chatListView->setItemDelegate(chatListDelegate);
    chatListView->setUniformItemSizes(true);
    appController->conversationController()->loadConversations();

    connect(chatListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous){
                qint64 conversationId = current.data(ConversationIdRole).toLongLong();
                appController->messageController()->setCurrentConversationId(conversationId);
                qDebug() << "会话id:" << conversationId;

                // 简单延迟滚动
                QTimer::singleShot(100, this, [this]() {
                    conversationsView->scrollToBottom();
                });
            });


    // 初始化消息列表
    conversationsView = ui->messageListView;
    conversationsView->setListType(CustomListView::MessageList);
    conversationsView->setModel(appController->messageController()->messagesModel());
    // 连接消息列表菜单信号
    conversationsView->setMessageMenuSignals(
        appController->messageController(),
        SLOT(handleCopy()),
        SLOT(handleZoom()),
        SLOT(handleTranslate()),
        SLOT(handleSearch()),
        SLOT(handleForward()),
        SLOT(handleFavorite()),
        SLOT(handleRemind()),
        SLOT(handleMultiSelect()),
        SLOT(handleQuote()),
        SLOT(handlePin()),
        SLOT(handleDelete())
        );
    conversationsView->setItemDelegate(chatMessageDelegate);
    conversationsView->setUniformItemSizes(false);
    conversationsView->setResizeMode(QListView::Adjust);
    conversationsView->setMarginRight(0);
    appController->messageController()->setCurrentUserId(10001);

    // // 滚动到最后一条消息
    // QTimer::singleShot(100, this, [this]() {
    //     QModelIndex lastIndex = appController->messageController()->messagesModel()
    //                                 ->index(appController->messageController()
    //                                 ->messagesModel()->rowCount() - 1, 0);
    //     conversationsView->scrollTo(lastIndex, QAbstractItemView::PositionAtBottom);
    // });

    connect(chatMessageDelegate, &ChatMessageDelegate::imageClicked, this, [&](const QPixmap &img){
        qDebug()<<"点击图片";
        if(!mediaDialog) mediaDialog = new MediaDialog();
        mediaDialog->setAttribute(Qt::WA_DeleteOnClose);
        mediaDialog->playPixmap(img);
        mediaDialog->show();
    });
    connect(chatMessageDelegate, &ChatMessageDelegate::videoClicked, this, [&](const QString &videoPath){
        qDebug()<<"点击视频";
        if(!mediaDialog) mediaDialog = new MediaDialog();
        mediaDialog->setAttribute(Qt::WA_DeleteOnClose);
        // media_Dialog->;
        mediaDialog->show();
    });
    connect(chatMessageDelegate, &ChatMessageDelegate::fileClicked, this, [&](const QString &filePath){
        qDebug()<<"点击文件";
        bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    });

    //检查信息输入框状态，设置初始样式
    updateSendButtonStyle();
    connect(ui->sendTextEdit, &QTextEdit::textChanged,
            this, &WeChatWidget::updateSendButtonStyle);

    qApp->installEventFilter(this);

}


WeChatWidget::~WeChatWidget()
{
    delete ui;
    qApp->removeEventFilter(this);
}


// 绘制边框-------------------------------------------
void WeChatWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(160,160,160),1);
    painter.setPen(pen);
    painter.setBrush(QColor(238,238,238));

    QRect rect = QRect(1,1,width()-2,height()-2);
    painter.drawRoundedRect(rect,m_isMaximized? 0:6,m_isMaximized? 0:6);
}

// 判断鼠标位置是否在窗口边缘------------------------------------------
WeChatWidget::Edge WeChatWidget::getEdge(const QPoint &pos)
{
    // 获取窗口矩形
    QRect rect = this->rect();
    if (pos.x() <= m_borderWidth && pos.y() <= m_borderWidth)
        return TopLeft;

    if (pos.x() >= rect.width() - m_borderWidth && pos.y() <= m_borderWidth)
        return TopRight;

    if (pos.x() <= m_borderWidth && pos.y() >= rect.height() - m_borderWidth)
        return BottomLeft;

    if (pos.x() >= rect.width() - m_borderWidth && pos.y() >= rect.height() - m_borderWidth)
        return BottomRight;

    if (pos.x() <= m_borderWidth)
        return Left;

    if (pos.x() >= rect.width() - m_borderWidth)
        return Right;

    if (pos.y() <= m_borderWidth)
        return Top;

    if (pos.y() >= rect.height() - m_borderWidth)
        return Bottom;

    return None;
}

// 更新鼠标样式------------------------------------------------
void WeChatWidget::updateCursorShape(const QPoint &pos)
{
    Edge edge = getEdge(pos);

    switch (edge) {
    case Left:
    case Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case Top:
    case Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

// 处理拉伸逻辑
void WeChatWidget::handleResize(const QPoint &currentGlobalPos)
{
    if (m_currentEdge == None || m_isMaximized) return;

    QRect newGeometry = m_windowGeometry;
    switch (m_currentEdge) {
    case Left:
        newGeometry.setLeft(qMin(currentGlobalPos.x(), newGeometry.right() - minimumWidth()));
        break;
    case Right:
        newGeometry.setRight(qMax(currentGlobalPos.x(), newGeometry.left() + minimumWidth()));
        break;
    case Top:
        newGeometry.setTop(qMin(currentGlobalPos.y(), newGeometry.bottom() - minimumHeight()));
        break;
    case Bottom:
        newGeometry.setBottom(qMax(currentGlobalPos.y(), newGeometry.top() + minimumHeight()));
        break;
    case TopLeft:
        newGeometry.setLeft(qMin(currentGlobalPos.x(), newGeometry.right() - minimumWidth()));
        newGeometry.setTop(qMin(currentGlobalPos.y(), newGeometry.bottom() - minimumHeight()));
        break;
    case TopRight:
        newGeometry.setRight(qMax(currentGlobalPos.x(), newGeometry.left() + minimumWidth()));
        newGeometry.setTop(qMin(currentGlobalPos.y(), newGeometry.bottom() - minimumHeight()));
        break;
    case BottomLeft:
        newGeometry.setLeft(qMin(currentGlobalPos.x(), newGeometry.right() - minimumWidth()));
        newGeometry.setBottom(qMax(currentGlobalPos.y(), newGeometry.top() + minimumHeight()));
        break;
    case BottomRight:
        newGeometry.setRight(qMax(currentGlobalPos.x(), newGeometry.left() + minimumWidth()));
        newGeometry.setBottom(qMax(currentGlobalPos.y(), newGeometry.top() + minimumHeight()));
        break;
    default:
        break;
    }
    setGeometry(newGeometry);
}

// 处理移动逻辑
void WeChatWidget::handleDrag(const QPoint &currentGlobalPos)
{
    if (m_isDraggingMax) {
        // 最大化时拖动逻辑
        on_maxWinButton_clicked();
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();
        double horizontalRatio = static_cast<double>(currentGlobalPos.x()) / screenWidth;
        int mouseXInWindow = static_cast<int>(horizontalRatio * width());
        int mouseYInWindow = m_titleBarHeight / 2;
        int newWindowX = currentGlobalPos.x() - mouseXInWindow;
        int newWindowY = currentGlobalPos.y() - mouseYInWindow;
        newWindowX = qMax(0, qMin(newWindowX, screenWidth - width()));
        newWindowY = qMax(0, qMin(newWindowY, screenHeight - height()));
        move(newWindowX, newWindowY);
        m_dragStartPosition = currentGlobalPos - frameGeometry().topLeft();
        m_isDraggingMax = false;
    } else {
        // 正常拖动
        move(currentGlobalPos - m_dragStartPosition);
    }
}

// 窗口鼠标按下事件
void WeChatWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_currentEdge = getEdge(event->pos());
        // 边缘拉伸初始化
        if (m_currentEdge != None && !m_isMaximized) {
            m_isResizing = true;
            m_windowGeometry = geometry();
            event->accept();
        }
        // 标题栏移动初始化
        else if (event->pos().y() <= m_titleBarHeight) {
            if (m_isMaximized) m_isDraggingMax = true;
            m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            m_isDragging = true;
            event->accept();
        }
    }
    QWidget::mousePressEvent(event);
}

// 窗口鼠标移动事件（调用公共函数处理逻辑）
void WeChatWidget::mouseMoveEvent(QMouseEvent *event)
{
    updateCursorShape(event->pos());
    // 拉伸：调用handleResize
    if (m_isResizing && m_currentEdge != None && !m_isMaximized) {
        handleResize(event->globalPosition().toPoint()); // 传入全局坐标
        event->accept();
    }
    // 移动：调用handleDrag
    else if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        handleDrag(event->globalPosition().toPoint()); // 传入全局坐标
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

// 鼠标释放事件
void WeChatWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    m_isDraggingMax = false;
    m_isResizing = false;
    m_currentEdge = None;
    setCursor(Qt::ArrowCursor);
    event->accept();
    QWidget::mouseReleaseEvent(event);
}

bool WeChatWidget::eventFilter(QObject *watched, QEvent *event) {
    // 筛选「鼠标移动事件」（QApplication的事件）更新光标
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPosition().toPoint();//全局坐标
        if (this->geometry().contains(this->mapFromGlobal(globalPos))) {
            QPoint windowLocalPos = this->mapFromGlobal(globalPos);
            this->updateCursorShape(windowLocalPos);
        }
        else {
            this->setCursor(Qt::ArrowCursor);
        }
    }
    return false;
}


void WeChatWidget::resizeEvent(QResizeEvent *event)
{
    rightStackedWidgetPageSizeChange();
    QWidget::resizeEvent(event);
}


//动态设置发送按钮样式
void WeChatWidget::updateSendButtonStyle(){
    QTextEdit *sendTextEdit = this->findChild<QTextEdit*>("sendTextEdit");
    QPushButton* sendButton = this->findChild<QPushButton*>("sendPushButton");
    QString text = sendTextEdit->toPlainText().trimmed();
    bool isEmpty = text.isEmpty();
    if(isEmpty){
        //文本框为空：应用空状态样式
        sendButton->setProperty("state","empty");
        sendButton->setStyleSheet("QPushButton[state=\"empty\"] { "
                                  "background-color: rgb(220, 220, 220); "
                                  "color: rgb(150, 150, 150); "
                                  "font: 15px \"黑体\"; "
                                  "border-radius: 3px; "
                                  "}");
    }else{
        // 文本框有内容：恢复原样式
        sendButton->setProperty("state", "normal");
        sendButton->setStyleSheet("QPushButton[state=\"normal\"] { "
                                      "background-color: rgb(7, 193, 96); "
                                      "color: rgb(255, 255, 255); "
                                      "font: 15px \"黑体\"; "
                                      "border-radius: 3px; "
                                      "}"
                                      "QPushButton[state=\"normal\"]:hover { "
                                      "background-color: rgb(7, 182, 88); "
                                      "}"
                                      "QPushButton[state=\"normal\"]:pressed { "
                                      "background-color: rgb(6, 178, 83); "
                                      "}");
    }
    sendButton->style()->unpolish(sendButton);
    sendButton->style()->polish(sendButton);
    sendButton->update();
}


void WeChatWidget::on_contactsToolButton_clicked()
{
    QStackedWidget* rightStackedWidget = this->findChild<QStackedWidget*>("rightStackedWidget");
    QStackedWidget* leftStackedWidget = this->findChild<QStackedWidget*>("leftStackedWidget");
    rightStackedWidget->setCurrentIndex(1);
    leftStackedWidget->setCurrentIndex(1);
    //顺便关一下右侧弹窗
    if(rightPopover)
    {
        on_rightDialogToolButton_clicked();
    }
}


void WeChatWidget::on_collectionToolButton_clicked()
{
    QStackedWidget* rightStackedWidget = this->findChild<QStackedWidget*>("rightStackedWidget");
    QStackedWidget* leftStackedWidget = this->findChild<QStackedWidget*>("leftStackedWidget");
    rightStackedWidget->setCurrentIndex(2);
    leftStackedWidget->setCurrentIndex(2);
    //顺便关一下右侧弹窗
    if(rightPopover)
    {
        on_rightDialogToolButton_clicked();
    }
}

void WeChatWidget::on_chatInterfaceToolButton_clicked()
{
    QStackedWidget* rightStackedWidget = this->findChild<QStackedWidget*>("rightStackedWidget");
    QStackedWidget* leftStackedWidget = this->findChild<QStackedWidget*>("leftStackedWidget");
    rightStackedWidget->setCurrentIndex(0);
    leftStackedWidget->setCurrentIndex(0);
    //顺便关一下右侧弹窗
    if(rightPopover)
    {
        on_rightDialogToolButton_clicked();
    }
}


void WeChatWidget::on_rightDialogToolButton_clicked()
{
    if(rightPopover){
        //隐藏动画：滑回主窗口右侧外部
        QPropertyAnimation *anim = new QPropertyAnimation(rightPopover,"pos");
        anim->setDuration(300);
        QPoint startPos = rightPopover->pos();
        QPoint endPos(startPos.x() + 254, startPos.y());
        anim->setStartValue(startPos);
        anim->setEndValue(endPos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        connect(anim,&QPropertyAnimation::finished, rightPopover,&QWidget::close);
    }else{
        rightPopover = new RightPopover(this->findChild<QWidget*>("rightStackedWidgetPage0"));
        rightPopover->setAttribute(Qt::WA_DeleteOnClose);
        //先在窗口右外侧看不到的地方显示
        int startX = this->findChild<QWidget*>("rightStackedWidgetPage0")->width();
        int startY = this->findChild<QSplitter*>("messageSplitter")->pos().y();
        rightPopover->setGeometry(startX,startY,254,this->height());
        rightPopover->show();
        //再从右侧滑动出来
        QPropertyAnimation *anim = new QPropertyAnimation(rightPopover,"pos");
        anim->setDuration(300);
        QPoint startPos(startX,startY);
        QPoint endPos(startX-254,startY);
        anim->setStartValue(startPos);
        anim->setEndValue(endPos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}


void WeChatWidget::rightStackedWidgetPageSizeChange()
{
    if(rightPopover){
        int rpfX = this->findChild<QWidget*>("rightStackedWidgetPage0")->width() - 254;
        int rpfY = rightPopover->pos().y();
        int rpfWidth = 254;
        int rpfHeight = this->findChild<QWidget*>("rightStackedWidgetPage0")->height();
        rightPopover->setGeometry(rpfX,rpfY,rpfWidth,rpfHeight);
    }
}


void WeChatWidget::on_addToolButton_clicked()
{
    //显示对话框
    if(!addDialog){
        addDialog = new AddDialog(this);
        addDialog->setAttribute(Qt::WA_DeleteOnClose);
        addDialog->showAtPos(QCursor::pos());
    }
}


void WeChatWidget::on_moreToolButton_clicked()
{
    if(!moreDialog){
        moreDialog = new MoreDialog();
        moreDialog->setAttribute(Qt::WA_DeleteOnClose);
        QToolButton* btn = this->findChild<QToolButton*>("moreToolButton");
        QPoint buttonPos = btn->mapToGlobal(QPoint(0,0));
        int x = buttonPos.x() + btn->width();
        int y = buttonPos.y()-200 + btn->height();
        moreDialog->showAtPos(QPoint(x,y));
    }
}


void WeChatWidget::on_floatingToolButton_clicked()
{
    if(!floatingDialog){
        floatingDialog = new FloatingDialog();
        floatingDialog->setAttribute(Qt::WA_DeleteOnClose);
        QToolButton *btn = this->findChild<QToolButton*>("floatingToolButton");
        QPoint buttonPos = btn->mapToGlobal(QPoint(0,0));
        int x = buttonPos.x() + btn->width();
        int y = buttonPos.y()-395 + btn->height();
        floatingDialog->showAtPos(QPoint(x,y));
    }
}


void WeChatWidget::on_avatarPushButton_clicked()
{
    if(!personalInfoDialog)
    {
        personalInfoDialog = new PersonalInfoDialog(this);
        personalInfoDialog ->setAttribute(Qt::WA_DeleteOnClose);

        connect(personalInfoDialog->findChild<ImgLabel*>("avatarLabel"),
            &ImgLabel::labelClicked, this,
            [&](const QPixmap &pixmap){
                    if(!mediaDialog) mediaDialog = new MediaDialog();
                    mediaDialog->setAttribute(Qt::WA_DeleteOnClose);
                    mediaDialog->playPixmap(pixmap);
                    mediaDialog->show();
                    personalInfoDialog->close();});

        QPushButton* btn = this->findChild<QPushButton*>("avatarPushButton");
        QPoint btnGlobalPos = btn->mapToGlobal(QPoint(0,0));
        personalInfoDialog->showAtPos(QPoint(btnGlobalPos.x()+btn->width(), btnGlobalPos.y()));
    }
}


void WeChatWidget::on_closeButton_clicked()
{
    close();
}

// 窗口最大化和还原
void WeChatWidget::on_maxWinButton_clicked()
{
    m_isMaximized = !m_isMaximized;
    if (m_isMaximized) {
        showMaximized();
        QToolButton* toolBtn = this->findChild<QToolButton*>("maxWinButton");
        if (toolBtn) {
            toolBtn->setIcon(QIcon(":/a/icons/还原.svg"));
        }
    } else {
        showNormal();
        QToolButton* toolBtn = this->findChild<QToolButton*>("maxWinButton");
        if (toolBtn) {
            toolBtn->setIcon(QIcon(":/a/icons/窗口最大化.svg"));
        }
    }
}


void WeChatWidget::on_minWinButton_clicked()
{
    showMinimized();
}


void WeChatWidget::on_pinButton_clicked()
{
    // 切换置顶状态
    m_isOnTop = !m_isOnTop;
    if (m_isOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        ui->pinButton->setToolTip("取消置顶");
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        ui->pinButton->setToolTip("置顶");
    }
    // 重新显示窗口（设置窗口标志后需要重新显示才能生效）
    show();
}


