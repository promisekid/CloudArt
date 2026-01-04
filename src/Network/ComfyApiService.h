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

    /**
     * @brief 获取图片数据
     * @param filename 图片文件名
     * @param subfolder 子文件夹路径
     * @param type 图片类型
     * @param promptId 提示词ID
     */
    void getImage(const QString& filename, const QString& subfolder, const QString& type, const QString& promptId);

    /**
     * @brief 上传图片到服务器
     * @param localPath 本地图片路径
     */
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

    /**
     * @brief 图片下载完成信号
     * @param promptId 提示词ID
     * @param filename 文件名
     * @param img 图片数据
     */
    void imageReceived(const QString& promptId, const QString& filename, const QPixmap& img);

    /**
     * @brief 图片上传成功信号
     * @param serverFileName 服务器文件名
     */
    void imageUploaded(const QString& serverFileName);

    /**
     * @brief 流式文字接收信号
     * @param token 接收到的文本
     * @param finished 是否完成
     */
    void streamTokenReceived(const QString& token, bool finished);

private slots:
    /**
     * @brief 处理HTTP POST请求完成
     */
    void onPostFinished();

    /**
     * @brief 处理WebSocket文本消息
     * @param message 接收到的消息内容
     */
    void onTextMessageReceived(const QString &message);

    /**
     * @brief 处理图片下载完成
     */
    void onImageDownloadFinished();

private:
    QNetworkAccessManager* m_networkManager; ///< HTTP网络管理器
    QWebSocket* m_webSocket; ///< WebSocket连接
    QString m_apiBaseUrl; ///< API基础URL
    QString m_currentPromptId; ///< 当前任务ID，用于匹配
    QString m_clientId; ///< 客户端ID
};
