#ifndef USERINFOWIDGET_H
#define USERINFOWIDGET_H

#include <QWidget>

namespace Ui {
class UserInfoWidget;
}

class UserInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoWidget(QWidget *parent = nullptr);
    ~UserInfoWidget();

private:
    Ui::UserInfoWidget *ui;
};

#endif // USERINFOWIDGET_H
