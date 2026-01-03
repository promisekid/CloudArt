/**
 * @file WorkflowTypes.h
 * @brief 工作流类型定义头文件
 * 
 * 该文件定义了工作流相关的枚举类型和数据结构。
 * 包含工作流类型枚举和工作流信息结构体定义。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QString>

/**
 * @brief 工作流类型枚举
 * 
 * 定义了应用程序支持的所有工作流类型。
 */
enum class WorkflowType {
    TextToImage = 1,      ///< 文生图
    ImageToImage = 2,     ///< 图生图
    Inpaint = 3,          ///< 局部重绘
    Upscale = 4,          ///< 图像放大
    StyleTransfer = 5,    ///< 风格转换
    PortraitEnhance = 6,  ///< 人像美化
    BackgroundRemove = 7, ///< 背景移除
    ColorCorrection = 8,   ///< 色彩校正
    VisionCaption = 9 // 【新增】视觉反推
};

/**
 * @brief 工作流信息结构体
 * 
 * 包含单个工作流的完整信息，用于界面显示和功能处理。
 */
struct WorkflowInfo {
    int id; ///< 工作流唯一标识符
    QString name; ///< 工作流名称
    QString imagePath; ///< 工作流图标路径
    QString gifPath; ///< 工作流动画路径（可选）
    QString description; ///< 工作流描述（可选）
    WorkflowType type; ///< 工作流类型
    
    /**
     * @brief 构造函数
     * @param id 工作流ID
     * @param name 工作流名称
     * @param imagePath 图标路径
     * @param gifPath 动画路径（可选，默认为空）
     * @param description 描述信息（可选，默认为空）
     * @param type 工作流类型（可选，默认为文生图）
     */
    WorkflowInfo(int id, const QString& name, const QString& imagePath, 
                 const QString& gifPath = "", const QString& description = "", WorkflowType type = WorkflowType::TextToImage)
        : id(id), name(name), imagePath(imagePath), gifPath(gifPath), description(description), type(type) {}
};
