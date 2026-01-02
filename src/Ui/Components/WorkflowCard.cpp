/**
 * @file WorkflowCard.cpp
 * @brief 工作流卡片组件实现文件
 * 
 * 该文件实现了WorkflowCard类，用于显示工作流卡片，支持静态图片和GIF动画背景，
 * 包含鼠标悬停效果、缩放动画和文字信息展示功能。
 * 
 * @author 系统自动生成
 * @version 1.0
 * @date 2024
 */

#include "WorkflowCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QLinearGradient>
#include <QPixmap>
#include <QFile>
#include <QPainterPath>
#include <QMovie>

/**
 * @brief WorkflowCard构造函数
 * @param info 工作流信息结构体，包含名称、描述、图片路径等
 * @param parent 父窗口指针
 * 
 * 初始化工作流卡片，设置UI布局、动画效果和事件处理。
 * 支持GIF动画的延迟初始化以优化性能。
 */
WorkflowCard::WorkflowCard(const WorkflowInfo& info, QWidget* parent)
    : QWidget(parent)
    , m_info(info)
    , m_scale(1.0)
    , m_currentScale(1.0)
    , m_isHovering(false)
    , m_backgroundLabel(nullptr)
{
    setupUi();
    
    // 设置缩放动画
    m_scaleAnimation->setDuration(200);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 初始化GIF动画 - 延迟初始化
    if (!m_info.gifPath.isEmpty()) {
        m_movie = nullptr; // 延迟初始化，在enterEvent中创建
        qDebug() << "GIF路径已设置，将延迟初始化:" << m_info.gifPath;
    }
    
    // 添加阴影效果
    auto *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setOffset(0, 5);
    this->setGraphicsEffect(shadowEffect);
    
    // 设置鼠标跟踪
    this->setMouseTracking(true);
}

/**
 * @brief 设置UI界面
 * 
 * 创建卡片的主要UI组件，包括背景图片、文字容器、名称和描述标签。
 * 设置16:9比例的卡片大小，并配置样式表和布局。
 */
void WorkflowCard::setupUi()
{
    // 设置16:9比例的卡片大小 (320x180)
    this->setFixedSize(320, 180);
    this->setStyleSheet(
        "WorkflowCard {"
        "    background-color: #252525;"
        "    border: 1px solid #444444;"
        "    border-radius: 12px;"
        "}"
        "WorkflowCard:hover {"
        "    border: 1px solid #666666;"
        "}"
    );
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建背景图片标签
    m_backgroundLabel = new QLabel(this);
    m_backgroundLabel->setFixedSize(320, 180);
    m_backgroundLabel->setScaledContents(true);
    m_backgroundLabel->lower(); // 确保背景在最底层
    
    // 文字容器（在最左上角）
    QWidget *textContainer = new QWidget(m_backgroundLabel);
    textContainer->setStyleSheet(
        "background: rgba(0, 0, 0, 120);"
        "border-radius: 8px;"
    );
    textContainer->setGeometry(0, 0, 288, 80); // 增加高度以容纳更大的文字
    
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);// 设置布局边距和间距
    textLayout->setContentsMargins(8, 8, 8, 8); // 调整边距以适应更大的容器
    textLayout->setSpacing(6); // 增加间距以适应更大的文字
    textLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // 名称标签
    m_nameLabel = new QLabel(m_info.name, textContainer);
    m_nameLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 600;"
        "color: white;"
        "background: transparent;"
        "padding: 0;"
        "margin: 0;"
        "border: none"
    );
    
    // 描述标签
    m_descriptionLabel = new QLabel(m_info.description, textContainer);
    m_descriptionLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 400;"
        "color: #cccccc;"
        "background: transparent;"
        "padding: 0;"
        "margin: 0;"
        "border: none;"
        "min-height: 40px;" // 增加最小高度
    );
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumWidth(250);
    m_descriptionLabel->setMinimumHeight(40); // 设置最小高度
    
    textLayout->addWidget(m_nameLabel);
    textLayout->addWidget(m_descriptionLabel);
    
    mainLayout->addWidget(m_backgroundLabel);
    
    // 创建缩放动画
    m_scaleAnimation = new QPropertyAnimation(this, "scale", this);
    
    // 设置初始静态图片
    updateBackgroundImage();
}

/**
 * @brief 更新背景图片
 * 
 * 根据当前状态更新背景图片，优先使用GIF动画的当前帧，
 * 如果没有GIF动画则使用静态图片，如果都没有则使用默认背景。
 */
void WorkflowCard::updateBackgroundImage()
{
    // 如果有GIF动画正在播放，使用GIF当前帧
    if (m_movie && m_movie->isValid() && m_movie->state() == QMovie::Running) {
        QPixmap currentFrame = m_movie->currentPixmap();
        if (!currentFrame.isNull()) {
            m_backgroundLabel->setPixmap(currentFrame);
            m_backgroundLabel->setVisible(true);
            qDebug() << "更新背景图片为GIF当前帧";
        } else {
            qDebug() << "GIF当前帧为空";
        }
    } else {
        // 使用静态图片
        if (!m_info.imagePath.isEmpty()) {
            QPixmap pixmap(m_info.imagePath);
            if (!pixmap.isNull()) {
                m_backgroundLabel->setPixmap(pixmap);
                m_backgroundLabel->setVisible(true);
                qDebug() << "更新背景图片为静态图片:" << m_info.imagePath;
            } else {
                qDebug() << "无法加载静态图片:" << m_info.imagePath;
                // 使用默认背景
                QPixmap defaultPixmap(320, 180);
                defaultPixmap.fill(QColor(40, 40, 40));
                m_backgroundLabel->setPixmap(defaultPixmap);
            }
        } else {
            m_backgroundLabel->clear();
            m_backgroundLabel->setVisible(false);
            qDebug() << "背景图片路径为空，隐藏背景";
        }
    }
}

/**
 * @brief 设置卡片缩放比例
 * @param scale 缩放比例值
 * 
 * 设置卡片的缩放比例，限制在0.5到2.0之间，
 * 触发重绘并发出缩放变化信号。
 */
void WorkflowCard::setScale(qreal scale)
{
    // 限制缩放范围在0.5到2.0之间
    qreal boundedScale = qBound(0.5, scale, 2.0);
    
    if (m_scale != boundedScale) {
        m_scale = boundedScale;
        m_currentScale = boundedScale;
        
        // 触发重绘以应用新的缩放
        this->update();
        
        // 可选：发出缩放变化信号
        emit scaleChanged(m_scale);
    }
}

/**
 * @brief 鼠标进入事件处理
 * @param event 鼠标进入事件
 * 
 * 处理鼠标悬停效果，开始GIF动画，更新背景图片和鼠标样式。
 */
void WorkflowCard::enterEvent(QEnterEvent *event)
{
    m_isHovering = true;
    
    // 移除缩放动画，保持卡片原始大小
    // 开始GIF动画
    startGifAnimation();
    
    // 更新背景图片
    updateBackgroundImage();
    
    // 更新鼠标样式
    this->setCursor(Qt::PointingHandCursor);
    
    QWidget::enterEvent(event);
}

/**
 * @brief 鼠标离开事件处理
 * @param event 鼠标离开事件
 * 
 * 处理鼠标离开效果，停止GIF动画，恢复背景图片和鼠标样式。
 */
void WorkflowCard::leaveEvent(QEvent *event)
{
    m_isHovering = false;
    
    // 停止GIF动画
    stopGifAnimation();
    
    // 更新背景图片
    updateBackgroundImage();
    
    // 恢复鼠标样式
    this->setCursor(Qt::ArrowCursor);
    
    QWidget::leaveEvent(event);
}

/**
 * @brief 鼠标按下事件处理
 * @param event 鼠标事件
 * 
 * 处理鼠标点击事件，当左键点击时发出clicked信号。
 */
void WorkflowCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_info);
    }
    QWidget::mousePressEvent(event);
}

/**
 * @brief 窗口大小改变事件处理
 * @param event 大小改变事件
 * 
 * 处理窗口大小改变，触发重绘以确保UI正确显示。
 */
void WorkflowCard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    this->update(); // 直接调用update()触发重绘
}

/**
 * @brief 绘制事件处理
 * @param event 绘制事件
 * 
 * 重写绘制事件，明确不处理背景绘制，完全由QLabel负责。
 * 此方法仅保留作为占位，确保不会意外绘制背景。
 */
void WorkflowCard::paintEvent(QPaintEvent *event)
{
    // 明确不处理背景绘制，完全由QLabel负责
    // 此方法仅保留作为占位，确保不会意外绘制背景
    QWidget::paintEvent(event);
}

/**
 * @brief 开始GIF动画
 * 
 * 延迟初始化GIF动画，连接帧更新信号，启动动画播放。
 * 支持GIF动画的缓存和错误处理。
 */
void WorkflowCard::startGifAnimation()
{
    // 延迟初始化GIF
    if (!m_info.gifPath.isEmpty() && !m_movie) {
        qDebug() << "延迟初始化GIF:" << m_info.gifPath;
        m_movie = new QMovie(m_info.gifPath, QByteArray(), this);
        m_movie->setCacheMode(QMovie::CacheAll);
        
        if (!m_movie->isValid()) {
            qDebug() << "无法加载GIF:" << m_info.gifPath << "错误:" << m_movie->lastErrorString();
            delete m_movie;
            m_movie = nullptr;
            return;
        }
        
        qDebug() << "成功加载GIF:" << m_info.gifPath << "帧数:" << m_movie->frameCount();
        
        // 连接GIF帧更新信号，触发重绘
        connect(m_movie, &QMovie::frameChanged, this, [this](int frameNumber) {
            Q_UNUSED(frameNumber)
            updateBackgroundImage();
        });
    }
    
    if (m_movie && m_movie->isValid()) {
        qDebug() << "启动GIF动画:" << m_info.gifPath;
        m_movie->start();
        
        // 确保GIF正在播放
        if (m_movie->state() != QMovie::Running) {
            qDebug() << "GIF未能启动，状态:" << m_movie->state();
        }
        
        // 更新背景图片
        updateBackgroundImage();
    } else {
        qDebug() << "无法启动GIF动画，movie无效或路径为空";
    }
}

/**
 * @brief 停止GIF动画
 * 
 * 停止GIF动画播放，重置到第一帧，确保下次播放从头开始。
 */
void WorkflowCard::stopGifAnimation()
{
    if (m_movie && m_movie->isValid()) {
        qDebug() << "停止GIF动画:" << m_info.gifPath;
        m_movie->stop();
        
        // 重置GIF到第一帧，确保下次播放从头开始
        m_movie->jumpToFrame(0);
        
        // 更新背景图片
        updateBackgroundImage();
    }
}
