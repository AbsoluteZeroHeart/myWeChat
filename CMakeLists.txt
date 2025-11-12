cmake_minimum_required(VERSION 3.16)

project(myWeChat VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 关键配置


# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置自动处理Qt的moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# 查找FFmpeg包
find_package(FFmpeg REQUIRED)

# 查找Qt包 - 添加 Concurrent 模块
find_package(QT NAMES Qt6 REQUIRED COMPONENTS
    Widgets
    Multimedia
    MultimediaWidgets
    Concurrent
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    Multimedia
    MultimediaWidgets
    Concurrent
)

# 设置编译定义
if(QT_VERSION_MAJOR EQUAL 6)
    add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
else()
    add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050000)
endif()

# 添加子目录
add_subdirectory(src/common)
add_subdirectory(src/network)
add_subdirectory(src/storage)
add_subdirectory(src/media)
add_subdirectory(src/controller)
add_subdirectory(src/ui)

 
