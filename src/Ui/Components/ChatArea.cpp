/**
 * @file ChatArea.cpp
 * @brief 聊天区域组件实现文件
 * 
 * 该文件实现了ChatArea类，提供聊天消息的展示功能，支持用户消息、AI消息和图片的显示。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "ChatArea.h"
#include "ChatBubble.h"
#include <QScrollBar>
#include <QTimer>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ChatArea::ChatArea(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ChatArea::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollArea->setStyleSheet(
        "QScrollArea { background: #343541; border: none; }"
        "QScrollBar:vertical { border: none; background: #343541; width: 10px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #565869; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background-color: transparent;");

    m_contentLayout = new QVBoxLayout(m_scrollContent);
    m_contentLayout->setContentsMargins(20, 20, 20, 20);
    m_contentLayout->setSpacing(20);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);
}

void ChatArea::addUserMessage(const QString& text)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::User, text, m_scrollContent);

    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);

    scrollToBottom();
}

ChatBubble* ChatArea::addLoadingBubble()
{
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, "", m_scrollContent);

    connect(bubble, &ChatBubble::upscaleRequested, this, &ChatArea::upscaleRequested);

    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();

    return bubble;
}

void ChatArea::addAiImage(const QPixmap& img)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, img, m_scrollContent);
    connect(bubble, &ChatBubble::upscaleRequested, this, &ChatArea::upscaleRequested);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}

/**
 * @brief 滚动到底部
 */
void ChatArea::scrollToBottom()
{
    QTimer::singleShot(10, this, [=](){
        QScrollBar* bar = m_scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void ChatArea::clear()
{
    while (m_contentLayout->count() > 1) {
        QLayoutItem *item = m_contentLayout->takeAt(0);

        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    m_currentSessionId = -1;
    m_currentStreamBubble = nullptr;

    this->update();
}

void ChatArea::handleStreamToken(const QString& token, bool finished)
{
    if (finished && token.isEmpty() && !m_currentStreamBubble) {
        return;
    }

    if (token.isEmpty() && !finished) {
        return;
    }

    if (!m_currentStreamBubble) {
        m_currentStreamBubble = new ChatBubble(ChatRole::AI, token, m_scrollContent);
        m_contentLayout->insertWidget(m_contentLayout->count() - 1, m_currentStreamBubble);
    }
    else {
        if (!token.isEmpty()) {
            m_currentStreamBubble->appendText(token);
        }
    }

    QTimer::singleShot(10, this, [this](){ scrollToBottom(); });

    if (finished) {
        m_currentStreamBubble = nullptr;
    }
}

void ChatArea::addUserImage(const QPixmap& img)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::User, img, m_scrollContent);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}

/**
 * @brief 添加AI消息
 * @param text 消息文本
 */
void ChatArea::addAiMessage(const QString& text)
{
    ChatBubble* bubble = new ChatBubble(ChatRole::AI, text, m_scrollContent);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, bubble);
    scrollToBottom();
}
