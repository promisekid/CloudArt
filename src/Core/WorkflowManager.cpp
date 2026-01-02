#include "WorkflowManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

WorkflowManager::WorkflowManager(QObject *parent) : QObject(parent)
{
}


QJsonObject WorkflowManager::buildWorkflow(WorkflowType type, const QMap<QString, QVariant>& params)
{
    qDebug() << "WorkflowManager: 构建工作流类型 ->" << (int)type;

    switch (type) {
    case WorkflowType::TextToImage:
        return buildTextToImage(params);

    case WorkflowType::Upscale:
        return buildUpscale(params);

    case WorkflowType::ImageToImage:
        return buildImageToImage(params); // 预留

    default:
        qDebug() << "❌ 未知的工作流类型:" << (int)type;
        return QJsonObject();
    }
}

QJsonObject WorkflowManager::buildTextToImage(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/t2i");
    if (workflow.isEmpty()) return QJsonObject();

    // 1. 设置提示词 (Node 5: CLIPTextEncode)
    if (params.contains("prompt")) {
        setNodeInput(workflow, "5", "text", params["prompt"].toString());
    }

    // 2. 设置种子 (Node 4: KSampler)
    if (params.contains("seed")) {
        setNodeInput(workflow, "4", "seed", params["seed"].toLongLong());
    }

    // 3. 设置分辨率 (Node 7: EmptyLatentImage)
    if (params.contains("width") && params.contains("height")) {
        setNodeInput(workflow, "7", "width", params["width"].toInt());
        setNodeInput(workflow, "7", "height", params["height"].toInt());
    }

    return workflow;
}

QJsonObject WorkflowManager::buildUpscale(const QMap<QString, QVariant>& params)
{
    QJsonObject workflow = loadTemplate(":/workflows/upscale");
    if (workflow.isEmpty()) return QJsonObject();

    // 1. 设置原图路径 (Node 6: LoadImage)
    if (params.contains("image_path")) {
        setNodeInput(workflow, "6", "image", params["image_path"].toString());
    }

    // 2. 设置种子 (Node 2: SeedVR2)
    if (params.contains("seed")) {
        setNodeInput(workflow, "2", "seed", params["seed"].toLongLong());
    }

    return workflow;
}

QJsonObject WorkflowManager::buildImageToImage(const QMap<QString, QVariant>& params)
{
    // 这里预留给图生图逻辑
    // 逻辑类似：loadTemplate(":/workflows/render") -> setNodeInput...
    Q_UNUSED(params);
    return QJsonObject();
}

// ---------------------------------------------------------
// 工具函数实现
// ---------------------------------------------------------

void WorkflowManager::setNodeInput(QJsonObject& workflow, const QString& nodeId, const QString& inputKey, const QVariant& value)
{
    // 检查节点是否存在
    if (!workflow.contains(nodeId)) {
        qDebug() << "⚠️ Warning: JSON中找不到节点 ID:" << nodeId;
        return;
    }

    // 剥洋葱：workflow -> node -> inputs -> value
    QJsonObject node = workflow[nodeId].toObject();

    if (!node.contains("inputs")) {
        qDebug() << "⚠️ Warning: 节点" << nodeId << "没有 inputs 字段";
        return;
    }

    QJsonObject inputs = node["inputs"].toObject();

    // 根据值的类型进行转换
    // 注意：JSON 数字对应 Qt 的 double/long long
    if (value.typeId() == QMetaType::LongLong || value.typeId() == QMetaType::Int) {
        inputs[inputKey] = value.toLongLong();
    }
    else {
        inputs[inputKey] = value.toString();
    }

    // 封包回去 (这是修改 QJsonObject 的必须步骤，因为它是隐式共享的)
    node["inputs"] = inputs;
    workflow[nodeId] = node;
}


QJsonObject WorkflowManager::loadTemplate(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "❌ 无法加载模板文件:" << resourcePath;
        return QJsonObject();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qDebug() << "❌ JSON 格式错误:" << resourcePath;
        return QJsonObject();
    }

    return doc.object();
}
