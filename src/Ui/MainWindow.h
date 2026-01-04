/**
 * @file MainWindow.h
 * @brief 主窗口头文件
 * 
 * 该文件定义了MainWindow类，作为应用程序的主窗口。
 * 包含界面组件管理、信号槽声明和成员变量定义。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "../Model/WorkflowTypes.h"
#include "../Network/ComfyApiService.h"
#include "../Database/DatabaseManager.h"


// --- 前向声明 (Forward Declarations) ---
// 这样做的好处是不用在这里 #include 所有头文件，编译更快，也不容易报错
class InputPanel;
class WorkflowSelector;
class ReferencePopup;
class SessionList;
class ChatArea;
class QToolButton;
class ComfyApiService;
class WorkflowManager;
class ChatBubble;
class SidebarControl;
class HistoryGallery;

/**
 * @brief 主窗口类
 * 
 * 继承自QMainWindow，作为应用程序的主窗口容器。
 * 管理左侧会话列表、右侧聊天区域和输入面板，以及浮动窗口组件。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    /**
     * @brief 工作流按钮点击槽函数
     */
    void onWorkflowBtnClicked();
    
    /**
     * @brief 参考图按钮点击槽函数
     */
    void onRefBtnClicked();
    
    /**
     * @brief 工作流选择槽函数
     * @param info 选中的工作流信息
     */
    void onWorkflowSelected(const WorkflowInfo& info);
    
    /**
     * @brief 生成按钮点击槽函数
     * @param prompt 用户输入的提示词
     */
    void onGenerateClicked(const QString& prompt);
    
    /**
     * @brief 切换左侧容器显示状态
     */
    void onToggleLeftContainer();
    
    /**
     * @brief 切换到会话列表页面
     */
    void switchToSessionList();
    
    /**
     * @brief 切换到历史记录页面
     */
    void switchToHistoryWindow();

    // 【新增】处理反推按钮点击
    void onInterrogateClicked();

protected:
    /**
     * @brief 窗口大小改变事件处理函数
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief 初始化UI布局
     */
    void setupUi();

    /**
     * @brief 更新侧边栏位置
     */
    void updateSidebarPosition();

    // 【新增】加载所有历史会话
    void loadSessionList();

    // 【新增】创建新会话的逻辑
    void createNewSession();

    /**
     * @brief 切换左侧面板
     * @param targetIndex 目标页面索引
     */
    void switchLeftPanel(int targetIndex);

    QString saveImageToLocal(const QPixmap& img);

    // --- 成员变量 (持有各个组件的指针) ---
    // 左侧
    QStackedWidget* m_leftStack; ///< 左侧容器堆栈
    SessionList* m_sessionList; ///< 会话列表组件

    // 右侧
    ChatArea* m_chatArea; ///< 聊天区域组件
    InputPanel* m_inputPanel; ///< 底部控制面板组件

    // 浮动窗口 (Popups)
    WorkflowSelector* m_wfSelector; ///< 工作流选择面板组件
    ReferencePopup* m_refPopup; ///< 参考图上传面板组件

    // 切换按钮和动画
    SidebarControl* m_sidebarControl; ///< 侧边栏控制组件
    QPropertyAnimation* m_leftContainerAnimation; ///< 左侧容器动画效果
    bool m_leftContainerVisible; ///< 左侧容器是否可见
    int m_leftContainerOriginalWidth; ///< 左侧容器的初始宽度
    int m_currentPageIndex; ///< 当前显示的页面索引
    
    HistoryGallery* m_historyGallery; // 【新增】新的类型
    
    /** 主布局 */
    QHBoxLayout* m_mainLayout;
    
    /** API服务 */
    ComfyApiService* m_apiService;

    // 【新增】业务逻辑管理器
    WorkflowManager* m_wfManager;

    // 【新增】记录当前选中的工作流类型 (默认为文生图)
    WorkflowType m_currentWorkflowType = WorkflowType::TextToImage;

    // 1. 暂存刚刚创建的加载气泡（等待 API 返回 prompt_id）
    ChatBubble* m_tempBubbleForId = nullptr;

    // 2. 映射表：任务ID -> 气泡指针 (用于在图片生成后找到对应的气泡)
    QMap<QString, ChatBubble*> m_pendingBubbles;

    // 【新增】标记当前上传操作是否为了高清修复
    bool m_isUploadingForUpscale = false;

    // 【新增】暂存高清修复的气泡（用于绑定ID）
    ChatBubble* m_tempUpscaleBubble = nullptr;

    // 【新增】是否正在执行任务（忙碌状态）
    bool m_isJobRunning = false;

    // 【新增】切换忙碌状态的辅助函数
    void setJobRunning(bool running);

    // 【新增】标记：当前上传是否是为了反推提示词？
    bool m_isUploadingForInterrogate = false;

    // 【新增】记住反推用的图片名（给后续图生图备用，可选）
    QString m_currentServerRefImg;

    // 【新增】标记：当前上传是否为了图生图生成
    bool m_isUploadingForI2I = false;

    // 【新增】暂存图生图需要的参数 (提示词、种子)，等上传完了一起用
    QMap<QString, QVariant> m_pendingI2IParams;

    // 【新增】用于暂存流式传输的完整文本
    QString m_accumulatedStreamText;

    // 【新增】加载指定会话的历史记录到聊天区
    void loadSessionHistory(int sessionId);


};
