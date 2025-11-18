#include "LoginAndRegisterDialog.h"
#include "ui_LoginAndRegisterDialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>
#include <QStackedWidget>
#include <QAbstractButton>
#include "LoginAndRegisterController.h"


LoginAndRegisterDialog::LoginAndRegisterDialog(LoginAndRegisterController *loginAndRegisterController, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginAndRegisterDialog)
    , m_isDragging(false)
{
    ui->setupUi(this);

    // 窗口样式
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    background.load(":/a/image/login.png");

    // 为界面上的所有按钮安装 event filter（用于修改鼠标指针）
    const QList<QAbstractButton*> allButtons = findChildren<QAbstractButton*>();
    for (QAbstractButton* button : allButtons) {
        button->installEventFilter(this);
    }

}

LoginAndRegisterDialog::~LoginAndRegisterDialog()
{
    delete ui;
}

void LoginAndRegisterDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = QRect(1, 1, width() - 2, height() - 2);

    // 裁剪为圆角区域（这样背景图和内容区域都受裁剪）
    QPainterPath path;
    path.addRoundedRect(rect, 6, 6);
    painter.setClipPath(path);

    if (!background.isNull()) {
        painter.drawPixmap(rect, background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        painter.fillRect(rect, Qt::white);
    }

    // 画边框（取消裁剪以确保边框完整显示）
    painter.setClipping(false);
    QPen pen(QColor(190, 190, 190), 1);
    painter.setPen(pen);
    painter.drawRoundedRect(rect, 6, 6);
}

void LoginAndRegisterDialog::mousePressEvent(QMouseEvent *event)
{
    // 允许拖动：限制为窗口顶部区域
    if (event->button() == Qt::LeftButton && event->pos().y() <= 45) {
        m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_isDragging = true;
        event->accept();
        setCursor(Qt::ClosedHandCursor);
        return; // 吞掉，防止传递给子控件
    }
    QDialog::mousePressEvent(event);
}

void LoginAndRegisterDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStartPosition);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void LoginAndRegisterDialog::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_isDragging = false;
    setCursor(Qt::ArrowCursor);
    QDialog::mouseReleaseEvent(event);
}

bool LoginAndRegisterDialog::eventFilter(QObject *watched, QEvent *event)
{
    // 只针对按钮的 Enter/Leave 改变指针，不吞掉事件（返回 false 以保留其他默认行为）
    QAbstractButton* button = qobject_cast<QAbstractButton*>(watched);
    if (button) {
        if (event->type() == QEvent::Enter) {
            button->setCursor(Qt::PointingHandCursor);
            // 不要 return true，防止阻止后续事件处理
            return false;
        } else if (event->type() == QEvent::Leave) {
            button->setCursor(Qt::ArrowCursor);
            return false;
        }
    }
    return QDialog::eventFilter(watched, event);
}


void LoginAndRegisterDialog::on_closeToolButton_clicked()
{
    close();
}


void LoginAndRegisterDialog::on_SMSLoginButton_clicked()
{
    ui->SMSLoginButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: rgb(7, 207, 100); font: 13pt '微软雅黑'; }");
    ui->passwordLoginButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: rgb(10, 10, 10); font: 13pt '微软雅黑'; }");
    ui->inputStackedWidget->setCurrentIndex(1);
}


void LoginAndRegisterDialog::on_passwordLoginButton_clicked()
{
    ui->passwordLoginButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: rgb(7, 207, 100); font: 13pt '微软雅黑'; }");
    ui->SMSLoginButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: rgb(10, 10, 10); font: 13pt '微软雅黑'; }");
    ui->inputStackedWidget->setCurrentIndex(0);
}

void LoginAndRegisterDialog::on_registerButton_clicked()
{
    on_SMSLoginButton_clicked();
}


void LoginAndRegisterDialog::on_wechatNumButton_clicked()
{
    ui->inputStackedWidget->setCurrentIndex(2);
}


void LoginAndRegisterDialog::on_registerButton_2_clicked()
{
    on_SMSLoginButton_clicked();
}


void LoginAndRegisterDialog::on_LoginOrRegister_clicked()
{
    accept();
}


void LoginAndRegisterDialog::on_loginButton1_clicked()
{
    accept();
}


void LoginAndRegisterDialog::on_loginButton2_clicked()
{
    accept();
}

