file(GLOB_RECURSE MEDIA_SOURCES "src/*.cpp" "include/*.h")

add_library(media STATIC ${MEDIA_SOURCES})

target_include_directories(media PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# 链接依赖
target_link_libraries(media PRIVATE
    Qt6::Core
    Qt6::Widgets
    common
)


