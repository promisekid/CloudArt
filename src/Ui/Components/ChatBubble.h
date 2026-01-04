/**
 * @file ChatBubble.h
 * @brief 聊天气泡组件头文件
 * 
 * 该文件定义了ChatBubble类，作为聊天区域中的消息气泡。
 * 支持用户文本消息和AI图片消息的显示，包含保存和查看功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QMenu>
#include <QMovie>

/**
 * @brief 聊天角色枚举
 * 
 * 定义消息发送者的角色类型。
 */
enum class ChatRole {
    User, ///< 用户角色（靠右显示，文本消息）
    AI    ///< AI角色（靠左显示，图片消息）
};

/**
 * @brief 聊天气泡类
 * 
 * 继承自QWidget，表示聊天区域中的单个消息气泡。
 * 支持不同角色的消息显示、图片保存和查看功能。
 */
class ChatBubble : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param role 消息角色
     * @param data 消息数据（文本或图片）
     * @param parent 父窗口指针
     */
    explicit ChatBubble(ChatRole role, const QVariant& data, QWidget *parent = nullptr);

    /**
     * @brief 控制加载动画
     * @param loading 是否显示加载动画
     */
    void setLoading(bool loading);

    /**
     * @brief 更新图片数据（生成完成后调用）
     * @param img 生成的图片数据
     * @param serverFileName 服务器文件名
     */
    void updateImage(const QPixmap& img, const QString& serverFileName);

    /**
     * @brief 获取服务器文件名（用于高清修复）
     * @return QString 服务器文件名
     */
    QString serverFileName() const { return m_serverFileName; }

    /**
     * @brief 往气泡里追加文字
     * @param text 要追加的文本内容
     */
    void appendText(const QString& text);

signals:
    /**
     * @brief 高清修复请求信号
     * @param fileName 服务器文件名
     * @param img 图片数据
     */
    void upscaleRequested(const QString& fileName, const QPixmap& img);

protected:
    /**
     * @brief 事件过滤器处理
     * @param watched 被监视的对象
     * @param event 事件
     * @return bool 是否处理事件
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /**
     * @brief 初始化UI布局
     * @param data 消息数据
     */
    void setupUi(const QVariant& data);

    /**
     * @brief 初始化文本气泡
     * @param text 文本内容
     */
    void initTextBubble(const QString& text);
    
    /**
     * @brief 初始化图片气泡
     * @param img 图片数据
     */
    void initImageBubble(const QPixmap& img);
    
    /**
     * @brief 保存图片
     */
    void saveImage();
    
    /**
     * @brief 显示图片查看器
     */
    void showViewer();

private:
    ChatRole m_role = ChatRole::User; ///< 消息角色
    QHBoxLayout* m_layout = nullptr; ///< 水平布局
    QPixmap m_currentImage; ///< 当前图片数据
    // 【新增】成员变量
    QLabel* m_contentLabel = nullptr; // 统一管理显示内容的 Label
    QMovie* m_loadingMovie = nullptr; // 加载动画对象
    QString m_serverFileName = "";         // 服务器上的原始文件名
};
