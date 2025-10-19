cmake_minimum_required(VERSION 3.16)
project(storage)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找必要的Qt模块
find_package(Qt6 REQUIRED COMPONENTS Core Sql)

file(GLOB_RECURSE STORAGE_SOURCES "src/*.cpp" "include/*.h")

# 创建库
add_library(storage STATIC
    include/UserTable.h
    include/ContactTable.h include/ConversationTable.h include/DatabaseManager.h include/DatabaseSchema.h include/DataService.h include/GroupMemberTable.h include/GroupTable.h include/MediaCacheTable.h include/MessageTable.h include/UserTable.h
    src/DatabaseManager.cpp src/DatabaseSchema.cpp src/DataService.cpp src/UserTable.cpp)

target_sources(storage PRIVATE ${STORAGE_SOURCES})

# 包含目录
target_include_directories(storage PUBLIC include)

# 设置目标属性
target_compile_features(storage PUBLIC cxx_std_17)

# 链接Qt库
target_link_libraries(storage PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
)



