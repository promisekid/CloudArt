#include "ChatArea.h"
#include "ChatBubble.h"
#include <QScrollBar>
#include <QTimer>

ChatArea::ChatArea(QWidget *parent) : QWidget(parent)
{
    setupUi();

    // 测试数据：你可以解开注释看看效果
    addUserMessage("帮我生成一张丰川祥子的图片。");
    addAiImage(QPixmap("C:/Users/24462/Pictures/comfyui/祥子/426DAA5942BF4BCE235CD3004582BC3A.png")); // 记得换成存在的图片路径测试
    addUserMessage("帮我生成一张丰川祥子的图片的撒范德萨发大水发射点范德萨范德萨发生爱上发大水范德萨发大水。");
    addAiImage(QPixmap("C:/Users/24462/Pictures/comfyui/祥子/426DAA5942BF4BCE235CD3004582BC3A.png"));
    addUserMessage("帮我生成一张丰川祥子的图片。");
    addAiImage(QPixmap("C:/Users/24462/Pictures/comfyui/祥子/426DAA5942BF4BCE235CD3004582BC3A.png"));
    addUserMessage("帮我生成一张美丽女孩的横板图片。");
    addAiImage(QPixmap("C:/Users/24462/Pictures/comfyui/模特/ComfyUI_temp_yqjmp_00006_.png"));
}

void ChatArea::setupUi()
{
    // 1. 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 2. 滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true); // 关键！否则内部无法撑开
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 只需垂直滚动
    m_scrollArea->setFrameShape(QFrame::NoFrame); // 无边框

    // 美化滚动条 (和 SessionList 一致)
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: #343541; border: none; }"
        "QScrollBar:vertical { border: none; background: #343541; width: 10px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #565869; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );

    // 3. 滚动内容容器
    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background-color: transparent;"); // 透明，透出 ScrollArea 背景

    m_contentLayout = new QVBoxLayout(m_scrollContent);
    m_contentLayout->setContentsMargins(20, 20, 20, 20); // 给气泡留出四周空隙
    m_contentLayout->setSpacing(20); // 气泡之间的间距
    m_contentLayout->addStretch();   // 初始弹簧，把第一条消息顶在最上面

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);
}

void ChatArea::addUserMessage(const QString& text)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::User, text, m_scrollContent);

    // 插入到弹簧之前 (count()-1)
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);

    scrollToBottom();
}

ChatBubble* ChatArea::addLoadingBubble()
{
    // 1. 创建一个空内容的 AI 气泡
    // (我们在 ChatBubble 构造函数里处理了空字符串 -> Loading 态)
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, "", m_scrollContent);

    // 2. 连接它的信号
    // 当用户在气泡上右键点击“高清修复”时，信号会一路传出去
    connect(bubble, &ChatBubble::upscaleRequested, this, &ChatArea::upscaleRequested);

    // 3. 插入界面
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();

    return bubble;
}

// 顺便修改一下旧的 addAiImage，也要连信号
void ChatArea::addAiImage(const QPixmap& img)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, img, m_scrollContent);
    connect(bubble, &ChatBubble::upscaleRequested, this, &ChatArea::upscaleRequested); // 连接信号
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}

void ChatArea::scrollToBottom()
{
    // 使用 QTimer::singleShot 0ms 延时
    // 原因是：界面布局刷新需要时间，直接 setValue 可能滚动不到最底部
    QTimer::singleShot(10, this, [=](){
        QScrollBar* bar = m_scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void ChatArea::clear()
{
    // 遍历布局中的所有项目
    QLayoutItem *item;
    // 我们要保留第一个 item (那是 addStretch 加的弹簧)
    // 如果你的弹簧是最后加的，逻辑就不一样。
    // 在之前的代码中，我在构造函数里加了 m_contentLayout->addStretch();
    // 它是布局里的第 0 个元素。

    // 从最后往前删，直到只剩 1 个（即弹簧）
    while (m_contentLayout->count() > 1) {
        item = m_contentLayout->takeAt(m_contentLayout->count() - 1); // 取出最后一个
        if (item->widget()) {
            delete item->widget(); // 删除气泡控件
        }
        delete item; // 删除布局项本身
    }

    // 重置状态
    m_currentSessionId = -1;

    // 强制刷新一下 UI
    this->update();
}
