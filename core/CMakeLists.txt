

include_directories("${TYRSOUND_SRC_ROOT}/decoders")
include_directories("${TYRSOUND_SRC_ROOT}/output")

set(src
    tyrsound.cpp
    tyrsound_begin.h
    tyrsound_end.h
    tyrsound_streams.cpp
    tyrsound_load.cpp
    tyrsound_internal.h
    tyrsound_device.cpp
    tyrsound_sound.cpp
    SoundObject.h
    SoundObject.cpp
    "${TYRSOUND_SRC_ROOT}/include/tyrsound.h"
)


add_library(tyrsound ${src})

target_link_libraries(tyrsound tyrsound_decoders tyrsound_output)
