/**
 * @file DatabaseManager.cpp
 * @brief 数据库管理器实现文件
 * 
 * 该文件实现了DatabaseManager类，负责应用程序的数据库操作。
 * 使用单例模式管理数据库连接，提供会话和消息的增删改查接口。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QVariant>

/**
 * @brief 获取单例实例
 * @return DatabaseManager& 数据库管理器单例引用
 */
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

/**
 * @brief 析构函数
 */
DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) m_db.close();
}

/**
 * @brief 初始化数据库
 * @return bool 初始化是否成功
 * 
 * 打开数据库连接并创建必要的数据表
 */
bool DatabaseManager::init()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(dataDir);
    if (!dir.exists()) dir.mkpath(".");

    QString dbPath = dataDir + "/cloudart.db";
    qDebug() << "数据库路径:" << dbPath;

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "打开数据库失败:" << m_db.lastError().text();
        return false;
    }

    createTables();
    return true;
}

/**
 * @brief 创建数据表
 * 
 * 创建会话表和消息表（如果不存在）
 */
void DatabaseManager::createTables()
{
    QSqlQuery query;

    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS tb_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "created_at INTEGER"
        ")"
        );
    if (!success) qDebug() << "tb_sessions 建表失败:" << query.lastError();

    success = query.exec(
        "CREATE TABLE IF NOT EXISTS tb_messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "session_id INTEGER, "
        "role TEXT, "
        "content TEXT, "
        "image_path TEXT, "
        "timestamp INTEGER"
        ")"
        );
    if (!success) qDebug() << "tb_messages 建表失败:" << query.lastError();
}

/**
 * @brief 创建新会话
 * @param name 会话名称
 * @return int 新创建的会话ID，失败返回-1
 */
int DatabaseManager::createSession(const QString& name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO tb_sessions (title, created_at) VALUES (:name, :time)");

    query.bindValue(":name", name);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }

    qDebug() << "创建会话失败:" << query.lastError();
    return -1;
}

/**
 * @brief 获取所有会话
 * @return QVector<SessionData> 所有会话的数据列表
 * 
 * 用于初始化左侧会话列表
 */
QVector<SessionData> DatabaseManager::getAllSessions()
{
    QVector<SessionData> list;
    QSqlQuery query("SELECT * FROM tb_sessions ORDER BY created_at DESC");

    while (query.next()) {
        SessionData session;
        session.id = query.value("id").toInt();
        session.name = query.value("title").toString();
        session.createdAt = query.value("created_at").toLongLong();
        list.append(session);
    }
    return list;
}

/**
 * @brief 重命名会话
 * @param id 会话ID
 * @param newName 新会话名称
 * @return bool 重命名是否成功
 */
bool DatabaseManager::renameSession(int id, const QString& newName)
{
    QSqlQuery query;
    query.prepare("UPDATE tb_sessions SET title = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);
    return query.exec();
}

/**
 * @brief 删除会话
 * @param id 会话ID
 * @return bool 删除是否成功
 */
bool DatabaseManager::deleteSession(int id)
{
    QSqlQuery queryMsg;
    queryMsg.prepare("DELETE FROM tb_messages WHERE session_id = :sid");
    queryMsg.bindValue(":sid", id);
    queryMsg.exec();

    QSqlQuery query;
    query.prepare("DELETE FROM tb_sessions WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

/**
 * @brief 添加新消息
 * @param msg 消息数据
 * @return int 新消息的ID，失败返回-1
 */
int DatabaseManager::addMessage(const MessageData& msg)
{
    QSqlQuery query;
    query.prepare("INSERT INTO tb_messages (session_id, role, content, image_path, timestamp) "
                  "VALUES (:sid, :role, :content, :img, :time)");

    query.bindValue(":sid", msg.sessionId);
    query.bindValue(":role", msg.role == MessageRole::User ? "user" : "ai");
    query.bindValue(":content", msg.text);
    query.bindValue(":img", msg.imagePath);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }

    qDebug() << "插入消息失败:" << query.lastError();
    return -1;
}

/**
 * @brief 获取指定会话的所有消息
 * @param sessionId 会话ID
 * @return QVector<MessageData> 消息数据列表
 */
QVector<MessageData> DatabaseManager::getMessages(int sessionId)
{
    QVector<MessageData> list;
    QSqlQuery query;
    query.prepare("SELECT * FROM tb_messages WHERE session_id = :sid ORDER BY timestamp ASC");
    query.bindValue(":sid", sessionId);

    if (!query.exec()) {
        qDebug() << "查询消息失败:" << query.lastError();
        return list;
    }

    while (query.next()) {
        int id = query.value("id").toInt();
        int sid = query.value("session_id").toInt();
        QString roleStr = query.value("role").toString();
        QString content = query.value("content").toString();
        QString imgPath = query.value("image_path").toString();
        qint64 time = query.value("timestamp").toLongLong();

        MessageRole role = (roleStr == "user") ? MessageRole::User : MessageRole::AI;

        MessageData msg(sid, role, content, imgPath);
        msg.id = id;
        msg.timestamp = time;

        list.append(msg);
    }
    return list;
}

/**
 * @brief 获取所有生成的图片路径
 * @return QVector<QString> 图片路径列表（按时间倒序）
 */
QVector<QString> DatabaseManager::getAllAiImages()
{
    QVector<QString> list;
    QSqlQuery query;
    query.prepare("SELECT image_path FROM tb_messages WHERE role = 'ai' AND image_path != '' ORDER BY timestamp DESC");

    if (query.exec()) {
        while (query.next()) {
            list.append(query.value("image_path").toString());
        }
    } else {
        qDebug() << "查询历史图片失败:" << query.lastError();
    }
    return list;
}
