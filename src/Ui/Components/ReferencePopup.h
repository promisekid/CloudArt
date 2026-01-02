/**
 * @file ReferencePopup.h
 * @brief 参考图上传弹窗组件头文件
 * 
 * 该文件定义了ReferencePopup类，作为参考图上传的浮动窗口。
 * 支持拖拽上传图片、图片预览和状态管理功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QPixmap>

class QLabel;
class QStackedLayout;

/**
 * @brief 参考图上传弹窗类
 * 
 * 继承自QWidget，提供参考图上传功能，支持拖拽操作。
 * 包含图片预览、状态切换和数据管理功能。
 */
class ReferencePopup : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit ReferencePopup(QWidget *parent = nullptr);

    /**
     * @brief 显示弹窗
     * @param pos 弹窗显示位置
     */
    void popup(const QPoint& pos);
    
    /**
     * @brief 隐藏弹窗
     */
    void hide();

    /**
     * @brief 获取当前图片
     * @return QPixmap 当前图片
     */
    QPixmap currentImage() const { return m_currentImage; }
    
    /**
     * @brief 获取图片路径
     * @return QString 图片文件路径
     */
    QString currentPath() const { return m_currentPath; }
    
    /**
     * @brief 检查是否有图片
     * @return bool 是否有图片
     */
    bool hasImage() const { return !m_currentImage.isNull(); }

protected:
    /**
     * @brief 拖拽进入事件处理
     * @param event 拖拽进入事件
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
    
    /**
     * @brief 拖拽释放事件处理
     * @param event 拖拽释放事件
     */
    void dropEvent(QDropEvent *event) override;

    // 焦点失去时隐藏（模拟 Popup 行为）
    // void focusOutEvent(QFocusEvent *event) override;

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();
    
    /**
     * @brief 加载图片处理逻辑
     * @param path 图片文件路径
     */
    void loadImage(const QString& path);
    
    /**
     * @brief 切换UI状态
     */
    void updateUiState();

private:
    // 数据
    QPixmap m_currentImage; ///< 当前图片数据
    QString m_currentPath; ///< 当前图片路径

    // UI 组件
    QStackedLayout* m_stackLayout; ///< 堆叠布局，用于界面切换
    QWidget* m_pageEmpty; ///< 空状态页面
    QWidget* m_pagePreview; ///< 预览状态页面

    QLabel* m_lblPreview; ///< 图片预览标签
};
