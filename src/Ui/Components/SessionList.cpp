/**
 * @file SessionList.cpp
 * @brief 会话列表组件实现文件
 * 
 * 该文件实现了SessionList类，提供会话列表管理功能，包括会话的添加、选择、重命名和删除。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "SessionList.h"
#include "SessionItem.h"
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

SessionList::SessionList(QWidget *parent)
    : QWidget(parent)
    , m_currentSessionItem(nullptr)
{
    setupUi();
}

/**
 * @brief 设置UI界面
 */
void SessionList::setupUi()
{
    this->setFixedWidth(260);
    this->setStyleSheet("background-color: #202123; border-right: 1px solid #4D4D4F;");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 20, 0, 0);
    rootLayout->setSpacing(10);

    QWidget* topContainer = new QWidget(this);
    QVBoxLayout* topLayout = new QVBoxLayout(topContainer);
    topLayout->setContentsMargins(10, 0, 10, 0);

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

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    scrollArea->setStyleSheet(
        "QScrollArea { "
        "   background: transparent; "
        "   border: none; "
        "}"

        "QScrollBar:vertical { "
        "    border: none; "
        "    background: #111111; "
        "    width: 14px; "
        "    margin: 0px; "
        "}"

        "QScrollBar::handle:vertical { "
        "    background: #666666; "
        "    min-height: 30px; "
        "    border-radius: 7px; "
        "    margin: 2px; "
        "}"

        "QScrollBar::handle:vertical:hover { "
        "    background: #999999; "
        "}"

        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "    height: 0px; "
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
        "    background: none; "
        "}"
        );

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");

    m_scrollLayout = new QVBoxLayout(scrollContent);
    m_scrollLayout->setContentsMargins(10, 10, 10, 10);
    m_scrollLayout->setSpacing(5);

    QLabel* labelTitle = new QLabel("最近历史", scrollContent);
    labelTitle->setStyleSheet("color: #8E8EA0; font-size: 12px; margin-bottom: 5px; border: none;");
    m_scrollLayout->addWidget(labelTitle);

    scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(scrollArea);

    m_scrollLayout->addStretch();
}

void SessionList::addSession(int id, const QString& title)
{
    SessionItem* item = new SessionItem(id, title, this);

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

        m_scrollLayout->removeWidget(item);
        item->deleteLater();
    });

    m_items.append(item);

    m_scrollLayout->insertWidget(m_scrollLayout->count() - 1, item);

    if (m_currentSessionItem == nullptr) {
        handleItemSelection(item);
    }
}

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
    clear();

    for (const auto& s : sessions) {
        addSession(s.id, s.name);
    }
}

void SessionList::selectSession(int id)
{
    for (SessionItem* item : m_items) {
        if (item->id() == id) {
            handleItemSelection(item);
            return;
        }
    }
}

/**
 * @brief 获取第一个会话ID
 * @return 会话ID，如果没有会话则返回-1
 */
int SessionList::getFirstSessionId() const
{
    if (m_items.isEmpty()) return -1;
    return m_items.first()->id();
}
