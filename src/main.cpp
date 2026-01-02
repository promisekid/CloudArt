/**
 * @file main.cpp
 * @brief 应用程序主入口文件
 * 
 * 该文件包含应用程序的main函数，负责初始化Qt应用程序、
 * 设置应用元数据、创建主窗口并启动事件循环。
 * 
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include <QApplication>
#include "Ui/MainWindow.h"

/**
 * @brief 应用程序主入口函数
 * 
 * 初始化Qt应用程序，设置应用属性，创建主窗口并启动事件循环。
 * 
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 应用程序退出码
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用元数据
    app.setApplicationName("CloudArt");
    app.setApplicationDisplayName("CloudArt - AI Client");

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}