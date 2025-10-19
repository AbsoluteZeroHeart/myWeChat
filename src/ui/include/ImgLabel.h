#ifndef IMGLABEL_H
#define IMGLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>

class ImgLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImgLabel(QWidget *parent = nullptr, int radius = 6);

    // 设置圆角半径
    void setRadius(int radius);
    // 重写设置Pixmap的方法
    void setPixmap(const QPixmap &pixmap);

signals:
    // 点击信号，传递当前显示的pixmap
    void labelClicked(const QPixmap &pixmap);

protected:
    // 重写鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;
    // 重写绘制事件，实现圆角显示
    void paintEvent(QPaintEvent *event) override;

private:
    int m_radius;         // 圆角半径
    QPixmap m_pixmap;     // 存储要显示的图片
};

#endif // IMGLABEL_H
