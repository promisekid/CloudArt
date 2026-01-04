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
#include <QToolButton>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include "../../Model/WorkflowTypes.h"

/**
 * @brief 输入面板类
 * 
 * 继承自QWidget，提供工作流选择、参考图上传和文本输入功能。
 * 包含按钮组件、输入框和状态管理功能。
 */
class InputPanel : public QWidget
{
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
     * @return QPlainTextEdit* 输入框指针
     */
    QPlainTextEdit* getInputEdit() const { return m_inputEdit; }

    /**
     * @brief 根据工作流类型更新UI状态
     * @param type 工作流类型
     */
    void updateState(WorkflowType type);

    /**
     * @brief 获取当前选择的分辨率
     * @return QSize 当前分辨率
     */
    QSize currentResolution() const;

    /**
     * @brief 获取反推按钮
     * @return QToolButton* 反推按钮指针
     */
    QToolButton* getInterrogateBtn() const { return m_btnInterrogate; }

    /**
     * @brief 锁定/解锁面板所有控件
     * @param locked 是否锁定
     */
    void setLocked(bool locked);

    /**
     * @brief 设置连接状态
     * @param isConnected 是否已连接
     * 
     * 专门用于网络断开时的锁定
     */
    void setConnectionStatus(bool isConnected);

signals:
    /**
     * @brief 生成按钮点击信号
     * @param prompt 输入的提示词文本
     */
    void generateClicked(const QString& prompt);

    /**
     * @brief 分辨率改变信号
     * @param w 宽度
     * @param h 高度
     */
    void resolutionChanged(int w, int h);

protected:
    /**
     * @brief 重写事件过滤器，处理回车键
     * @param obj 被监视的对象
     * @param e 事件对象
     * @return bool 是否处理了该事件
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

private slots:
    /**
     * @brief 处理生成按钮点击
     */
    void onGenerateClicked();

    /**
     * @brief 处理比例选择
     * @param action 被选中的动作
     */
    void onRatioSelected(QAction* action);

    /**
     * @brief 文本变化时调整高度
     */
    void adjustInputHeight();

private:
    /**
     * @brief 初始化菜单
     */
    void setupRatioMenu();

private:
    QPushButton* m_btnWorkflow = nullptr; ///< 工作流按钮
    QToolButton* m_btnRef = nullptr; ///< 参考图按钮
    QToolButton* m_btnRatio = nullptr; ///< 比例按钮
    QMenu* m_ratioMenu = nullptr; ///< 比例菜单
    QSize m_currentResolution = QSize(1024, 1024); ///< 当前选中的分辨率
    QPlainTextEdit* m_inputEdit = nullptr; ///< 输入框
    QPushButton* m_btnGenerate = nullptr; ///< 生成按钮
    QToolButton* m_btnInterrogate = nullptr; ///< 反推按钮
};
