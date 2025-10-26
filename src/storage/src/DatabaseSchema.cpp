#include "DatabaseSchema.h"

// 数据库表名常量定义
const char* DatabaseSchema::TABLE_CURRENT_USER = "users";
const char* DatabaseSchema::TABLE_CONTACTS = "contacts";
const char* DatabaseSchema::TABLE_GROUP_MEMBERS = "group_members";
const char* DatabaseSchema::TABLE_GROUPS = "groups";
const char* DatabaseSchema::TABLE_CONVERSATIONS = "conversations";
const char* DatabaseSchema::TABLE_MESSAGES = "messages";
const char* DatabaseSchema::TABLE_MEDIA_CACHE = "media_cache";

/**
 * @brief 获取创建"当前用户表"的SQL语句
 * 存储当前登录用户的核心信息，user_id使用INTEGER支持雪花算法
 */
QString DatabaseSchema::getCreateTableCurrentUser() {
    return R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER NOT NULL PRIMARY KEY,    -- 用户ID
            account TEXT NOT NULL,                   -- 用户账号
            nickname TEXT NOT NULL,                  -- 用户昵称
            avatar TEXT,                             -- 头像远程URL
            avatar_local_path TEXT,                  -- 头像本地缓存路径
            gender INTEGER DEFAULT 0,                -- 性别（0:未知 1:男 2:女）
            region TEXT,                             -- 地区（如"中国-北京"）
            signature TEXT,                          -- 个性签名
            is_current INTEGER DEFAULT 0             -- 标记当前登录用户
        )
    )";
}

/**
 * @brief 获取创建"联系人表"的SQL语句
 * 联系人ID（user_id）使用INTEGER，关联用户表的雪花ID
 */
QString DatabaseSchema::getCreateTableContacts() {
    return R"(
        CREATE TABLE IF NOT EXISTS contacts (
            user_id INTEGER NOT NULL PRIMARY KEY,  -- 联系人ID
            remark_name TEXT,                      -- 备注名
            description TEXT,                      -- 描述信息
            tags TEXT,                             -- 标签（JSON）
            phone_note TEXT,                       -- 记录的电话
            email_note TEXT,                       -- 记录的邮箱
            source TEXT,                           -- 添加来源
            is_starred INTEGER DEFAULT 0,          -- 是否星标
            is_blocked INTEGER DEFAULT 0,          -- 是否拉黑
            add_time INTEGER,                      -- 添加时间戳

            -- 外键关联用户表
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
        )
    )";
}

/**
 * @brief 获取创建"群成员表"的SQL语句
 * 群ID和用户ID均为INTEGER，支持雪花算法，复合主键关联群组和用户
 */
QString DatabaseSchema::getCreateTableGroupMembers() {
    return R"(
        CREATE TABLE IF NOT EXISTS group_members (
            group_id INTEGER NOT NULL,                   -- 群ID
            user_id INTEGER NOT NULL,                    -- 用户ID

            nickname TEXT NOT NULL,                      -- 群内昵称
            role INTEGER DEFAULT 0,                      -- 群内角色（0:普通 1:管理员 2:群主）
            join_time INTEGER,                           -- 加入时间戳
            is_contact INTEGER DEFAULT 0,                -- 是否为当前用户的联系人

            -- 复合主键：群ID+用户ID唯一标识群成员
            PRIMARY KEY (group_id, user_id),

            -- 外键关联
            FOREIGN KEY (group_id) REFERENCES groups(group_id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
        )
    )";
}

/**
 * @brief 获取创建"群组表"的SQL语句
 * 群ID（group_id）使用INTEGER支持雪花算法
 */
QString DatabaseSchema::getCreateTableGroups() {
    return R"(
        CREATE TABLE IF NOT EXISTS groups (
            group_id INTEGER PRIMARY KEY,                -- 群ID
            group_name TEXT NOT NULL,                    -- 群名称
            avatar TEXT,                                 -- 群头像远程URL
            avatar_local_path TEXT,                      -- 群头像本地缓存路径
            announcement TEXT,                           -- 群公告
            max_members INTEGER DEFAULT 500,             -- 最大成员限制
            group_note TEXT                             -- 群备注
        )
    )";
}

/**
 * @brief 获取创建"会话表"的SQL语句
 * 会话ID、群ID、用户ID均为INTEGER，支持雪花算法，外键关联一致
 */
QString DatabaseSchema::getCreateTableConversations() {
    return R"(
        CREATE TABLE IF NOT EXISTS conversations (
            conversation_id INTEGER PRIMARY KEY,         -- 会话ID
            group_id INTEGER,                            -- 群聊目标ID
            user_id INTEGER,                             -- 用户目标ID
            type INTEGER NOT NULL,                       -- 会话类型（0:单聊 1:群聊）

            -- 冗余字段（列表展示时快速查询）
            title TEXT,                                  -- 会话标题
            avatar TEXT,                                 -- 会话头像URL
            avatar_local_path TEXT,                      -- 头像本地路径
            last_message_content TEXT,                   -- 最后一条消息内容
            last_message_time INTEGER,                   -- 最后一条消息时间戳

            unread_count INTEGER DEFAULT 0,              -- 未读数量
            is_top INTEGER DEFAULT 0,                    -- 是否置顶

            -- 外键关联
            FOREIGN KEY (group_id) REFERENCES groups(group_id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES contacts(user_id) ON DELETE CASCADE
        )
    )";
}

/**
 * @brief 获取创建"消息表"的SQL语句
 * 消息ID、会话ID、发送者ID均为INTEGER，支持雪花算法（高频生成场景适配）
 */
QString DatabaseSchema::getCreateTableMessages() {
    return R"(
        CREATE TABLE IF NOT EXISTS messages (
            message_id INTEGER PRIMARY KEY,              -- 消息ID
            conversation_id INTEGER NOT NULL,            -- 所属会话ID
            sender_id INTEGER NOT NULL,                  -- 发送者用户ID

            type INTEGER NOT NULL,                       -- 消息类型（0:文本 1:图片 2：视频 3：文件 4：语音）
            content TEXT,                                -- 消息内容
            file_path TEXT,                              -- 媒体本地路径
            file_url TEXT,                               -- 媒体远程URL
            file_size INTEGER,                           -- 媒体大小（字节）
            duration INTEGER,                            -- 音视频时长（秒）
            thumbnail_path TEXT,                         -- 缩略图路径
            msg_time INTEGER,                            -- 发送/接收时间戳

            -- 外键关联
            FOREIGN KEY (conversation_id) REFERENCES conversations(conversation_id) ON DELETE CASCADE,
            FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE RESTRICT
        )
    )";
}

/**
 * @brief 获取创建"媒体缓存表"的SQL语句
 * 本地缓存ID保持自增INTEGER（无需雪花算法）
 */
QString DatabaseSchema::getCreateTableMediaCache() {
    return R"(
        CREATE TABLE IF NOT EXISTS media_cache (
            cache_id INTEGER PRIMARY KEY AUTOINCREMENT,  -- 本地自增ID
            file_path TEXT UNIQUE NOT NULL,              -- 本地文件路径（唯一）
            file_type INTEGER NOT NULL,                  -- 文件类型（0:图片 1:语音等）
            original_url TEXT,                           -- 原始URL
            file_size INTEGER,                           -- 文件大小
            access_count INTEGER DEFAULT 0,              -- 访问次数
            last_access_time INTEGER,                    -- 最后访问时间
            created_time INTEGER                         -- 创建时间
        )
    )";
}

/**
 * @brief 获取创建数据库索引的SQL语句
 * 索引适配INTEGER类型字段，查询性能更优
 */
QString DatabaseSchema::getCreateIndexes() {
    return R"(
        -- 用户表索引：确保只有一个当前用户
        CREATE UNIQUE INDEX IF NOT EXISTS idx_users_current ON users(is_current) WHERE is_current = 1;

        -- 联系人表索引
        CREATE INDEX IF NOT EXISTS idx_contacts_is_starred ON contacts(is_starred);
        CREATE INDEX IF NOT EXISTS idx_contacts_last_contact ON contacts(last_contact_time);
        CREATE INDEX IF NOT EXISTS idx_contacts_tags ON contacts(tags);

        -- 群成员表索引（适配INTEGER类型group_id/user_id）
        CREATE INDEX IF NOT EXISTS idx_group_members_user_id ON group_members(user_id);

        -- 会话表索引（按时间排序更高效，INTEGER比TEXT排序快）
        CREATE INDEX IF NOT EXISTS idx_conversations_last_time ON conversations(last_message_time);
        CREATE INDEX IF NOT EXISTS idx_conversations_top_time ON conversations(is_top, last_message_time);

        -- 消息表核心索引（INTEGER类型联合索引性能最优）
        CREATE INDEX IF NOT EXISTS idx_messages_conv_time ON messages(conversation_id, msg_time);

        -- 媒体缓存表索引
        CREATE INDEX IF NOT EXISTS idx_media_original_url ON media_cache(original_url);
        CREATE INDEX IF NOT EXISTS idx_media_access ON media_cache(last_access_time, access_count);
    )";
}
