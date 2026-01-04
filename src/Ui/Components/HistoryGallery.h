#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector>

class HistoryGallery : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryGallery(QWidget *parent = nullptr);

    // 刷新显示图片
    void loadImages();

signals:
    // 当点击某张图片时触发信号
    void imageClicked(const QString& imagePath);

private:
    void setupUi();
    // 清除当前的列表项
    void clearLayout();

private:
    // 使用 ScrollArea 实现滚轮和长列表
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;    // 滚动区域里的实体容器
    QVBoxLayout* m_scrollLayout; // 垂直布局（单列）
};
