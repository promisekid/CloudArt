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

private:
    QToolButton* createBtn(const QString& iconPath, const QString& tooltip);

    QVBoxLayout* m_layout;
    QToolButton* m_toggleBtn;
    QToolButton* m_historyBtn;
};
