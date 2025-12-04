file(GLOB_RECURSE  test_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE  test_HEADERS
    "*.h"
)

add_library(test STATIC
    ${test_SOURCES}
    ${test_HEADERS}
    TestWidget.ui
)

# 设置包含目录
target_include_directories(test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/aiClient
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# 链接依赖
target_link_libraries(test PRIVATE
    Qt6::Core
    Qt6::Widgets
    common
    storage
    controller
    Qt6::Network
)


