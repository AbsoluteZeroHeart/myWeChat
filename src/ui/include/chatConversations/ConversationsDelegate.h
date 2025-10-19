#ifndef CONVERSATIONSDELEGATE_H
#define CONVERSATIONSDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include "ChatMessage.h"

class ConversationsDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ConversationsDelegate(QObject* parent = nullptr);

    // QStyledItemDelegate interface
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const
                     QModelIndex& index) override;

signals:
    void imageClicked(const QPixmap imagePixmap);
    void videoClicked(const QString videoPath);
    void fileClicked(const QString& filePath);
    void voiceClicked(const QString& voicePath);

private:
    void paintTextMessage(QPainter* painter, const QStyleOptionViewItem& option,
                          const ChatMessage& message) const;

    void paintImageMessage(QPainter* painter, const QStyleOptionViewItem& option,
                           const ChatMessage& message) const;

    void paintVideoMessage(QPainter* painter, const QStyleOptionViewItem& option,
                           const ChatMessage& message) const;

    void paintFileMessage(QPainter* painter, const QStyleOptionViewItem& option,
                          const ChatMessage& message) const;

    void paintVoiceMessage(QPainter* painter, const QStyleOptionViewItem& option,
                           const ChatMessage& message) const;

    void paintFileIcon(QPainter* painter, const QRect& rect,
                       const QString& extension) const;

    void paintAvatar(QPainter* painter, const QRect& avatarRect,
                     const ChatMessage& message) const;

    void paintDurationText(QPainter* painter, const QRect& bubbleRect,
                                                int duration, bool isOwnMessage) const;

    void paintPlayButtonAndWaveform(QPainter* painter, const QRect& bubbleRect,
                                    bool isOwnMessage) const ;

    void paintVoiceWaveform(QPainter* painter, const QRect& rect,bool isOwnMessage) const ;

    void paintTime(QPainter *painter, const QRect &Rect,const QStyleOptionViewItem& option,
                   ChatMessage msg, bool isOwnMessage)const;

    QString getFileExtension(const QString& fileName) const;
    QString formatFileSize(qint64 bytes) const;
    QSize calculateTextSize(const QString& text, const QFont& font, int maxWidth) const;
    QRect getClickableRect(const QStyleOptionViewItem& option, const ChatMessage& message) const;

};

#endif // CONVERSATIONSDELEGATE_H
