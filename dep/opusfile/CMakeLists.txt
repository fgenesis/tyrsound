
include_directories("${OPUS_INCLUDE_DIR}")
include_directories("${OPUSFILE_INCLUDE_DIR}")
include_directories("${VORBIS_INCLUDE_DIR}")
include_directories("${OGG_INCLUDE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(opusfile
    src/info.c
    src/internal.c
    src/internal.h
    src/opusfile.c
    src/stream.c
    src/winerrno.h
    include/opusfile.h
)
