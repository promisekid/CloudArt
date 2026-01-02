#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QScreen>
#include <QApplication>
#include <QTimer>

ImageViewer::ImageViewer(const QPixmap& pixmap, QWidget* parent) : QDialog(parent) {
    this->setWindowTitle("查看图片 (滚轮缩放/左键拖拽/双击还原)");
    this->setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    // 1. 设置合理的初始窗口大小 (屏幕的 70%)
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->availableGeometry().size();
    this->resize(screenSize * 0.7);

    // 2. 初始化 Graphics 体系
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);

    // --- 关键设置 ---
    // 开启抗锯齿，让图片缩放后依然平滑
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    // 开启“抓手”模式：左键按住拖拽
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    // 隐藏滚动条 (我们用无限画布模式，通常不需要滚动条)
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置背景色为深灰
    m_view->setStyleSheet("background-color: #1e1e1e; border: none;");
    // 缩放时的锚点：以鼠标为中心缩放，体验最好
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // 安装事件过滤器用于监听滚轮（因为 QGraphicsView 可能会吃掉滚轮事件）
    m_view->viewport()->installEventFilter(this);

    // 添加图片
    m_item = new QGraphicsPixmapItem(pixmap);
    m_scene->addItem(m_item);

    layout->addWidget(m_view);

    // 3. 初始状态：适应窗口
    // 使用 Timer 延时 0ms 调用，确保 layout 完成后再计算大小
    QTimer::singleShot(0, this, [=](){
        fitImageToWindow();
    });
}

// 核心逻辑：自动适配
void ImageViewer::fitImageToWindow() {
    if (!m_item || !m_view) return;

    // Qt 神技：自动把 item 缩放到 view 的大小，并保持比例
    m_view->fitInView(m_item, Qt::KeepAspectRatio);

    // 标记为"自适应模式"
    m_isFitWindow = true;
}

// 事件1: 窗口大小改变
void ImageViewer::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);

    // 如果用户还没手动缩放过，那么窗口变大，图片也跟着变大
    if (m_isFitWindow) {
        fitImageToWindow();
    }
}

// 事件2: 双击还原
void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    // 双击任意地方，恢复到“适应窗口”状态
    fitImageToWindow();
}

// 事件3: 滚轮缩放
bool ImageViewer::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_view->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

        // 只要用户动了滚轮，就退出“自适应模式”
        m_isFitWindow = false;

        // 计算缩放比例
        double angle = wheelEvent->angleDelta().y();
        double factor;
        if (angle > 0) {
            factor = 1.15; // 放大 15%
        } else {
            factor = 1.0 / 1.15; // 缩小
        }

        // 执行缩放
        m_view->scale(factor, factor);

        return true; // 事件已处理，不再向下传递
    }
    return QDialog::eventFilter(watched, event);
}
