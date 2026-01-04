/**
 * @file DatabaseManager.h
 * @brief 数据库管理器头文件
 * 
 * 该文件定义了DatabaseManager类，负责应用程序的数据库操作。
 * 使用单例模式管理数据库连接，提供会话和消息的增删改查接口。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "../Model/DataModels.h"

/**
 * @brief 数据库管理器类
 * 
 * 采用单例模式管理数据库连接，提供会话和消息的持久化存储功能。
 * 支持会话的创建、查询、重命名、删除以及消息的添加和查询操作。
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return DatabaseManager& 数据库管理器单例引用
     */
    static DatabaseManager& instance();

    /**
     * @brief 初始化数据库
     * @return bool 初始化是否成功
     * 
     * 打开数据库连接并创建必要的数据表
     */
    bool init();

    /**
     * @brief 创建新会话
     * @param name 会话名称
     * @return int 新创建的会话ID，失败返回-1
     */
    int createSession(const QString& name);

    /**
     * @brief 获取所有会话
     * @return QVector<SessionData> 所有会话的数据列表
     * 
     * 用于初始化左侧会话列表
     */
    QVector<SessionData> getAllSessions();

    /**
     * @brief 重命名会话
     * @param id 会话ID
     * @param newName 新会话名称
     * @return bool 重命名是否成功
     */
    bool renameSession(int id, const QString& newName);

    /**
     * @brief 删除会话
     * @param id 会话ID
     * @return bool 删除是否成功
     * 
     * 删除会话时会连带删除该会话下的所有消息
     */
    bool deleteSession(int id);

    /**
     * @brief 添加新消息
     * @param msg 消息数据
     * @return int 新消息的ID，失败返回-1
     */
    int addMessage(const MessageData& msg);

    /**
     * @brief 获取指定会话的所有消息
     * @param sessionId 会话ID
     * @return QVector<MessageData> 消息数据列表
     * 
     * 用于点击会话后加载历史消息
     */
    QVector<MessageData> getMessages(int sessionId);

    /**
     * @brief 获取所有生成的图片路径
     * @return QVector<QString> 图片路径列表（按时间倒序）
     */
    QVector<QString> getAllAiImages();

private:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit DatabaseManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~DatabaseManager();

    /**
     * @brief 禁用拷贝构造函数
     */
    DatabaseManager(const DatabaseManager&) = delete;

    /**
     * @brief 禁用拷贝赋值运算符
     */
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief 创建数据表
     * 
     * 创建会话表和消息表（如果不存在）
     */
    void createTables();

private:
    QSqlDatabase m_db; ///< 数据库连接对象
};
