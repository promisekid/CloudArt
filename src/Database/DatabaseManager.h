#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "../Model/DataModels.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式：全局只有一个数据库连接
    static DatabaseManager& instance();

    // 初始化：打开数据库、建表
    bool init();

    // ==========================================
    // 【新增】会话操作接口 (Session API)
    // ==========================================

    // 创建新会话，返回新生成的 ID
    int createSession(const QString& name);

    // 获取所有会话 (用于初始化左侧列表)
    QVector<SessionData> getAllSessions();

    // 重命名会话
    bool renameSession(int id, const QString& newName);

    // 删除会话 (会连带删除该会话下的所有消息)
    bool deleteSession(int id);

    // ==========================================
    // 【新增】消息操作接口 (Message API)
    // ==========================================

    // 存入一条新消息，返回新 ID
    int addMessage(const MessageData& msg);

    // 获取某个会话的所有消息 (用于点击会话后加载历史)
    QVector<MessageData> getMessages(int sessionId);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    void createTables();

private:
    QSqlDatabase m_db;
};
