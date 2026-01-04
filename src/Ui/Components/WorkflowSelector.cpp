/**
 * @file WorkflowSelector.cpp
 * @brief 工作流选择器组件实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "WorkflowSelector.h"
#include "WorkflowCard.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QMouseEvent>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 初始化工作流选择器窗口，设置窗口属性并创建UI界面。
 * 预置了四种工作流类型：文生图、图生图、局部重绘、图像放大。
 */
WorkflowSelector::WorkflowSelector(QWidget* parent) : QWidget(parent) {
    // 设置窗口属性：Popup 类型(无标题栏)，透明背景
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 初始化测试工作流数据
    m_workflows.append(WorkflowInfo(1, "文生图", ":/images/文生图演示.png", ":/images/文生图演示.gif", "基础生成模式，从文字创建图像", WorkflowType::TextToImage));
    m_workflows.append(WorkflowInfo(2, "图生图", ":/images/图生图演示.png", ":/images/图生图演示.gif", "基于参考图生成新图像", WorkflowType::ImageToImage));
    
    setupUi();
}

/**
 * @brief 析构函数
 * 
 * Qt会自动清理子控件，无需手动删除。
 */
WorkflowSelector::~WorkflowSelector() {
    // Qt会自动清理子控件，无需手动删除
}

/**
 * @brief 初始化UI界面
 */
void WorkflowSelector::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(3, 3, 3, 3);
    
    m_container = new QWidget(this);
    m_container->setObjectName("Container");
    m_container->setStyleSheet(
        "QWidget#Container {"
        "  background-color: #2a2a2a;"
        "  border: 1px solid #444;"
        "  border-radius: 12px;"
        "}"
    );
    
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 8);
    m_container->setGraphicsEffect(shadow);
    
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(20, 20, 20, 20);
    m_containerLayout->setSpacing(15);
    
    QLabel* title = new QLabel("选择工作流", m_container);
    title->setStyleSheet("color: white; font-size: 18px; font-weight: bold; border: none; background: transparent;");
    m_containerLayout->addWidget(title);
    
    m_scrollArea = new QScrollArea(m_container);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: #343541; border: none; }"
        "QScrollBar:vertical { border: none; background: #343541; width: 10px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #565869; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
    );
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");
    
    m_cardsLayout = new QVBoxLayout(m_scrollContent);
    m_cardsLayout->setContentsMargins(0, 0, 10, 0);
    m_cardsLayout->setSpacing(15);
    
    createWorkflowCards();
    
    m_scrollArea->setWidget(m_scrollContent);
    m_containerLayout->addWidget(m_scrollArea);
    
    m_mainLayout->addWidget(m_container);
    
    this->setFixedSize(380, 500);
}

/**
 * @brief 创建工作流卡片
 * 
 * 根据工作流数据创建对应的卡片组件，并设置点击事件连接。
 * 清除现有卡片后重新创建，确保数据同步。
 */
void WorkflowSelector::createWorkflowCards() {
    // 清除现有卡片
    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_workflowCards.clear();
    
    // 创建新卡片
    for (const auto& workflow : m_workflows) {
        WorkflowCard* card = new WorkflowCard(workflow, m_scrollContent);
        
        connect(card, &WorkflowCard::clicked, this, [this](const WorkflowInfo& info) {
            emit workflowSelected(info);
            this->hide();
        });
        
        m_workflowCards.append(card);
        m_cardsLayout->addWidget(card, 0, Qt::AlignHCenter); // 居中添加
    }
    
    // 添加底部弹簧，防止卡片紧贴底部
    m_cardsLayout->addStretch();
}

/**
 * @brief 设置工作流数据
 * @param workflows 工作流信息向量
 * 
 * 更新工作流数据并重新创建卡片，用于动态更新工作流列表。
 */
void WorkflowSelector::setWorkflows(const QVector<WorkflowInfo>& workflows) {
    m_workflows = workflows;
    createWorkflowCards();
}

/**
 * @brief 绘制事件处理
 * @param event 绘制事件
 * 
 * 使用透明模式绘制窗口背景，确保窗口完全透明。
 * 这是实现无边框透明窗口的关键方法。
 */
void WorkflowSelector::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    // 用 "Clear" 模式把整个窗口刷成完全透明
    // 这样操作系统就不会绘制黑色底色了
    p.setCompositionMode(QPainter::CompositionMode_Clear);
    p.fillRect(rect(), Qt::transparent);
}

/**
 * @brief 事件处理
 * @param event 事件对象
 * @return 是否处理了该事件
 * 
 * 处理窗口失去焦点事件，当用户点击窗口外部时自动关闭窗口。
 */
bool WorkflowSelector::event(QEvent* event) {
    // 如果窗口失去焦点（用户点了别处），就隐藏
    if (event->type() == QEvent::WindowDeactivate) {
        this->hide();
        return true;
    }
    return QWidget::event(event);
}

/**
 * @brief 弹出窗口
 * @param pos 弹出位置（相对于屏幕坐标）
 */
void WorkflowSelector::popup(const QPoint& pos) {
    int x = pos.x() - width() / 2;
    int y = pos.y() - height() - 10;
    
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    
    if (x < 10) x = 10;
    if (x + width() > screenGeometry.width() - 10) x = screenGeometry.width() - width() - 10;
    
    if (y < 10) {
        y = pos.y() + 20;
    }
    if (y + height() > screenGeometry.height() - 10) {
        y = screenGeometry.height() - height() - 10;
    }
    
    move(x, y);
    show();
    raise();
    setFocus();
}
