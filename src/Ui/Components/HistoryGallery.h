/**
 * @file HistoryGallery.h
 * @brief 历史记录画廊组件头文件
 * 
 * 该文件定义了HistoryGallery类，用于显示所有生成的图片历史记录。
 * 支持图片列表展示、滚动浏览和点击查看功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector>

/**
 * @brief 历史记录画廊类
 * 
 * 继承自QWidget，提供历史图片的展示功能。
 * 使用滚动区域支持大量图片的浏览，点击图片可触发查看事件。
 */
class HistoryGallery : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit HistoryGallery(QWidget *parent = nullptr);

    /**
     * @brief 刷新显示图片
     * 
     * 从数据库加载所有生成的图片并显示在画廊中
     */
    void loadImages();

signals:
    /**
     * @brief 图片点击信号
     * @param imagePath 被点击的图片路径
     */
    void imageClicked(const QString& imagePath);

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();

    /**
     * @brief 清除当前的列表项
     * 
     * 清空滚动区域中的所有图片项
     */
    void clearLayout();

private:
    QScrollArea* m_scrollArea = nullptr; ///< 滚动区域组件
    QWidget* m_scrollContent = nullptr; ///< 滚动区域里的实体容器
    QVBoxLayout* m_scrollLayout = nullptr; ///< 垂直布局（单列）
};
