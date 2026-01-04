/**
 * @file ComfyApiService.h
 * @brief ComfyUI API服务类头文件
 * 
 * 该文件定义了ComfyApiService类，用于与ComfyUI服务器进行通信。
 * 包含HTTP请求和WebSocket连接功能，支持图片生成进度监控和结果接收。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QObject>
#include <QPixmap>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUuid>
#include <QHttpMultiPart>

// 前向声明
class QNetworkAccessManager;
class QWebSocket;

/**
 * @brief ComfyUI API服务类
 * 
 * 负责与ComfyUI服务器进行HTTP和WebSocket通信，
 * 处理图片生成请求、进度监控和结果接收。
 */
class ComfyApiService : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit ComfyApiService(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ComfyApiService();

    /**
     * @brief 连接到ComfyUI服务器
     * @param address 服务器地址
     * @param port 服务器端口
     */
    void connectToHost(const QString& baseUrl);

    /**
     * @brief 发送提示词生成任务
     * @param workflow 工作流JSON对象
     */
    void queuePrompt(const QJsonObject& workflow);

    // 【新增】获取图片数据的接口 (通过 HTTP GET)
    void getImage(const QString& filename, const QString& subfolder, const QString& type, const QString& promptId);

    // 【新增】上传图片函数
    void uploadImage(const QString& localPath);

signals:
    /**
     * @brief 服务器连接成功信号
     */
    void serverConnected();

    /**
     * @brief 服务器连接断开信号
     */
    void serverDisconnected();

    /**
     * @brief 生成进度更新信号
     * @param step 当前步骤
     * @param total 总步骤数
     */
    void progressUpdated(int step, int total);

    /**
     * @brief 错误发生信号
     * @param msg 错误消息
     */
    void errorOccurred(const QString& msg);

    void promptQueued(const QString& promptId);

    // 【新增】图片下载完成信号
    void imageReceived(const QString& promptId, const QString& filename, const QPixmap& img);

    // 【新增】上传成功信号
    void imageUploaded(const QString& serverFileName);

    // 【新增】流式文字信号(如果之前没加)
    void streamTokenReceived(const QString& token, bool finished);

private slots:
    // 【新增】处理 HTTP 回复
    void onPostFinished();

    // 【新增】处理 WebSocket 文本消息
    void onTextMessageReceived(const QString &message);

    // 【新增】处理图片下载完成
    void onImageDownloadFinished();

private:
    QNetworkAccessManager* m_networkManager; ///< HTTP网络管理器
    QWebSocket* m_webSocket; ///< WebSocket连接
    QString m_apiBaseUrl;
    // 【新增】记录当前的任务ID，用于匹配
    QString m_currentPromptId;
    QString m_clientId;
};
