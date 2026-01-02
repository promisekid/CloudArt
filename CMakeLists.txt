cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(CloudArt VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准 (C++17 是现代 Qt6 开发的标配)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 核心设置：自动处理元对象系统(MOC)、资源(RCC)和UI文件(UIC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 组件 (核心组件、界面组件、网络组件)
find_package(Qt6 COMPONENTS Widgets Network WebSockets REQUIRED)

# 添加子目录 (将构建逻辑下放到 src 文件夹)
add_subdirectory(src)
