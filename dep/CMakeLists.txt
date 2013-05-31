
if(TYRSOUND_SUPPORT_OGG)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libogg/include")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/include")
    add_subdirectory(libogg)
    add_subdirectory(libvorbis)
endif()

if(TYRSOUND_OUTPUT_OPENAL)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/AL/include")
endif()
