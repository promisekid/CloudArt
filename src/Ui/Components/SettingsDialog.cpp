#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings> // 必须引用这个

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("服务器设置");
    setFixedSize(400, 200); // 稍微改大一点

    // 1. 读取保存的配置 (如果没有保存过，默认就是本地 8000)
    QSettings settings("CloudArt", "AppConfig");
    QString savedUrl = settings.value("Server/Url", "http://127.0.0.1:8000").toString();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 输入框
    QLabel* lbl = new QLabel("地址:", this);
    m_editUrl = new QLineEdit(savedUrl, this);
    m_editUrl->setPlaceholderText("例如: http://frp-fly.top:12345");

    mainLayout->addWidget(lbl);
    mainLayout->addWidget(m_editUrl);

    // 提示信息
    QLabel* tip = new QLabel("复制完整的穿透链接填入", this);
    tip->setStyleSheet("color: #666; font-size: 12px; margin-top: 5px;");
    tip->setWordWrap(true);
    mainLayout->addWidget(tip);

    mainLayout->addStretch();

    // 按钮
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [=](){
        // 【关键】点击确定时，保存配置
        QSettings settings("CloudArt", "AppConfig");
        settings.setValue("Server/Url", m_editUrl->text().trimmed());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

QString SettingsDialog::getUrl() const {
    return m_editUrl->text().trimmed();
}
