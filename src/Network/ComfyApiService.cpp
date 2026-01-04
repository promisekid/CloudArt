/**
 * @file ComfyApiService.cpp
 * @brief ComfyUI API服务实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "ComfyApiService.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QWebSocket>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QSslConfiguration>
#include <QSslSocket>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ComfyApiService::ComfyApiService(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    m_clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    connect(m_webSocket, &QWebSocket::connected, this, [=](){
        qDebug() << "WebSocket 连接成功!";
        emit serverConnected();
    });

    connect(m_webSocket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "WebSocket 连接断开";
        emit serverDisconnected();
    });

    connect(m_webSocket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error){
        Q_UNUSED(error);
        QString errStr = m_webSocket->errorString();
        qDebug() << "WebSocket 错误:" << errStr;
        emit errorOccurred(errStr);
    });

    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &ComfyApiService::onTextMessageReceived);
}

/**
 * @brief 析构函数
 */
ComfyApiService::~ComfyApiService()
{
    if (m_webSocket) {
        m_webSocket->close();
    }
}

/**
 * @brief 连接到ComfyUI服务器
 * @param fullUrl 服务器完整URL
 */
void ComfyApiService::connectToHost(const QString& fullUrl)
{
    QString urlStr = fullUrl.trimmed();

    if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://")) {
        urlStr = "http://" + urlStr;
    }

    if (urlStr.endsWith("/")) {
        urlStr.chop(1);
    }

    m_apiBaseUrl = urlStr;

    QString wsUrl = m_apiBaseUrl;
    if (wsUrl.startsWith("https://")) {
        wsUrl.replace(0, 8, "wss://");
    } else {
        wsUrl.replace(0, 7, "ws://");
    }

    wsUrl += QString("/ws?clientId=%1").arg(m_clientId);

    qDebug() << "准备连接:" << wsUrl;

    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->close();
    }

    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::AnyProtocol);
    m_webSocket->setSslConfiguration(sslConfig);

    connect(m_webSocket, &QWebSocket::sslErrors, this, [=](const QList<QSslError>& errors){
        qDebug() << "捕获到 SSL 错误 (已忽略):" << errors.first().errorString();
        m_webSocket->ignoreSslErrors();
    });

    m_webSocket->open(QUrl(wsUrl));
}

/**
 * @brief 发送提示词生成任务
 * @param workflow 工作流JSON对象
 */
void ComfyApiService::queuePrompt(const QJsonObject& workflow)
{
    QUrl url(m_apiBaseUrl + "/prompt");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject payload;
    payload["prompt"] = workflow;
    payload["client_id"] = m_clientId;
    QByteArray data = QJsonDocument(payload).toJson();

    qDebug() << "Posting prompt to:" << url.toString();
    QNetworkReply* reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

    connect(reply, &QNetworkReply::finished, this, &ComfyApiService::onPostFinished);
}

/**
 * @brief 处理HTTP POST请求完成
 */
void ComfyApiService::onPostFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        QString promptId = obj["prompt_id"].toString();

        m_currentPromptId = promptId;

        qDebug() << "任务发送成功! ID:" << promptId;

        emit promptQueued(promptId);
    } else {
        QString err = "发送任务失败: " + reply->errorString();
        qDebug() << err;
        emit errorOccurred(err);
    }

    reply->deleteLater();
}

/**
 * @brief 处理WebSocket文本消息
 * @param message 接收到的消息内容
 */
void ComfyApiService::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject root = doc.object();
    QString msgType = root["type"].toString();
    QJsonObject data = root["data"].toObject();

    if (msgType == "cloudart_stream") {
        QString token = data["token"].toString();
        bool finished = data["finished"].toBool();
        emit streamTokenReceived(token, finished);
        return;
    }

    if (msgType == "executed") {
        QString nodeId = QString::number(data["node"].toInt());
        if (nodeId == "0") nodeId = data["node"].toString();

        QString promptId = data["prompt_id"].toString();

        qDebug() << "检查结束条件 | 收到ID:" << nodeId << " | 目标ID: 4 | 任务匹配:" << (promptId == m_currentPromptId);

        if (promptId == m_currentPromptId && (nodeId == "20" || nodeId == "1" || nodeId == "9")) {
            QJsonObject output = data["output"].toObject();
            QJsonArray images = output["images"].toArray();
            if (!images.isEmpty()) {
                QJsonObject imgInfo = images[0].toObject();
                getImage(imgInfo["filename"].toString(),
                         imgInfo["subfolder"].toString(),
                         imgInfo["type"].toString(), promptId);
            }
        }

        if (promptId == m_currentPromptId && nodeId == "4") {
            qDebug() << "触发反推强制解锁";
            emit streamTokenReceived("", true);
        }
    }
}

/**
 * @brief 获取图片数据
 * @param filename 图片文件名
 * @param subfolder 子文件夹路径
 * @param type 图片类型
 * @param promptId 提示词ID
 */
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

    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

    reply->setProperty("promptId", promptId);
    reply->setProperty("filename", filename);

    connect(reply, &QNetworkReply::finished, this, &ComfyApiService::onImageDownloadFinished);
}

/**
 * @brief 处理图片下载完成
 */
void ComfyApiService::onImageDownloadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QString promptId = reply->property("promptId").toString();
    QString filename = reply->property("filename").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(data)) {
            qDebug() << "图片下载成功:" << filename;

            emit imageReceived(promptId, filename, pixmap);
        } else {
            qDebug() << "图片数据损坏";
        }
    } else {
        qDebug() << "图片下载失败:" << reply->errorString();
    }
    reply->deleteLater();
}

/**
 * @brief 上传图片到服务器
 * @param localPath 本地图片路径
 */
void ComfyApiService::uploadImage(const QString& localPath)
{
    QFile* file = new QFile(localPath);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开本地图片:" << localPath;
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    QString fileName = QFileInfo(localPath).fileName();

    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QString("form-data; name=\"image\"; filename=\"%1\"").arg(fileName)));

    imagePart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(imagePart);

    QUrl url(m_apiBaseUrl + "/upload/image");
    QNetworkRequest request(url);

    qDebug() << "正在上传图片:" << localPath;

    QNetworkReply* reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors){
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

    connect(reply, &QNetworkReply::finished, this, [=](){
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();

            QString serverName = obj["name"].toString();

            qDebug() << "图片上传成功! 服务器文件名:" << serverName;
            emit imageUploaded(serverName);
        } else {
            qDebug() << "上传失败:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
