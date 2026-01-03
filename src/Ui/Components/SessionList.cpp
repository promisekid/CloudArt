#include "SessionList.h"
#include "SessionItem.h"
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar> // 用于美化滚动条

SessionList::SessionList(QWidget *parent)
    : QWidget(parent)
    , m_currentSessionItem(nullptr)
{
    setupUi();
}

void SessionList::setupUi()
{
    // 1. 设置侧边栏基本属性
    this->setFixedWidth(260);
    this->setStyleSheet("background-color: #202123; border-right: 1px solid #4D4D4F;");

    // 2. 最外层布局 (垂直)
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 20, 0, 0); // 顶部留白，左右下贴边
    rootLayout->setSpacing(10);

    // 3. 固定在顶部的“新建会话”按钮
    // 为了让按钮左右有间距，我们给它包一层容器，或者直接设置 margin
    QWidget* topContainer = new QWidget(this);
    QVBoxLayout* topLayout = new QVBoxLayout(topContainer);
    topLayout->setContentsMargins(10, 0, 10, 0); // 按钮左右留白

    m_btnNew = new QPushButton("+ 新建会话", this);
    m_btnNew->setFixedHeight(45);
    m_btnNew->setCursor(Qt::PointingHandCursor);
    m_btnNew->setStyleSheet(
        "QPushButton { "
        "   background-color: transparent; "
        "   border: 1px solid #565869; "
        "   border-radius: 5px; "
        "   color: white; "
        "   text-align: left; "
        "   padding-left: 15px;"
        "}"
        "QPushButton:hover { background-color: #2A2B32; }"
        );
    connect(m_btnNew, &QPushButton::clicked, this, &SessionList::createNewSessionRequest);

    topLayout->addWidget(m_btnNew);
    rootLayout->addWidget(topContainer);

    // 4. 创建滚动区域 (QScrollArea)
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // 关键：让内部容器宽度自适应 ScrollArea
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 只要垂直滚动，不要水平
    scrollArea->setFrameShape(QFrame::NoFrame); // 去掉 ScrollArea 自带的边框

    // 美化滚动条 (QSS) - 这一大段是为了让滚动条变细、变深色
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { border: none; background: #202123; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #4D4D4F; min-height: 20px; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );

    // 5. 创建滚动区域内部的“容器画布”
    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;"); // 必须透明，否则会挡住背景

    // 6. 内部布局 (用来放 Label 和 SessionItem)
    m_scrollLayout = new QVBoxLayout(scrollContent);
    m_scrollLayout->setContentsMargins(10, 10, 10, 10); // 内容的边距
    m_scrollLayout->setSpacing(5);

    // 添加标题
    QLabel* labelTitle = new QLabel("最近历史", scrollContent);
    labelTitle->setStyleSheet("color: #8E8EA0; font-size: 12px; margin-bottom: 5px; border: none;");
    m_scrollLayout->addWidget(labelTitle);

    // 7. 组装：把容器塞进 ScrollArea，把 ScrollArea 塞进最外层布局
    scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(scrollArea);

    // 8. 添加弹簧 (Stretch)
    // 把它放在内部布局的最后，把 Item 往上顶
    m_scrollLayout->addStretch();
}

void SessionList::addSession(int id, const QString& title)
{
    SessionItem* item = new SessionItem(id, title, this);

    // --- 信号连接部分保持不变 ---
    connect(item, &SessionItem::itemClicked, this, [=](SessionItem* clickedItem){
        handleItemSelection(clickedItem);
        emit sessionSwitchRequest(clickedItem->id());
    });

    connect(item, &SessionItem::itemRenamed, this, [=](int itemId, const QString& newName){
        emit sessionRenameRequest(itemId, newName);
    });

    connect(item, &SessionItem::itemDeleted, this, [=](int deletedId){
        if (m_currentSessionItem == item) {
            m_currentSessionItem = nullptr;
        }
        emit sessionDeleteRequest(deletedId);

        // 【注意】这里要从 m_scrollLayout 移除，而不是 m_mainLayout
        m_scrollLayout->removeWidget(item);
        item->deleteLater();
    });

    // ---------------------------------------------------------
    // 插入 UI
    // ---------------------------------------------------------

    // 【新增】加入管理列表
    m_items.append(item);

    // 【注意】插入到 m_scrollLayout，且在弹簧之前
    m_scrollLayout->insertWidget(m_scrollLayout->count() - 1, item);

    if (m_currentSessionItem == nullptr) {
        handleItemSelection(item);
    }
}

// handleItemSelection 保持不变...
void SessionList::handleItemSelection(SessionItem* clickedItem)
{
    if (m_currentSessionItem == clickedItem) return;
    if (m_currentSessionItem) m_currentSessionItem->setSelected(false);
    if (clickedItem) clickedItem->setSelected(true);
    m_currentSessionItem = clickedItem;
}


void SessionList::clear()
{
    m_currentSessionItem = nullptr;

    for (auto* item : m_items) {
        m_scrollLayout->removeWidget(item);
        delete item;
    }
    m_items.clear();
}

void SessionList::loadSessions(const QVector<SessionData>& sessions)
{
    clear(); // 先清空旧的

    for (const auto& s : sessions) {
        addSession(s.id, s.name);
    }
}
