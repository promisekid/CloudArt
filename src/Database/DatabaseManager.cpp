#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QVariant>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::init()
{
    // 1. 确定数据库存放在哪里
    // 也就是 C:/Users/用户名/AppData/Local/CloudArt/cloudart.db
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 确保文件夹存在
    QDir dir(dataDir);
    if (!dir.exists()) dir.mkpath(".");

    QString dbPath = dataDir + "/cloudart.db";
    qDebug() << "📂 数据库路径:" << dbPath;

    // 2. 连接 SQLite
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "❌ 打开数据库失败:" << m_db.lastError().text();
        return false;
    }

    // 3. 检查并创建表
    createTables();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlQuery query;

    // --- 表1：会话表 ---
    // id: 自增主键
    // title: 会话名
    // created_at: 创建时间
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS tb_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "created_at INTEGER"
        ")"
        );
    if (!success) qDebug() << "❌ tb_sessions 建表失败:" << query.lastError();

    // --- 表2：消息表 ---
    // session_id: 属于哪个会话
    // role: 'user' 或 'ai'
    // content: 文字内容
    // image_path: 图片路径
    // timestamp: 时间
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
    if (!success) qDebug() << "❌ tb_messages 建表失败:" << query.lastError();
}

int DatabaseManager::createSession(const QString& name)
{
    QSqlQuery query;
    // 使用 prepare 预处理语句，:name 是占位符
    query.prepare("INSERT INTO tb_sessions (title, created_at) VALUES (:name, :time)");

    query.bindValue(":name", name);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());

    if (query.exec()) {
        // 返回新插入行的自增 ID
        return query.lastInsertId().toInt();
    }

    qDebug() << "❌ 创建会话失败:" << query.lastError();
    return -1;
}

QVector<SessionData> DatabaseManager::getAllSessions()
{
    QVector<SessionData> list;
    QSqlQuery query("SELECT * FROM tb_sessions ORDER BY created_at DESC"); // 按时间倒序，新的在上面

    while (query.next()) {
        SessionData session;
        session.id = query.value("id").toInt();
        session.name = query.value("title").toString();
        session.createdAt = query.value("created_at").toLongLong();
        list.append(session);
    }
    return list;
}

bool DatabaseManager::renameSession(int id, const QString& newName)
{
    QSqlQuery query;
    query.prepare("UPDATE tb_sessions SET title = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);
    return query.exec();
}

bool DatabaseManager::deleteSession(int id)
{
    // 1. 先删消息 (虽然 SQLite 可以设级联删除，但手动删更稳妥)
    QSqlQuery queryMsg;
    queryMsg.prepare("DELETE FROM tb_messages WHERE session_id = :sid");
    queryMsg.bindValue(":sid", id);
    queryMsg.exec();

    // 2. 再删会话
    QSqlQuery query;
    query.prepare("DELETE FROM tb_sessions WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

// =========================================================
// 消息操作实现
// =========================================================

int DatabaseManager::addMessage(const MessageData& msg)
{
    QSqlQuery query;
    query.prepare("INSERT INTO tb_messages (session_id, role, content, image_path, timestamp) "
                  "VALUES (:sid, :role, :content, :img, :time)");

    query.bindValue(":sid", msg.sessionId);
    // 枚举转字符串存入
    query.bindValue(":role", msg.role == MessageRole::User ? "user" : "ai");
    query.bindValue(":content", msg.text);
    query.bindValue(":img", msg.imagePath);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }

    qDebug() << "❌ 插入消息失败:" << query.lastError();
    return -1;
}

QVector<MessageData> DatabaseManager::getMessages(int sessionId)
{
    QVector<MessageData> list;
    QSqlQuery query;
    // 按时间正序，早说的话在上面
    query.prepare("SELECT * FROM tb_messages WHERE session_id = :sid ORDER BY timestamp ASC");
    query.bindValue(":sid", sessionId);

    if (!query.exec()) {
        qDebug() << "❌ 查询消息失败:" << query.lastError();
        return list;
    }

    while (query.next()) {
        // 使用构造函数方便一点，或者手动赋值
        // int sid, MessageRole r, const QString& t, const QString& img
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


QVector<QString> DatabaseManager::getAllAiImages()
{
    QVector<QString> list;
    QSqlQuery query;
    // 查询所有 role 为 'ai' 且 image_path 不为空的记录，按时间倒序排列
    query.prepare("SELECT image_path FROM tb_messages WHERE role = 'ai' AND image_path != '' ORDER BY timestamp DESC");

    if (query.exec()) {
        while (query.next()) {
            list.append(query.value("image_path").toString());
        }
    } else {
        qDebug() << "❌ 查询历史图片失败:" << query.lastError();
    }
    return list;
}
