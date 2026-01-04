/**
 * @file SidebarControl.h
 * @brief 侧边栏控制组件头文件
 * 
 * 该文件定义了SidebarControl类，提供侧边栏的切换、历史记录和设置功能。
 * 包含侧边栏展开/收起、历史记录查看和设置按钮等功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QWidget>
#include <QToolButton>

class QVBoxLayout;

/**
 * @brief 侧边栏控制组件类
 * 
 * 继承自QWidget，提供侧边栏的控制功能。
 * 包含侧边栏展开/收起按钮、历史记录按钮和设置按钮。
 */
class SidebarControl : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SidebarControl(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SidebarControl();

    /**
     * @brief 获取切换按钮
     * @return QToolButton* 切换按钮指针
     */
    QToolButton* toggleBtn() const;

    /**
     * @brief 获取历史记录按钮
     * @return QToolButton* 历史记录按钮指针
     */
    QToolButton* historyBtn() const;

    /**
     * @brief 获取设置按钮
     * @return QToolButton* 设置按钮指针
     */
    QToolButton* settingsBtn() const;

    /**
     * @brief 更新切换按钮状态
     * @param isExpanded 侧边栏是否展开
     */
    void updateToggleState(bool isExpanded);

private:
    /**
     * @brief 创建工具按钮
     * @param iconPath 图标路径
     * @param tooltip 工具提示文本
     * @return QToolButton* 创建的按钮指针
     */
    QToolButton* createBtn(const QString& iconPath, const QString& tooltip);

private:
    QVBoxLayout* m_layout; ///< 主布局
    QToolButton* m_toggleBtn; ///< 切换按钮
    QToolButton* m_historyBtn; ///< 历史记录按钮
    QToolButton* m_settingsBtn; ///< 设置按钮
};
