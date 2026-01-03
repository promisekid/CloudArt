#include <QApplication>
#include "Ui/MainWindow.h"
// 引入数据库头文件
#include "Database/DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("CloudArt");

    // 【新增】启动时初始化数据库
    // 如果失败了，直接打印错误（或者弹窗提示），但不一定要退出
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
