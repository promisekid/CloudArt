#include "ComfyApiService.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>   // 解析数组需要
#include <QUrl>
#include <QUrlQuery>    // 【关键修复】
#include <QDebug>
#include <QWebSocket>
#include <QHttpMultiPart> // 【新增】
#include <QHttpPart>      // 【新增】
#include <QFile>          // 【新增】
#include <QFileInfo>      // 【新增】

ComfyApiService::ComfyApiService(QObject *parent)
    : QObject(parent)
{
    // 1. 初始化网络管理器 (用于后续发 HTTP POST 请求)
    m_networkManager = new QNetworkAccessManager(this);

    // 2. 初始化 WebSocket (用于监听服务器发回的进度消息)
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // 3. 连接 WebSocket 的基础信号
    // 当 socket 连接成功时 -> 转发我们的 serverConnected 信号
    connect(m_webSocket, &QWebSocket::connected, this, [=](){
        qDebug() << "WebSocket Connected!";
        emit serverConnected();
    });

    // 当 socket 断开时 -> 转发 serverDisconnected
    connect(m_webSocket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "WebSocket Disconnected!";
        emit serverDisconnected();
    });

    // 当 socket 出错时 -> 打印错误并转发
    // 注意：error 信号在 Qt6 中可能有重载，使用 lambda 接收 QAbstractSocket::SocketError
    typedef void (QWebSocket::*ErrorSignal)(QAbstractSocket::SocketError);
    connect(m_webSocket, static_cast<ErrorSignal>(&QWebSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError error){
        Q_UNUSED(error);
        qDebug() << "WebSocket Error:" << m_webSocket->errorString();
        emit errorOccurred(m_webSocket->errorString());
    });

    // 【新增】连接收到消息的信号
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &ComfyApiService::onTextMessageReceived);

    // 【修改 1】生成一个唯一的 UUID 作为身份证
    // 格式类似：{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    m_clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "客户端 ID 已生成:" << m_clientId;
}

ComfyApiService::~ComfyApiService()
{
    // socket 和 networkManager 指定了 parent 为 this，析构时会自动清理
    // 但为了优雅退出，可以在这里手动 close 一下
    if (m_webSocket) {
        m_webSocket->close();
    }
}

void ComfyApiService::connectToHost(const QString& address, int port)
{
    // 保存 HTTP 地址 (注意没有 /ws 后缀)
    m_serverAddress = QString("http://%1:%2").arg(address).arg(port);

    // 【修改 2】在 URL 后面加上 ?clientId=xxx
    // 注意：wsUrl 是 ws://127.0.0.1:8188/ws?clientId=xxxx
    QString wsUrl = QString("ws://%1:%2/ws?clientId=%3")
                        .arg(address)
                        .arg(port)
                        .arg(m_clientId);

    qDebug() << "Connecting WebSocket with ID:" << wsUrl;

    m_webSocket->close();
    m_webSocket->open(QUrl(wsUrl));
}

void ComfyApiService::queuePrompt(const QJsonObject& workflow)
{
    // 1. 构造请求 URL: http://127.0.0.1:8188/prompt
    QUrl url(m_serverAddress + "/prompt");
    QNetworkRequest request(url);

    // 2. 设置头信息 (告诉服务器我们要发 JSON)
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. 构造发送的数据包
    // ComfyUI 要求格式: { "prompt": { ...工作流节点... } }
    QJsonObject payload;
    payload["prompt"] = workflow;
    // client_id 最好加上，用于区分是谁发的，这里暂时先不加，后面再完善

    // 【修改 3】告诉服务器：这个任务是 m_clientId 发起的
    // 这样服务器执行完后，就会把 executed 消息发回给这个 ID 对应的 WebSocket
    payload["client_id"] = m_clientId;

    QByteArray data = QJsonDocument(payload).toJson();

    qDebug() << "Posting prompt (Client ID:" << m_clientId << ")...";
    QNetworkReply* reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, &ComfyApiService::onPostFinished);
}

void ComfyApiService::onPostFinished()
{
    // 获取触发这个槽的 reply 对象
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        // 请求成功！
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        // ComfyUI 会返回 {"prompt_id": "xxx", "number": ...}
        QString promptId = obj["prompt_id"].toString();

        // 【新增】保存这个 ID，后面收到消息时核对是不是我们发的
        m_currentPromptId = promptId;

        qDebug() << "✅ 任务发送成功! ID:" << promptId;

        emit promptQueued(promptId);
    } else {
        // 请求失败
        QString err = "发送任务失败: " + reply->errorString();
        qDebug() << "❌" << err;
        emit errorOccurred(err);
    }

    reply->deleteLater(); // 记得清理内存
}


void ComfyApiService::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject root = doc.object();
    QString msgType = root["type"].toString();
    QJsonObject data = root["data"].toObject();

    // 1. 处理流式消息
    if (msgType == "cloudart_stream") {
        QString token = data["token"].toString();
        bool finished = data["finished"].toBool();
        emit streamTokenReceived(token, finished);
        return;
    }

    // 2. 处理节点执行完成
    if (msgType == "executed") {
        // 【强制转换】把 node 转成字符串，防止 JSON 数字/字符串类型不匹配
        QString nodeId = QString::number(data["node"].toInt());
        // 如果转数字失败（说明本身是字符串），再直接转字符串
        if (nodeId == "0") nodeId = data["node"].toString();

        QString promptId = data["prompt_id"].toString();

        // 【强力调试】打印这一行，看看到底收到了什么
        qDebug() << "🔍 检查结束条件 | 收到ID:" << nodeId << " | 目标ID: 4 | 任务匹配:" << (promptId == m_currentPromptId);

        // 逻辑 A: 文生图/高清修复 (SaveImage)
        if (promptId == m_currentPromptId && (nodeId == "20" || nodeId == "1" || nodeId == "9")) {
            // ... (保持原有的图片下载逻辑) ...
            QJsonObject output = data["output"].toObject();
            QJsonArray images = output["images"].toArray();
            if (!images.isEmpty()) {
                QJsonObject imgInfo = images[0].toObject();
                getImage(imgInfo["filename"].toString(),
                         imgInfo["subfolder"].toString(),
                         imgInfo["type"].toString(), promptId);
            }
        }

        // 逻辑 B: 视觉反推 (PreviewAny)
        // 【修改】放宽条件，只要是当前任务且 ID 是 4，或者它是唯一的输出节点
        if (promptId == m_currentPromptId && nodeId == "4") {
            qDebug() << "🛑 触发反推强制解锁";
            emit streamTokenReceived("", true);
        }
    }
}


void ComfyApiService::getImage(const QString& filename, const QString& subfolder, const QString& type, const QString& promptId)
{
    QUrl url(m_serverAddress + "/view");
    QUrlQuery query;
    query.addQueryItem("filename", filename);
    query.addQueryItem("subfolder", subfolder);
    query.addQueryItem("type", type);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);

    // 【关键】把 ID 和 文件名 存入 reply 的属性中，以便回调时使用
    reply->setProperty("promptId", promptId);
    reply->setProperty("filename", filename);

    connect(reply, &QNetworkReply::finished, this, &ComfyApiService::onImageDownloadFinished);
}

void ComfyApiService::onImageDownloadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // 【关键】取出刚才存的数据
    QString promptId = reply->property("promptId").toString();
    QString filename = reply->property("filename").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(data)) {
            qDebug() << "🖼️ 图片下载成功:" << filename;

            // 【修改】发送包含 ID 和文件名的信号
            emit imageReceived(promptId, filename, pixmap);
        } else {
            qDebug() << "❌ 图片数据损坏";
        }
    } else {
        qDebug() << "❌ 图片下载失败:" << reply->errorString();
    }
    reply->deleteLater();
}


void ComfyApiService::uploadImage(const QString& localPath)
{
    // 1. 打开本地文件
    QFile* file = new QFile(localPath);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "❌ 无法打开本地图片:" << localPath;
        delete file;
        return;
    }

    // 2. 构造 Multipart 表单
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 图片部分
    QHttpPart imagePart;
    QString fileName = QFileInfo(localPath).fileName();

    // 设置头信息
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png")); // 假设是png/jpg
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QString("form-data; name=\"image\"; filename=\"%1\"").arg(fileName)));

    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // 让 multiPart 管理 file 的生命周期

    multiPart->append(imagePart);

    // 3. 构造请求 URL: http://ip:port/upload/image
    QUrl url(m_serverAddress + "/upload/image");
    QNetworkRequest request(url);

    qDebug() << "📤 正在上传图片:" << localPath;

    // 4. 发送 POST
    QNetworkReply* reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply); // 让 reply 管理 multiPart 的生命周期

    // 5. 处理结果
    connect(reply, &QNetworkReply::finished, this, [=](){
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();

            // ComfyUI 返回: {"name": "xxx.png", "subfolder": "...", "type": "input"}
            QString serverName = obj["name"].toString();
            // 如果有 subfolder，可以拼一下，通常 simple workflow 不需要

            qDebug() << "✅ 图片上传成功! 服务器文件名:" << serverName;
            emit imageUploaded(serverName); // 发出信号
        } else {
            qDebug() << "❌ 上传失败:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
