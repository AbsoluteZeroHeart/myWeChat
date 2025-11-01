#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QTextEdit>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QMimeData>
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

struct FileItem {
    QString filePath;
    QString fileName;
    bool isImage;
    QImage thumbnail; // 用于图片的缩略图
};

class MessageTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit MessageTextEdit(QWidget *parent = nullptr);
    ~MessageTextEdit();
    // 插入图片
    void insertImage(const QString &imagePath);
    // 插入文件
    void insertFile(const QString &filePath);
    // 插入多个文件
    void insertFiles(const QStringList &filePaths);
    // 获取所有文件项
    QList<FileItem> getFileItems() const;
    // 清空所有内容
    void clearContent();

protected:
    // 重写粘贴事件
    void keyPressEvent(QKeyEvent *event) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;

private:
    // 文件项列表
    QList<FileItem> m_fileItems;

    // 创建图片缩略图
    QImage createThumbnail(const QImage &image);
    // 创建文件图标
    QImage createFileIcon(const QString &fileName, const QString &filePath);
    QString getFileExtension(const QString &fileName) const;
    void paintFileIcon(QPainter *painter, const QRect &fileRect, const QString &extension) const;


    // 插入文件到文档
    void insertFileToDocument(const FileItem &fileItem);
    // 处理剪贴板图片
    void handleClipboardImage();


};

#endif // MESSAGETEXTEDIT_H
