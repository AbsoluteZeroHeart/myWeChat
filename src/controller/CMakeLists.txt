# 收集源文件和头文件
file(GLOB CORE_SOURCES
    "src/*.cpp"
)

file(GLOB CORE_HEADERS
    "include/*.h"
)

# 创建库
add_library(core_lib STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# 设置包含目录
target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(core_lib PRIVATE
    common_lib
)

# 设置编译定义
target_compile_definitions(core_lib PRIVATE 
    QT_VERSION_MAJOR=${QT_VERSION_MAJOR}
)


#如果core需要连接其他库，可以
# target_link_libraries(core PUBLIC common)
