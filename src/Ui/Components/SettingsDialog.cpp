/**
 * @file SettingsDialog.cpp
 * @brief 服务器设置对话框组件实现文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("服务器设置");
    setFixedSize(400, 200);

    QSettings settings("CloudArt", "AppConfig");
    QString savedUrl = settings.value("Server/Url", "http://127.0.0.1:8000").toString();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* lbl = new QLabel("地址:", this);
    m_editUrl = new QLineEdit(savedUrl, this);
    m_editUrl->setPlaceholderText("例如: http://frp-fly.top:12345");

    mainLayout->addWidget(lbl);
    mainLayout->addWidget(m_editUrl);

    QLabel* tip = new QLabel("复制完整的穿透链接填入", this);
    tip->setStyleSheet("color: #666; font-size: 12px; margin-top: 5px;");
    tip->setWordWrap(true);
    mainLayout->addWidget(tip);

    mainLayout->addStretch();

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [=](){
        QSettings settings("CloudArt", "AppConfig");
        settings.setValue("Server/Url", m_editUrl->text().trimmed());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

/**
 * @brief 获取服务器地址
 * @return QString 服务器地址
 */
QString SettingsDialog::getUrl() const {
    return m_editUrl->text().trimmed();
}
