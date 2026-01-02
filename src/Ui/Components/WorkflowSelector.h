/**
 * @file WorkflowSelector.h
 * @brief 工作流选择器组件头文件
 * 
 * 该文件定义了WorkflowSelector类，用于显示工作流选择弹出窗口。
 * 包含工作流卡片的布局管理、窗口样式设置、事件处理等声明。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include "../../Model/WorkflowTypes.h"

class WorkflowCard;

/**
 * @brief 工作流选择器类
 * 
 * 继承自QWidget，提供工作流选择功能的弹出窗口。
 * 支持显示多个工作流卡片，具有透明背景、阴影效果和自动关闭功能。
 */
class WorkflowSelector : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit WorkflowSelector(QWidget* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WorkflowSelector();
    
    /**
     * @brief 弹出窗口
     * @param pos 弹出位置（相对于屏幕坐标）
     */
    void popup(const QPoint& pos);
    
    /**
     * @brief 设置工作流数据
     * @param workflows 工作流信息向量
     */
    void setWorkflows(const QVector<WorkflowInfo>& workflows);
    
    /**
     * @brief 获取当前工作流列表
     * @return 工作流信息向量引用
     */
    const QVector<WorkflowInfo>& workflows() const { return m_workflows; }

signals:
    /**
     * @brief 工作流选择信号
     * @param info 选中的工作流信息
     */
    void workflowSelected(const WorkflowInfo& info);

protected:
    /**
     * @brief 绘制事件处理
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief 事件处理
     * @param event 事件对象
     * @return 是否处理了该事件
     */
    bool event(QEvent* event) override;

private:
    /**
     * @brief 初始化UI界面
     */
    void setupUi();
    
    /**
     * @brief 创建工作流卡片
     */
    void createWorkflowCards();

private:
    QVector<WorkflowInfo> m_workflows; ///< 工作流数据列表
    QVector<WorkflowCard*> m_workflowCards; ///< 工作流卡片指针列表
    
    // UI组件
    QVBoxLayout* m_mainLayout = nullptr; ///< 主布局
    QWidget* m_container = nullptr; ///< 内容容器
    QVBoxLayout* m_containerLayout = nullptr; ///< 容器布局
    QScrollArea* m_scrollArea = nullptr; ///< 滚动区域
    QWidget* m_scrollContent = nullptr; ///< 滚动内容
    QVBoxLayout* m_cardsLayout = nullptr; ///< 卡片布局
};