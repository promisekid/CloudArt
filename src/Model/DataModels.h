/**
 * @file DataModels.h
 * @brief 核心数据模型定义
 * 
 * 该文件定义了应用程序的核心数据模型，包括消息角色枚举、会话数据结构和消息数据结构。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QString>
#include <QDateTime>

/**
 * @brief 消息发送者角色枚举
 * 
 * 定义消息发送者的角色类型，对应数据库里的role字段。
 */
enum class MessageRole {
    User, ///< 用户角色
    AI    ///< AI角色
};

/**
 * @brief 会话数据结构
 * 
 * 包含会话的基本信息，用于数据库存储和界面显示。
 */
struct SessionData {
    int id = -1;                ///< 数据库ID
    QString name;               ///< 会话标题
    qint64 createdAt = 0;       ///< 创建时间戳

    /**
     * @brief 默认构造函数
     */
    SessionData() {}

    /**
     * @brief 便捷构造函数
     * @param _id 会话ID
     * @param _name 会话名称
     */
    SessionData(int _id, const QString& _name)
        : id(_id), name(_name), createdAt(QDateTime::currentMSecsSinceEpoch()) {}
};

/**
 * @brief 消息数据结构
 * 
 * 包含消息的完整信息，支持文本和图片两种类型。
 */
struct MessageData {
    int id = -1;                ///< 消息ID
    int sessionId = -1;         ///< 外键，关联的会话ID
    MessageRole role = MessageRole::User; ///< 消息发送者角色

    QString text;               ///< 文字内容
    QString imagePath;          ///< 本地图片路径（如果是文字则为空）

    qint64 timestamp = 0;       ///< 消息时间戳

    /**
     * @brief 判断是否是图片消息
     * @return bool 是否为图片消息
     */
    bool isImage() const { return !imagePath.isEmpty(); }

    /**
     * @brief 默认构造函数
     */
    MessageData() {}

    /**
     * @brief 便捷构造函数
     * @param sid 会话ID
     * @param r 消息角色
     * @param t 文本内容
     * @param img 图片路径（可选，默认为空）
     */
    MessageData(int sid, MessageRole r, const QString& t, const QString& img = "")
        : sessionId(sid), role(r), text(t), imagePath(img)
    {
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
};
