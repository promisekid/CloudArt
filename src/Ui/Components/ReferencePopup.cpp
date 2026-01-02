#include "ReferencePopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QStackedLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QFileInfo>

ReferencePopup::ReferencePopup(QWidget *parent) : QWidget(parent) {
    // 【重要】：允许拖拽
    this->setAcceptDrops(true);

    // 设置窗口属性
    // 使用 Qt::Tool 而不是 Popup，防止点击外部时窗口自动关闭
    // 同时配合 Qt::FramelessWindowHint 去掉边框
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    
    // 设置焦点策略，允许窗口获取焦点
    this->setFocusPolicy(Qt::StrongFocus);

    this->setFixedSize(320, 240); //稍微大一点

    // 初始化成员变量，确保能记住已添加的参考图
    m_currentImage = QPixmap();
    m_currentPath.clear();

    setupUi();
}

void ReferencePopup::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 主容器（深色圆角背景）
    QWidget* container = new QWidget(this);
    container->setStyleSheet(
        "QWidget { background-color: #2D2D2D; border: 1px solid #444; border-radius: 8px; }"
        );
    // 给 container 加阴影会更好看（这里省略，保持简洁）

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(15, 15, 15, 15);

    // 2. 标题栏
    QLabel* title = new QLabel("参考图 (Reference)", container);
    title->setStyleSheet("color: #ECECF1; font-weight: bold; border: none;");
    containerLayout->addWidget(title);

    // 3. 堆栈布局 (核心：切换空状态和预览状态)
    m_stackLayout = new QStackedLayout();

    // --- 页面 A: 空状态 (拖拽区 + 按钮) ---
    m_pageEmpty = new QWidget(container);
    m_pageEmpty->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_pageEmpty);
    emptyLayout->setContentsMargins(0, 10, 0, 0);

    // 虚线框 Label
    QLabel* lblDropZone = new QLabel("拖拽图片到此处\n\n或", m_pageEmpty);
    lblDropZone->setAlignment(Qt::AlignCenter);
    lblDropZone->setStyleSheet(
        "QLabel { "
        "  border: 2px dashed #555; "
        "  border-radius: 6px; "
        "  color: #888; "
        "  background-color: #343541; "
        "}"
        );

    // 上传按钮
    QPushButton* btnUpload = new QPushButton("选择本地文件", m_pageEmpty);
    btnUpload->setCursor(Qt::PointingHandCursor);
    btnUpload->setFixedHeight(36);
    btnUpload->setStyleSheet(
        "QPushButton { background-color: #40414F; color: white; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #50515F; }"
        );
    connect(btnUpload, &QPushButton::clicked, this, [=](){
        QString path = QFileDialog::getOpenFileName(this, "选择参考图",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!path.isEmpty()) {
            loadImage(path);
        }
    });

    emptyLayout->addWidget(lblDropZone);
    emptyLayout->addWidget(btnUpload);

    // --- 页面 B: 预览状态 (图片 + 删除按钮) ---
    m_pagePreview = new QWidget(container);
    m_pagePreview->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* previewLayout = new QVBoxLayout(m_pagePreview);
    previewLayout->setContentsMargins(0, 10, 0, 0);

    // 图片预览区
    m_lblPreview = new QLabel(m_pagePreview);
    m_lblPreview->setAlignment(Qt::AlignCenter);
    m_lblPreview->setStyleSheet("border: 1px solid #444; border-radius: 4px; background-color: #000;");
    m_lblPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 删除按钮
    QPushButton* btnRemove = new QPushButton("🗑 移除参考图", m_pagePreview);
    btnRemove->setCursor(Qt::PointingHandCursor);
    btnRemove->setFixedHeight(36);
    btnRemove->setStyleSheet(
        "QPushButton { background-color: #7f1d1d; color: #fecaca; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #991b1b; }"
        );
    connect(btnRemove, &QPushButton::clicked, this, [=](){
        m_currentImage = QPixmap(); // 清空
        m_currentPath.clear();
        updateUiState(); // 回到空状态
    });

    previewLayout->addWidget(m_lblPreview);
    previewLayout->addWidget(btnRemove);

    // 将两个页面加入堆栈
    m_stackLayout->addWidget(m_pageEmpty);
    m_stackLayout->addWidget(m_pagePreview);

    // 把堆栈加入主布局
    containerLayout->addLayout(m_stackLayout);
    mainLayout->addWidget(container);

    // 初始状态
    updateUiState();
}

void ReferencePopup::popup(const QPoint& pos) {
    // 水平居中，垂直在按钮上方
    int x = pos.x() - (this->width() / 2);
    int y = pos.y() - this->height() - 10;
    this->move(x, y);
    this->show();
    this->raise(); // 确保在最上层
    this->setFocus(); // 获取焦点，用于处理焦点失去事件
}

void ReferencePopup::hide() {
    QWidget::hide();
    // 清除焦点，避免干扰其他窗口
    this->clearFocus();
}

// ---------------------------------------------------------
// 拖拽核心逻辑
// ---------------------------------------------------------

void ReferencePopup::dragEnterEvent(QDragEnterEvent *event) {
    // 1. 检查是否有文件
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) return;

        // 2. 检查后缀名是否是图片
        QString filePath = urls.first().toLocalFile();
        QFileInfo info(filePath);
        QString suffix = info.suffix().toLower();

        if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "bmp" || suffix == "webp") {
            // 是图片，允许拖入
            event->acceptProposedAction();
        }
    }
}

void ReferencePopup::dropEvent(QDropEvent *event) {
    const QMimeData* mime = event->mimeData();
    if (mime->hasUrls()) {
        QString filePath = mime->urls().first().toLocalFile();
        loadImage(filePath);
        event->acceptProposedAction();
    }
}

// ---------------------------------------------------------
// 图片加载与渲染
// ---------------------------------------------------------

void ReferencePopup::loadImage(const QString& path) {
    QPixmap img(path);
    if (img.isNull()) return;

    m_currentPath = path;
    m_currentImage = img;

    // 为了美观，我们需要根据 Label 的大小对图片进行缩放（保持比例）
    // 注意：这里简单处理，实际渲染可能需要 resizeEvent 配合
    QSize targetSize = m_lblPreview->size();
    if (targetSize.isEmpty()) targetSize = QSize(280, 150); // 兜底尺寸

    // 缩放图片
    QPixmap scaled = img.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_lblPreview->setPixmap(scaled);

    updateUiState();
}

void ReferencePopup::updateUiState() {
    if (m_currentImage.isNull()) {
        m_stackLayout->setCurrentWidget(m_pageEmpty);
    } else {
        m_stackLayout->setCurrentWidget(m_pagePreview);

        // 如果是在预览页，这里也可以再次触发一下缩放，防止 label 大小未更新
        if (!m_currentImage.isNull()) {
            QSize s = QSize(280, 150);
            m_lblPreview->setPixmap(m_currentImage.scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}
