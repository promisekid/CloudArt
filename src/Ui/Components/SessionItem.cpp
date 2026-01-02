#include "SessionItem.h"
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include <QResizeEvent>

SessionItem::SessionItem(int id, const QString& title, QWidget *parent)
    : QWidget(parent), m_id(id), m_fullTitle(title)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setFixedHeight(50);
    // 默认透明
    this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    setupUi();
}

void SessionItem::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->setSpacing(0);

    // 1. 标题
    m_lblTitle = new QLabel(this);
    m_lblTitle->setStyleSheet("color: #ECECF1; font-size: 13px; background: transparent;border: none;");
    m_lblTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_lblTitle->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // 2. 菜单按钮
    m_btnOption = new QToolButton(this);
    m_btnOption->setText("···");
    m_btnOption->setFixedSize(30, 30);
    m_btnOption->setCursor(Qt::PointingHandCursor);

    // 【核心修复】：加上 border: none; 消除那个灰线
    // 默认状态：完全透明，连边框都不要
    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    border: none; "             // <--- 这一行去掉灰线
        "    background: transparent; "  // <--- 背景透明
        "    color: transparent; "       // <--- 文字透明(看起来像隐藏了)
        "}"
        );

    connect(m_btnOption, &QToolButton::clicked, this, &SessionItem::showMenu);

    layout->addWidget(m_lblTitle);
    layout->addWidget(m_btnOption);

    updateTitleText();
}

void SessionItem::enterEvent(QEnterEvent *event)
{
    if (!m_isSelected) {
        this->setStyleSheet("SessionItem { background-color: #2A2B32; border-radius: 6px; }");
    }

    // 悬浮时：文字变白，背景变深灰
    // 同样记得带上 border: none，否则悬浮时线又出来了
    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    color: #ECECF1; "
        "    background-color: #40414F; "
        "    border-radius: 4px; "
        "    border: none; "          // <--- 保持无边框
        "    font-weight: bold; "
        "}"
        "QToolButton:hover { color: white; background-color: #50515F; }" // 按钮自己的hover效果
        );

    QWidget::enterEvent(event);
}

void SessionItem::leaveEvent(QEvent *event)
{
    if (!m_isSelected) {
        this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    }

    // 离开时：变回完全透明（占位但不显示）
    // 依然要带上 border: none
    m_btnOption->setStyleSheet(
        "QToolButton { "
        "    border: none; "
        "    background: transparent; "
        "    color: transparent; "
        "}"
        );

    QWidget::leaveEvent(event);
}

void SessionItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 发送 this 指针，方便 SessionList 直接操作
        emit itemClicked(this);
    }
    QWidget::mousePressEvent(event);
}

void SessionItem::setSelected(bool selected)
{
    m_isSelected = selected;
    if (selected) {
        // 选中态
        this->setStyleSheet("SessionItem { background-color: #343541; border-radius: 6px; border: 1px solid #565869; }");
    } else {
        // 非选中态
        this->setStyleSheet("SessionItem { background-color: transparent; border-radius: 6px; border: none; }");
    }
}

void SessionItem::setTitle(const QString& newTitle)
{
    m_fullTitle = newTitle;
    updateTitleText();
}

void SessionItem::resizeEvent(QResizeEvent *event)
{
    updateTitleText();
    QWidget::resizeEvent(event);
}

void SessionItem::updateTitleText()
{
    // 计算宽度：总宽 - 按钮宽 - 左右边距余量
    int availableWidth = this->width() - m_btnOption->width() - 20;
    if (availableWidth <= 0) return;

    QFontMetrics metrics(m_lblTitle->font());
    QString elidedText = metrics.elidedText(m_fullTitle, Qt::ElideRight, availableWidth);
    m_lblTitle->setText(elidedText);

    // 这样无论鼠标什么时候移上来，Tooltip 早就准备好了
    if (elidedText != m_fullTitle) {
        this->setToolTip(m_fullTitle);
    } else {
        this->setToolTip(""); // 没截断就清空，防止显示多余的提示
    }
}

// 弹出右键菜单
void SessionItem::showMenu()
{
    QMenu menu(this);
    // 设置 QMenu 的样式，让它匹配黑暗主题
    menu.setStyleSheet("QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; border-radius: 8px }"
                       "QMenu::item:selected { background-color: #40414F; }");

    // 1. 添加重命名动作
    QAction* actRename = menu.addAction("✎ 重命名");
    connect(actRename, &QAction::triggered, this, [=](){
        bool ok;
        // 弹出输入框
        QString text = QInputDialog::getText(this, "重命名会话",
                                             "请输入新名称:", QLineEdit::Normal,
                                             m_fullTitle, &ok);
        // 如果用户点了OK且输入不为空
        if (ok && !text.isEmpty()) {
            setTitle(text); // 更新 UI
            emit itemRenamed(m_id, text); // 通知外部/数据库
        }
    });

    // 2. 添加复制动作
    QAction* actCopy = menu.addAction("❐ 复制标题");
    connect(actCopy, &QAction::triggered, this, [=](){
        // 获取系统剪贴板并写入
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_fullTitle);
    });

    // 加一条横线分隔
    menu.addSeparator();

    // 3. 添加删除动作
    QAction* actDelete = menu.addAction("🗑 删除会话");
    connect(actDelete, &QAction::triggered, this, [=](){
        // 删除是危险操作，必须二次确认
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除", "确定要删除这个会话吗？\n此操作无法撤销。",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            emit itemDeleted(m_id); // 发送删除信号
        }
    });

    // 在按钮的正下方弹出菜单
    // mapToGlobal: 把按钮相对于窗口的坐标(0, height)转换成屏幕绝对坐标
    menu.exec(m_btnOption->mapToGlobal(QPoint(0, m_btnOption->height())));
}
