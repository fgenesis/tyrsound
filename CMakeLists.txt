cmake_minimum_required(VERSION 2.8)
project(tyrsound)

option(TYRSOUND_BUILD_EXAMPLES "Build examples?" TRUE)
option(TYRSOUND_BUILD_DYNAMIC "Build as DLL/Shared object?" TRUE)


option(TYRSOUND_SUPPORT_OGG "Support OGG format?" TRUE)
#option(TYRSOUND_SUPPORT_OPUS "Support OPUS format?" TRUE)
#option(TYRSOUND_SUPPORT_MP3 "Support MP3 format?" TRUE)
#option(TYRSOUND_SUPPORT_MOD "Support various tracker formats?", TRUE)
#option(TYRSOUND_SUPPORT_GME "Support various console formats (NES, SNES, Sega Genesis, ... using GameMusicEmu)?", TRUE)

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
    # MSVC builds require installed runtime library by default
    option(TYRSOUND_NO_RT "Link without runtime dependencies (Note: To get rid of this setting with MSVC, the cmake cache must be cleared)" FALSE)
    add_definitions("/GR-") # run-time type info (RTTI) not required
    
    if(TYRSOUND_NO_RT)
        # this is ugly - hackfix compiler flags
        foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
            if(${flag_var} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MDd")
        endforeach()
        
        # hackfix linker flags - no idea why, but MSVC will produce linker errors otherwise
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib,msvcrtd.lib") # not sure if this is correct
        set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib,msvcrtd.lib")
    endif()
endif()

# non-msvc needs build type - if no build type was provided, set a default one
if(NOT MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    endif()
endif()

set(INCLUDE_LIST)

# dependencies first
add_subdirectory(dep)


set(TYRSOUND_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND INCLUDE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)
