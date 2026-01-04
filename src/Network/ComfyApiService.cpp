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
#include <QSslConfiguration>
#include <QSslSocket>

ComfyApiService::ComfyApiService(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // 生成 ID
    m_clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // --- 1. 连接成功信号 ---
    connect(m_webSocket, &QWebSocket::connected, this, [=](){
        qDebug() << "✅ WebSocket 连接成功!";
        emit serverConnected();
    });

    // --- 2. 连接断开信号 (关键) ---
    connect(m_webSocket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "❌ WebSocket 连接断开";
        emit serverDisconnected();
    });

    // --- 3. 连接错误信号 (关键) ---
    // Qt6 写法：使用 lambda 接收错误信息
    connect(m_webSocket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error){
        Q_UNUSED(error);
        QString errStr = m_webSocket->errorString();
        qDebug() << "⚠️ WebSocket 错误:" << errStr;
        emit errorOccurred(errStr);
    });

    // --- 4. 收到消息信号 ---
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &ComfyApiService::onTextMessageReceived);
}

ComfyApiService::~ComfyApiService()
{
    // socket 和 networkManager 指定了 parent 为 this，析构时会自动清理
    // 但为了优雅退出，可以在这里手动 close 一下
    if (m_webSocket) {
        m_webSocket->close();
    }
}

void ComfyApiService::connectToHost(const QString& fullUrl)
{
    QString urlStr = fullUrl.trimmed();

    // 1. 容错处理：如果用户没写 http://，默认补上
    if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://")) {
        urlStr = "http://" + urlStr;
    }

    // 2. 去掉末尾的斜杠 (为了后续拼接方便)
    if (urlStr.endsWith("/")) {
        urlStr.chop(1);
    }

    // 3. 保存 HTTP 基础地址 (例如: http://frp.top:12345)
    m_apiBaseUrl = urlStr;

    // 4. 生成 WebSocket 地址
    // 把 http 换成 ws，把 https 换成 wss
    QString wsUrl = m_apiBaseUrl;
    if (wsUrl.startsWith("https://")) {
        wsUrl.replace(0, 8, "wss://");
    } else {
        wsUrl.replace(0, 7, "ws://");
    }

    // 加上 WebSocket 路径和 ClientID
    wsUrl += QString("/ws?clientId=%1").arg(m_clientId);

    qDebug() << "🔗 准备连接:" << wsUrl;

    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->close();
    }

    // ================== 【新增代码开始】 ==================
    // 配置 SSL，允许自签名证书或不安全的证书
    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 核心：不验证服务器证书
    sslConfig.setProtocol(QSsl::AnyProtocol);
    m_webSocket->setSslConfiguration(sslConfig);

    // 额外保险：如果发生 SSL 错误，强制忽略
    connect(m_webSocket, &QWebSocket::sslErrors, this, [=](const QList<QSslError>& errors){
        qDebug() << "⚠️ 捕获到 SSL 错误 (已忽略):" << errors.first().errorString();
        m_webSocket->ignoreSslErrors();
    });
    // ================== 【新增代码结束】 ==================


    // 打开新连接
    m_webSocket->open(QUrl(wsUrl));
}

void ComfyApiService::queuePrompt(const QJsonObject& workflow)
{
    // 修改这里：使用 m_apiBaseUrl
    QUrl url(m_apiBaseUrl + "/prompt");
    QNetworkRequest request(url);

    // ... 下面的代码保持不变 ...
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject payload;
    payload["prompt"] = workflow;
    payload["client_id"] = m_clientId;
    QByteArray data = QJsonDocument(payload).toJson();

    qDebug() << "Posting prompt to:" << url.toString(); // 方便调试
    QNetworkReply* reply = m_networkManager->post(request, data);

    // 【修改为 Lambda 写法】
    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);          // 防止编译器警告“未使用的变量”
        reply->ignoreSslErrors();  // 强制忽略错误
    });

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
    QUrl url(m_apiBaseUrl + "/view");
    QUrlQuery query;
    query.addQueryItem("filename", filename);
    query.addQueryItem("subfolder", subfolder);
    query.addQueryItem("type", type);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);

    // 【修改为 Lambda 写法】
    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

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
    QUrl url(m_apiBaseUrl + "/upload/image");
    QNetworkRequest request(url);

    qDebug() << "📤 正在上传图片:" << localPath;

    // 4. 发送 POST
    QNetworkReply* reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply); // 让 reply 管理 multiPart 的生命周期

    // 【修改为 Lambda 写法】
    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

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
