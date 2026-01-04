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

    /**
     * @brief 添加加载态气泡
     * @return ChatBubble* 新创建的加载气泡指针
     */
    ChatBubble* addLoadingBubble();

    /**
     * @brief 自动滚动到底部
     */
    void scrollToBottom();

    /**
     * @brief 处理流式文字
     * @param token 流式文本片段
     * @param finished 是否完成
     */
    void handleStreamToken(const QString& token, bool finished);

    /**
     * @brief 添加用户图片消息
     * @param img 用户上传的图片
     */
    void addUserImage(const QPixmap& img);

    /**
     * @brief 直接添加AI文本消息（用于加载历史，无动画）
     * @param text AI回复的文本内容
     */
    void addAiMessage(const QString& text);

signals:
    /**
     * @brief 高清修复请求信号
     * @param filename 服务器文件名
     * @param img 图片数据
     */
    void upscaleRequested(const QString& filename, const QPixmap& img);

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();
    


private:
    QScrollArea* m_scrollArea = nullptr; ///< 滚动区域组件
    QWidget* m_scrollContent = nullptr; ///< 滚动内容组件
    QVBoxLayout* m_contentLayout = nullptr; ///< 内容布局
    int m_currentSessionId = -1; ///< 当前会话ID，-1表示无选中会话
    // 【新增】记录当前正在"打字"的气泡指针
    ChatBubble* m_currentStreamBubble = nullptr;

};
