
set(src
    DecoderBase.cpp
    DecoderBase.h
    WavDecoder.cpp
    WavDecoder.h
)
set(dep)

if(TYRSOUND_SUPPORT_OGG)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libogg/include")
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libvorbis/include")
    list(APPEND src OggDecoder.cpp OggDecoder.h)
    list(APPEND dep ogg vorbis)
endif()

# TODO: the others



add_library(tyrsound_decoders ${src})

target_link_libraries(tyrsound_decoders ${dep})
