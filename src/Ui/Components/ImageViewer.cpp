/**
 * @file ImageViewer.cpp
 * @brief 图片查看器组件实现文件
 * 
 * 该文件实现了ImageViewer类，提供图片查看功能，支持自适应显示、缩放和双击还原。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QScreen>
#include <QApplication>
#include <QTimer>

ImageViewer::ImageViewer(const QPixmap& pixmap, QWidget* parent) : QDialog(parent) {
    this->setWindowTitle("查看图片 (滚轮缩放/左键拖拽/双击还原)");
    this->setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->availableGeometry().size();
    this->resize(screenSize * 0.7);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);

    m_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setStyleSheet("background-color: #1e1e1e; border: none;");
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    m_view->viewport()->installEventFilter(this);

    m_item = new QGraphicsPixmapItem(pixmap);
    m_scene->addItem(m_item);

    layout->addWidget(m_view);

    QTimer::singleShot(0, this, [=](){
        fitImageToWindow();
    });
}

void ImageViewer::fitImageToWindow() {
    if (!m_item || !m_view) return;

    m_view->fitInView(m_item, Qt::KeepAspectRatio);

    m_isFitWindow = true;
}

/**
 * @brief 窗口大小改变事件
 * @param event 窗口大小改变事件对象
 */
void ImageViewer::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);

    if (m_isFitWindow) {
        fitImageToWindow();
    }
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    fitImageToWindow();
}

/**
 * @brief 事件过滤器
 * @param watched 监听的对象
 * @param event 事件对象
 * @return bool 是否处理了该事件
 */
bool ImageViewer::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_view->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

        m_isFitWindow = false;

        double angle = wheelEvent->angleDelta().y();
        double factor;
        if (angle > 0) {
            factor = 1.15;
        } else {
            factor = 1.0 / 1.15;
        }

        m_view->scale(factor, factor);

        return true;
    }
    return QDialog::eventFilter(watched, event);
}
