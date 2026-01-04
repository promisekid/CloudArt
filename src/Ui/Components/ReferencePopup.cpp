/**
 * @file ReferencePopup.cpp
 * @brief 参考图弹窗组件实现文件
 * 
 * 该文件实现了ReferencePopup类，提供参考图上传和预览功能，支持拖拽和文件选择。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

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

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 *
 * 初始化参考图弹窗，设置窗口属性为无边框、透明背景、置顶显示。
 */
ReferencePopup::ReferencePopup(QWidget *parent) : QWidget(parent) {
    this->setAcceptDrops(true);

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    
    this->setFocusPolicy(Qt::StrongFocus);

    this->setFixedSize(320, 240);

    m_currentImage = QPixmap();
    m_currentPath.clear();

    setupUi();
}

void ReferencePopup::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* container = new QWidget(this);
    container->setStyleSheet(
        "QWidget { background-color: #2D2D2D; border: 1px solid #444; border-radius: 8px; }"
        );

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(15, 15, 15, 15);

    QLabel* title = new QLabel("参考图 (Reference)", container);
    title->setStyleSheet("color: #ECECF1; font-weight: bold; border: none;");
    containerLayout->addWidget(title);

    m_stackLayout = new QStackedLayout();

    m_pageEmpty = new QWidget(container);
    m_pageEmpty->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_pageEmpty);
    emptyLayout->setContentsMargins(0, 10, 0, 0);

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

    m_pagePreview = new QWidget(container);
    m_pagePreview->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* previewLayout = new QVBoxLayout(m_pagePreview);
    previewLayout->setContentsMargins(0, 10, 0, 0);

    m_lblPreview = new QLabel(m_pagePreview);
    m_lblPreview->setAlignment(Qt::AlignCenter);
    m_lblPreview->setStyleSheet("border: 1px solid #444; border-radius: 4px; background-color: #000;");
    m_lblPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPushButton* btnRemove = new QPushButton("🗑 移除参考图", m_pagePreview);
    btnRemove->setCursor(Qt::PointingHandCursor);
    btnRemove->setFixedHeight(36);
    btnRemove->setStyleSheet(
        "QPushButton { background-color: #7f1d1d; color: #fecaca; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #991b1b; }"
        );
    connect(btnRemove, &QPushButton::clicked, this, [=](){
        m_currentImage = QPixmap();
        m_currentPath.clear();
        updateUiState();
    });

    previewLayout->addWidget(m_lblPreview);
    previewLayout->addWidget(btnRemove);

    m_stackLayout->addWidget(m_pageEmpty);
    m_stackLayout->addWidget(m_pagePreview);

    containerLayout->addLayout(m_stackLayout);
    mainLayout->addWidget(container);

    updateUiState();
}

/**
 * @brief 显示弹窗
 * @param pos 弹窗位置
 */
void ReferencePopup::popup(const QPoint& pos) {
    int x = pos.x() - (this->width() / 2);
    int y = pos.y() - this->height() - 10;
    this->move(x, y);
    this->show();
    this->raise();
    this->setFocus();
}

void ReferencePopup::hide() {
    QWidget::hide();
    this->clearFocus();
}

/**
 * @brief 拖拽进入事件
 * @param event 拖拽事件对象
 */
void ReferencePopup::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) return;

        QString filePath = urls.first().toLocalFile();
        QFileInfo info(filePath);
        QString suffix = info.suffix().toLower();

        if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "bmp" || suffix == "webp") {
            event->acceptProposedAction();
        }
    }
}

/**
 * @brief 放置事件
 * @param event 放置事件对象
 */
void ReferencePopup::dropEvent(QDropEvent *event) {
    const QMimeData* mime = event->mimeData();
    if (mime->hasUrls()) {
        QString filePath = mime->urls().first().toLocalFile();
        loadImage(filePath);
        event->acceptProposedAction();
    }
}

/**
 * @brief 加载图片
 * @param path 图片路径
 */
void ReferencePopup::loadImage(const QString& path) {
    QPixmap img(path);
    if (img.isNull()) return;

    m_currentPath = path;
    m_currentImage = img;

    QSize targetSize = m_lblPreview->size();
    if (targetSize.isEmpty()) targetSize = QSize(280, 150);

    QPixmap scaled = img.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_lblPreview->setPixmap(scaled);

    updateUiState();
}

/**
 * @brief 更新UI状态
 */
void ReferencePopup::updateUiState() {
    if (m_currentImage.isNull()) {
        m_stackLayout->setCurrentWidget(m_pageEmpty);
    } else {
        m_stackLayout->setCurrentWidget(m_pagePreview);

        if (!m_currentImage.isNull()) {
            QSize s = QSize(280, 150);
            m_lblPreview->setPixmap(m_currentImage.scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}
