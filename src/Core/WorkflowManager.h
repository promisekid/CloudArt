/**
 * @file WorkflowManager.h
 * @brief 工作流管理器头文件
 * 
 * 该文件定义了WorkflowManager类，负责构建和管理各种工作流的JSON数据。
 * 支持文生图、图生图、高清修复、视觉反推等多种工作流类型。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QVariant>
#include "../Model/WorkflowTypes.h"

/**
 * @brief 工作流管理器类
 * 
 * 负责构建不同类型的工作流JSON对象，用于与ComfyUI API交互。
 * 支持文生图、图生图、高清修复和视觉反推等多种工作流类型。
 */
class WorkflowManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit WorkflowManager(QObject *parent = nullptr);

    /**
     * @brief 构建工作流JSON
     * @param type 工作流类型 (例如 TextToImage)
     * @param params 用户输入的参数 (例如 prompt, seed)
     * @return QJsonObject 准备好发送给API的JSON对象
     */
    QJsonObject buildWorkflow(WorkflowType type, const QMap<QString, QVariant>& params);

private:
    /**
     * @brief 加载资源文件中的JSON模板
     * @param resourcePath 资源文件路径
     * @return QJsonObject JSON模板对象
     */
    QJsonObject loadTemplate(const QString& resourcePath);

    /**
     * @brief 设置指定节点的输入参数值
     * @param workflow 工作流JSON对象（引用传递，直接修改）
     * @param nodeId 节点ID (如 "3")
     * @param inputKey 参数名 (如 "seed", "text")
     * @param value 参数值
     */
    void setNodeInput(QJsonObject& workflow, const QString& nodeId, const QString& inputKey, const QVariant& value);

    /**
     * @brief 构建文生图工作流
     * @param params 用户输入参数
     * @return QJsonObject 文生图工作流JSON对象
     */
    QJsonObject buildTextToImage(const QMap<QString, QVariant>& params);

    /**
     * @brief 构建高清修复工作流
     * @param params 用户输入参数
     * @return QJsonObject 高清修复工作流JSON对象
     */
    QJsonObject buildUpscale(const QMap<QString, QVariant>& params);

    /**
     * @brief 构建图生图工作流
     * @param params 用户输入参数
     * @return QJsonObject 图生图工作流JSON对象
     */
    QJsonObject buildImageToImage(const QMap<QString, QVariant>& params);

    /**
     * @brief 构建视觉反推工作流
     * @param params 用户输入参数
     * @return QJsonObject 视觉反推工作流JSON对象
     */
    QJsonObject buildVisionCaption(const QMap<QString, QVariant>& params);
};
