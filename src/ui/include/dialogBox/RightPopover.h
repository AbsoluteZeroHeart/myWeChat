#ifndef RIGHTPOPOVER_H
#define RIGHTPOPOVER_H

#include <QWidget>

namespace Ui {
class RightPopover;
}

class RightPopover : public QWidget
{
    Q_OBJECT

public:
    explicit RightPopover(QWidget *parent = nullptr);
    ~RightPopover();

private slots:

    void on_pushButton_4_clicked();

private:
    Ui::RightPopover *ui;
};

#endif // RIGHTPOPOVER_H
