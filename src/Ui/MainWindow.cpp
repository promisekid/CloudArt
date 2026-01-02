/**
 * @file MainWindow.cpp
 * @brief 主窗口实现文件
 * 
 * 该文件实现了MainWindow类，作为应用程序的主窗口。
 * 包含界面布局、组件管理、信号连接和事件处理等功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "MainWindow.h"
#include "Components/SessionList.h"
#include "Components/ChatArea.h"
#include "Components/InputPanel.h"      // 必须包含这个
#include "Components/WorkflowSelector.h"
#include "Components/ReferencePopup.h"  // 必须包含这个
#include "Components/ChatBubble.h"
#include "../Network/ComfyApiService.h"  // 新增
#include "../Core/WorkflowManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QTimer>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <limits>
#include <QDebug>
#include <QRandomGenerator> // 用于生成随机种子
#include <QStandardPaths>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 初始化主窗口，设置窗口属性并创建UI界面。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_leftStack(nullptr)
    , m_sessionList(nullptr)
    , m_chatArea(nullptr)
    , m_inputPanel(nullptr)
    , m_wfSelector(nullptr)
    , m_refPopup(nullptr)
    , m_leftContainerVisible(true)
    , m_leftContainerOriginalWidth(250) // 默认宽度
    , m_currentPageIndex(0) // 默认显示会话列表
    , m_historyWindow(nullptr)
    , m_historyBtn(nullptr)
    , m_leftContainerAnimation(nullptr)
    , m_mainLayout(nullptr)
    , m_apiService(nullptr) // 新增
{
    setupUi();
}

/**
 * @brief 析构函数
 * 
 * Qt会自动清理子控件，无需手动删除。
 */
MainWindow::~MainWindow()
{
}

/**
 * @brief 初始化UI界面
 * 
 * 创建主窗口的所有UI组件，包括：
 * - 中心部件和主布局
 * - 左侧会话列表
 * - 右侧聊天区域和输入面板
 * - 浮动窗口（工作流选择器和参考图弹窗）
 * - 信号连接和初始状态设置
 */
void MainWindow::setupUi()
{
    this->resize(1280, 800);
    this->setWindowTitle("CloudArt");
    // 1. 创建中心部件

    QWidget* central = new QWidget(this);
    this->setCentralWidget(central);

    // 2. 顶级水平布局 (左侧列表 | 右侧工作区)
    m_mainLayout = new QHBoxLayout(central);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // --- 左侧容器堆栈 ---
    m_leftStack = new QStackedWidget(central);
    
    // 添加会话列表页面
    m_sessionList = new SessionList(m_leftStack);
    m_leftStack->addWidget(m_sessionList);
    
    // 添加历史记录页面
    m_historyWindow = new QWidget(m_leftStack);
    m_historyWindow->setStyleSheet(
        "QWidget { "
        "  background-color: #2A2B32; "
        "  border-right: 1px solid #40414F; "
        "}"
    );
    m_leftStack->addWidget(m_historyWindow);
    
    // 设置默认显示会话列表
    m_leftStack->setCurrentIndex(0);
    
    // 保存左侧容器的初始宽度，最大不超过250
    m_leftContainerOriginalWidth = 250;
    m_leftStack->setMaximumWidth(250);
    
    m_mainLayout->insertWidget(0, m_leftStack);

    // --- 右侧容器 ---
    QWidget* rightWidget = new QWidget(central);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // A. 右上：聊天区域 (ChatArea) - 占据主要空间 (Stretch = 1)
    m_chatArea = new ChatArea(rightWidget);
    rightLayout->addWidget(m_chatArea, 1);

    // B. 右下：输入控制板 (InputPanel) - 【这里就是创建它的地方！】
    m_inputPanel = new InputPanel(rightWidget);
    rightLayout->addWidget(m_inputPanel);

    // 将右侧整体加入主布局
    m_mainLayout->addWidget(rightWidget);

    // ---------------------------------------------------------
    // 下面是浮动窗口的初始化 (不加入 Layout，独立存在的)
    // ---------------------------------------------------------

    // 初始化业务管理器
    m_wfManager = new WorkflowManager(this);

    // 1. 初始化工作流选择器
    m_wfSelector = new WorkflowSelector(this);


    // 2. 初始化参考图弹窗
    m_refPopup = new ReferencePopup(this);

    // ---------------------------------------------------------
    // 切换按钮和动画初始化
    // ---------------------------------------------------------
    
    // 创建切换按钮
    m_toggleSessionListBtn = new QToolButton(this);
    m_toggleSessionListBtn->setIcon(QIcon(":/images/HideConversation.png"));
    m_toggleSessionListBtn->setIconSize(QSize(24, 24));
    m_toggleSessionListBtn->setFixedSize(32, 32);
    m_toggleSessionListBtn->setCursor(Qt::PointingHandCursor);
    m_toggleSessionListBtn->setStyleSheet(
        "QToolButton { "
        "  background-color: #40414F; "
        "  border: none; "
        "  border-radius: 4px; "
        "}"
        "QToolButton:hover { "
        "  background-color: #50515F; "
        "}"
    );
    
    // 设置工具提示
    m_toggleSessionListBtn->setToolTip("对话记录");
    
    // 创建动画效果 - 使用minimumWidth和maximumWidth控制收缩
    m_leftContainerAnimation = new QPropertyAnimation(m_leftStack, "minimumWidth", this);
    m_leftContainerAnimation->setDuration(300); // 300毫秒动画
    m_leftContainerAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // 连接按钮点击信号
    connect(m_toggleSessionListBtn, &QToolButton::clicked,
            this, &MainWindow::switchToSessionList);

    // ---------------------------------------------------------
    // 历史记录按钮初始化
    // ---------------------------------------------------------
    
    // 创建历史记录按钮
    m_historyBtn = new QToolButton(this);
    m_historyBtn->setIcon(QIcon(":/images/historypic.png"));
    m_historyBtn->setIconSize(QSize(24, 24));
    m_historyBtn->setFixedSize(32, 32);
    m_historyBtn->setCursor(Qt::PointingHandCursor);
    m_historyBtn->setStyleSheet(
        "QToolButton { "
        "  background-color: #40414F; "
        "  border: none; "
        "  border-radius: 4px; "
        "}"
        "QToolButton:hover { "
        "  background-color: #50515F; "
        "}"
    );
    m_historyBtn->setToolTip("生成记录");
    
    // 连接历史记录按钮点击信号
    connect(m_historyBtn, &QToolButton::clicked,
            this, &MainWindow::switchToHistoryWindow);

    // ---------------------------------------------------------
    // 信号连接
    // ---------------------------------------------------------

    // 1. 点击"选择工作流"按钮 -> 呼出工作流面板
    connect(m_inputPanel->getWorkflowBtn(), &QPushButton::clicked,
            this, &MainWindow::onWorkflowBtnClicked);

    // 2. 点击"参考图(回形针)"按钮 -> 呼出参考图面板
    connect(m_inputPanel->getRefBtn(), &QToolButton::clicked,
            this, &MainWindow::onRefBtnClicked);
            
    // 3. 点击"生成"按钮 -> 处理生成请求
    connect(m_inputPanel, &InputPanel::generateClicked,
            this, &MainWindow::onGenerateClicked);
            
    // 4. 工作流选择器选中工作流 -> 更新界面状态
    connect(m_wfSelector, &WorkflowSelector::workflowSelected,
            this, &MainWindow::onWorkflowSelected);



    // ---------------------------------------------------------
    // 初始状态设置
    // ---------------------------------------------------------

    // 默认我们假设当前是“文生图”模式，所以禁用参考图按钮

    // 默认我们假设当前是“文生图”模式
    m_inputPanel->updateState(WorkflowType::TextToImage);

    // =========================================================
    // 【修复代码】初始化悬浮按钮的位置
    // =========================================================

    // 1. 强制设置初始位置。
    // 注意：此时 m_leftStack->width() 可能还没计算好，所以我们直接使用
    // 已知的 m_leftContainerOriginalWidth (250) 来计算，确保软件一启动按钮就在正确位置。
    if (m_leftContainerVisible) {
        int initialBtnX = m_leftContainerOriginalWidth + 10;
        m_toggleSessionListBtn->move(initialBtnX, 10);

        // 历史按钮在切换按钮下方
        int historyBtnY = 10 + m_toggleSessionListBtn->height() + 10;
        m_historyBtn->move(initialBtnX, historyBtnY);
    } else {
        m_toggleSessionListBtn->move(10, 10);
        int historyBtnY = 10 + m_toggleSessionListBtn->height() + 10;
        m_historyBtn->move(10, historyBtnY);
    }

    // 2. 确保按钮在所有控件的最上层（防止被 Sidebar 遮挡）
    m_toggleSessionListBtn->raise();
    m_historyBtn->raise();

    // 3. 使用 0ms 定时器进行二次校准
    // 这是一个 Qt 常用技巧：0ms 定时器会在当前事件循环结束后（即界面显示、布局计算完成后）立刻执行。
    // 这样能确保 updateToggleButtonPosition 获取到的是 Layout 计算后的真实坐标。
    QTimer::singleShot(0, this, [this](){
        updateToggleButtonPosition();
        updateHistoryButtonPosition();
    });

    // ---------------------------------------------------------
    // API服务初始化
    // ---------------------------------------------------------

    // 初始化服务
    m_apiService = new ComfyApiService(this);

    // 监听连接状态 (为了测试)
    connect(m_apiService, &ComfyApiService::serverConnected, this, [](){
        qDebug() << "主窗口收到消息：ComfyUI 连接成功！✅";
    });

    // 尝试连接本地 ComfyUI (默认端口 8000)
    // 确保你的 ComfyUI 已经启动了！
    m_apiService->connectToHost("127.0.0.1", 8000);

    // =========================================================
    // 【核心逻辑 1】任务提交成功，服务器返回了 ID
    // =========================================================
    connect(m_apiService, &ComfyApiService::promptQueued, this, [this](const QString& promptId){
        // 如果当前有一个正在等待 ID 的气泡
        if (m_tempBubbleForId) {
            qDebug() << "🔗 绑定任务 ID:" << promptId << " 到当前气泡";
            // 存入映射表：以后看到这个 ID，就知道是这个气泡
            m_pendingBubbles.insert(promptId, m_tempBubbleForId);
            // 清空暂存指针，准备下一次使用
            m_tempBubbleForId = nullptr;
        }
    });

    // =========================================================
    // 【核心逻辑 2】图片下载完毕
    // =========================================================
    // 注意：请确保你的 ComfyApiService 信号是这个签名：
    // void imageReceived(const QString& promptId, const QString& filename, const QPixmap& img);
    connect(m_apiService, &ComfyApiService::imageReceived, this,
            [this](const QString& promptId, const QString& filename, const QPixmap& img){

                // 检查这个 ID 是否在我们的等待列表中
                if (m_pendingBubbles.contains(promptId)) {
                    qDebug() << "🖼️ 找到对应的气泡，更新图片...";

                    ChatBubble* bubble = m_pendingBubbles[promptId];
                    if (bubble) {
                        // 1. 气泡变身 (高度瞬间变高)
                        bubble->updateImage(img, filename);


                        QTimer::singleShot(100, this, [this](){
                            m_chatArea->scrollToBottom();
                        });
                    }

                    setJobRunning(false);

                    // 任务完成，从等待列表中移除
                    m_pendingBubbles.remove(promptId);
                } else {
                    // 可能是旧的或者其他来源的图片，直接加到最后（兜底策略）
                    if (m_chatArea) {
                        m_chatArea->addAiImage(img);
                        // 兜底逻辑也要滚
                        QTimer::singleShot(100, this, [this](){ m_chatArea->scrollToBottom(); });
                    }
                }
            });

    connect(m_chatArea, &ChatArea::upscaleRequested, this,
            [this](const QString& serverFileName, const QPixmap& img){

                if (m_isJobRunning) {
                    qDebug() << "⚠️ 任务进行中，忽略高清修复请求";
                    return;
                }

                // 【新增】上锁
                setJobRunning(true);

                qDebug() << "收到高清修复请求，准备回环上传...";

                // 1. 在界面上加个转圈气泡
                m_tempUpscaleBubble = m_chatArea->addLoadingBubble();

                // 2. 将图片保存为本地临时文件
                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                                   + "/temp_upscale_source.png";
                if (img.save(tempPath)) {
                    // 3. 设置标记位，开始上传
                    m_isUploadingForUpscale = true;

                    // 这里的 m_tempBubbleForId 需要指向这个新气泡，以便 upload 完发任务时使用
                    // 但我们在 imageUploaded 里处理发任务，所以这里只需要传文件
                    m_apiService->uploadImage(tempPath);
                } else {
                    qDebug() << "❌ 临时文件保存失败";
                    // 应该删除转圈气泡...
                    setJobRunning(false);
                }
            });

    connect(m_apiService, &ComfyApiService::imageUploaded, this, [this](const QString& serverName){

        // --- 分支：如果是为了高清修复 ---
        if (m_isUploadingForUpscale) {
            qDebug() << "🔄 高清修复原图上传完毕 (" << serverName << ")，开始发送生成任务...";

            // 1. 复位标记
            m_isUploadingForUpscale = false;

            // 2. 准备参数
            QMap<QString, QVariant> params;
            params["image_path"] = serverName; // 填入刚才上传返回的文件名

            qint64 seed = QRandomGenerator::global()->generate();
            if (seed < 0) seed = -seed;
            params["seed"] = seed;

            // 3. 构建高清修复工作流
            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::Upscale, params);

            // 4. 绑定气泡 ID
            // 把刚才创建的转圈气泡 (m_tempUpscaleBubble) 转移给 m_tempBubbleForId
            // 这样当 queuePrompt 返回 promptID 时，就会自动绑定到这个气泡
            m_tempBubbleForId = m_tempUpscaleBubble;
            m_tempUpscaleBubble = nullptr;

            // 5. 发送任务
            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }
            return;
        }

        // (未来这里还可以加 else if 处理图生图的上传逻辑)
    });
}

/**
 * @brief 工作流按钮点击事件处理
 * 
 * 当用户点击工作流选择按钮时，弹出工作流选择器窗口。
 * 窗口位置自动计算在按钮上方居中显示。
 */
void MainWindow::onWorkflowBtnClicked() {
    // 获取按钮位置，让面板出现在按钮上方
    QPushButton* btn = m_inputPanel->getWorkflowBtn();
    if (btn) {
        QPoint btnPos = btn->mapToGlobal(QPoint(btn->width() / 2, 0));
        m_wfSelector->popup(btnPos);
    }
}

/**
 * @brief 参考图按钮点击事件处理
 * 
 * 当用户点击参考图按钮时，切换参考图选择窗口的显示和隐藏状态。
 * 如果窗口已显示，则隐藏；如果窗口已隐藏，则显示。
 */
void MainWindow::onRefBtnClicked() {
    // 如果窗口已显示，则隐藏；否则显示
    if (m_refPopup->isVisible()) {
        m_refPopup->hide();
    } else {
        // 获取按钮位置，让面板出现在按钮上方
        QToolButton* btn = m_inputPanel->getRefBtn();
        if (btn) {
            QPoint btnPos = btn->mapToGlobal(QPoint(btn->width() / 2, 0));
            m_refPopup->popup(btnPos);
        }
    }
}

/**
 * @brief 工作流选择事件处理
 * @param info 选中的工作流信息
 * 
 * 当用户从工作流选择器中选择工作流时，更新输入面板状态。
 * 根据工作流类型启用或禁用相关功能按钮。
 */
void MainWindow::onWorkflowSelected(const WorkflowInfo& info)
{
    // 根据工作流类型更新输入面板状态
    m_inputPanel->updateState(info.type);

    // 【新增】记录当前类型，供生成时使用
    m_currentWorkflowType = info.type;
    
    qDebug() << "切换到工作流:" << info.name << " (ID:" << info.id << ")";
}

/**
 * @brief 生成按钮点击事件处理
 * @param prompt 用户输入的提示词
 * 
 * 当用户点击生成按钮时，处理生成请求。
 * 只有在输入框有内容时才会触发此信号。
 * 首先在聊天区域添加用户对话，然后处理生成逻辑。
 */
void MainWindow::onGenerateClicked(const QString& prompt)
{
    // 【新增】检查锁
    if (m_isJobRunning) return;

    qDebug() << "生成请求 - 提示词:" << prompt;

    // 【新增】上锁
    setJobRunning(true);

    qDebug() << "生成请求 - 提示词:" << prompt;

    // 1. 界面显示用户气泡
    if (m_chatArea) {
        m_chatArea->addUserMessage(prompt);
    }

    // 2. 立即在界面上添加一个“转圈圈”的 AI 气泡 (左侧占位)
    // 这个函数会返回新创建的气泡指针，我们需要拿住它
    ChatBubble* loadingBubble = m_chatArea->addLoadingBubble();

    // 【关键】把它暂存起来，因为下一行 send 还是异步的，ID 还没回来
    m_tempBubbleForId = loadingBubble;

    // 2. 准备参数包 (Map)
    QMap<QString, QVariant> params;

    // 参数 A: 提示词
    params["prompt"] = prompt;

    // 参数 B: 随机种子 (ComfyUI 需要一个大整数)
    qint64 seed = QRandomGenerator::global()->generate();
    if (seed < 0) seed = -seed; // 转正数
    params["seed"] = seed;

    // [参数] 分辨率 (从 InputPanel 获取)
    // 只有文生图模式才需要这个，但传进去也无妨，WorkflowManager 内部会判断
    if (m_currentWorkflowType == WorkflowType::TextToImage) {
        QSize size = m_inputPanel->currentResolution();
        // 如果 InputPanel 还没设置过，给个默认值 1024x1024
        if (size.isEmpty()) size = QSize(1024, 1024);

        params["width"] = size.width();
        params["height"] = size.height();

        qDebug() << "设定分辨率:" << size.width() << "x" << size.height();
    }

    // [参数] 参考图 (如果是图生图模式)
    if (m_currentWorkflowType == WorkflowType::ImageToImage) {
        // 这里需要你之前实现的上传逻辑返回的服务器路径
        // 假设你把路径存到了 m_uploadedRefImagePath 变量里
        // params["image_path"] = m_uploadedRefImagePath;
    }


    qDebug() << "正在构建工作流, 类型:" << (int)m_currentWorkflowType << " 种子:" << seed;

    // 3. 调用管理器构建 JSON
    QJsonObject workflow = m_wfManager->buildWorkflow(m_currentWorkflowType, params);

    // 4. 检查并发送
    if (workflow.isEmpty()) {
        qDebug() << "❌ 工作流构建失败";
        setJobRunning(false); // 【新增】解锁
        return;
    }

    if (m_apiService) {
        m_apiService->queuePrompt(workflow);
    } else {
        qDebug() << "❌ ApiService 未初始化";
    }
}

/**
 * @brief 切换左侧容器显示状态
 * 
 * 当左侧容器不在场时，呼出容器；
 * 当左侧容器在场时，收起容器。
 */
void MainWindow::onToggleLeftContainer()
{
    if (m_leftContainerVisible) {
        // 收起左侧容器 - 向左动画收缩到0宽度
        m_leftContainerAnimation->setStartValue(m_leftStack->width());
        m_leftContainerAnimation->setEndValue(0);
        m_leftContainerAnimation->start();
        
        // 设置maximumWidth为0，确保完全隐藏
        m_leftStack->setMaximumWidth(0);
        
        m_leftContainerVisible = false;
        // 工具提示保持固定文本"对话记录"和"生成记录"
        
        // 更新按钮位置
        QTimer::singleShot(300, this, [this]() {
            updateToggleButtonPosition();
            updateHistoryButtonPosition();
        });
    } else {
        // 呼出左侧容器 - 向右动画恢复到原始宽度
        // 设置最大宽度限制为250
        m_leftStack->setMaximumWidth(250);
        
        // 使用保存的初始宽度
        m_leftContainerAnimation->setStartValue(0);
        m_leftContainerAnimation->setEndValue(m_leftContainerOriginalWidth);
        m_leftContainerAnimation->start();
        
        m_leftContainerVisible = true;
        // 工具提示保持固定文本"对话记录"和"生成记录"
        
        // 更新按钮位置
        QTimer::singleShot(300, this, [this]() {
            updateToggleButtonPosition();
            updateHistoryButtonPosition();
        });
    }
}

/**
 * @brief 更新切换按钮位置
 * 
 * 根据左侧容器的显示状态，调整切换按钮的位置。
 * 左侧容器显示时按钮在容器右侧上方，左侧容器隐藏时按钮在窗口左上角。
 */
void MainWindow::updateToggleButtonPosition()
{
    if (m_leftContainerVisible) {
        // 左侧容器显示时，按钮在容器右侧上方
        // 使用容器的实际宽度来计算位置
        int containerWidth = m_leftStack->width();
        if (containerWidth <= 0) {
            containerWidth = 250; // 默认宽度
        }
        // 按钮放在容器右侧，距离容器右边框10像素
        QPoint pos = m_leftStack->mapToParent(QPoint(containerWidth + 10, 10));
        m_toggleSessionListBtn->move(pos);
    } else {
        // 左侧容器隐藏时，按钮在窗口左上角
        m_toggleSessionListBtn->move(10, 10);
    }
    
    m_toggleSessionListBtn->raise(); // 确保按钮在最上层
}

/**
 * @brief 窗口大小改变事件处理
 * @param event 窗口大小改变事件
 * 
 * 当窗口大小改变时，更新切换按钮的位置。
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateToggleButtonPosition();
    updateHistoryButtonPosition();
}

/**
 * @brief 切换到会话列表页面
 * 
 * 当左侧容器不在场时，呼出容器并显示会话列表；
 * 当左侧容器在场时，切换到会话列表页面。
 */
void MainWindow::switchToSessionList()
{
    if (!m_leftContainerVisible) {
        // 左侧容器不在场，先呼出容器
        onToggleLeftContainer();
        // 设置当前页面为会话列表
        m_leftStack->setCurrentIndex(0);
        m_currentPageIndex = 0;
    } else if (m_currentPageIndex != 0) {
        // 左侧容器在场但当前不是会话列表，切换到会话列表
        m_leftStack->setCurrentIndex(0);
        m_currentPageIndex = 0;
        // 工具提示保持固定文本"对话记录"和"生成记录"
    } else {
        // 左侧容器在场且当前是会话列表，收起容器
        onToggleLeftContainer();
    }
}

/**
 * @brief 切换到历史记录页面
 * 
 * 当左侧容器不在场时，呼出容器并显示历史记录；
 * 当左侧容器在场时，切换到历史记录页面。
 */
void MainWindow::switchToHistoryWindow()
{
    if (!m_leftContainerVisible) {
        // 左侧容器不在场，先呼出容器
        onToggleLeftContainer();
        // 设置当前页面为历史记录
        m_leftStack->setCurrentIndex(1);
        m_currentPageIndex = 1;
    } else if (m_currentPageIndex != 1) {
        // 左侧容器在场但当前不是历史记录，切换到历史记录
        m_leftStack->setCurrentIndex(1);
        m_currentPageIndex = 1;
        // 工具提示保持固定文本"对话记录"和"生成记录"
    } else {
        // 左侧容器在场且当前是历史记录，收起容器
        onToggleLeftContainer();
    }
}

/**
 * @brief 更新历史按钮位置
 * 
 * 历史按钮始终位于切换按钮的下方，保持与切换按钮相同的水平位置。
 */
void MainWindow::updateHistoryButtonPosition()
{
    // 历史按钮位于切换按钮的下方
    QPoint togglePos = m_toggleSessionListBtn->pos();
    int buttonHeight = m_toggleSessionListBtn->height();
    
    // 历史按钮放在切换按钮下方，保持相同的水平位置
    QPoint pos = QPoint(togglePos.x(), togglePos.y() + buttonHeight + 10);
    m_historyBtn->move(pos);
    
    m_historyBtn->raise(); // 确保按钮在最上层
}


void MainWindow::setJobRunning(bool running)
{
    m_isJobRunning = running;

    // 1. 生成按钮变态
    QPushButton* btnGen = m_inputPanel->getGenerateBtn();
    btnGen->setEnabled(!running);
    btnGen->setText(running ? "生成中..." : "生成");

    // 2. 输入框禁用（防止生成过程中改词）
    m_inputPanel->getInputEdit()->setEnabled(!running);

}
