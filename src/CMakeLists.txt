# 定义源代码文件列表
set(SOURCES
    main.cpp
    logo.rc
    Ui/MainWindow.h
    Ui/MainWindow.cpp

    #Core
    Core/WorkflowManager.h
    Core/WorkflowManager.cpp

    # Model
    Model/WorkflowTypes.h
    Model/DataModels.h

    # Database
    Database/DatabaseManager.h
    Database/DatabaseManager.cpp

    # Network
    Network/ComfyApiService.h
    Network/ComfyApiService.cpp

    # Components
    Ui/Components/SidebarControl.h
    Ui/Components/SidebarControl.cpp
    Ui/Components/SessionList.h
    Ui/Components/SessionList.cpp
    Ui/Components/ChatArea.h
    Ui/Components/ChatArea.cpp
    Ui/Components/InputPanel.h
    Ui/Components/InputPanel.cpp
    Ui/Components/WorkflowSelector.h
    Ui/Components/WorkflowSelector.cpp
    Ui/Components/WorkflowCard.h
    Ui/Components/WorkflowCard.cpp
    Ui/Components/ReferencePopup.h
    Ui/Components/ReferencePopup.cpp
    Ui/Components/SessionItem.h
    Ui/Components/SessionItem.cpp
    Ui/Components/ChatBubble.h
    Ui/Components/ChatBubble.cpp
    Ui/Components/ImageViewer.h
    Ui/Components/ImageViewer.cpp
    Ui/Components/HistoryGallery.h
    Ui/Components/HistoryGallery.cpp
    Ui/Components/SettingsDialog.h
    Ui/Components/SettingsDialog.cpp
    ../resources/resources.qrc
)

# 定义可执行文件
add_executable(CloudArt ${SOURCES})

# 链接 Qt 库
target_link_libraries(CloudArt PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

# 设置 Windows 子系统 (避免运行时弹出黑色 cmd 窗口，但在开发调试期可以先注释掉下面这行)
# set_target_properties(CloudArt PROPERTIES WIN32_EXECUTABLE ON)
