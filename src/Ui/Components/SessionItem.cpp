/**
 * @file SessionItem.cpp
 * @brief 会话项组件实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "SessionItem.h"
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include <QResizeEvent>

/**
 * @brief 构造函数
 * @param id 会话ID
 * @param title 会话标题
 * @param parent 父窗口指针
 */
SessionItem::SessionItem(int id, const QString& title, QWidget *parent)
    : QWidget(parent), m_id(id), m_fullTitle(title)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setFixedHeight(50);
    this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    setupUi();
}

/**
 * @brief 初始化UI布局
 */
void SessionItem::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->setSpacing(0);

    m_lblTitle = new QLabel(this);
    m_lblTitle->setStyleSheet("color: #ECECF1; font-size: 13px; background: transparent;border: none;");
    m_lblTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_lblTitle->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_btnOption = new QToolButton(this);
    m_btnOption->setText("···");
    m_btnOption->setFixedSize(30, 30);
    m_btnOption->setCursor(Qt::PointingHandCursor);

    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    border: none; "
        "    background: transparent; "
        "    color: transparent; "
        "}"
        );

    connect(m_btnOption, &QToolButton::clicked, this, &SessionItem::showMenu);

    layout->addWidget(m_lblTitle);
    layout->addWidget(m_btnOption);

    updateTitleText();
}

/**
 * @brief 鼠标进入事件处理
 * @param event 鼠标进入事件
 */
void SessionItem::enterEvent(QEnterEvent *event)
{
    if (!m_isSelected) {
        this->setStyleSheet("SessionItem { background-color: #2A2B32; border-radius: 6px; }");
    }

    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    color: #ECECF1; "
        "    background-color: #40414F; "
        "    border-radius: 4px; "
        "    border: none; "
        "    font-weight: bold; "
        "}"
        "QToolButton:hover { color: white; background-color: #50515F; }"
        );

    QWidget::enterEvent(event);
}

/**
 * @brief 鼠标离开事件处理
 * @param event 鼠标离开事件
 */
void SessionItem::leaveEvent(QEvent *event)
{
    if (!m_isSelected) {
        this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    }

    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    border: none; "
        "    background: transparent; "
        "    color: transparent; "
        "}"
        );

    QWidget::leaveEvent(event);
}

/**
 * @brief 鼠标按下事件处理
 * @param event 鼠标按下事件
 */
void SessionItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit itemClicked(this);
    }
    QWidget::mousePressEvent(event);
}

/**
 * @brief 设置选中状态
 * @param selected 是否选中
 */
void SessionItem::setSelected(bool selected)
{
    m_isSelected = selected;
    if (selected) {
        this->setStyleSheet("SessionItem { background-color: #343541; border-radius: 6px; border: 1px solid #565869; }");
    } else {
        this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    }
}

/**
 * @brief 更新标题
 * @param newTitle 新标题
 */
void SessionItem::setTitle(const QString& newTitle)
{
    m_fullTitle = newTitle;
    updateTitleText();
}

/**
 * @brief 窗口大小改变事件处理
 * @param event 大小改变事件
 */
void SessionItem::resizeEvent(QResizeEvent *event)
{
    updateTitleText();
    QWidget::resizeEvent(event);
}

/**
 * @brief 更新标题文本显示
 */
void SessionItem::updateTitleText()
{
    int availableWidth = this->width() - m_btnOption->width() - 20;
    if (availableWidth <= 0) return;

    QFontMetrics metrics(m_lblTitle->font());
    QString elidedText = metrics.elidedText(m_fullTitle, Qt::ElideRight, availableWidth);
    m_lblTitle->setText(elidedText);

    if (elidedText != m_fullTitle) {
        this->setToolTip(m_fullTitle);
    } else {
        this->setToolTip("");
    }
}

/**
 * @brief 显示右键菜单
 */
void SessionItem::showMenu()
{
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; border-radius: 8px }"
                       "QMenu::item:selected { background-color: #40414F; }");

    QAction* actRename = menu.addAction("✎ 重命名");
    connect(actRename, &QAction::triggered, this, [=](){
        bool ok;
        QString text = QInputDialog::getText(this, "重命名会话",
                                             "请输入新名称:", QLineEdit::Normal,
                                             m_fullTitle, &ok);
        if (ok && !text.isEmpty()) {
            setTitle(text);
            emit itemRenamed(m_id, text);
        }
    });

    QAction* actCopy = menu.addAction("❐ 复制标题");
    connect(actCopy, &QAction::triggered, this, [=](){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_fullTitle);
    });

    menu.addSeparator();

    QAction* actDelete = menu.addAction("🗑 删除会话");
    connect(actDelete, &QAction::triggered, this, [=](){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除", "确定要删除这个会话吗？\n此操作无法撤销。",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            emit itemDeleted(m_id);
        }
    });

    menu.exec(m_btnOption->mapToGlobal(QPoint(0, m_btnOption->height())));
}
