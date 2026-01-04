#pragma once
#include <QDialog>
#include <QLineEdit>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // 【修改】只留一个获取 URL 的函数
    QString getUrl() const;

private:
    // 【修改】只留一个输入框指针
    QLineEdit* m_editUrl;
};
