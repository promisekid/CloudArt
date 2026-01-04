/**
 * @file ChatBubble.cpp
 * @brief 聊天气泡组件实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

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

/**
 * @brief 构造函数
 * @param role 消息角色
 * @param data 消息数据
 * @param parent 父窗口指针
 */
ChatBubble::ChatBubble(ChatRole role, const QVariant& data, QWidget *parent)
    : QWidget(parent)
    , m_role(role)
{
    this->setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(0);

    m_loadingMovie = new QMovie(":/images/loading.gif", QByteArray(), this);
    m_loadingMovie->setScaledSize(QSize(40, 40));

    setupUi(data);
}

/**
 * @brief 初始化UI布局
 * @param data 消息数据
 */
void ChatBubble::setupUi(const QVariant& data)
{
    if (m_role == ChatRole::User) {
        m_layout->addStretch();

        if (data.canConvert<QPixmap>()) {
            initImageBubble(data.value<QPixmap>());
        } else {
            initTextBubble(data.toString());
        }
    }
    else {
        if (data.canConvert<QPixmap>()) {
            initImageBubble(data.value<QPixmap>());
        }
        else if (data.typeId() == QMetaType::QString && data.toString().isEmpty()) {
            initImageBubble(QPixmap());
            setLoading(true);
        }
        else {
            initTextBubble(data.toString());
        }
        m_layout->addStretch();
    }
}

/**
 * @brief 初始化文本气泡
 * @param text 文本内容
 */
void ChatBubble::initTextBubble(const QString& text)
{
    QFrame* frame = new QFrame(this);

    QString style = (m_role == ChatRole::User)
                        ? "background-color: #444654; border-radius: 8px; color: #ECECF1; padding: 10px;"
                        : "background-color: #2A2B32; border-radius: 8px; color: #ECECF1; padding: 10px; border: 1px solid #444;";
    frame->setStyleSheet(style);

    QHBoxLayout* frameLayout = new QHBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    m_contentLabel = new QLabel(text, frame);

    m_contentLabel->setWordWrap(true);
    m_contentLabel->setStyleSheet("border: none; background: transparent;");
    m_contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_contentLabel->setMaximumWidth(600);

    m_contentLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_contentLabel, &QLabel::customContextMenuRequested, this, [=](const QPoint& pos){
        QMenu menu;
        menu.setStyleSheet(
            "QMenu { background: #2D2D2D; color: white; border: 1px solid #555; padding: 5px; }"
            "QMenu::item { padding: 5px 20px; }"
            "QMenu::item:selected { background-color: #40414F; }"
            );

        QAction* actCopyAll = menu.addAction("📋 复制全部内容");
        connect(actCopyAll, &QAction::triggered, [=](){
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(m_contentLabel->text());
        });

        if (m_contentLabel->hasSelectedText()) {
            QAction* actCopySelected = menu.addAction("✂️ 复制选中内容");
            connect(actCopySelected, &QAction::triggered, [=](){
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(m_contentLabel->selectedText());
            });
        }

        menu.exec(m_contentLabel->mapToGlobal(pos));
    });

    frameLayout->addWidget(m_contentLabel);
    m_layout->addWidget(frame);
}

/**
 * @brief 初始化图片气泡
 * @param originalImg 原始图片
 */
void ChatBubble::initImageBubble(const QPixmap& originalImg)
{
    m_currentImage = originalImg;

    m_contentLabel = new QLabel(this);
    m_contentLabel->setStyleSheet("border-radius: 8px; border: 2px solid #444;");

    if (!originalImg.isNull()) {
        QSize maxDisplaySize(512, 512);
        QPixmap scaledImg = originalImg.scaled(maxDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_contentLabel->setPixmap(scaledImg);
        m_contentLabel->setFixedSize(scaledImg.size());

        m_contentLabel->setCursor(Qt::PointingHandCursor);
        m_contentLabel->installEventFilter(this);
    } else {
        m_contentLabel->setFixedSize(200, 200);
        m_contentLabel->setAlignment(Qt::AlignCenter);
    }

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    m_contentLabel->setGraphicsEffect(shadow);

    m_layout->addWidget(m_contentLabel);
}

/**
 * @brief 显示图片查看器
 */
void ChatBubble::showViewer() {
    ImageViewer* viewer = new ImageViewer(m_currentImage, this);
    viewer->exec();
    delete viewer;
}

/**
 * @brief 保存图片
 */
void ChatBubble::saveImage() {
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString fileName = QFileDialog::getSaveFileName(this, "保存图片",
                                                    desktopPath + "/cloudart_gen.png",
                                                    "Images (*.png *.jpg)");
    if (!fileName.isEmpty()) {
        m_currentImage.save(fileName);
    }
}

/**
 * @brief 切换加载状态
 * @param loading 是否显示加载动画
 */
void ChatBubble::setLoading(bool loading)
{
    if (!m_contentLabel) return;

    if (loading) {
        m_contentLabel->setMovie(m_loadingMovie);
        m_loadingMovie->start();
    } else {
        m_loadingMovie->stop();
        m_contentLabel->setMovie(nullptr);
    }
}

/**
 * @brief 更新图片数据
 * @param img 生成的图片数据
 * @param serverFileName 服务器文件名
 */
void ChatBubble::updateImage(const QPixmap& img, const QString& serverFileName)
{
    setLoading(false);

    m_currentImage = img;
    m_serverFileName = serverFileName;

    QSize maxDisplaySize(512, 512);
    QPixmap scaledImg = img.scaled(maxDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_contentLabel->setPixmap(scaledImg);
    m_contentLabel->setFixedSize(scaledImg.size());

    m_contentLabel->setCursor(Qt::PointingHandCursor);
    m_contentLabel->removeEventFilter(this);
    m_contentLabel->installEventFilter(this);
}

/**
 * @brief 事件过滤器处理
 * @param watched 被监视的对象
 * @param event 事件
 * @return bool 是否处理事件
 */
bool ChatBubble::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<QLabel*>(watched) && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            if (!m_currentImage.isNull()) showViewer();
            return true;
        }
        else if (mouseEvent->button() == Qt::RightButton) {
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

                QAction* actUpscale = menu.addAction("✨ 高清修复 (1.5x)");
                connect(actUpscale, &QAction::triggered, this, [=](){
                    emit upscaleRequested(m_serverFileName, m_currentImage);
                });
            }

            else if (m_contentLabel && !m_contentLabel->text().isEmpty()) {
                QAction* actCopyText = menu.addAction("📋 复制内容");
                connect(actCopyText, &QAction::triggered, [=](){
                    QApplication::clipboard()->setText(m_contentLabel->text());
                });
            }

            menu.exec(mouseEvent->globalPosition().toPoint());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

/**
 * @brief 追加文本
 * @param text 要追加的文本内容
 */
void ChatBubble::appendText(const QString& text)
{
    if (!m_contentLabel) {
        initTextBubble("");
    }

    if (m_loadingMovie && m_loadingMovie->state() == QMovie::Running) {
        setLoading(false);
    }

    QString current = m_contentLabel->text();
    m_contentLabel->setText(current + text);
}
