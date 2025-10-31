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

signals:
    void cloesDialog();


protected:
    bool eventFilter(QObject *obj, QEvent *event) override;



private slots:

    void on_pushButton_4_clicked();



private:
    Ui::RightPopover *ui;
};

#endif // RIGHTPOPOVER_H
