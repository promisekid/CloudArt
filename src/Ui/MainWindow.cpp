/**
 * @file MainWindow.cpp
 * @brief 主窗口实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "MainWindow.h"
#include "Components/SidebarControl.h"
#include "Components/SessionList.h"
#include "Components/ChatArea.h"
#include "Components/InputPanel.h"
#include "Components/WorkflowSelector.h"
#include "Components/ReferencePopup.h"
#include "Components/ChatBubble.h"
#include "../Network/ComfyApiService.h"
#include "../Core/WorkflowManager.h"
#include "../Model/DataModels.h"
#include "Components/HistoryGallery.h"
#include "Components/ImageViewer.h"

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
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QSettings>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_leftStack(nullptr)
    , m_sessionList(nullptr)
    , m_chatArea(nullptr)
    , m_inputPanel(nullptr)
    , m_wfSelector(nullptr)
    , m_refPopup(nullptr)
    , m_sidebarControl(nullptr)
    , m_leftContainerVisible(true)
    , m_leftContainerOriginalWidth(250)
    , m_currentPageIndex(0)
    , m_historyGallery(nullptr)
    , m_leftContainerAnimation(nullptr)
    , m_mainLayout(nullptr)
    , m_apiService(nullptr)
{
    setupUi();
}

/**
 * @brief 析构函数
 */
MainWindow::~MainWindow()
{
}

/**
 * @brief 初始化UI界面
 */
void MainWindow::setupUi()
{
    this->resize(1280, 800);
    this->setWindowTitle("CloudArt");

    QWidget* central = new QWidget(this);
    this->setCentralWidget(central);

    m_mainLayout = new QHBoxLayout(central);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_leftStack = new QStackedWidget(central);

    m_sessionList = new SessionList(m_leftStack);
    m_leftStack->addWidget(m_sessionList);

    m_historyGallery = new HistoryGallery(m_leftStack);
    m_leftStack->addWidget(m_historyGallery);

    connect(m_historyGallery, &HistoryGallery::imageClicked, this, [this](const QString& path){
        QPixmap pix(path);
        if (!pix.isNull()) {
            ImageViewer* viewer = new ImageViewer(pix, this);
            viewer->exec();
            delete viewer;
        }
    });

    m_leftStack->setCurrentIndex(0);

    m_leftContainerOriginalWidth = 250;
    m_leftStack->setMaximumWidth(250);

    m_mainLayout->insertWidget(0, m_leftStack);

    QWidget* rightWidget = new QWidget(central);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_chatArea = new ChatArea(rightWidget);
    rightLayout->addWidget(m_chatArea, 1);

    m_inputPanel = new InputPanel(rightWidget);
    rightLayout->addWidget(m_inputPanel);

    m_mainLayout->addWidget(rightWidget);

    m_wfManager = new WorkflowManager(this);

    m_wfSelector = new WorkflowSelector(this);

    m_refPopup = new ReferencePopup(this);

    m_sidebarControl = new SidebarControl(this);

    m_leftContainerAnimation = new QPropertyAnimation(m_leftStack, "minimumWidth", this);
    m_leftContainerAnimation->setDuration(300);
    m_leftContainerAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_sidebarControl->toggleBtn(), &QToolButton::clicked,
            this, &MainWindow::switchToSessionList);
    connect(m_sidebarControl->historyBtn(), &QToolButton::clicked,
            this, &MainWindow::switchToHistoryWindow);

    connect(m_inputPanel->getWorkflowBtn(), &QPushButton::clicked,
            this, &MainWindow::onWorkflowBtnClicked);

    connect(m_inputPanel->getRefBtn(), &QToolButton::clicked,
            this, &MainWindow::onRefBtnClicked);

    connect(m_inputPanel, &InputPanel::generateClicked,
            this, &MainWindow::onGenerateClicked);

    connect(m_wfSelector, &WorkflowSelector::workflowSelected,
            this, &MainWindow::onWorkflowSelected);

    connect(m_inputPanel->getInterrogateBtn(), &QToolButton::clicked,
            this, &MainWindow::onInterrogateClicked);

    m_inputPanel->updateState(WorkflowType::TextToImage);

    if (m_leftContainerVisible) {
        int initialBtnX = m_leftContainerOriginalWidth + 10;
        m_sidebarControl->move(initialBtnX, 10);
    } else {
        m_sidebarControl->move(10, 10);
    }

    m_sidebarControl->raise();

    QTimer::singleShot(0, this, [this](){
        updateSidebarPosition();
    });

    m_apiService = new ComfyApiService(this);

    connect(m_apiService, &ComfyApiService::serverConnected, this, [this](){
        this->setWindowTitle("CloudArt - 已连接");
        m_inputPanel->setConnectionStatus(true);
    });

    connect(m_apiService, &ComfyApiService::serverDisconnected, this, [this](){
        this->setWindowTitle("CloudArt - 未连接");
        m_inputPanel->setConnectionStatus(false);
    });

    connect(m_apiService, &ComfyApiService::errorOccurred, this, [this](const QString& msg){
        this->setWindowTitle("CloudArt - 连接失败");
        m_inputPanel->setConnectionStatus(false);
    });

    m_inputPanel->setConnectionStatus(false);

    connect(m_sidebarControl->settingsBtn(), &QToolButton::clicked, this, [this](){
        SettingsDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            loadAndConnect();
        }
    });

    loadAndConnect();

    connect(m_apiService, &ComfyApiService::promptQueued, this, [this](const QString& promptId){
        if (m_tempBubbleForId) {
            qDebug() << "绑定任务 ID:" << promptId << " 到当前气泡";
            m_pendingBubbles.insert(promptId, m_tempBubbleForId);
            m_tempBubbleForId = nullptr;
        }
    });

    connect(m_apiService, &ComfyApiService::imageReceived, this,
            [this](const QString& promptId, const QString& filename, const QPixmap& img){

                QString localPath = saveImageToLocal(img);

                int currentSid = m_chatArea->currentSessionId();
                if (currentSid != -1 && !localPath.isEmpty()) {
                    MessageData msg(currentSid, MessageRole::AI, "", localPath);
                    DatabaseManager::instance().addMessage(msg);
                }

                if (m_pendingBubbles.contains(promptId)) {
                    qDebug() << "找到对应的气泡，更新图片...";

                    ChatBubble* bubble = m_pendingBubbles[promptId];
                    if (bubble) {
                        bubble->updateImage(img, filename);


                        QTimer::singleShot(100, this, [this](){
                            m_chatArea->scrollToBottom();
                        });
                    }

                    setJobRunning(false);

                    m_pendingBubbles.remove(promptId);
                } else {
                    if (m_chatArea) {
                        m_chatArea->addAiImage(img);
                        QTimer::singleShot(100, this, [this](){ m_chatArea->scrollToBottom(); });
                    }
                }
            });

    connect(m_chatArea, &ChatArea::upscaleRequested, this,
            [this](const QString& serverFileName, const QPixmap& img){

                if (m_isJobRunning) {
                    qDebug() << "任务进行中，忽略高清修复请求";
                    return;
                }

                setJobRunning(true);

                qDebug() << "收到高清修复请求，准备回环上传...";

                m_tempUpscaleBubble = m_chatArea->addLoadingBubble();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                                   + "/temp_upscale_source.png";
                if (img.save(tempPath)) {
                    m_isUploadingForUpscale = true;

                    m_apiService->uploadImage(tempPath);
                } else {
                    qDebug() << "临时文件保存失败";
                    setJobRunning(false);
                }
            });

    connect(m_apiService, &ComfyApiService::imageUploaded, this, [this](const QString& serverName){

        if (m_isUploadingForUpscale) {
            qDebug() << "高清修复原图上传完毕 (" << serverName << ")，开始发送生成任务...";

            m_isUploadingForUpscale = false;

            QMap<QString, QVariant> params;
            params["image_path"] = serverName;

            qint64 seed = QRandomGenerator::global()->generate();
            if (seed < 0) seed = -seed;
            params["seed"] = seed;

            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::Upscale, params);

            m_tempBubbleForId = m_tempUpscaleBubble;
            m_tempUpscaleBubble = nullptr;

            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }

            return;
        }

        if (m_isUploadingForInterrogate) {
            qDebug() << "反推图片上传成功，正在构建任务...";
            m_isUploadingForInterrogate = false;
            m_currentServerRefImg = serverName;

            QMap<QString, QVariant> params;
            params["image_path"] = serverName;

            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::VisionCaption, params);

            if (wf.isEmpty()) {
                qDebug() << "反推工作流构建失败";
                setJobRunning(false);
                return;
            }

            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }

            return;
        }

        if (m_isUploadingForI2I) {
            qDebug() << "图生图素材上传完毕:" << serverName;
            m_isUploadingForI2I = false;

            QMap<QString, QVariant> params = m_pendingI2IParams;

            params["image_path"] = serverName;

            QJsonObject wf = m_wfManager->buildWorkflow(WorkflowType::ImageToImage, params);

            if (wf.isEmpty()) {
                qDebug() << "图生图工作流构建失败";
                setJobRunning(false);
                return;
            }

            if (m_apiService) {
                m_apiService->queuePrompt(wf);
            }

            return;
        }
    });

    connect(m_apiService, &ComfyApiService::streamTokenReceived, this,
            [this](const QString& token, bool finished){

                if (!token.isEmpty()) {
                    m_accumulatedStreamText += token;
                }

                if (m_chatArea) {
                    m_chatArea->handleStreamToken(token, finished);
                }

                if (finished) {
                    qDebug() << "反推结束，完整文本长度:" << m_accumulatedStreamText.length();

                    int currentSid = m_chatArea->currentSessionId();

                    if (currentSid != -1 && !m_accumulatedStreamText.isEmpty()) {
                        MessageData msg(currentSid, MessageRole::AI, m_accumulatedStreamText);
                        DatabaseManager::instance().addMessage(msg);
                        qDebug() << "反推文本已保存到数据库";
                    }

                    m_accumulatedStreamText.clear();

                    setJobRunning(false);
                }
            });

    connect(m_sessionList, &SessionList::createNewSessionRequest,
            this, &MainWindow::createNewSession);

    connect(m_sessionList, &SessionList::sessionRenameRequest, this,
            [this](int id, const QString& newName){

                DatabaseManager::instance().renameSession(id, newName);
                qDebug() << "会话" << id << "重命名为" << newName;
            });

    connect(m_sessionList, &SessionList::sessionDeleteRequest, this,
            [this](int id){

                DatabaseManager::instance().deleteSession(id);

                if (m_chatArea->currentSessionId() == id) {
                    m_chatArea->clear();
                    m_chatArea->setCurrentSessionId(-1);
                }
                qDebug() << "会话" << id << "已删除";
            });

    connect(m_sessionList, &SessionList::sessionSwitchRequest, this, [this](int id){
        loadSessionHistory(id);
    });

    loadSessionList();
}

/**
 * @brief 工作流按钮点击事件处理
 */
void MainWindow::onWorkflowBtnClicked() {
    QPushButton* btn = m_inputPanel->getWorkflowBtn();
    if (btn) {
        QPoint btnPos = btn->mapToGlobal(QPoint(btn->width() / 2, 0));
        m_wfSelector->popup(btnPos);
    }
}

/**
 * @brief 参考图按钮点击事件处理
 */
void MainWindow::onRefBtnClicked() {
    if (m_refPopup->isVisible()) {
        m_refPopup->hide();
    } else {
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
 */
void MainWindow::onWorkflowSelected(const WorkflowInfo& info)
{
    m_inputPanel->updateState(info.type);

    m_currentWorkflowType = info.type;

    qDebug() << "切换到工作流:" << info.name << " (ID:" << info.id << ")";
}

/**
 * @brief 生成按钮点击事件处理
 * @param prompt 用户输入的提示词
 */
void MainWindow::onGenerateClicked(const QString& prompt)
{
    if (m_isJobRunning) return;

    qDebug() << "生成请求 - 提示词:" << prompt;

    int currentSid = m_chatArea->currentSessionId();
    if (currentSid != -1) {
        // A. 存库
        MessageData msg(currentSid, MessageRole::User, prompt);
        DatabaseManager::instance().addMessage(msg);

        // B. 上屏
        m_chatArea->addUserMessage(prompt);
    }

    ChatBubble* loadingBubble = m_chatArea->addLoadingBubble();
    m_tempBubbleForId = loadingBubble;

    setJobRunning(true);

    QMap<QString, QVariant> params;
    params["prompt"] = prompt;

    qint64 seed = QRandomGenerator::global()->generate();
    if (seed < 0) seed = -seed;
    params["seed"] = seed;

    qDebug() << "准备生成, 类型:" << (int)m_currentWorkflowType << " 种子:" << seed;

    if (m_currentWorkflowType == WorkflowType::ImageToImage) {
        QString localPath = m_refPopup->currentPath();

        if (localPath.isEmpty()) {
            qDebug() << "图生图模式必须先选择参考图";
            setJobRunning(false);
            return;
        }

        m_isUploadingForI2I = true;

        m_pendingI2IParams = params;

        if (m_apiService) {
            m_apiService->uploadImage(localPath);
        }

        return;
    }

    if (m_currentWorkflowType == WorkflowType::TextToImage) {
        QSize size = m_inputPanel->currentResolution();
        if (size.isEmpty()) size = QSize(1024, 1024);

        params["width"] = size.width();
        params["height"] = size.height();

        qDebug() << "设定分辨率:" << size.width() << "x" << size.height();
    }

    QJsonObject workflow = m_wfManager->buildWorkflow(m_currentWorkflowType, params);

    if (workflow.isEmpty()) {
        qDebug() << "工作流构建失败";
        setJobRunning(false);
        return;
    }

    if (m_apiService) {
        m_apiService->queuePrompt(workflow);
    } else {
        qDebug() << "ApiService 未初始化";
        setJobRunning(false);
    }
}

/**
 * @brief 切换左侧容器显示状态
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

        // 更新侧边栏位置
        QTimer::singleShot(300, this, [this]() {
            updateSidebarPosition();
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

        // 更新侧边栏位置
        QTimer::singleShot(300, this, [this]() {
            updateSidebarPosition();
        });
    }
}

/**
 * @brief 更新侧边栏位置
 */
void MainWindow::updateSidebarPosition()
{
    if (m_leftContainerVisible) {
        int containerWidth = m_leftStack->width();
        if (containerWidth <= 0) {
            containerWidth = 250;
        }
        QPoint pos = m_leftStack->mapToParent(QPoint(containerWidth + 10, 10));
        m_sidebarControl->move(pos);
    } else {
        m_sidebarControl->move(10, 10);
    }

    m_sidebarControl->raise();
}

/**
 * @brief 切换左侧面板
 * @param targetIndex 目标页面索引
 */
void MainWindow::switchLeftPanel(int targetIndex)
{
    if (!m_leftContainerVisible) {
        onToggleLeftContainer();
        m_leftStack->setCurrentIndex(targetIndex);
        m_currentPageIndex = targetIndex;
    } else if (m_currentPageIndex != targetIndex) {
        m_leftStack->setCurrentIndex(targetIndex);
        m_currentPageIndex = targetIndex;
    } else {
        onToggleLeftContainer();
    }
}

/**
 * @brief 窗口大小改变事件处理
 * @param event 窗口大小改变事件
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateSidebarPosition();
}

/**
 * @brief 切换到会话列表页面
 */
void MainWindow::switchToSessionList()
{
    switchLeftPanel(0);
}

/**
 * @brief 切换到历史记录页面
 */
void MainWindow::switchToHistoryWindow()
{
    // 1. 切换界面
    switchLeftPanel(1);

    // 2. 每次切换过来时，刷新数据
    // 这样刚生成的图也能立刻看到
    if (m_leftStack->currentIndex() == 1) {
        m_historyGallery->loadImages();
    }
}

void MainWindow::setJobRunning(bool running)
{
    m_isJobRunning = running;

    if (m_inputPanel) {
        m_inputPanel->setLocked(running);
        m_inputPanel->getGenerateBtn()->setText(running ? "生成中..." : "生成");
    }

    if (m_sessionList) {
        m_sessionList->setEnabled(!running);
    }

    if (m_sidebarControl->toggleBtn()) m_sidebarControl->toggleBtn()->setEnabled(!running);
    if (m_sidebarControl->historyBtn()) m_sidebarControl->historyBtn()->setEnabled(!running);

}


void MainWindow::onInterrogateClicked()
{
    if (m_isJobRunning) return;

    QString localPath = m_refPopup->currentPath();

    if (localPath.isEmpty()) {
        QToolButton* btn = m_inputPanel->getRefBtn();
        if (btn) {
            QPoint btnPos = btn->mapToGlobal(QPoint(btn->width() / 2, 0));
            m_refPopup->popup(btnPos);
        }
        return;
    }

    m_accumulatedStreamText.clear();

    QPixmap pix = m_refPopup->currentImage();
    if (!pix.isNull()) {
        if (m_chatArea) m_chatArea->addUserImage(pix);
    }

    setJobRunning(true);
    m_isUploadingForInterrogate = true;

    if (m_apiService) {
        m_apiService->uploadImage(localPath);
    }
}

/**
 * @brief 加载会话列表
 */
void MainWindow::loadSessionList()
{
    QVector<SessionData> sessions = DatabaseManager::instance().getAllSessions();

    m_sessionList->loadSessions(sessions);

    if (!sessions.isEmpty()) {
        int firstId = sessions.first().id;

        m_sessionList->selectSession(firstId);

        loadSessionHistory(firstId);

        m_chatArea->setCurrentSessionId(firstId);
    }
    else {
        qDebug() << "数据库为空，自动创建新会话...";
        createNewSession();
    }
}

/**
 * @brief 创建新会话
 */
void MainWindow::createNewSession()
{
    int newId = DatabaseManager::instance().createSession("新会话");
    if (newId != -1) {
        loadSessionList();

        if (!m_leftContainerVisible) onToggleLeftContainer();
    }
}

QString MainWindow::saveImageToLocal(const QPixmap& img)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString outputDir = dataDir + "/outputs";

    QDir dir(outputDir);
    if (!dir.exists()) dir.mkpath(".");

    QString fileName = QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    QString fullPath = outputDir + "/" + fileName;

    if (img.save(fullPath, "PNG")) {
        return fullPath;
    }
    return QString();
}

/**
 * @brief 加载会话历史
 * @param sessionId 会话ID
 */
void MainWindow::loadSessionHistory(int sessionId)
{
    qDebug() << "正在加载会话历史:" << sessionId;

    m_chatArea->clear();
    m_chatArea->setCurrentSessionId(sessionId);

    QVector<MessageData> messages = DatabaseManager::instance().getMessages(sessionId);

    for (const auto& msg : messages) {

        ChatRole role = (msg.role == MessageRole::User) ? ChatRole::User : ChatRole::AI;

        if (msg.isImage()) {
            QPixmap pix(msg.imagePath);
            if (!pix.isNull()) {
                if (role == ChatRole::User) {
                    m_chatArea->addUserImage(pix);
                } else {
                    m_chatArea->addAiImage(pix);
                }
            } else {
                if (role == ChatRole::User) m_chatArea->addUserMessage("[图片文件已丢失]");
                else m_chatArea->addAiMessage("[图片文件已丢失]");
            }
        }
        else {
            if (role == ChatRole::User) {
                m_chatArea->addUserMessage(msg.text);
            } else {
                m_chatArea->addAiMessage(msg.text);
            }
        }
    }

    QTimer::singleShot(100, this, [this](){ m_chatArea->scrollToBottom(); });
}

/**
 * @brief 加载配置并连接服务器
 */
void MainWindow::loadAndConnect()
{
    QSettings settings("CloudArt", "AppConfig");
    QString url = settings.value("Server/Url", "http://127.0.0.1:8000").toString();

    if (url.isEmpty()) return;

    this->setWindowTitle("CloudArt - 正在连接...");
    qDebug() << "正在尝试连接服务器:" << url;

    m_inputPanel->setConnectionStatus(false);

    if (m_apiService) {
        m_apiService->connectToHost(url);
    }
}
