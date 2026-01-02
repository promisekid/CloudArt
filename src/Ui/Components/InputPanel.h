/**
 * @file InputPanel.h
 * @brief 输入面板组件头文件
 * 
 * 该文件定义了InputPanel类，作为应用程序底部的输入控制面板。
 * 包含工作流按钮、参考图按钮和状态更新功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QPushButton>
#include <QToolButton> // 使用 ToolButton 做纯图标按钮更合适
#include <QLineEdit>
#include <QMenu>
#include "../../Model/WorkflowTypes.h"

/**
 * @brief 输入面板类
 * 
 * 继承自QWidget，提供工作流选择、参考图上传和文本输入功能。
 * 包含按钮组件、输入框和状态管理功能。
 */
class InputPanel : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit InputPanel(QWidget *parent = nullptr);

    /**
     * @brief 获取工作流按钮
     * @return QPushButton* 工作流按钮指针
     */
    QPushButton* getWorkflowBtn() const { return m_btnWorkflow; }
    
    /**
     * @brief 获取参考图按钮
     * @return QToolButton* 参考图按钮指针
     */
    QToolButton* getRefBtn() const { return m_btnRef; }

    /**
     * @brief 获取生成按钮
     * @return QPushButton* 生成按钮指针
     */
    QPushButton* getGenerateBtn() const { return m_btnGenerate; }

    /**
     * @brief 获取输入框
     * @return QLineEdit* 输入框指针
     */
    QLineEdit* getInputEdit() const { return m_inputEdit; }

    /**
     * @brief 根据工作流类型更新UI状态
     * @param type 工作流类型
     */
    void updateState(WorkflowType type);

    // 【新增】获取当前选择的分辨率
    QSize currentResolution() const;

    // 【新增】获取反推按钮
    QToolButton* getInterrogateBtn() const { return m_btnInterrogate; }

signals:
    /**
     * @brief 生成按钮点击信号
     * @param prompt 输入的提示词文本
     */
    void generateClicked(const QString& prompt);

    // 【新增】比例改变信号（可选，如果MainWindow需要立即知道）
    void resolutionChanged(int w, int h);

private slots:
    /**
     * @brief 处理生成按钮点击
     */
    void onGenerateClicked();

    // // 【新增】处理比例选择
    // void onRatioSelected(QAction* action);

private:
    void setupRatioMenu(); // 初始化菜单

private:
    QPushButton* m_btnWorkflow;
    QToolButton* m_btnRef;

    // 【新增】比例按钮
    QToolButton* m_btnRatio;
    QMenu* m_ratioMenu;
    QSize m_currentResolution; // 当前选中的分辨率

    QLineEdit* m_inputEdit;
    QPushButton* m_btnGenerate;

    // QToolButton* m_btnInterrogate; // 【新增】
};
