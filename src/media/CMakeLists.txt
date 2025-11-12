file(GLOB_RECURSE MEDIA_SOURCES "src/*.cpp" "include/*.h")

add_library(media STATIC ${MEDIA_SOURCES})

target_include_directories(media PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIRS}  # 添加FFmpeg头文件路径
)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# 链接依赖
target_link_libraries(media PRIVATE
    Qt6::Core
    Qt6::Widgets
    common
    ${FFMPEG_LIBRARIES}  # 添加FFmpeg库链接
    Qt${QT_VERSION_MAJOR}::Multimedia
)

