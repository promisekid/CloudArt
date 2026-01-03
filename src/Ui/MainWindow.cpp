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
#include "../Model/DataModels.h"

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
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

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

    connect(m_inputPanel->getInterrogateBtn(), &QToolButton::clicked,
            this, &MainWindow::onInterrogateClicked);



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

                // 1. 【新增】保存图片到本地存储
                QString localPath = saveImageToLocal(img);

                // 2. 【新增】存入数据库 (AI Role)
                int currentSid = m_chatArea->currentSessionId();
                if (currentSid != -1 && !localPath.isEmpty()) {
                    MessageData msg(currentSid, MessageRole::AI, "", localPath); // 文本为空，路径有值
                    DatabaseManager::instance().addMessage(msg);
                }

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

        // --- 分支 B: 【新增】视觉反推 ---
        if (m_isUploadingForInterrogate) {
            qDebug() << "反推图片上传成功，正在构建任务...";
            m_isUploadingForInterrogate = false; // 复位标记
            m_currentServerRefImg = serverName;  // 记下来供后续使用

            // 1. 准备参数
            QMap<QString, QVariant> params;
            params["image_path"] = serverName; // 填入刚才上传的文件名

            // 2. 调用管理器构建 JSON
            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::VisionCaption, params);

            // 3. 发送任务
            if (wf.isEmpty()) {
                qDebug() << "❌ 反推工作流构建失败";
                setJobRunning(false); // 记得解锁
                return;
            }

            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }

            // 反推不需要 m_tempBubbleForId，因为它是流式输出，我们会动态创建气泡
            return;
        }

        // --- 分支 C: 【新增】图生图生成接力 ---
        if (m_isUploadingForI2I) {
            qDebug() << "图生图素材上传完毕:" << serverName;
            m_isUploadingForI2I = false; // 复位标记

            // 1. 取出之前暂存的参数 (提示词、种子)
            QMap<QString, QVariant> params = m_pendingI2IParams;

            // 2. 填入最关键的参数：服务器上的文件名
            params["image_path"] = serverName;

            // 3. 构建工作流 JSON
            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::ImageToImage, params);

            // 4. 发送任务
            if (wf.isEmpty()) {
                qDebug() << "❌ 图生图工作流构建失败";
                setJobRunning(false);
                return;
            }

            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }

            // 注意：此时 m_tempBubbleForId 依然指向我们在 onGenerateClicked 里创建的那个气泡
            // 等一会儿 promptQueued 信号回来，就会自动把它和任务 ID 绑定上
            return;
        }



        // (未来这里还可以加 else if 处理图生图的上传逻辑)
    });

    // 【新增】监听流式文本 (反推提示词)
    // =========================================================
    connect(m_apiService, &ComfyApiService::streamTokenReceived, this,
            [this](const QString& token, bool finished){

                // 1. 【新增】累加文本 (只累加有效内容)
                if (!token.isEmpty()) {
                    m_accumulatedStreamText += token;
                }

                // 2. UI 显示 (原有逻辑)
                if (m_chatArea) {
                    m_chatArea->handleStreamToken(token, finished);
                }

                // 3. 结束处理
                if (finished) {
                    qDebug() << "✅ 反推结束，完整文本长度:" << m_accumulatedStreamText.length();

                    // 【新增】存入数据库 (AI Role)
                    int currentSid = m_chatArea->currentSessionId();

                    // 只有当有内容且有会话时才存
                    if (currentSid != -1 && !m_accumulatedStreamText.isEmpty()) {
                        MessageData msg(currentSid, MessageRole::AI, m_accumulatedStreamText);
                        DatabaseManager::instance().addMessage(msg);
                        qDebug() << "💾 反推文本已保存到数据库";
                    }

                    // 【新增】清空缓存，为下次做准备 (可选，双重保险)
                    m_accumulatedStreamText.clear();

                    setJobRunning(false);
                }
            });

    // 左侧列表请求新建会话
    connect(m_sessionList, &SessionList::createNewSessionRequest,
            this, &MainWindow::createNewSession); // 【修改】连到新写的函数

    // 2. 【修复】处理重命名
    connect(m_sessionList, &SessionList::sessionRenameRequest, this,
            [this](int id, const QString& newName){

                // 更新数据库
                DatabaseManager::instance().renameSession(id, newName);
                qDebug() << "会话" << id << "重命名为" << newName;
            });

    // 3. 【修复】处理删除
    connect(m_sessionList, &SessionList::sessionDeleteRequest, this,
            [this](int id){

                // 更新数据库 (级联删除消息)
                DatabaseManager::instance().deleteSession(id);

                // 如果删除的是当前正在看的会话，清空右侧并重置 ID
                if (m_chatArea->currentSessionId() == id) {
                    m_chatArea->clear();
                    m_chatArea->setCurrentSessionId(-1);
                }
                qDebug() << "会话" << id << "已删除";
            });

    // 左侧列表请求切换会话 (之前可能没实现具体逻辑，现在要补上)
    connect(m_sessionList, &SessionList::sessionSwitchRequest, this, [this](int id){
        loadSessionHistory(id);
    });

    // ...

    // 【最后一步】启动时加载数据
    // 放在 setupUi 的最后，或者 show() 之前
    loadSessionList();
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
    // 1. 检查锁
    if (m_isJobRunning) return;

    qDebug() << "生成请求 - 提示词:" << prompt;

    // 【新增】存入数据库 -> UI显示
    // 只有当前有选中的会话才存 (currentSessionId != -1)
    // 如果没有选中会话（比如刚启动），应该先 createNewSession()，这里假设已有
    int currentSid = m_chatArea->currentSessionId();
    if (currentSid != -1) {
        // A. 存库
        MessageData msg(currentSid, MessageRole::User, prompt);
        DatabaseManager::instance().addMessage(msg);

        // B. 上屏
        m_chatArea->addUserMessage(prompt);
    }

    // 3. 立即添加“转圈圈”气泡，并暂存指针
    // (这个气泡会一直转，直到文生图的任务ID回来，或者图生图的上传+任务ID回来)
    ChatBubble* loadingBubble = m_chatArea->addLoadingBubble();
    m_tempBubbleForId = loadingBubble;

    // 4. 上锁
    setJobRunning(true);

    // 5. 准备基础参数 (通用部分)
    QMap<QString, QVariant> params;
    params["prompt"] = prompt;

    qint64 seed = QRandomGenerator::global()->generate();
    if (seed < 0) seed = -seed;
    params["seed"] = seed;

    qDebug() << "准备生成, 类型:" << (int)m_currentWorkflowType << " 种子:" << seed;

    // =================================================
    // 分支 A: 图生图 (ImageToImage) -> 需要先上传
    // =================================================
    if (m_currentWorkflowType == WorkflowType::ImageToImage) {
        // 获取本地参考图路径
        QString localPath = m_refPopup->currentPath();

        if (localPath.isEmpty()) {
            qDebug() << "❌ 图生图模式必须先选择参考图";
            // 失败处理：解锁，并把刚才生成的转圈气泡删掉(或者显示错误)
            setJobRunning(false);
            // 这里简单处理，你可以加个 delete loadingBubble;
            return;
        }

        // 标记状态：这次上传是为了 I2I
        m_isUploadingForI2I = true;

        // 暂存参数 (等上传完了，再把 filename 塞进去)
        m_pendingI2IParams = params;

        // 开始上传 (上传成功后会触发 imageUploaded 信号，逻辑在那边继续)
        if (m_apiService) {
            m_apiService->uploadImage(localPath);
        }

        // 【重要】直接返回，不要往下走了，等待异步回调
        return;
    }

    // =================================================
    // 分支 B: 文生图 (TextToImage) -> 直接发送
    // =================================================
    if (m_currentWorkflowType == WorkflowType::TextToImage) {
        // 获取分辨率设置
        QSize size = m_inputPanel->currentResolution();
        if (size.isEmpty()) size = QSize(1024, 1024);

        params["width"] = size.width();
        params["height"] = size.height();

        qDebug() << "设定分辨率:" << size.width() << "x" << size.height();
    }

    // 6. 构建工作流 JSON
    QJsonObject workflow = m_wfManager->buildWorkflow(m_currentWorkflowType, params);

    // 7. 发送给 ComfyUI
    if (workflow.isEmpty()) {
        qDebug() << "❌ 工作流构建失败";
        setJobRunning(false); // 解锁
        return;
    }

    if (m_apiService) {
        m_apiService->queuePrompt(workflow);
    } else {
        qDebug() << "❌ ApiService 未初始化";
        setJobRunning(false);
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

    // 1. 【关键】锁定底部面板的所有操作 (包括比例、工作流、输入框等)
    if (m_inputPanel) {
        m_inputPanel->setLocked(running);
        // 虽然按钮被禁用了，但改个文字提示一下用户当前状态还是友好的
        m_inputPanel->getGenerateBtn()->setText(running ? "生成中..." : "生成");
    }

    // 2. 锁定左侧会话列表 (禁止切换、删除)
    if (m_sessionList) {
        m_sessionList->setEnabled(!running);
    }

    // 3. 【新增】锁定左上角的切换按钮和历史按钮
    // 防止用户在生成时把侧边栏收起来，或者跳到历史记录页
    if (m_toggleSessionListBtn) m_toggleSessionListBtn->setEnabled(!running);
    if (m_historyBtn) m_historyBtn->setEnabled(!running);

}


void MainWindow::onInterrogateClicked()
{
    // 1. 检查忙碌锁
    if (m_isJobRunning) return;

    // 2. 【修改】不再打开文件对话框，而是从参考图面板获取路径
    QString localPath = m_refPopup->currentPath();

    // 3. 校验：如果没有选参考图
    if (localPath.isEmpty()) {
        // 自动弹出参考图面板，引导用户
        QToolButton* btn = m_inputPanel->getRefBtn();
        if (btn) {
            QPoint btnPos = btn->mapToGlobal(QPoint(btn->width() / 2, 0));
            m_refPopup->popup(btnPos);
            // 这里可以加一个 ToolTip 或者简单的 Message 提示用户
            // btn->showToolTip("请先在这里上传图片");
        }
        return;
    }

    // 【新增】开始新任务前，务必清空文本缓存
    m_accumulatedStreamText.clear();

    // 4. 界面反馈：在聊天区显示这张图
    // 获取 ReferencePopup 里的缓存图片 (QPixmap) 直接显示，不用重新加载文件
    QPixmap pix = m_refPopup->currentImage();
    if (!pix.isNull()) {
        if (m_chatArea) m_chatArea->addUserImage(pix); // 还是暂时借用这个接口
    }

    // 5. 上锁并标记状态
    setJobRunning(true);
    m_isUploadingForInterrogate = true; // 标记：这是为了反推

    // 6. 开始上传本地文件
    if (m_apiService) {
        m_apiService->uploadImage(localPath);
    }
}

void MainWindow::loadSessionList()
{
    // 1. 从数据库查数据
    QVector<SessionData> sessions = DatabaseManager::instance().getAllSessions();

    // 2. 刷新 UI
    m_sessionList->loadSessions(sessions);

    // 3. 如果有历史会话，默认选中第一个 (最新的)
    // 也可以不做，留空
}

void MainWindow::createNewSession()
{
    // 1. 数据库插入
    int newId = DatabaseManager::instance().createSession("新会话");

    if (newId != -1) {
        // 2. 刷新左侧列表 (把新会话显示出来)
        // 这一步很重要，否则左侧列表里没有这个新会话
        loadSessionList();

        // 3. 加载这个新会话
        // loadSessionHistory 内部已经做了 clear() 和 setCurrentSessionId(newId)
        // 并且因为是新会话，数据库没消息，它加载出来就是空的
        loadSessionHistory(newId);

        // 4. 确保左侧栏可见
        if (!m_leftContainerVisible) onToggleLeftContainer();
    }
}

QString MainWindow::saveImageToLocal(const QPixmap& img)
{
    // 1. 确定保存目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString outputDir = dataDir + "/outputs";

    QDir dir(outputDir);
    if (!dir.exists()) dir.mkpath(".");

    // 2. 生成唯一文件名 (时间戳.png)
    QString fileName = QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    QString fullPath = outputDir + "/" + fileName;

    // 3. 保存
    if (img.save(fullPath, "PNG")) {
        return fullPath;
    }
    return QString();
}


void MainWindow::loadSessionHistory(int sessionId)
{
    qDebug() << "正在加载会话历史:" << sessionId;

    // 1. 清空当前界面
    m_chatArea->clear();
    m_chatArea->setCurrentSessionId(sessionId);

    // 2. 从数据库查数据
    QVector<MessageData> messages = DatabaseManager::instance().getMessages(sessionId);

    // 3. 遍历并恢复显示
    for (const auto& msg : messages) {

        // 判断角色
        // 注意：数据库存的是字符串 "user"/"ai"，MessageData里转成了枚举
        // 我们的 ChatBubble 用的是 ChatRole，可能需要对应一下
        ChatRole role = (msg.role == MessageRole::User) ? ChatRole::User : ChatRole::AI;

        if (msg.isImage()) {
            // --- 图片消息 ---
            // msg.imagePath 是本地绝对路径
            QPixmap pix(msg.imagePath);
            if (!pix.isNull()) {
                if (role == ChatRole::User) {
                    // 如果你之前写了 addUserImage 就用那个
                    m_chatArea->addUserImage(pix);
                } else {
                    m_chatArea->addAiImage(pix);
                }
            } else {
                // 图片文件丢失的情况
                if (role == ChatRole::User) m_chatArea->addUserMessage("[图片文件已丢失]");
                else m_chatArea->addAiMessage("[图片文件已丢失]");
            }
        }
        else {
            // --- 文字消息 ---
            if (role == ChatRole::User) {
                m_chatArea->addUserMessage(msg.text);
            } else {
                // AI 的文字 (反推结果)
                m_chatArea->addAiMessage(msg.text);
            }
        }
    }

    // 4. 滚到底部 (给点延时让布局算好)
    QTimer::singleShot(100, this, [this](){ m_chatArea->scrollToBottom(); });
}
