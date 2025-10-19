# 收集源文件和头文件
file(GLOB COMMON_SOURCES
    "src/*.cpp"
)

file(GLOB COMMON_HEADERS
    "include/*.h"
)

# 创建库
add_library(common_lib STATIC
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

# 设置包含目录
target_include_directories(common_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置编译定义
target_compile_definitions(common_lib PRIVATE 
    QT_VERSION_MAJOR=${QT_VERSION_MAJOR}
)