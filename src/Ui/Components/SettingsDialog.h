/**
 * @file SettingsDialog.h
 * @brief 设置对话框组件头文件
 * 
 * 该文件定义了SettingsDialog类，用于配置应用程序的连接设置。
 * 提供ComfyUI服务器地址的配置界面。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <QDialog>
#include <QLineEdit>

/**
 * @brief 设置对话框类
 * 
 * 继承自QDialog，提供应用程序设置界面。
 * 用于配置ComfyUI服务器的连接地址。
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SettingsDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取配置的URL
     * @return QString ComfyUI服务器地址
     */
    QString getUrl() const;

private:
    QLineEdit* m_editUrl = nullptr; ///< URL输入框
};
