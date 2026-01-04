/**
 * @file main.cpp
 * @brief 应用程序主入口文件
 * @author CloudArt Team
 * @version 1.0
 * @date 2024
 */

#include <QApplication>
#include "Ui/MainWindow.h"
#include "Database/DatabaseManager.h"

/**
 * @brief 应用程序主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 应用程序退出码
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("CloudArt");
    app.setWindowIcon(QIcon(":/images/logo.png"));

    if (!DatabaseManager::instance().init()) {
        qDebug() << "⚠️ 警告：数据库初始化失败，历史记录将无法保存！";
    }

    if (!DatabaseManager::instance().init()) {
        return -1;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
