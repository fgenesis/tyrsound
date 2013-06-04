set(INCLUDE_LIST ${INCLUDE_LIST})

if(TYRSOUND_SUPPORT_OGG)
    set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libogg/include")
    set(VORBIS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/include")
    add_subdirectory(libvorbis)
    add_subdirectory(libogg)
    list(APPEND INCLUDE_LIST "${OGG_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}")
endif()


if(TYRSOUND_SUPPORT_OPUS)
    set(OPUS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opus/include")
    set(OPUSFILE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opusfile/include")
    set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libogg/include")
    set(VORBIS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/include")
    add_subdirectory(opus)
    add_subdirectory(opusfile)
    list(APPEND INCLUDE_LIST "${OPUS_INCLUDE_DIR}" "${OPUSFILE_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}")
endif()

if(TYRSOUND_SUPPORT_GME)
    set(GME_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gme/include")
    add_subdirectory(gme)
    list(APPEND INCLUDE_LIST "${GME_INCLUDE_DIR}")
endif()

if(TYRSOUND_OUTPUT_OPENAL)
    set(OPENAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AL/include")
    list(APPEND INCLUDE_LIST "${OPENAL_INCLUDE_DIR}")
endif()

set(INCLUDE_LIST "${INCLUDE_LIST}" PARENT_SCOPE)
