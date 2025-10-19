file(GLOB_RECURSE NET_SOURCES "src/*.cpp" "include/*.h")
add_library(net ${NET_SOURCES})
target_include_directories(net PUBLIC "include")
# 如果有 protobuf、QtNetwork 等依赖：
# target_link_libraries(net PUBLIC Qt${QT_VERSION_MAJOR}::Network)