
include_directories("${FLAC_INCLUDE_DIR}")
include_directories("${OGG_INCLUDE_DIR}")

## FIXME: what?
if(MSVC)
    add_definitions(-D_LIB)
endif()


# private includes
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")
add_definitions(-DFLAC__NO_ASM)
#add_definitions(-DFLAC_API_EXPORTS)
add_definitions(-DFLAC__NO_DLL)
add_definitions(-DFLAC__HAS_OGG)
add_definitions(-DVERSION="1.3.0")

#add_definitions(-DFLAC__CPU_IA32)
#add_definitions(-DFLAC__HAS_NASM)
#add_definitions(-DFLAC__HAS_X86INTRIN)
#add_definitions(-DFLAC__USE_3DNOW)
#add_definitions(-DFLAC__ALIGN_MALLOC_DATA)
add_definitions(-DFLaC__INLINE=inline)

set(FLAC_src
    src/bitmath.c
    src/bitreader.c
    src/bitwriter.c
    src/cpu.c
    src/crc.c
    src/fixed.c
    src/float.c
    src/format.c
    src/lpc.c
    src/lpc_intrin_sse.c
    src/lpc_intrin_sse2.c
    src/lpc_intrin_sse41.c
    src/md5.c
    src/memory.c
    src/metadata_iterators.c
    src/metadata_object.c
    src/ogg_decoder_aspect.c
    src/ogg_encoder_aspect.c
    src/ogg_helper.c
    src/ogg_mapping.c
    src/stream_decoder.c
    src/stream_encoder.c
    src/stream_encoder_framing.c
    src/stream_encoder_intrin_sse2.c
    src/stream_encoder_intrin_ssse3.c
    src/window.c
)

if(WIN32)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/share")
list(APPEND FLAC_src
    share/utf8/charmaps.h
    share/utf8/charset.c
    share/utf8/charset.h
    share/utf8/charsetmap.h
    share/utf8/iconvert.c
    share/utf8/iconvert.h
    share/utf8/utf8.c
    share/win_utf8_io/win_utf8_io.c
)
endif()

add_library(FLAC ${FLAC_src})
