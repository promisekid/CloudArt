/**
 * @file HistoryGallery.cpp
 * @brief 生成历史画廊组件实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "HistoryGallery.h"
#include "../../Database/DatabaseManager.h"
#include <QScrollArea>
#include <QMouseEvent>
#include <QFileInfo>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

class GalleryItem : public QLabel
{
public:
    QString imagePath;
    std::function<void(QString)> onClick;

    GalleryItem(const QString& path, int targetWidth, QWidget* parent = nullptr)
        : QLabel(parent), imagePath(path)
    {
        this->setStyleSheet(
            "QLabel { "
            "  background-color: black; "
            "  border: 1px solid #333; "
            "  border-radius: 6px; "
            "}"
            "QLabel:hover { "
            "  border: 1px solid #19C37D; "
            "  cursor: pointer; "
            "}"
            );
        this->setAlignment(Qt::AlignCenter);

        QPixmap pix(path);
        if (!pix.isNull()) {
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
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            if (onClick) onClick(imagePath);
        }
        else if (event->button() == Qt::RightButton) {
            showContextMenu(event->globalPosition().toPoint());
        }

        QLabel::mousePressEvent(event);
    }

private:
    void showContextMenu(const QPoint& pos) {
        QMenu menu;
        menu.setStyleSheet(
            "QMenu { background-color: #2D2D2D; color: white; border: 1px solid #555; padding: 5px; }"
            "QMenu::item { padding: 5px 20px; }"
            "QMenu::item:selected { background-color: #40414F; }"
            );

        QAction* actCopy = menu.addAction("❐ 复制图片");
        QObject::connect(actCopy, &QAction::triggered, [this](){
            QPixmap originalPix(imagePath);
            if (!originalPix.isNull()) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setPixmap(originalPix);
                qDebug() << "图片已复制到剪贴板:" << imagePath;
            }
        });

        QAction* actPath = menu.addAction("📂 复制路径");
        QObject::connect(actPath, &QAction::triggered, [this](){
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(imagePath);
        });

        menu.exec(pos);
    }
};

HistoryGallery::HistoryGallery(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void HistoryGallery::setupUi()
{
    this->setFixedWidth(260);
    this->setStyleSheet("background-color: #202123; border-right: 1px solid #4D4D4F;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 20, 0, 0);
    mainLayout->setSpacing(10);

    QLabel* title = new QLabel("🎨 生成历史", this);
    title->setStyleSheet("color: #ECECF1; font-weight: bold; font-size: 14px; padding-left: 15px; border: none;");
    mainLayout->addWidget(title);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollArea->verticalScrollBar()->setSingleStep(20);

    m_scrollArea->setStyleSheet(
        "QScrollArea { "
        "   background: transparent; "
        "   border: none; "
        "}"

        "QScrollBar:vertical { "
        "    border: none; "
        "    background: #111111; "
        "    width: 14px; "
        "    margin: 0px; "
        "}"

        "QScrollBar::handle:vertical { "
        "    background: #666666; "
        "    min-height: 30px; "
        "    border-radius: 7px; "
        "    margin: 2px; "
        "}"

        "QScrollBar::handle:vertical:hover { "
        "    background: #999999; "
        "}"

        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "    height: 0px; "
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
        "    background: none; "
        "}"
        );

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");

    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    m_scrollLayout->setContentsMargins(15, 10, 5, 10);
    m_scrollLayout->setSpacing(15);
    m_scrollLayout->setAlignment(Qt::AlignTop);

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

    int cardWidth = 220;

    for (const QString& path : paths) {
        if (!QFileInfo::exists(path)) continue;

        GalleryItem* item = new GalleryItem(path, cardWidth, m_scrollContent);

        item->onClick = [this](QString p){
            emit imageClicked(p);
        };

        m_scrollLayout->addWidget(item);
    }
}
