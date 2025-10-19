#include "Chatlistmodel.h"
#include <QVector>


ChatListModel::ChatListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return conversationsList.size();
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row()>=conversationsList.size())
        return QVariant();

    const ConversationsInfo &conversationsInfo = conversationsList[index.row()];

    //根据绝色返回对应数据
    switch(role){
    case IdRole:                 return conversationsInfo.id;
    case TitleRole:               return conversationsInfo.title;
    case AvatarRole:             return conversationsInfo.avatar;
    case LastMsgRole:            return conversationsInfo.lastMsg;
    case LastTimeRole:           return conversationsInfo.lastTime;
    case UnreadCountRole:        return conversationsInfo.unreadCount;
    case IsGroupRole:            return conversationsInfo.isGroup;
    default:                     return QVariant();
    }
}


bool ChatListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || index.row()>=conversationsList.size())
        return false;
    ConversationsInfo &conversationsInfo = conversationsList[index.row()];

    //根据角色更新数据
    switch(role) {
    case TitleRole:               conversationsInfo.title = value.toString(); break;
    case LastMsgRole:            conversationsInfo.lastMsg = value.toString(); break;
    case LastTimeRole:           conversationsInfo.lastTime = value.toDateTime(); break;
    case UnreadCountRole:        conversationsInfo.unreadCount = value.toInt();break;
    default: return false;
    }
    //通知视图数据已更改
    emit dataChanged(index, index, {role});
    return true;
}


QHash<int, QByteArray> ChatListModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[AvatarRole] = "avatar";
    roles[LastMsgRole] = "lastMsg";
    roles[LastTimeRole] = "lastTime";
    roles[UnreadCountRole] = "unreadCount";
    roles[IsGroupRole] = "isGroup";
    return roles;
}

void ChatListModel::addFriend(const ConversationsInfo &conversationsInfo)
{
    beginInsertRows(QModelIndex(),0,0);
    conversationsList.insert(0,conversationsInfo);
    endInsertRows();
}

void ChatListModel::updateLastMessage(const QString &conversationsId, const QString & msg, const QString &time)
{
    for(int i = 0; i < conversationsList.size(); ++i){
        if(conversationsList[i].id == conversationsId){
            QModelIndex index = createIndex(i,0);
            setData(index, msg, LastMsgRole);
            setData(index, time, LastTimeRole);
        }
    }
}


void ChatListModel::updateUnreadCount(const QString &conversationsId, int count)
{
    for(int i = 0; i < conversationsList.size(); ++i){
        if(conversationsList[i].id == conversationsId){
            QModelIndex index = createIndex(i,0);
            setData(index, count, UnreadCountRole);
            break;
        }
    }
}


ConversationsInfo ChatListModel::getConversationsInfo(const QString &ConversationsId)const
{
    for(const auto &info : conversationsList){
        if(info.id == ConversationsId){
            return info;
        }
    }
    return ConversationsInfo();
}














