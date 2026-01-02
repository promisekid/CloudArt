#include "InputPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QActionGroup>

InputPanel::InputPanel(QWidget *parent) : QWidget(parent) {
    m_currentResolution = QSize(1024, 1024);

    this->setFixedHeight(120);
    this->setStyleSheet("background-color: #343541; border-top: 1px solid #5D5D67;");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20); // 调整边距
    layout->setSpacing(15); // 控件间距

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

    // 3. 真正的输入框 (替换模拟的QLabel)
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText("输入提示词...");
    m_inputEdit->setStyleSheet(
        "QLineEdit { background-color: #40414F; color: white; border: 1px solid #555; border-radius: 4px; padding: 0 10px; }"
        "QLineEdit:focus { border-color: #19C37D; }"
        "QLineEdit::placeholder { color: #CCC; }"
    );
    m_inputEdit->setFixedHeight(40);
    m_inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 只有它会伸缩
    layout->addWidget(m_inputEdit);

    // 4. 生成按钮
    m_btnGenerate = new QPushButton("生成", this);
    m_btnGenerate->setFixedSize(80, 40);
    m_btnGenerate->setStyleSheet(
        "QPushButton { background-color: #19C37D; color: white; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1AD48A; }"
    );
    layout->addWidget(m_btnGenerate);

    // 连接信号槽
    connect(m_btnGenerate, &QPushButton::clicked, this, &InputPanel::onGenerateClicked);
    
    // 连接回车键信号
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &InputPanel::onGenerateClicked);
}



void InputPanel::onGenerateClicked() {
    QString prompt = m_inputEdit->text().trimmed();
    if (!prompt.isEmpty()) {
        emit generateClicked(prompt);
        // 清空输入框
        m_inputEdit->clear();
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
    // 只有文生图才允许调整分辨率
    // 图生图通常跟随参考图比例 (或者你也想强制改)
    if (type == WorkflowType::TextToImage) {
        m_btnRef->setEnabled(false);
        m_btnRatio->setEnabled(true);  // 启用比例
    } else {
        m_btnRef->setEnabled(true);
        m_btnRatio->setEnabled(false); // 禁用比例 (假设图生图用 SmartResize 自动控制)
        m_btnRatio->setText("Auto");
    }
}

QSize InputPanel::currentResolution() const {
    return m_currentResolution;
}
