
include_directories("${VORBIS_INCLUDE_DIR}")
include_directories("${OGG_INCLUDE_DIR}")

add_library(ogg
    src/bitwise.c
    src/framing.c
)
