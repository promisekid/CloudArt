/**
 * @file SidebarControl.cpp
 * @brief 侧边栏控制组件实现文件
 * 
 * 该文件实现了SidebarControl类，提供侧边栏控制按钮，包括对话记录、生成记录和服务器设置。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "SidebarControl.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QIcon>

SidebarControl::SidebarControl(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_toggleBtn(nullptr)
    , m_historyBtn(nullptr)
    , m_settingsBtn(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(0, 10, 0, 10);
    
    m_toggleBtn = createBtn(":/images/HideConversation.png", "对话记录");
    m_historyBtn = createBtn(":/images/historypic.png", "生成记录");
    
    m_layout->addWidget(m_toggleBtn);
    m_layout->addWidget(m_historyBtn);
    
    m_layout->addStretch();

    m_settingsBtn = createBtn(":/images/setting.png", "服务器设置");

    m_layout->addWidget(m_settingsBtn);

    setFixedWidth(40);
    adjustSize();
}

SidebarControl::~SidebarControl()
{
}

QToolButton* SidebarControl::toggleBtn() const
{
    return m_toggleBtn;
}

/**
 * @brief 获取历史按钮
 * @return 历史按钮指针
 */
QToolButton* SidebarControl::historyBtn() const
{
    return m_historyBtn;
}

void SidebarControl::updateToggleState(bool isExpanded)
{
    if (m_toggleBtn) {
        m_toggleBtn->setToolTip(isExpanded ? "对话记录" : "对话记录");
    }
}

QToolButton* SidebarControl::createBtn(const QString& iconPath, const QString& tooltip)
{
    QToolButton* btn = new QToolButton(this);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(24, 24));
    btn->setFixedSize(32, 32);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QToolButton { "
        "  background-color: #40414F; "
        "  border: none; "
        "  border-radius: 4px; "
        "}"
        "QToolButton:hover { "
        "  background-color: #50515F; "
        "}"
    );
    btn->setToolTip(tooltip);
    return btn;
}

/**
 * @brief 获取设置按钮
 * @return 设置按钮指针
 */
QToolButton* SidebarControl::settingsBtn() const
{
    return m_settingsBtn;
}
