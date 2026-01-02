/**
 * @file ChatArea.h
 * @brief 聊天区域组件头文件
 * 
 * 该文件定义了ChatArea类，作为应用程序右侧的聊天显示区域。
 * 包含用户消息和AI图片的显示、会话切换和清空功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>


class ChatBubble;

/**
 * @brief 聊天区域类
 * 
 * 继承自QWidget，管理聊天消息的显示和会话切换。
 * 支持用户文字消息和AI图片消息的添加和显示。
 */
class ChatArea : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit ChatArea(QWidget *parent = nullptr);

    /**
     * @brief 清空当前聊天界面
     */
    void clear();

    /**
     * @brief 设置当前会话ID
     * @param id 会话ID
     */
    void setCurrentSessionId(int id) { m_currentSessionId = id; }
    
    /**
     * @brief 获取当前会话ID
     * @return int 当前会话ID，-1表示无选中会话
     */
    int currentSessionId() const { return m_currentSessionId; }

    /**
     * @brief 添加用户文字消息
     * @param text 用户消息文本
     */
    void addUserMessage(const QString& text);

    /**
     * @brief 添加AI图片消息
     * @param img AI生成的图片
     */
    void addAiImage(const QPixmap& img);

    // 【新增】添加一个加载态气泡，并返回指针
    ChatBubble* addLoadingBubble();

    /**
     * @brief 自动滚动到底部
     */
    void scrollToBottom();

signals:
    // 【新增】转发气泡的高清修复请求给 MainWindow
    void upscaleRequested(const QString& filename, const QPixmap& img);

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();
    


private:
    QScrollArea* m_scrollArea; ///< 滚动区域组件
    QWidget* m_scrollContent; ///< 滚动内容组件
    QVBoxLayout* m_contentLayout; ///< 内容布局
    int m_currentSessionId = -1; ///< 当前会话ID，-1表示无选中会话
};
