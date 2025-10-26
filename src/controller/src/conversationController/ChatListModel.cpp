#include "ChatListModel.h"
#include <QDebug>
#include <QVector>

ChatListModel::ChatListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_conversations.size();
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_conversations.size())
        return QVariant();

    const ConversationInfo &conversation = m_conversations.at(index.row());

    switch (role) {
    case ConversationIdRole:
        return conversation.conversationId;
    case GroupIdRole:
        return conversation.groupId;
    case UserIdRole:
        return conversation.userId;
    case TypeRole:
        return conversation.type;
    case TitleRole:
        return conversation.title;
    case AvatarRole:
        return conversation.avatar;
    case AvatarLocalPathRole:
        return conversation.avatarLocalPath;
    case LastMessageContentRole:
        return conversation.lastMessageContent;
    case LastMessageTimeRole:
        return conversation.lastMessageTime;
    case UnreadCountRole:
        return conversation.unreadCount;
    case IsTopRole:
        return conversation.isTop;
    case IsGroupRole:
        return conversation.isGroup();
    case TargetIdRole:
        return conversation.targetId();
    default:
        return QVariant();
    }
}

bool ChatListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_conversations.size())
        return false;

    ConversationInfo &conversation = m_conversations[index.row()];
    bool changed = false;

    switch (role) {
    case TitleRole:
        if (conversation.title != value.toString()) {
            conversation.title = value.toString();
            changed = true;
        }
        break;
    case AvatarRole:
        if (conversation.avatar != value.toString()) {
            conversation.avatar = value.toString();
            changed = true;
        }
        break;
    case AvatarLocalPathRole:
        if (conversation.avatarLocalPath != value.toString()) {
            conversation.avatarLocalPath = value.toString();
            changed = true;
        }
        break;
    case LastMessageContentRole:
        if (conversation.lastMessageContent != value.toString()) {
            conversation.lastMessageContent = value.toString();
            changed = true;
        }
        break;
    case LastMessageTimeRole:
        if (conversation.lastMessageTime != value.toLongLong()) {
            conversation.lastMessageTime = value.toLongLong();
            changed = true;
        }
        break;
    case UnreadCountRole:
        if (conversation.unreadCount != value.toInt()) {
            conversation.unreadCount = value.toInt();
            changed = true;
        }
        break;
    case IsTopRole:
        if (conversation.isTop != value.toBool()) {
            conversation.isTop = value.toBool();
            changed = true;
        }
        break;
    default:
        return false;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ConversationIdRole] = "conversationId";
    roles[GroupIdRole] = "groupId";
    roles[UserIdRole] = "userId";
    roles[TypeRole] = "type";
    roles[TitleRole] = "title";
    roles[AvatarRole] = "avatar";
    roles[AvatarLocalPathRole] = "avatarLocalPath";
    roles[LastMessageContentRole] = "lastMessageContent";
    roles[LastMessageTimeRole] = "lastMessageTime";
    roles[UnreadCountRole] = "unreadCount";
    roles[IsTopRole] = "isTop";
    roles[IsGroupRole] = "isGroup";
    roles[TargetIdRole] = "targetId";
    return roles;
}

void ChatListModel::addConversation(const ConversationInfo &conversationInfo)
{
    // 检查是否已存在
    int existingIndex = findConversationIndex(conversationInfo.conversationId);
    if (existingIndex != -1) {
        // 更新现有会话
        m_conversations[existingIndex] = conversationInfo;
        QModelIndex index = createIndex(existingIndex, 0);
        emit dataChanged(index, index);
        return;
    }

    // 插入新会话
    beginInsertRows(QModelIndex(), m_conversations.size(), m_conversations.size());
    m_conversations.append(conversationInfo);
    endInsertRows();
}

void ChatListModel::updateLastMessage(qint64 conversationId, const QString &message, const qint64 &time)
{
    int index = findConversationIndex(conversationId);
    if (index != -1) {
        ConversationInfo &conversation = m_conversations[index];
        conversation.lastMessageContent = message;
        conversation.lastMessageTime = time;

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {LastMessageContentRole, LastMessageTimeRole});
    }
}

void ChatListModel::updateUnreadCount(qint64 conversationId, int count)
{
    int index = findConversationIndex(conversationId);
    if (index != -1) {
        ConversationInfo &conversation = m_conversations[index];
        conversation.unreadCount = count;

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {UnreadCountRole});
    }
}

void ChatListModel::updateTopStatus(qint64 conversationId, bool isTop)
{
    int index = findConversationIndex(conversationId);
    if (index != -1) {
        ConversationInfo &conversation = m_conversations[index];
        conversation.isTop = isTop;

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {IsTopRole});
    }
}

ConversationInfo ChatListModel::getConversationInfo(qint64 conversationId) const
{
    int index = findConversationIndex(conversationId);
    if (index != -1) {
        return m_conversations.at(index);
    }
    return ConversationInfo();
}

ConversationInfo ChatListModel::getConversationInfoAt(int index) const
{
    if (index >= 0 && index < m_conversations.size()) {
        return m_conversations.at(index);
    }
    return ConversationInfo();
}

void ChatListModel::removeConversation(qint64 conversationId)
{
    int index = findConversationIndex(conversationId);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_conversations.removeAt(index);
        endRemoveRows();
    }
}

void ChatListModel::clearAll()
{
    if (!m_conversations.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_conversations.size() - 1);
        m_conversations.clear();
        endRemoveRows();
    }
}


int ChatListModel::findConversationIndex(qint64 conversationId) const
{
    for (int i = 0; i < m_conversations.size(); ++i) {
        if (m_conversations.at(i).conversationId == conversationId) {
            return i;
        }
    }
    return -1;
}
