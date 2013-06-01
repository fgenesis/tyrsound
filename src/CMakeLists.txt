
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
    core/tyrsound_begin.h
    core/tyrsound_end.h
    core/tyrsound_streams.cpp
    core/tyrsound_load.cpp
    core/tyrsound_internal.h
    core/tyrsound_device.cpp
    core/tyrsound_sound.cpp
    core/tyrsound_misc.cpp
    core/SoundObject.h
    core/SoundObject.cpp
    "${TYRSOUND_SRC_ROOT}/include/tyrsound.h"
)

set(src_output
    output/DeviceBase.cpp
    output/DeviceBase.h
    output/ChannelBase.cpp
    output/ChannelBase.h
    output/NullDevice.cpp
    output/NullDevice.h
    output/NullChannel.cpp
    output/NullChannel.h
)

set(src_decoders
    decoders/DecoderBase.cpp
    decoders/DecoderBase.h
    decoders/WavDecoder.cpp
    decoders/WavDecoder.h
)

if(TYRSOUND_SUPPORT_OGG)
    list(APPEND src_decoders
        decoders/OggDecoder.cpp
        decoders/OggDecoder.h
    )
    list(APPEND libs ogg vorbis)
endif()

if(TYRSOUND_OUTPUT_OPENAL)
    list(APPEND src_output
        output/OpenALDevice.cpp
        output/OpenALDevice.h
        output/OpenALChannel.cpp
        output/OpenALChannel.h
    )
    list(APPEND libs "${OPENAL_LIBRARY}")
endif()

set(src ${src_core} ${src_output} ${src_decoders})

source_group("core" FILES ${src_core})
source_group("output" FILES ${src_output})
source_group("decoders" FILES ${src_decoders})


add_library(tyrsound ${LIB_TYPE} ${src})

target_link_libraries(tyrsound ${libs})




if(TYRSOUND_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
