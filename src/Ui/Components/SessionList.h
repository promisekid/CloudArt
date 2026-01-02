/**
 * @file SessionList.h
 * @brief 会话列表组件头文件
 * 
 * 该文件定义了SessionList类，作为应用程序左侧的会话管理面板。
 * 包含会话项的添加、选择、删除、重命名等功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QScrollArea>

class SessionItem;
class QPushButton;

/**
 * @brief 会话列表类
 * 
 * 继承自QWidget，管理多个会话项，提供会话的增删改查功能。
 * 支持会话切换、删除、重命名和新建操作。
 */
class SessionList : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SessionList(QWidget *parent = nullptr);
    
    /**
     * @brief 添加新会话
     * @param id 会话ID
     * @param title 会话标题
     */
    void addSession(int id, const QString& title);

signals:
    /**
     * @brief 会话切换请求信号
     * @param id 目标会话ID
     */
    void sessionSwitchRequest(int id);
    
    /**
     * @brief 会话删除请求信号
     * @param id 目标会话ID
     */
    void sessionDeleteRequest(int id);
    
    /**
     * @brief 会话重命名请求信号
     * @param id 目标会话ID
     * @param newName 新会话名称
     */
    void sessionRenameRequest(int id, const QString& newName);
    
    /**
     * @brief 新建会话请求信号
     */
    void createNewSessionRequest();

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();

    /**
     * @brief 处理会话项选择事件
     * @param clickedItem 被点击的会话项指针
     */
    void handleItemSelection(SessionItem* clickedItem);

private:
    QVBoxLayout* m_mainLayout; ///< 主布局
    QVBoxLayout* m_scrollLayout; ///< 滚动区域布局
    QPushButton* m_btnNew; ///< 新建会话按钮

    SessionItem* m_currentSessionItem = nullptr; ///< 当前选中的会话项指针
};
