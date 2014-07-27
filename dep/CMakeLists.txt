set(INCLUDE_LIST ${INCLUDE_LIST})

set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libogg/include")
set(VORBIS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/include")
set(OPUS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opus/include")
set(OPUSFILE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opusfile/include")
set(FLAC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libFLAC/include")
set(GME_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gme/include")
# MP3 is dynamically loaded and not required here
set(MINIZ_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/miniz")
set(OPENMPT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt")

set(OPENAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AL/include")


if(TYRSOUND_SUPPORT_OGG)
    add_subdirectory(libvorbis)
    add_subdirectory(libogg)
    list(APPEND INCLUDE_LIST "${OGG_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}")
endif()

if(TYRSOUND_SUPPORT_FLAC)
    add_subdirectory(libFLAC)
    list(APPEND INCLUDE_LIST "${FLAC_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}")
endif()

if(TYRSOUND_SUPPORT_OPUS)
    add_subdirectory(opus)
    add_subdirectory(opusfile)
    list(APPEND INCLUDE_LIST "${OPUS_INCLUDE_DIR}" "${OPUSFILE_INCLUDE_DIR}" "${OGG_INCLUDE_DIR}" "${VORBIS_INCLUDE_DIR}")
endif()

if(TYRSOUND_SUPPORT_GME)
    add_subdirectory(gme)
    list(APPEND INCLUDE_LIST "${GME_INCLUDE_DIR}")
endif()

if(TYRSOUND_SUPPORT_MOD)
    add_subdirectory(miniz)
    add_subdirectory(libopenmpt)
    list(APPEND INCLUDE_LIST "${MINIZ_INCLUDE_DIR}" "${OPENMPT_INCLUDE_DIR}")
endif()

if(TYRSOUND_OUTPUT_OPENAL)
    list(APPEND INCLUDE_LIST "${OPENAL_INCLUDE_DIR}")
endif()


set(INCLUDE_LIST "${INCLUDE_LIST}" PARENT_SCOPE)
