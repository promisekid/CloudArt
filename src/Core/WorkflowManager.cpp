/**
 * @file WorkflowManager.cpp
 * @brief 工作流管理器实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "WorkflowManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QRandomGenerator>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
WorkflowManager::WorkflowManager(QObject *parent) : QObject(parent)
{
}

/**
 * @brief 构建工作流JSON
 * @param type 工作流类型
 * @param params 用户输入的参数
 * @return QJsonObject 准备好发送给API的JSON对象
 */
QJsonObject WorkflowManager::buildWorkflow(WorkflowType type, const QMap<QString, QVariant>& params)
{
    qDebug() << "WorkflowManager: 构建工作流类型 ->" << (int)type;

    switch (type) {
    case WorkflowType::TextToImage:
        return buildTextToImage(params);

    case WorkflowType::Upscale:
        return buildUpscale(params);

    case WorkflowType::ImageToImage:
        return buildImageToImage(params);

    case WorkflowType::VisionCaption:
        return buildVisionCaption(params);

    default:
        qDebug() << "未知的工作流类型:" << (int)type;
        return QJsonObject();
    }
}

/**
 * @brief 构建文生图工作流
 * @param params 用户输入参数
 * @return QJsonObject 文生图工作流JSON对象
 */
QJsonObject WorkflowManager::buildTextToImage(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/t2i");
    if (workflow.isEmpty()) return QJsonObject();

    if (params.contains("prompt")) {
        setNodeInput(workflow, "5", "text", params["prompt"].toString());
    }

    if (params.contains("seed")) {
        setNodeInput(workflow, "4", "seed", params["seed"].toLongLong());
    }

    if (params.contains("width") && params.contains("height")) {
        setNodeInput(workflow, "7", "width", params["width"].toInt());
        setNodeInput(workflow, "7", "height", params["height"].toInt());
    }

    return workflow;
}

/**
 * @brief 构建高清修复工作流
 * @param params 用户输入参数
 * @return QJsonObject 高清修复工作流JSON对象
 */
QJsonObject WorkflowManager::buildUpscale(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/upscale");
    if (workflow.isEmpty()) return QJsonObject();

    if (params.contains("image_path")) {
        setNodeInput(workflow, "6", "image", params["image_path"].toString());
    }

    if (params.contains("seed")) {
        setNodeInput(workflow, "2", "seed", params["seed"].toLongLong());
    }

    return workflow;
}

/**
 * @brief 设置指定节点的输入参数值
 * @param workflow 工作流JSON对象（引用传递，直接修改）
 * @param nodeId 节点ID
 * @param inputKey 参数名
 * @param value 参数值
 */
void WorkflowManager::setNodeInput(QJsonObject& workflow, const QString& nodeId, const QString& inputKey, const QVariant& value)
{
    if (!workflow.contains(nodeId)) {
        qDebug() << "Warning: JSON中找不到节点 ID:" << nodeId;
        return;
    }

    QJsonObject node = workflow[nodeId].toObject();

    if (!node.contains("inputs")) {
        qDebug() << "Warning: 节点" << nodeId << "没有 inputs 字段";
        return;
    }

    QJsonObject inputs = node["inputs"].toObject();

    if (value.typeId() == QMetaType::LongLong || value.typeId() == QMetaType::Int) {
        inputs[inputKey] = value.toLongLong();
    }
    else {
        inputs[inputKey] = value.toString();
    }

    node["inputs"] = inputs;
    workflow[nodeId] = node;
}

/**
 * @brief 加载资源文件中的JSON模板
 * @param resourcePath 资源文件路径
 * @return QJsonObject JSON模板对象
 */
QJsonObject WorkflowManager::loadTemplate(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法加载模板文件:" << resourcePath;
        return QJsonObject();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qDebug() << "JSON 格式错误:" << resourcePath;
        return QJsonObject();
    }

    return doc.object();
}

/**
 * @brief 构建视觉反推工作流
 * @param params 用户输入参数
 * @return QJsonObject 视觉反推工作流JSON对象
 */
QJsonObject WorkflowManager::buildVisionCaption(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/vision");

    if (workflow.isEmpty()) {
        qDebug() << "无法加载反推模板 :/workflows/vision";
        return QJsonObject();
    }

    if (params.contains("image_path")) {
        setNodeInput(workflow, "3", "image", params["image_path"].toString());
    }

    qint64 seed = QRandomGenerator::global()->generate();
    if (seed < 0) seed = -seed;

    if (params.contains("seed")) {
        seed = params["seed"].toLongLong();
    }

    setNodeInput(workflow, "1", "seed", seed);

    return workflow;
}

/**
 * @brief 构建图生图工作流
 * @param params 用户输入参数
 * @return QJsonObject 图生图工作流JSON对象
 */
QJsonObject WorkflowManager::buildImageToImage(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/render");
    if (workflow.isEmpty()) return QJsonObject();

    if (params.contains("image_path")) {
        setNodeInput(workflow, "30", "image", params["image_path"].toString());
    }

    if (params.contains("prompt")) {
        setNodeInput(workflow, "6", "text", params["prompt"].toString());
    }

    if (params.contains("seed")) {
        setNodeInput(workflow, "3", "seed", params["seed"].toLongLong());
    }

    return workflow;
}
