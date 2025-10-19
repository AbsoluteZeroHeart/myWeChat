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
#include "Chatlistmodel.h"
#include "customlistview.h"
#include "ConversationsDelegate.h"
#include "ConversationsModel.h"
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

    // 聊天列表
    , chatListDelegate(new ChatListDelegate())
    , chatListModel(new ChatListModel())

    // 消息列表
    , conversationsModel(new ConversationsModel())
    , conversationsDelegate(new ConversationsDelegate())

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
    setMouseTracking(true);  // 设置鼠标跟踪

    // 初始化聊天列表
    chatListView = ui->chatList_View;
    chatListView->setModel(chatListModel);
    chatListView->setItemDelegate(chatListDelegate);
    chatListView->setUniformItemSizes(true);

    // 初始化消息列表
    conversationsView = ui->messageListView;
    conversationsView->setModel(conversationsModel);
    conversationsView->setItemDelegate(conversationsDelegate);
    conversationsView->setUniformItemSizes(false);
    conversationsView->setResizeMode(QListView::Adjust);
    conversationsView->setMarginRight(0);
    // 滚动到最后一条消息
    QTimer::singleShot(100, this, [this]() {
        QModelIndex lastIndex = conversationsModel->index(conversationsModel->rowCount() - 1, 0);
        conversationsView->scrollTo(lastIndex, QAbstractItemView::PositionAtBottom);
    });

    connect(conversationsDelegate, &ConversationsDelegate::imageClicked, this, [&](const QPixmap &img){
        qDebug()<<"点击图片";
        if(!mediaDialog) mediaDialog = new MediaDialog();
        mediaDialog->setAttribute(Qt::WA_DeleteOnClose);
        mediaDialog->playPixmap(img);
        mediaDialog->show();
    });
    connect(conversationsDelegate, &ConversationsDelegate::videoClicked, this, [&](const QString &videoPath){
        qDebug()<<"点击视频";
        if(!mediaDialog) mediaDialog = new MediaDialog();
        mediaDialog->setAttribute(Qt::WA_DeleteOnClose);
        // media_Dialog->;
        mediaDialog->show();
    });
    connect(conversationsDelegate, &ConversationsDelegate::fileClicked, this, [&](const QString &filePath){
        qDebug()<<"点击文件";
        bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    });

    //检查信息输入框状态，设置初始样式
    updateSendButtonStyle();
    connect(this->findChild<QTextEdit*>("sendTextEdit"),&QTextEdit::textChanged,
            this,&WeChatWidget::updateSendButtonStyle);

    qApp->installEventFilter(this);



    // --------------------------------------------------------------------------------------------------------------------------
    // 在初始化代码后面添加测试数据
    auto addTestData = [this]() {
        // 获取当前时间
        QDateTime currentTime = QDateTime::currentDateTime();

        // 添加多条测试消息
        conversationsModel->addMessage(ChatMessage(
            MessageType::TEXT,
            "你好！这是一条文本消息",
            "用户A",
            ":/a/image/avatar.jpg",
            currentTime.addSecs(-300)  // 5分钟前
            ));

        conversationsModel->addMessage(ChatMessage(
            MessageType::IMAGE,
            "收到你的消息了，这是一条比较长的回复消息，用来测试消息换行和显示效果，看看界面布局是否正常",
            "用户B",
            "",
            currentTime.addSecs(-240),  // 4分钟前
            {{"path",":/a/image/login.png"}}
            ));

        conversationsModel->addMessage(ChatMessage(
            MessageType::IMAGE,
            "发送一个图片文件",
            "用户A",
            ":/a/image/avatar.jpg",
            currentTime.addSecs(-13330), // 3分钟前
            QVariantMap{{"path", ":/a/image/.jpg"}, {"fileSize", "2.5MB"}}
            ));

        conversationsModel->addMessage(ChatMessage(
            MessageType::TEXT,
            QString("[太阳]莞工图书资源利用入门必备："
                    "1、【查找馆藏图书】：登录图书馆主页（http://opac.lib.dgut.edu.cn/opac/search_adv.php#/index） 或 绑定“东莞理工学院图书馆”微信公众号“微服务”后进行查询。"
                    "2、【图书荐购】：“我的图书馆—东莞理工学院图书馆书目检索系统—读者荐购”栏目中自主荐购或查询征订书目荐购：http://opac.lib.dgut.edu.cn/asord/asord_hist.php。"
                    "[玫瑰]图书荐购且到馆后（该平台的个人中央认证帐号下会提示），[玫瑰]可到图书馆新书展示区（松山湖校区馆2楼，莞城校区馆8楼）找到图书办理借阅手续。[玫瑰]通常，经审核同意荐购的图书，大概1个月左右会到馆（寒暑假除外）。"
                    ),
            "用户A",
            "",
            currentTime.addSecs(-120), // 2分钟前
            QVariantMap{{"duration", 30}, {"path", "/path/to/voice.amr"}}
            ));

        conversationsModel->addMessage(ChatMessage(
            MessageType::VOICE,
            "测试短消息",
            "用户B",
            ":/a/image/avatar.jpg",
            currentTime.addSecs(-60),  // 1分钟前
            QVariantMap{{"duration", 25}}
            ));

        conversationsModel->addMessage(ChatMessage(
            MessageType::TEXT,
            "最后一条测试消息，包含各种特殊字符：@#$%^&*()，以及中文测试",
            "用户B",
            "",
            currentTime  // 当前时间
            ));
        conversationsModel->addMessage(ChatMessage(
            MessageType::FILE,
            "最后一条测试消息，包含各种特殊字符：@#$%^&*()，以及中文测试",
            "用户A",
            "",
            currentTime,  // 当前时间
            {{"path","C:\\Users\\GodPrograms\\Desktop\\项WWWWWWWWWWWWWWWWW目报告.doc.docx"},{"name","项WWWWWWWWWWWWWWWWW目报告.docx"},{"size",1024*1024}}
            ));
        conversationsModel->addMessage(ChatMessage(
            MessageType::VIDEO,
            "最后一条测试消息，包含各种特殊字符：@#$%^&*()，以及中文测试",
            "用户A",
            ":/a/image/login.png",
            currentTime,  // 当前时间
            {{"path","C:\\Users\\GodPrograms\\Downloads\\azh.mp41"},{"thumbnailPath","C:\\Users\\GodPrograms\\Pictures\\Camera Roll\\微信图片_2025-10-11_223555_236.jpg"}}
            ));
        conversationsModel->addMessage(ChatMessage(
            MessageType::VOICE,
            "最后一条测试消息，包含各种特殊字符：@#$%^&*()，以及中文测试",
            "用户A",
            "",
            currentTime,  // 当前时间
            QVariantMap{{"duration",30}}
            ));
    };

    // 调用测试函数
    addTestData();
    addTestData();

    //-----------------------------------------------------------------------------------------------------------------------------

    // 无参匿名函数：内部写死所有测试数据和添加逻辑，调用即执行
    auto addTestChatData = [&]() {
        // 内部定义原add逻辑（参数仍保留，用于内部调用）
        auto add = [&](const QString &name, const QString &msg, const QDateTime &t, int unread, const QString &avatarPath) {
            ConversationsInfo it;
            it.title = name;
            it.lastMsg = msg;
            it.lastTime = t;
            it.unreadCount = unread;
            it.avatar = avatarPath;
            chatListModel->addFriend(it);
        };

        // -------------------------- 所有测试数据（已写死，无需外部传参） --------------------------
        // 注意："fffff"为无效路径，请替换为实际图片路径（如"C:/test_avatars/li4.png"）
        add("张三", "昨天我们讨论的接口我已经改好了，麻烦你 review 下eeeeeaaaaaaaaaaaaaeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee。",
            QDateTime::currentDateTime().addSecs(-3600), 3,
            "C:/test_avatars/zhang3.png");
        add("李四eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", "收到，晚点给你回复。",
            QDateTime::currentDateTime().addDays(-1), 0,
            "fffff");  // 需替换为有效路径
        add("王五", "👍",
            QDateTime::currentDateTime().addDays(-3), 120,
            "C:/test_avatars/wang5.png");
        add("Alice", "See you tomorrow at 10am",
            QDateTime::currentDateTime().addSecs(-60*20), 1,
            "C:/test_avatars/alice.png");

        // 以下为重复测试数据（已完整保留原逻辑）
        add("李四eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", "收到，晚点给你回复。",
            QDateTime::currentDateTime().addDays(-1), 0,
            "C:/test_avatars/li4.png");
        add("王五", "👍",
            QDateTime::currentDateTime().addDays(-3), 120,
            "C:/test_avatars/wang5.png");
        add("Alice", "See you tomorrow at 10am",
            QDateTime::currentDateTime().addSecs(-60*20), 1,
            "C:/test_avatars/alice.png");
        // ...（此处省略其余重复的add调用，完整代码中需保留所有原add语句）
        // 最后一条测试数据
        add("Alice", "See you tomorrow at 10am",
            QDateTime::currentDateTime().addSecs(-60*20), 1,
            "C:/test_avatars/alice.png");
    };

    // 调用方式：直接执行无参匿名函数，一次性添加所有测试数据
    addTestChatData();
    addTestChatData();


    // ----------------------------------------------------------------------------------------------------------------------------







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


