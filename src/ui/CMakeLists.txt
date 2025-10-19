# 收集源文件
file(GLOB UI_SOURCES
    "src/*.cpp"
    "src/media/*.cpp"
    "src/dialogBox/*.cpp"
    "src/chatList/*.cpp"
    "src/chatConversations/*.cpp"
)

file(GLOB UI_HEADERS   
    "include/*.h"
    "include/media/*.h"
    "include/dialogBox/*.h"
    "include/chatList/*.h"
    "include/chatConversations/*.h"
)

# 收集资源文件
file(GLOB UI_RESOURCES
    "res/*.qrc"
)

# 设置项目源文件
set(PROJECT_SOURCES
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UI_RESOURCES}

    src/dialogBox/RightPopover.ui
    src/dialogBox/AddDialog.ui
    src/dialogBox/MoreDialog.ui
    src/dialogBox/FloatingDialog.ui
    src/dialogBox/PersonalInfoDialog.ui
    src/media/MediaDialog.ui
    src/LoginAndRegisterDialog.ui
    src/WeChatWidget.ui

)
 
# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(myWeChat
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(weChat SHARED ${PROJECT_SOURCES})
    else()
        add_executable(weChat ${PROJECT_SOURCES})
    endif()
endif()

# 设置包含目录
target_include_directories(myWeChat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/media
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dialogBox
    ${CMAKE_CURRENT_SOURCE_DIR}/include/chatList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/chatConversations
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../controller/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../network/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../storage/include
)

# 添加编译定义
target_compile_definitions(myWeChat PRIVATE
    QT_VERSION_MAJOR=${QT_VERSION_MAJOR}
)

# 链接Qt库
target_link_libraries(myWeChat PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt6::MultimediaWidgets
)

# 链接其他模块的库（根据你的实际库名调整）
target_link_libraries(myWeChat PRIVATE
#     common_lib
#     core_lib
#     net_lib
    storage
#     media_lib
)

# 平台特定设置
if(APPLE)
    set_target_properties(myWeChat PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.myWeChat
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
    )
elseif(WIN32)
    set_target_properties(myWeChat PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装配置
include(GNUInstallDirs)
install(TARGETS myWeChat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6 最终化
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(myWeChat)
endif()



