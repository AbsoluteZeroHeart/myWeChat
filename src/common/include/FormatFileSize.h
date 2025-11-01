#ifndef FORMATFILESIZE_H
#define FORMATFILESIZE_H

#include <QString>

inline QString formatFileSize(qint64 bytes) {
    if (bytes == 0) return "0 B";

    const QString units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unitIndex = 0;
    double size = bytes;

    // 计算合适的单位
    while (size >= 1024.0 && unitIndex < 5) {
        size /= 1024.0;
        unitIndex++;
    }

    // 格式化输出，不同单位使用不同精度
    if (unitIndex == 0) {
        return QString("%1 %2").arg(size).arg(units[unitIndex]); // 字节不需要小数
    } else if (unitIndex <= 2) { // KB, MB 保留1位小数
        return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
    } else { // GB, TB, PB 保留2位小数
        return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
    }
}

#endif // FORMATFILESIZE_H
