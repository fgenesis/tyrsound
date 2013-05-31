
set(src
    DeviceBase.cpp
    DeviceBase.h
    ChannelBase.cpp
    ChannelBase.h
    NullDevice.cpp
    NullDevice.h
    NullChannel.cpp
    NullChannel.h
)

set(libs)

if(TYRSOUND_OUTPUT_OPENAL)
    include_directories("${OPENAL_INCLUDE_DIR}")
    list(APPEND src OpenALDevice.cpp OpenALDevice.h OpenALChannel.cpp OpenALChannel.h)
    list(APPEND libs "${OPENAL_LIBRARY}")
endif()

add_library(tyrsound_output ${src})

target_link_libraries(tyrsound_output ${libs})
