cmake_minimum_required(VERSION 2.8)
project(tyrsound)

option(TYRSOUND_BUILD_EXAMPLES "Build examples?" TRUE)
option(TYRSOUND_BUILD_DYNAMIC "Build as DLL/Shared object?" TRUE)


option(TYRSOUND_SUPPORT_OGG "Support OGG format?" TRUE)
option(TYRSOUND_SUPPORT_OPUS "Support OPUS format?" TRUE)
option(TYRSOUND_SUPPORT_MP3 "Support MP3 format?" TRUE)
#option(TYRSOUND_SUPPORT_MOD "Support various tracker formats?" TRUE)
option(TYRSOUND_SUPPORT_GME "Support various console formats (using GameMusicEmu)?" TRUE)

option(TYRSOUND_OUTPUT_OPENAL "Use OpenAL for output?" TRUE)


if(TYRSOUND_OUTPUT_OPENAL)
    OPTION(TYRSOUND_INTERNAL_OPENAL "Always use included OpenAL library" FALSE)
    if(NOT TYRSOUND_INTERNAL_OPENAL)
        find_package(OpenAL)
    endif()
    if (NOT OPENAL_FOUND)
        if(WIN32)
            set(OPENALDIR "${CMAKE_CURRENT_SOURCE_DIR}/dep/AL")
            set(OPENAL_INCLUDE_DIR "${OPENALDIR}/include" CACHE PATH "OpenAL include directory" FORCE)
            message(STATUS "Using internal copy of OpenAL")
            set(OPENAL_LIBRARY "${OPENALDIR}/lib/win32/OpenAL32.lib" CACHE FILEPATH "Where the OpenAL library can be found" FORCE)
        else()
            message(SEND_ERROR "We don't have a prebuilt OpenAL for this platform.")
        endif()
    endif()
endif()

# compiler specific things
if(MSVC)
    add_definitions("/GR-") # run-time type info (RTTI) not required
    add_definitions("/GS-") # no buffer security check
    add_definitions("/GF") # string pooling on
    add_definitions("/Oi") # use intrinsics
    add_definitions("/Os") # favor small code
    add_definitions("/DEBUG") # linker should always output debug info
    
    # No exceptions (this is ugly - hackfix compiler flags)
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
                     CMAKE_C_FLAGS   CMAKE_C_FLAGS_DEBUG   CMAKE_C_FLAGS_RELEASE   CMAKE_C_FLAGS_MINSIZEREL   CMAKE_C_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/EHsc")
            string(REGEX REPLACE "/EHsc" "" "${flag_var}" "${${flag_var}}")
            set("${flag_var}" "${${flag_var}}" CACHE STRING "" FORCE)
        endif()
    endforeach()
endif()

# non-msvc needs build type - if no build type was provided, set a default one
if(NOT MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-fPIC") # required
    add_definitions("-fno-rtti")
    add_definitions("-fno-exceptions")
endif()

set(INCLUDE_LIST)

# dependencies first
add_subdirectory(dep)

set(TYRSOUND_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(TYRSOUND_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND INCLUDE_LIST "${TYRSOUND_INCLUDE_DIR}")
include_directories(${TYRSOUND_INCLUDE_DIR})

add_subdirectory(src)
