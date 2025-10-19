file(GLOB_RECURSE MEDIA_SOURCES "src/*.cpp" "include/*.h")
add_library(media STATIC ${MEDIA_SOURCES})
target_include_directories(media PUBLIC "include")

# target_link_libraries(media PUBLIC XXXX)
