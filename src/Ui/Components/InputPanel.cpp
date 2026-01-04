/**
 * @file InputPanel.cpp
 * @brief 输入面板组件实现文件
 * 
 * 该文件实现了InputPanel类，提供提示词输入、参考图上传、画幅比例选择和工作流选择等功能。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "InputPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QActionGroup>
#include <QScrollBar>
#include <QKeyEvent>

InputPanel::InputPanel(QWidget *parent) : QWidget(parent) {
    m_currentResolution = QSize(1024, 1024);

    this->setStyleSheet("InputPanel { background-color: #343541; border-top: 1px solid #5D5D67; }");

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignBottom);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    m_btnRef = new QToolButton(this);
    m_btnRef->setText("📎");
    m_btnRef->setFixedSize(40, 40);
    m_btnRef->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 20px; color: white; font-size: 20px; }"
        "QToolButton:hover { background-color: #444; }"
        "QToolButton:disabled { color: #555; border-color: #333; }"
        );
    layout->addWidget(m_btnRef);

    m_btnInterrogate = new QToolButton(this);
    m_btnInterrogate->setText("🪄");
    m_btnInterrogate->setFixedSize(40, 40);
    m_btnInterrogate->setToolTip("上传图片反推提示词");
    m_btnInterrogate->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 20px; color: white; font-size: 20px; }"
        "QToolButton:hover { background-color: #444; }"
        );
    layout->addWidget(m_btnInterrogate);

    m_btnRatio = new QToolButton(this);
    m_btnRatio->setText("1:1");
    m_btnRatio->setFixedSize(60, 40);
    m_btnRatio->setPopupMode(QToolButton::InstantPopup);
    m_btnRatio->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 4px; color: white; font-weight: bold; }"
        "QToolButton:hover { background-color: #444; }"
        "QToolButton::menu-indicator { image: none; }"
        );

    setupRatioMenu();
    layout->addWidget(m_btnRatio);

    m_btnWorkflow = new QPushButton("🎨 选择工作流", this);
    m_btnWorkflow->setFixedSize(120, 40);
    m_btnWorkflow->setStyleSheet(
        "QPushButton { background-color: #40414F; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #50515F; }"
    );
    layout->addWidget(m_btnWorkflow);

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setPlaceholderText("输入提示词... (Shift+Enter 换行)");

    m_inputEdit->setStyleSheet(
        "QPlainTextEdit { "
        "   background-color: #40414F; "
        "   color: white; "
        "   border: 1px solid #555; "
        "   border-radius: 4px; "
        "   padding: 8px; "
        "   font-size: 14px; "
        "}"
        "QPlainTextEdit:focus { border-color: #19C37D; }"

        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #666; border-radius: 4px; }"
        );

    m_inputEdit->setFixedHeight(40);
    m_inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_inputEdit->installEventFilter(this);

    connect(m_inputEdit, &QPlainTextEdit::textChanged, this, &InputPanel::adjustInputHeight);

    layout->addWidget(m_inputEdit);

    m_btnGenerate = new QPushButton("生成", this);
    m_btnGenerate->setFixedSize(80, 40);
    m_btnGenerate->setStyleSheet(
        "QPushButton { background-color: #19C37D; color: white; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1AD48A; }"
        "QPushButton:disabled { background-color: #2A2B32; color: #888; }"
        );
    layout->addWidget(m_btnGenerate);

    connect(m_btnGenerate, &QPushButton::clicked, this, &InputPanel::onGenerateClicked);
}

bool InputPanel::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_inputEdit && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                return false;
            } else {
                onGenerateClicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, e);
}

/**
 * @brief 调整输入框高度
 */
void InputPanel::adjustInputHeight()
{
    QTextDocument *doc = m_inputEdit->document();
    doc->setTextWidth(m_inputEdit->viewport()->width());

    int contentHeight = doc->size().height();

    int margins = 16;
    int totalHeight = contentHeight + margins;

    int minHeight = 40;
    int maxHeight = 120;

    int finalHeight = qBound(minHeight, totalHeight, maxHeight);

    if (m_inputEdit->height() != finalHeight) {
        m_inputEdit->setFixedHeight(finalHeight);
    }

    if (totalHeight > maxHeight) {
        m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void InputPanel::onGenerateClicked() {
    QString prompt = m_inputEdit->toPlainText().trimmed();
    if (!prompt.isEmpty()) {
        emit generateClicked(prompt);
        m_inputEdit->clear();
        adjustInputHeight();
    }
}

/**
 * @brief 设置画幅比例菜单
 */
void InputPanel::setupRatioMenu()
{
    m_ratioMenu = new QMenu(this);
    m_ratioMenu->setStyleSheet("QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; } QMenu::item:selected { background-color: #40414F; }");

    QActionGroup* group = new QActionGroup(this);

    struct Ratio { QString name; int w; int h; };
    QList<Ratio> ratios = {
        {"1:1 (方图)", 1024, 1024},
        {"3:4 (竖图)", 896, 1152},
        {"4:3 (横图)", 1152, 896},
        {"9:16 (手机)", 832, 1216},
        {"16:9 (电脑)", 1216, 832}
    };

    for (const auto& r : ratios) {
        QAction* action = m_ratioMenu->addAction(r.name);
        action->setData(QSize(r.w, r.h));
        action->setCheckable(true);
        group->addAction(action);

        if (r.name.startsWith("1:1")) {
            action->setChecked(true);
            m_currentResolution = QSize(r.w, r.h);
        }
    }

    connect(m_ratioMenu, &QMenu::triggered, this, &InputPanel::onRatioSelected);
    m_btnRatio->setMenu(m_ratioMenu);
}

/**
 * @brief 画幅比例选择槽函数
 * @param action 选中的菜单项
 *
 * 更新当前画幅比例并发出分辨率变化信号。
 */
void InputPanel::onRatioSelected(QAction* action)
{
    QSize size = action->data().toSize();
    m_currentResolution = size;

    QString text = action->text().split(" ").first();
    m_btnRatio->setText(text);

    emit resolutionChanged(size.width(), size.height());
}

/**
 * @brief 更新面板状态
 * @param type 工作流类型
 */
void InputPanel::updateState(WorkflowType type) {
    if (type == WorkflowType::TextToImage) {
        m_btnRef->setEnabled(false);

        m_btnRatio->setEnabled(true);

        if (m_btnInterrogate) m_btnInterrogate->setEnabled(false);

    } else {
        m_btnRef->setEnabled(true);

        m_btnRatio->setEnabled(false);
        m_btnRatio->setText("Auto");

        if (m_btnInterrogate) m_btnInterrogate->setEnabled(true);
    }
}

/**
 * @brief 获取当前分辨率
 * @return QSize 当前分辨率
 */
QSize InputPanel::currentResolution() const {
    return m_currentResolution;
}

/**
 * @brief 设置锁定状态
 * @param locked 是否锁定
 */
void InputPanel::setLocked(bool locked)
{
    bool enabled = !locked;

    m_btnRef->setEnabled(enabled);
    m_btnInterrogate->setEnabled(enabled);
    m_btnRatio->setEnabled(enabled);
    m_btnWorkflow->setEnabled(enabled);

    m_inputEdit->setEnabled(enabled);
    if (locked) {
        m_inputEdit->setPlaceholderText("生成中，请稍候...");
    } else {
        m_inputEdit->setPlaceholderText("输入提示词... (Shift+Enter 换行)");
    }

    m_btnGenerate->setEnabled(enabled);
}

/**
 * @brief 设置连接状态
 * @param isConnected 是否已连接
 */
void InputPanel::setConnectionStatus(bool isConnected)
{
    bool enable = isConnected;

    m_btnGenerate->setEnabled(enable);
    m_btnWorkflow->setEnabled(enable);
    m_btnRef->setEnabled(enable);
    m_inputEdit->setEnabled(enable);

    if (m_btnInterrogate) m_btnInterrogate->setEnabled(enable);
    if (m_btnRatio) m_btnRatio->setEnabled(enable);

    if (isConnected) {
        m_inputEdit->setPlaceholderText("输入提示词... (Shift+Enter 换行)");
        m_btnGenerate->setText("生成");
    } else {
        m_inputEdit->setPlaceholderText("⚠️ 未连接服务器，请点击左下角设置进行连接...");
        m_btnGenerate->setText("未连接");
    }
}
