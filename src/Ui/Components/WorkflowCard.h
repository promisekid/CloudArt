/**
 * @file WorkflowCard.h
 * @brief 工作流卡片组件头文件
 * 
 * 该文件定义了WorkflowCard类，用于显示工作流卡片，支持静态图片和GIF动画背景，
 * 包含鼠标悬停效果、缩放动画和文字信息展示功能。
 * 
 * @author 系统自动生成
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "../../Model/WorkflowTypes.h"

/**
 * @class WorkflowCard
 * @brief 工作流卡片组件类
 * 
 * 该类实现了一个可交互的工作流卡片，支持以下功能：
 * - 显示工作流名称和描述
 * - 支持静态图片和GIF动画背景
 * - 鼠标悬停时播放GIF动画
 * - 缩放动画效果
 * - 点击事件处理
 */
class WorkflowCard : public QWidget {
    Q_OBJECT
    // 添加一个属性用于缩放动画
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    /**
     * @brief 构造函数
     * @param info 工作流信息结构体
     * @param parent 父窗口指针
     */
    explicit WorkflowCard(const WorkflowInfo& info, QWidget* parent = nullptr);
    
    /**
     * @brief 获取工作流信息
     * @return 工作流信息结构体引用
     */
    const WorkflowInfo& workflowInfo() const { return m_info; }
    
    /**
     * @brief 获取当前缩放比例
     * @return 缩放比例值
     */
    qreal scale() const { return m_scale; }
    
    /**
     * @brief 设置缩放比例
     * @param scale 缩放比例值
     */
    void setScale(qreal scale);

signals:
    /**
     * @brief 卡片点击信号
     * @param info 被点击的工作流信息
     */
    void clicked(const WorkflowInfo& info);
    
    /**
     * @brief 缩放变化信号
     * @param scale 新的缩放比例值
     */
    void scaleChanged(qreal scale);

protected:
    /**
     * @brief 绘制事件处理
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief 鼠标进入事件处理
     * @param event 鼠标进入事件
     */
    void enterEvent(QEnterEvent* event) override;
    
    /**
     * @brief 鼠标离开事件处理
     * @param event 鼠标离开事件
     */
    void leaveEvent(QEvent* event) override;
    
    /**
     * @brief 鼠标按下事件处理
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent* event) override;
    
    /**
     * @brief 窗口大小改变事件处理
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief 设置UI界面
     */
    void setupUi();
    
    /**
     * @brief 开始GIF动画
     */
    void startGifAnimation();
    
    /**
     * @brief 停止GIF动画
     */
    void stopGifAnimation();
    
    /**
     * @brief 更新背景图片
     */
    void updateBackgroundImage();

private:
    WorkflowInfo m_info;              ///< 工作流信息
    qreal m_scale = 1.0;              ///< 当前缩放比例
    qreal m_currentScale = 1.0;       ///< 实际缩放比例
    bool m_isHovering = false;        ///< 鼠标悬停状态
    
    // UI组件
    QLabel* m_nameLabel = nullptr;        ///< 名称标签
    QLabel* m_descriptionLabel = nullptr; ///< 描述标签
    QLabel* m_backgroundLabel = nullptr;  ///< 背景图片标签
    
    // 动画相关
    QMovie* m_movie = nullptr;                ///< GIF动画对象
    QPropertyAnimation* m_scaleAnimation = nullptr; ///< 缩放动画对象
    
    // 静态图片和GIF路径
    QString m_imagePath; ///< 静态图片路径
    QString m_gifPath;   ///< GIF动画路径
};