#include "InputPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QActionGroup>
#include <QScrollBar> // 引入滚动条头文件以便美化
#include <QKeyEvent>

InputPanel::InputPanel(QWidget *parent) : QWidget(parent) {
    m_currentResolution = QSize(1024, 1024);

    // 设置底部面板的背景和边框
    this->setStyleSheet("InputPanel { background-color: #343541; border-top: 1px solid #5D5D67; }");

    // 设置一个合理的初始高度策略
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QHBoxLayout* layout = new QHBoxLayout(this);
    // 对齐方式设为 Bottom，这样当输入框变高时，按钮保持在底部对齐（可选，看你喜好）
    layout->setAlignment(Qt::AlignBottom);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // 1. [新增] 参考图按钮 (使用 QToolButton)
    m_btnRef = new QToolButton(this);
    m_btnRef->setText("📎"); // 暂时用文字代替图标，以后换 QIcon
    m_btnRef->setFixedSize(40, 40);
    // 样式：正常是白色，禁用是灰色
    m_btnRef->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 20px; color: white; font-size: 20px; }"
        "QToolButton:hover { background-color: #444; }"
        "QToolButton:disabled { color: #555; border-color: #333; }" // 禁用样式
        );
    layout->addWidget(m_btnRef);

    // =================================================
    // 【新增】反推按钮 (魔法棒)
    // =================================================
    m_btnInterrogate = new QToolButton(this);
    m_btnInterrogate->setText("🪄"); // 魔法棒图标
    m_btnInterrogate->setFixedSize(40, 40);
    m_btnInterrogate->setToolTip("上传图片反推提示词");
    m_btnInterrogate->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 20px; color: white; font-size: 20px; }"
        "QToolButton:hover { background-color: #444; }"
        );
    layout->addWidget(m_btnInterrogate);
    // =================================================


    // =========================================================
    // 【新增】2. 画幅比例按钮
    // =========================================================
    m_btnRatio = new QToolButton(this);
    m_btnRatio->setText("1:1"); // 默认文字
    m_btnRatio->setFixedSize(60, 40); // 稍微宽一点放文字
    m_btnRatio->setPopupMode(QToolButton::InstantPopup); // 点击直接弹菜单
    m_btnRatio->setStyleSheet(
        "QToolButton { background-color: transparent; border: 1px solid #555; border-radius: 4px; color: white; font-weight: bold; }"
        "QToolButton:hover { background-color: #444; }"
        "QToolButton::menu-indicator { image: none; }" // 隐藏自带的小三角
        );

    setupRatioMenu();
    layout->addWidget(m_btnRatio);
    // =========================================================

    // 2. 工作流按钮
    m_btnWorkflow = new QPushButton("🎨 选择工作流", this);
    m_btnWorkflow->setFixedSize(120, 40);
    m_btnWorkflow->setStyleSheet(
        "QPushButton { background-color: #40414F; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #50515F; }"
    );
    layout->addWidget(m_btnWorkflow);

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setPlaceholderText("输入提示词... (Shift+Enter 换行)");

    // 设置初始样式
    m_inputEdit->setStyleSheet(
        "QPlainTextEdit { "
        "   background-color: #40414F; "
        "   color: white; "
        "   border: 1px solid #555; "
        "   border-radius: 4px; "
        "   padding: 8px; " // 内边距大一点更好看
        "   font-size: 14px; "
        "}"
        "QPlainTextEdit:focus { border-color: #19C37D; }"

        // 隐藏/美化滚动条 (和之前 HistoryGallery 类似)
        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #666; border-radius: 4px; }"
        );

    // 初始高度设为一行的高度 (约40px)
    m_inputEdit->setFixedHeight(40);
    m_inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 安装事件过滤器 (用于拦截回车)
    m_inputEdit->installEventFilter(this);

    // 监听文字变化，自动调整高度
    connect(m_inputEdit, &QPlainTextEdit::textChanged, this, &InputPanel::adjustInputHeight);

    layout->addWidget(m_inputEdit);

    // 4. 生成按钮
    m_btnGenerate = new QPushButton("生成", this);
    m_btnGenerate->setFixedSize(80, 40); // 按钮高度固定
    m_btnGenerate->setStyleSheet(
        "QPushButton { background-color: #19C37D; color: white; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1AD48A; }"
        "QPushButton:disabled { background-color: #2A2B32; color: #888; }"
        );
    layout->addWidget(m_btnGenerate);

    // 连接信号
    connect(m_btnGenerate, &QPushButton::clicked, this, &InputPanel::onGenerateClicked);

}



bool InputPanel::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_inputEdit && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);

        // 如果按下的是 Enter (Key_Return)
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // 检查是否按下了 Shift
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                // Shift + Enter -> 允许换行 (默认行为，不做处理，返回 false 让控件自己处理)
                return false;
            } else {
                // 单独按 Enter -> 发送消息
                onGenerateClicked();
                return true; // 事件已处理，不再传递给控件(防止产生换行)
            }
        }
    }
    return QWidget::eventFilter(obj, e);
}

// 【新增】自动调整高度逻辑
void InputPanel::adjustInputHeight()
{
    QTextDocument *doc = m_inputEdit->document();
    // 调整文档布局宽度以匹配控件宽度 (防止换行计算错误)
    doc->setTextWidth(m_inputEdit->viewport()->width());

    // 计算内容总高度
    int contentHeight = doc->size().height();

    // 加上上下的 padding (CSS里设置了 padding: 8px，上下加起来约16，微调一下)
    int margins = 16;
    int totalHeight = contentHeight + margins;

    // 设定限制
    int minHeight = 40;  // 1行的高度
    int maxHeight = 120; // 约 4-5 行的高度

    // 限制在 min 和 max 之间
    int finalHeight = qBound(minHeight, totalHeight, maxHeight);

    if (m_inputEdit->height() != finalHeight) {
        m_inputEdit->setFixedHeight(finalHeight);

        // 如果你的 InputPanel 之前设置了 fixedHeight，这里需要让父控件也 updateGeometry
        // 因为我们去掉了 setFixedHeight，这里会自动触发布局重算
    }

    // 只有当内容超过最大高度时，才显示垂直滚动条
    if (totalHeight > maxHeight) {
        m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void InputPanel::onGenerateClicked() {
    // 获取纯文本
    QString prompt = m_inputEdit->toPlainText().trimmed();
    if (!prompt.isEmpty()) {
        emit generateClicked(prompt);
        // 清空并重置高度
        m_inputEdit->clear();
        adjustInputHeight();
    }
}





void InputPanel::setupRatioMenu()
{
    m_ratioMenu = new QMenu(this);
    m_ratioMenu->setStyleSheet("QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; } QMenu::item:selected { background-color: #40414F; }");

    QActionGroup* group = new QActionGroup(this);

    // 定义 SDXL 甜点分辨率 (总像素约 1024*1024)
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
        action->setData(QSize(r.w, r.h)); // 把宽高存入 data
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

void InputPanel::onRatioSelected(QAction* action)
{
    QSize size = action->data().toSize();
    m_currentResolution = size;

    // 更新按钮文字，取冒号前的部分 (例如 "16:9")
    QString text = action->text().split(" ").first();
    m_btnRatio->setText(text);

    emit resolutionChanged(size.width(), size.height());
}

void InputPanel::updateState(WorkflowType type) {
    if (type == WorkflowType::TextToImage) {
        // --- 文生图模式 ---
        // 1. 禁用参考图上传（因为纯文生图不需要）
        m_btnRef->setEnabled(false);

        // 2. 启用画幅比例选择
        m_btnRatio->setEnabled(true);

        // 3. 【新增】禁用反推按钮（因为没有图片可以反推）
        if (m_btnInterrogate) m_btnInterrogate->setEnabled(false);

    } else {
        // --- 图生图/其他模式 ---
        // 1. 启用参考图上传
        m_btnRef->setEnabled(true);

        // 2. 禁用画幅比例（通常跟随原图尺寸）
        m_btnRatio->setEnabled(false);
        m_btnRatio->setText("Auto");

        // 3. 【新增】启用反推按钮
        if (m_btnInterrogate) m_btnInterrogate->setEnabled(true);
    }
}

QSize InputPanel::currentResolution() const {
    return m_currentResolution;
}



// setLocked 函数也要修改一下，因为控件类型变了
void InputPanel::setLocked(bool locked)
{
    bool enabled = !locked;

    m_btnRef->setEnabled(enabled);
    m_btnInterrogate->setEnabled(enabled);
    m_btnRatio->setEnabled(enabled);
    m_btnWorkflow->setEnabled(enabled);

    // QPlainTextEdit 也有 setEnabled，或者用 setReadOnly
    m_inputEdit->setEnabled(enabled);
    if (locked) {
        m_inputEdit->setPlaceholderText("生成中，请稍候...");
    } else {
        m_inputEdit->setPlaceholderText("输入提示词... (Shift+Enter 换行)");
    }

    m_btnGenerate->setEnabled(enabled);
}
