#include "ChatBubble.h"
#include "ImageViewer.h"
#include <QFrame>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>

ChatBubble::ChatBubble(ChatRole role, const QVariant& data, QWidget *parent)
    : QWidget(parent)
    , m_role(role)
{
    // 允许自定义背景（虽然 Bubble 本身通常透明）
    this->setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(0); // 紧凑布局

    // 【新增】初始化加载动画
    // 请确保 qrc 里有这个文件，否则会显示空白
    m_loadingMovie = new QMovie(":/images/loading.gif", QByteArray(), this);
    m_loadingMovie->setScaledSize(QSize(40, 40)); // 设置合适的大小

    setupUi(data);
}

void ChatBubble::setupUi(const QVariant& data)
{
    if (m_role == ChatRole::User) {
        m_layout->addStretch();
        initTextBubble(data.toString());
    } else {
        // AI 气泡逻辑
        if (data.canConvert<QPixmap>()) {
            initImageBubble(data.value<QPixmap>());
        }
        // 【新增】如果是空字符串，说明是占位气泡
        else if (data.typeId() == QMetaType::QString && data.toString().isEmpty()) {
            initImageBubble(QPixmap()); // 传空图初始化
            setLoading(true);           // 开启转圈
        }
        else {
            initTextBubble(data.toString()); // 容错
        }
        m_layout->addStretch();
    }
}

void ChatBubble::initTextBubble(const QString& text)
{
    // 创建一个容器 Frame 用来画背景和圆角
    QFrame* frame = new QFrame(this);

    // 只有用户的气泡有背景色
    QString style = (m_role == ChatRole::User)
                        ? "background-color: #444654; border-radius: 8px; color: #ECECF1; padding: 10px;"
                        : "background-color: transparent; color: #ECECF1; padding: 10px;";

    frame->setStyleSheet(style);

    // 内部放 Label 显示文字
    QHBoxLayout* frameLayout = new QHBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* lblText = new QLabel(text, frame);
    lblText->setWordWrap(true); // 关键：自动换行
    lblText->setStyleSheet("border: none; background: transparent;");
    lblText->setTextInteractionFlags(Qt::TextSelectableByMouse); // 允许复制文字

    // 限制最大宽度，防止气泡太宽读起来累
    lblText->setMaximumWidth(600);

    // 如果不设置这个，右键 Label 只会弹出系统默认的菜单（或者没反应）
    lblText->setContextMenuPolicy(Qt::CustomContextMenu);

    // 4. 连接信号
    // 当用户在 Label 上右键时，会触发 customContextMenuRequested 信号
    connect(lblText, &QLabel::customContextMenuRequested, this, [=](const QPoint& pos){
        QMenu menu;
        menu.setStyleSheet(
            "QMenu { background: #2D2D2D; color: white; border: 1px solid #555; padding: 5px; }"
            "QMenu::item { padding: 5px 20px; }"
            "QMenu::item:selected { background-color: #40414F; }"
            );

        // 添加“复制全文”动作
        QAction* actCopyAll = menu.addAction("📋 复制全部内容");
        connect(actCopyAll, &QAction::triggered, [=](){
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(text); // 复制完整的原始文本
        });

        // 如果用户选中了部分文本，也可以添加“复制选中” (可选)
        if (lblText->hasSelectedText()) {
            QAction* actCopySelected = menu.addAction("✂️ 复制选中内容");
            connect(actCopySelected, &QAction::triggered, [=](){
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(lblText->selectedText());
            });
        }

        // 在鼠标位置弹出菜单
        // mapToGlobal(pos) 把 Label 内部坐标转为屏幕坐标
        menu.exec(lblText->mapToGlobal(pos));
    });

    frameLayout->addWidget(lblText);
    m_layout->addWidget(frame);
}

void ChatBubble::initImageBubble(const QPixmap& originalImg)
{
    m_currentImage = originalImg;

    // 创建 Label 并保存到成员变量
    m_contentLabel = new QLabel(this);
    m_contentLabel->setStyleSheet("border-radius: 8px; border: 2px solid #444;");

    if (!originalImg.isNull()) {
        // 有图：正常显示
        QSize maxDisplaySize(512, 512);
        QPixmap scaledImg = originalImg.scaled(maxDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_contentLabel->setPixmap(scaledImg);
        m_contentLabel->setFixedSize(scaledImg.size());

        // 开启交互
        m_contentLabel->setCursor(Qt::PointingHandCursor);
        m_contentLabel->installEventFilter(this);
    } else {
        // 无图（Loading态）：设置一个固定大小的占位
        m_contentLabel->setFixedSize(200, 200);
        m_contentLabel->setAlignment(Qt::AlignCenter);
    }

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    m_contentLabel->setGraphicsEffect(shadow);

    m_layout->addWidget(m_contentLabel);
}

void ChatBubble::showViewer() {
    // 创建查看器并显示（非模态或模态皆可，这里用模态简单点）
    ImageViewer* viewer = new ImageViewer(m_currentImage, this);
    viewer->exec(); // 模态运行，关闭后自动释放
    delete viewer;
}

void ChatBubble::saveImage() {
    // 获取系统的图片文件夹路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString fileName = QFileDialog::getSaveFileName(this, "保存图片",
                                                    desktopPath + "/cloudart_gen.png",
                                                    "Images (*.png *.jpg)");
    if (!fileName.isEmpty()) {
        m_currentImage.save(fileName);
    }
}

// 【新增】切换加载状态
void ChatBubble::setLoading(bool loading)
{
    if (!m_contentLabel) return;

    if (loading) {
        m_contentLabel->setMovie(m_loadingMovie);
        m_loadingMovie->start();
    } else {
        m_loadingMovie->stop();
        m_contentLabel->setMovie(nullptr); // 清除 Movie 绑定
    }
}

// 【新增】生成完成后更新图片
void ChatBubble::updateImage(const QPixmap& img, const QString& serverFileName)
{
    setLoading(false); // 停止转圈

    m_currentImage = img;
    m_serverFileName = serverFileName; // 记住服务器文件名 (关键！)

    // 重新计算大小并显示
    QSize maxDisplaySize(512, 512);
    QPixmap scaledImg = img.scaled(maxDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_contentLabel->setPixmap(scaledImg);
    m_contentLabel->setFixedSize(scaledImg.size());

    // 激活交互（因为初始化为空时可能没激活）
    m_contentLabel->setCursor(Qt::PointingHandCursor);
    m_contentLabel->removeEventFilter(this); // 防止重复安装
    m_contentLabel->installEventFilter(this);
}

bool ChatBubble::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<QLabel*>(watched) && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            if (!m_currentImage.isNull()) showViewer(); // 只有有图才能看
            return true;
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            // 右键菜单
            QMenu menu;
            menu.setStyleSheet(
                "QMenu { background: #2D2D2D; color: white; border: 1px solid #555; padding: 5px; }"
                "QMenu::item { padding: 5px 20px; }"
                "QMenu::item:selected { background-color: #40414F; }"
                );

            if (!m_currentImage.isNull()) {
                QAction* actCopy = menu.addAction("❐ 复制图片");
                connect(actCopy, &QAction::triggered, this, [=](){
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setPixmap(m_currentImage);
                });

                QAction* actSave = menu.addAction("💾 另存为...");
                connect(actSave, &QAction::triggered, this, &ChatBubble::saveImage);

                menu.addSeparator();

                // 【新增】只有当服务器文件名存在时，才显示高清修复
                if (!m_serverFileName.isEmpty()) {
                    QAction* actUpscale = menu.addAction("✨ 高清修复 (1.5x)");
                    connect(actUpscale, &QAction::triggered, this, [=](){
                        // 修改：把图片也发出去
                        emit upscaleRequested(m_serverFileName, m_currentImage);
                    });
                }
            }

            menu.exec(mouseEvent->globalPosition().toPoint());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
