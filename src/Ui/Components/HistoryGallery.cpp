#include "HistoryGallery.h"
#include "../../Database/DatabaseManager.h"
#include <QScrollArea>
#include <QMouseEvent>
#include <QFileInfo>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QMenu>          // 【新增】菜单
#include <QClipboard>     // 【新增】剪贴板
#include <QApplication>   // 【新增】获取全局剪贴板

// =========================================================
// 内部类：单个图片卡片 (支持左键查看，右键复制)
// =========================================================
class GalleryItem : public QLabel
{
public:
    QString imagePath; // 存储原图路径
    std::function<void(QString)> onClick; // 点击回调

    GalleryItem(const QString& path, int targetWidth, QWidget* parent = nullptr)
        : QLabel(parent), imagePath(path)
    {
        // 1. 样式设置
        this->setStyleSheet(
            "QLabel { "
            "  background-color: black; "
            "  border: 1px solid #333; "
            "  border-radius: 6px; "
            "}"
            "QLabel:hover { "
            "  border: 1px solid #19C37D; " // 悬停变绿
            "  cursor: pointer; "
            "}"
            );
        this->setAlignment(Qt::AlignCenter);

        // 2. 加载并缩放图片 (仅显示缩略图以节省内存)
        QPixmap pix(path);
        if (!pix.isNull()) {
            // 宽度固定，高度按比例自动缩放，使用平滑算法
            QPixmap scaled = pix.scaledToWidth(targetWidth, Qt::SmoothTransformation);
            this->setPixmap(scaled);
            this->setFixedSize(scaled.size());
        } else {
            this->setText("❌ 图片丢失");
            this->setFixedSize(targetWidth, 60);
            this->setStyleSheet("color: #666; border: 1px dashed #444; border-radius: 6px;");
        }
    }

protected:
    // 捕获鼠标点击事件
    void mousePressEvent(QMouseEvent* event) override {
        // --- 情况 A: 左键点击 -> 查看大图 ---
        if (event->button() == Qt::LeftButton) {
            if (onClick) onClick(imagePath);
        }
        // --- 情况 B: 右键点击 -> 弹出菜单 ---
        else if (event->button() == Qt::RightButton) {
            showContextMenu(event->globalPosition().toPoint());
        }

        QLabel::mousePressEvent(event);
    }

private:
    void showContextMenu(const QPoint& pos) {
        QMenu menu;
        // 统一深色风格样式
        menu.setStyleSheet(
            "QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; padding: 5px; }"
            "QMenu::item { padding: 5px 20px; }"
            "QMenu::item:selected { background-color: #40414F; }"
            );

        // 动作 1: 复制图片
        QAction* actCopy = menu.addAction("❐ 复制图片");
        QObject::connect(actCopy, &QAction::triggered, [this](){
            // 【关键】从路径重新加载原图，而不是复制缩略图
            QPixmap originalPix(imagePath);
            if (!originalPix.isNull()) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setPixmap(originalPix);
                qDebug() << "图片已复制到剪贴板:" << imagePath;
            }
        });

        // 动作 2: 复制文件路径 (可选，方便调试)
        QAction* actPath = menu.addAction("📂 复制路径");
        QObject::connect(actPath, &QAction::triggered, [this](){
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(imagePath);
        });

        menu.exec(pos);
    }
};

// =========================================================
// HistoryGallery 主逻辑 (无需改动，保持原样即可)
// =========================================================

HistoryGallery::HistoryGallery(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void HistoryGallery::setupUi()
{
    // 1. 面板基础样式
    this->setFixedWidth(260);
    this->setStyleSheet("background-color: #202123; border-right: 1px solid #4D4D4F;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 20, 0, 0);
    mainLayout->setSpacing(10);

    // 2. 标题
    QLabel* title = new QLabel("🎨 生成历史", this);
    title->setStyleSheet("color: #ECECF1; font-weight: bold; font-size: 14px; padding-left: 15px; border: none;");
    mainLayout->addWidget(title);

    // 3. 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 水平永远关

    // 【修改点 1】强制垂直滚动条总是显示（即使内容很少也能看到轨道）
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    m_scrollArea->setFrameShape(QFrame::NoFrame);

    // 4. 优化滚轮速度
    m_scrollArea->verticalScrollBar()->setSingleStep(20);

    // 5. 【修改点 2】高对比度样式，加宽滚动条
    m_scrollArea->setStyleSheet(
        "QScrollArea { "
        "   background: transparent; "
        "   border: none; "
        "}"

        // --- 滚动条轨道 (槽) ---
        "QScrollBar:vertical { "
        "    border: none; "
        "    background: #111111; "  // 改成接近黑色，和面板的深灰区分开
        "    width: 14px; "          // 加宽到 14px，非常明显
        "    margin: 0px; "
        "}"

        // --- 滚动滑块 (那个拖动的块) ---
        "QScrollBar::handle:vertical { "
        "    background: #666666; "  // 明显的灰色
        "    min-height: 30px; "     // 最小高度设大一点
        "    border-radius: 7px; "   // 圆角
        "    margin: 2px; "          // 留边距，让滑块悬浮在轨道里
        "}"

        // --- 鼠标悬停变亮 ---
        "QScrollBar::handle:vertical:hover { "
        "    background: #999999; "  // 悬停变白一点
        "}"

        // --- 隐藏上下箭头 (如果不隐藏，可能会占据空间显示不出来) ---
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "    height: 0px; "
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
        "    background: none; "
        "}"
        );

    // 6. 滚动容器 & 布局
    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");

    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    // 这里的边距要注意：右边距可以稍微改小一点(比如5)，因为滚动条占了12px空间
    m_scrollLayout->setContentsMargins(15, 10, 5, 10);
    m_scrollLayout->setSpacing(15);
    m_scrollLayout->setAlignment(Qt::AlignTop); // 必须置顶

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);
}

void HistoryGallery::clearLayout()
{
    QLayoutItem* item;
    while ((item = m_scrollLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void HistoryGallery::loadImages()
{
    clearLayout();

    QVector<QString> paths = DatabaseManager::instance().getAllAiImages();

    if (paths.isEmpty()) {
        QLabel* empty = new QLabel("暂无记录", m_scrollContent);
        empty->setStyleSheet("color: #666; font-size: 12px; margin-top: 20px; border:none;");
        empty->setAlignment(Qt::AlignHCenter);
        m_scrollLayout->addWidget(empty);
        return;
    }

    int cardWidth = 220; // 适配宽度的缩略图尺寸

    for (const QString& path : paths) {
        if (!QFileInfo::exists(path)) continue;

        GalleryItem* item = new GalleryItem(path, cardWidth, m_scrollContent);

        // 绑定左键点击
        item->onClick = [this](QString p){
            emit imageClicked(p);
        };

        m_scrollLayout->addWidget(item);
    }
}
