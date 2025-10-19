#ifndef CLICKCLOSEPOPUP_H
#define CLICKCLOSEPOPUP_H

#include <QDialog>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

class ClickClosePopup : public QDialog
{
    Q_OBJECT

public:
    explicit ClickClosePopup(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        // 设置无边框
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

        // 启用透明背景
        setAttribute(Qt::WA_TranslucentBackground);
        
        // 确保关闭时自动删除
        setAttribute(Qt::WA_DeleteOnClose);
        
        // 安装事件过滤器以捕获应用程序范围内的点击事件
        qApp->installEventFilter(this);
        
    }

    virtual ~ClickClosePopup()
    {
        qApp->removeEventFilter(this);
    }

    // 显示在指定位置，确保不超出屏幕边界
    void showAtPos(const QPoint &pos)
    {
        // 确保弹窗不会超出屏幕边界
        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        QPoint adjustedPos = pos;
        
        if (adjustedPos.x() + width() > screenGeometry.right()) {
            adjustedPos.setX(screenGeometry.right() - width());
        }
        if (adjustedPos.y() + height() > screenGeometry.bottom()) {
            adjustedPos.setY(screenGeometry.bottom() - height());
        }
        
        move(adjustedPos);
        show();
    }

protected:
    // 事件过滤器 - 处理点击外部关闭
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // 如果点击的是弹窗外部区域，关闭弹窗
            if (isVisible() && !geometry().contains(mouseEvent->globalPosition().toPoint())) {
                close();
                return true;
            }
        }
        
        return QDialog::eventFilter(obj, event);
    }

    // 绘制默认的白色圆角背景
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        painter.fillRect(rect(), Qt::transparent);
        QPen pen(QColor(160, 160, 160), 1);
        painter.setPen(pen);
        painter.setBrush(QColor(255, 255, 255));
        
        QRect contentRect(1, 1, width()-2, height()-2);
        painter.drawRoundedRect(contentRect, 6, 6);
    }

    // 支持Esc键关闭
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape) {
            close();
            return;
        }
        QDialog::keyPressEvent(event);
    }
};

#endif // CLICKCLOSEPOPUP_H
