
set(LIB_TYPE STATIC)
if(TYRSOUND_BUILD_DYNAMIC)
    set(LIB_TYPE SHARED)
endif()

message(STATUS "Including external dirs: ${INCLUDE_LIST}")
include_directories("${INCLUDE_LIST}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/decoders")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/output")

set(libs)

set(src_core
    core/tyrsound.cpp
    core/tyrsound_streams.cpp
    core/tyrsound_load.cpp
    core/tyrsound_internal.h
    core/tyrsound_device.cpp
    core/tyrsound_sound.cpp
    core/tyrsound_misc.cpp
    core/tyrsound_dyn.cpp
    core/SoundObject.h
    core/SoundObject.cpp
)

set(src_headers
    "${TYRSOUND_SRC_ROOT}/include/tyrsound.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrsound_ex.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrsound_begin.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrsound_end.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrChannelBase.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrDeviceBase.h"
    "${TYRSOUND_SRC_ROOT}/include/tyrDecoderBase.h"
)

set(src_output
    output/DeviceBase.cpp
    output/ChannelBase.cpp
    output/NullDevice.cpp
    output/NullDevice.h
    output/NullChannel.cpp
    output/NullChannel.h
)

set(src_decoders
    decoders/DecoderBase.cpp
    decoders/RawDecoder.cpp
    decoders/RawDecoder.h
)

if(TYRSOUND_SUPPORT_WAV)
    list(APPEND src_decoders
        decoders/WavDecoder.cpp
        decoders/WavDecoder.h
    )
endif()

if(TYRSOUND_SUPPORT_OGG)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libogg/include")
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libvorbis/include")
    list(APPEND src_decoders
        decoders/OggDecoder.cpp
        decoders/OggDecoder.h
    )
    list(APPEND libs ogg vorbis)
endif()

if(TYRSOUND_SUPPORT_MP3)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/mpg123")
    list(APPEND src_decoders
        decoders/Mp3Decoder.cpp
        decoders/Mp3Decoder.h
    )
endif()

if(TYRSOUND_SUPPORT_GME)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/gme/include")
    list(APPEND src_decoders
        decoders/GmeDecoder.cpp
        decoders/GmeDecoder.h
    )
    list(APPEND libs gme)
endif()

if(TYRSOUND_SUPPORT_OPUS)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/opus/include")
    include_directories("${TYRSOUND_SRC_ROOT}/dep/opusfile/include")
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libogg/include")
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libvorbis/include")
    list(APPEND src_decoders
        decoders/OpusDecoder.cpp
        decoders/OpusDecoder.h
    )
    list(APPEND libs opus opusfile)
endif()

if(TYRSOUND_SUPPORT_FLAC)
    add_definitions(-DFLAC__NO_DLL) # FIXME: this is a hack for proper static linking with MSVC. Remove later.
    include_directories("${TYRSOUND_SRC_ROOT}/dep/libFLAC/include")
    list(APPEND src_decoders
        decoders/FlacDecoder.cpp
        decoders/FlacDecoder.h
    )
    list(APPEND libs FLAC)
endif()

if(TYRSOUND_OUTPUT_OPENAL)
    include_directories("${TYRSOUND_SRC_ROOT}/dep/AL/include")
    list(APPEND src_output
        output/OpenALDevice.cpp
        output/OpenALDevice.h
        output/OpenALChannel.cpp
        output/OpenALChannel.h
    )
    list(APPEND libs "${OPENAL_LIBRARY}")
endif()

set(src ${src_headers} ${src_core} ${src_output} ${src_decoders})

source_group("headers" FILES ${src_headers})
source_group("core" FILES ${src_core})
source_group("output" FILES ${src_output})
source_group("decoders" FILES ${src_decoders})


add_library(tyrsound ${LIB_TYPE} ${src})

message(STATUS "Linking libs: ${libs}")

target_link_libraries(tyrsound ${libs})




if(TYRSOUND_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
