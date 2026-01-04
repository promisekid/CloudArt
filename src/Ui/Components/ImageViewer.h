/**
 * @file ImageViewer.h
 * @brief 图片查看器组件头文件
 * 
 * 该文件定义了ImageViewer类，提供图片查看功能，支持自适应显示、缩放和双击还原。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

/**
 * @brief 图片查看器类
 * 
 * 继承自QDialog，提供图片查看功能，支持自适应显示、鼠标滚轮缩放和双击还原操作。
 */
class ImageViewer : public QDialog {
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param pixmap 要显示的图片
     * @param parent 父窗口指针
     */
    explicit ImageViewer(const QPixmap& pixmap, QWidget* parent = nullptr);

protected:
    /**
     * @brief 窗口大小变化事件处理
     * @param event 大小变化事件
     * 
     * 实现图片自适应窗口大小功能。
     */
    void resizeEvent(QResizeEvent *event) override;
    
    /**
     * @brief 事件过滤器处理
     * @param watched 被监视的对象
     * @param event 事件
     * @return bool 是否处理事件
     * 
     * 监听鼠标滚轮事件，实现图片缩放功能。
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
    
    /**
     * @brief 鼠标双击事件处理
     * @param event 鼠标事件
     * 
     * 监听双击事件，实现图片还原到自适应模式。
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    /**
     * @brief 图片自适应窗口功能
     * 
     * 核心功能：将图片适配到当前窗口大小。
     */
    void fitImageToWindow();

private:
    QGraphicsView* m_view = nullptr;          ///< 图形视图窗口
    QGraphicsScene* m_scene = nullptr;        ///< 图形场景容器
    QGraphicsPixmapItem* m_item = nullptr;    ///< 图片图元对象

    /**
     * @brief 自适应窗口模式标记
     * 
     * true: 窗口变大，图片跟着变大（自适应模式）
     * false: 用户已手动缩放，窗口变大不再影响图片比例
     */
    bool m_isFitWindow = true;
};
