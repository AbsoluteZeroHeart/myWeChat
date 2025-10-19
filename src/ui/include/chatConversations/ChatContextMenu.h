// ChatContextMenu.h
#ifndef CHATCONTEXTMENU_H
#define CHATCONTEXTMENU_H

#include <QMenu>
#include <QAction>

class ChatContextMenu : public QMenu {
    Q_OBJECT

public:
    explicit ChatContextMenu(QWidget* parent = nullptr);

    enum ActionType {
        Copy,
        Zoom,
        Translate,
        Search,
        Forward,
        Favorite,
        Remind,
        MultiSelect,
        Quote,
        Pin,
        Delete
    };


    // 设置当前关联的数据
    void setCurrentData(const QVariant& data);

    // 重写 exec 方法，支持传入数据
    QAction* exec(const QPoint& pos, const QVariant& data = QVariant());

signals:
    void actionTriggered(ActionType action, const QVariant& data);

private slots:
    void onActionTriggered();

private:
    void setupActions();

    QAction* m_copyAction;
    QAction* m_zoomAction;
    QAction* m_translateAction;
    QAction* m_searchAction;
    QAction* m_forwardAction;
    QAction* m_favoriteAction;
    QAction* m_remindAction;
    QAction* m_multiSelectAction;
    QAction* m_quoteAction;
    QAction* m_pinAction;
    QAction* m_deleteAction;

    QVariant m_currentData;
};

#endif // CHATCONTEXTMENU_H
