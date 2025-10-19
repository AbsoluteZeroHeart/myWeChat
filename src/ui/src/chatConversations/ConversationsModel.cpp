#include "ConversationsModel.h"

ConversationsModel::ConversationsModel(QObject* parent)
    : QAbstractListModel(parent) {
}

int ConversationsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_messages.size();
}

QVariant ConversationsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const ChatMessage& message = m_messages.at(index.row());

    switch (role) {
    case Qt::UserRole: // 返回完整的消息对象
        return QVariant::fromValue(message);
    case Qt::DisplayRole: // 显示用文本
        return QString("[%1] %2: %3").arg(
            message.timestamp().toString("hh:mm"),
            message.sender(),
            message.content().left(50) // 预览前50个字符
            );
    default:
        return QVariant();
    }
}

bool ConversationsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_messages.size())
        return false;

    if (role == Qt::EditRole) {
        // 可以在这里实现消息编辑
        return true;
    }

    return false;
}

Qt::ItemFlags ConversationsModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    if (index.isValid()) {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return flags;
}

void ConversationsModel::addMessage(const ChatMessage& message) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();
}

void ConversationsModel::insertMessage(int row, const ChatMessage& message) {
    if (row < 0 || row > m_messages.size()) return;

    beginInsertRows(QModelIndex(), row, row);
    m_messages.insert(row, message);
    endInsertRows();
}

void ConversationsModel::removeMessage(int row) {
    if (row < 0 || row >= m_messages.size()) return;

    beginRemoveRows(QModelIndex(), row, row);
    m_messages.removeAt(row);
    endRemoveRows();
}

ChatMessage ConversationsModel::getMessage(int row) const {
    if (row >= 0 && row < m_messages.size())
        return m_messages.at(row);
    return ChatMessage(MessageType::TEXT, "", "","", QDateTime());
}

void ConversationsModel::clearAll() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
