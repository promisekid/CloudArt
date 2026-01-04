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
    
    // 【新增】添加一个弹簧，把设置按钮顶到最底部
    m_layout->addStretch();

    // 【新增】创建设置按钮
    // 注意：这里暂时复用 'HideConversation.png' 图标，你可以以后换成齿轮图标
    m_settingsBtn = createBtn(":/images/setting.png", "服务器设置");

    // 如果想区分，可以暂时给它变个色或者样式，这里先保持一致
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

QToolButton* SidebarControl::settingsBtn() const
{
    return m_settingsBtn;
}
