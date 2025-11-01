#include "ChatMessageListView.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QPushButton>
#include "ClickClosePopup.h"
#include <QVBoxLayout>
#include <QLabel>

ChatMessageListView::ChatMessageListView(QWidget *parent)
    : CustomListView(parent)
{
    createMessageContextMenu();
}

ChatMessageListView::~ChatMessageListView()
{
    delete m_messageMenu;
}

void ChatMessageListView::wheelEvent(QWheelEvent *event)
{
    CustomListView::wheelEvent(event);

    QPoint angleDelta = event->angleDelta();

    if (!angleDelta.isNull() && angleDelta.y() > 0) {

        // 检查是否滚动到顶部附近
        QScrollBar *scrollBar = this->verticalScrollBar();
        int scrollPos = scrollBar->value();

        if (scrollPos < 200) {
            emit loadmoreMsg(5);
        }
    }

    event->accept();
}

void ChatMessageListView::createMessageContextMenu()
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
    m_messageMenu->addSeparator();
    m_messageMenu->addAction(m_deleteAction);

    // 连接信号
    connect(m_copyAction, &QAction::triggered, this, [this](){
        emit messageCopy(currentMsg);
    });
    connect(m_zoomAction, &QAction::triggered, this, &ChatMessageListView::messageZoom);
    connect(m_translateAction, &QAction::triggered, this, &ChatMessageListView::messageTranslate);
    connect(m_searchAction, &QAction::triggered, this, &ChatMessageListView::messageSearch);
    connect(m_forwardAction, &QAction::triggered, this, &ChatMessageListView::messageForward);
    connect(m_favoriteAction, &QAction::triggered, this, &ChatMessageListView::messageFavorite);
    connect(m_remindAction, &QAction::triggered, this, &ChatMessageListView::messageRemind);
    connect(m_multiSelectAction, &QAction::triggered, this, [this](){
        // this->setSelectionMode(QAbstractItemView::ExtendedSelection);
        emit messageMultiSelect();
    });
    connect(m_quoteAction, &QAction::triggered, this, &ChatMessageListView::messageQuote);
    connect(m_deleteAction, &QAction::triggered, this, [this](){
        showDeleteConfirmationDialog();
    });

}

void ChatMessageListView::showDeleteConfirmationDialog()
{
    // 创建自定义确认对话框
    ClickClosePopup* confirmationDialog = new ClickClosePopup(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(confirmationDialog);

    // 添加提示文本
    QLabel* messageLabel = new QLabel("删除该消息？");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px; color: #333333; "
                                "padding: 10px; border: none; background-color: transparent;");
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* deleteButton = new QPushButton("删除");
    QPushButton* cancelButton = new QPushButton("取消");

    // 设置按钮样式
    deleteButton->setStyleSheet(R"(
        QPushButton {
            background-color: #ff4444;
            color: white;
            border: none;
            border-radius: 4px;
            min-width: 80px;
            min-height: 30px;
            font-family: "Microsoft YaHei";
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #cc3333;
        }
        QPushButton:pressed {
            background-color: #aa2222;
        }
    )");

    cancelButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f0f0f0;
            color: #333333;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            min-width: 80px;
            min-height: 30px;
            font-family: "Microsoft YaHei";
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
        }
    )");

    // 设置按钮布局
    buttonLayout->addStretch();
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    // 设置主布局
    mainLayout->addWidget(messageLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    // 连接按钮信号
    connect(deleteButton, &QPushButton::clicked, confirmationDialog, [this, confirmationDialog]() {
        confirmationDialog->close();
        emit messageDelete(currentMsg);
    });

    connect(cancelButton, &QPushButton::clicked, confirmationDialog, &ClickClosePopup::close);

    // 显示对话框
    confirmationDialog->show();
    confirmationDialog->adjustSize();
}

void ChatMessageListView::execMessageListMenu(const QPoint& globalPos, const Message &message)
{
    currentMsg = message;
    m_messageMenu->exec(globalPos);
}



