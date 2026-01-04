#pragma once

#include <QWidget>
#include <QToolButton>

class QVBoxLayout;

class SidebarControl : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarControl(QWidget* parent = nullptr);
    ~SidebarControl();

    QToolButton* toggleBtn() const;
    QToolButton* historyBtn() const;
    void updateToggleState(bool isExpanded);
    // 【新增】获取设置按钮的指针
    QToolButton* settingsBtn() const;

private:
    QToolButton* createBtn(const QString& iconPath, const QString& tooltip);

    QVBoxLayout* m_layout;
    QToolButton* m_toggleBtn;
    QToolButton* m_historyBtn;
    // 【新增】设置按钮变量
    QToolButton* m_settingsBtn;
};
