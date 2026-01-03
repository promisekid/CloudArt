/**
 * @file DataModels.h
 * @brief 核心数据模型定义
 */

#pragma once
#include <QString>
#include <QDateTime>

// 消息发送者角色 (对应数据库里的 role 字段)
enum class MessageRole {
    User, // 用户
    AI    // AI
};

// 会话数据结构
struct SessionData {
    int id = -1;                // 数据库ID
    QString name;               // 会话标题
    qint64 createdAt = 0;       // 创建时间戳

    // 1. 默认构造函数 (必须有)
    SessionData() {}

    // 2. 便捷构造函数 (方便代码里一行初始化)
    SessionData(int _id, const QString& _name)
        : id(_id), name(_name), createdAt(QDateTime::currentMSecsSinceEpoch()) {}
};

// 消息数据结构
struct MessageData {
    int id = -1;
    int sessionId = -1;         // 外键
    MessageRole role = MessageRole::User;

    QString text;               // 文字内容
    QString imagePath;          // 本地图片路径 (如果是文字则为空)

    qint64 timestamp = 0;       // 时间

    // 辅助函数：判断是否是图片
    bool isImage() const { return !imagePath.isEmpty(); }

    // 1. 默认构造函数 (必须有，用于 QSqlQuery 读取时先创建空对象)
    MessageData() {}

    // 2. 【关键修复】便捷构造函数 (DatabaseManager.cpp 第192行用的就是这个)
    // 参数: 会话ID, 角色, 文本内容, 图片路径(默认空)
    MessageData(int sid, MessageRole r, const QString& t, const QString& img = "")
        : sessionId(sid), role(r), text(t), imagePath(img)
    {
        // 自动填入当前时间
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
};
