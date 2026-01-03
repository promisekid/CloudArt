#include "ChatArea.h"
#include "ChatBubble.h"
#include <QScrollBar>
#include <QTimer>

ChatArea::ChatArea(QWidget *parent) : QWidget(parent)
{
    setupUi();
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
    // 【核心修复】
    // 这里的逻辑是：只要布局里的元素多于 1 个（那个 1 就是最后的弹簧），就一直删。
    // 必须从 index 0 (第一个) 开始删，这样永远删的是最上面的气泡。
    // 最后的弹簧会一直留在那里，直到它变成 index 0。

    while (m_contentLayout->count() > 1) {
        // 取出第一个元素 (气泡)
        QLayoutItem *item = m_contentLayout->takeAt(0);

        if (item->widget()) {
            delete item->widget(); // 销毁气泡控件
        }
        delete item; // 销毁布局项
    }

    // 重置状态
    m_currentSessionId = -1;
    m_currentStreamBubble = nullptr; // 别忘了重置这个

    this->update();
}


void ChatArea::handleStreamToken(const QString& token, bool finished)
{
    // 【关键修复 1】如果是单纯的结束信号（没有内容），且当前没有活跃气泡，直接忽略
    // 这能防止“强制解锁信号”创建一个空气泡
    if (finished && token.isEmpty() && !m_currentStreamBubble) {
        return;
    }

    // 【关键修复 2】过滤中间的空包
    if (token.isEmpty() && !finished) {
        return;
    }

    // 2. 如果没有正在打字的气泡，造一个新的
    if (!m_currentStreamBubble) {
        // 如果能走到这里，说明 token 肯定不为空
        m_currentStreamBubble = new ChatBubble(ChatRole::AI, token, m_scrollContent);
        m_contentLayout->insertWidget(m_contentLayout->count() - 1, m_currentStreamBubble);
    }
    else {
        // 3. 追加
        if (!token.isEmpty()) {
            m_currentStreamBubble->appendText(token);
        }
    }

    // 4. 滚动
    QTimer::singleShot(10, this, [this](){ scrollToBottom(); });

    // 5. 结束重置
    if (finished) {
        m_currentStreamBubble = nullptr;
    }
}

void ChatArea::addUserImage(const QPixmap& img)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::User, img, m_scrollContent);
    // User 的图通常不需要高清修复，所以不连 upscale 信号也可以
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}

void ChatArea::addAiMessage(const QString& text)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, text, m_scrollContent);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}
