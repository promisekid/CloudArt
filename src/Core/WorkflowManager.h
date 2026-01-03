#pragma once
#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QVariant>
#include "../Model/WorkflowTypes.h"

class WorkflowManager : public QObject
{
    Q_OBJECT
public:
    explicit WorkflowManager(QObject *parent = nullptr);

    /**
     * @brief 构建工作流 JSON
     * @param type 工作流类型 (例如 TextToImage)
     * @param params 用户输入的参数 (例如 prompt, seed)
     * @return 准备好发送给 API 的 JSON 对象
     */
    QJsonObject buildWorkflow(WorkflowType type, const QMap<QString, QVariant>& params);

private:
    // 加载资源文件中的 JSON
    QJsonObject loadTemplate(const QString& resourcePath);

    // 【新增】通用修改器：设置指定节点的输入参数值
    // workflow: 引用传递，直接修改
    // nodeId: 节点ID (如 "3")
    // inputKey: 参数名 (如 "seed", "text")
    // value: 参数值
    void setNodeInput(QJsonObject& workflow, const QString& nodeId, const QString& inputKey, const QVariant& value);

    // --- 具体工作流构建函数 ---

    // 构建文生图
    QJsonObject buildTextToImage(const QMap<QString, QVariant>& params);

    // 构建高清修复
    QJsonObject buildUpscale(const QMap<QString, QVariant>& params);

    // 构建图生图 (预留)
    QJsonObject buildImageToImage(const QMap<QString, QVariant>& params);

    // 【新增】构建视觉反推工作流
    QJsonObject buildVisionCaption(const QMap<QString, QVariant>& params);
};
