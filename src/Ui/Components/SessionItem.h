/**
 * @file SessionItem.h
 * @brief 会话项组件头文件
 * 
 * 该文件定义了SessionItem类，作为会话列表中的单个会话项。
 * 包含会话标题显示、选中状态、右键菜单和事件处理功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMenu>

class QLabel;
class QToolButton;

/**
 * @brief 会话项类
 * 
 * 继承自QWidget，表示会话列表中的单个会话项目。
 * 支持选中状态、标题编辑、删除和重命名操作。
 */
class SessionItem : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param id 会话ID
     * @param title 会话标题
     * @param parent 父窗口指针
     */
    explicit SessionItem(int id, const QString& title, QWidget *parent = nullptr);

    /**
     * @brief 获取会话ID
     * @return int 会话ID
     */
    int id() const { return m_id; }

    /**
     * @brief 设置选中状态
     * @param selected 是否选中
     */
    void setSelected(bool selected);

    /**
     * @brief 更新标题
     * @param newTitle 新标题
     */
    void setTitle(const QString& newTitle);



signals:
    /**
     * @brief 项点击信号
     * @param item 被点击的会话项指针
     */
    void itemClicked(SessionItem* item);
    
    /**
     * @brief 项删除信号
     * @param id 被删除的会话ID
     */
    void itemDeleted(int id);
    
    /**
     * @brief 项重命名信号
     * @param id 被重命名的会话ID
     * @param newName 新会话名称
     */
    void itemRenamed(int id, const QString& newName);

protected:
    /**
     * @brief 鼠标进入事件处理
     * @param event 鼠标进入事件
     */
    void enterEvent(QEnterEvent *event) override;
    
    /**
     * @brief 鼠标离开事件处理
     * @param event 鼠标离开事件
     */
    void leaveEvent(QEvent *event) override;
    
    /**
     * @brief 鼠标按下事件处理
     * @param event 鼠标按下事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    
    /**
     * @brief 窗口大小改变事件处理
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();
    
    /**
     * @brief 更新标题文本显示
     */
    void updateTitleText();
    
    /**
     * @brief 显示右键菜单
     */
    void showMenu();

private:
    int m_id = -1; ///< 会话ID
    QString m_fullTitle = ""; ///< 完整标题
    bool m_isSelected = false; ///< 是否选中状态

    QLabel* m_lblTitle = nullptr; ///< 标题标签
    QToolButton* m_btnOption = nullptr; ///< 选项按钮
};
