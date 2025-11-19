# 提示：目前仅用于快速开发项目，懒得编译ffmpeg库了，之间简单使用，以后改。

find_path(FFMPEG_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    PATHS "C:/Users/GodPrograms/Downloads/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/include"
)

find_library(AVCODEC_LIBRARY
    NAMES avcodec
    PATHS "C:/Users/GodPrograms/Downloads/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/lib"
)

find_library(AVFORMAT_LIBRARY
    NAMES avformat
    PATHS "C:/Users/GodPrograms/Downloads/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/lib"
)

find_library(AVUTIL_LIBRARY
    NAMES avutil
    PATHS "C:/Users/GodPrograms/Downloads/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/lib"
)

find_library(SWSCALE_LIBRARY
    NAMES swscale
    PATHS "C:/Users/GodPrograms/Downloads/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/lib"
)

set(FFMPEG_LIBRARIES
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
)

set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg DEFAULT_MSG
    FFMPEG_INCLUDE_DIR
    AVCODEC_LIBRARY
    AVFORMAT_LIBRARY
    AVUTIL_LIBRARY
)

mark_as_advanced(
    FFMPEG_INCLUDE_DIR
    AVCODEC_LIBRARY
    AVFORMAT_LIBRARY
    AVUTIL_LIBRARY
    SWSCALE_LIBRARY
)