
add_definitions(-DLIBOPENMPT_BUILD)

if(CMAKE_COMPILER_IS_GNUCXX OR CLANG)
    add_definitions("-fexceptions")
    add_definitions("-std=c++11")
elseif(MSVC)
    add_definitions("/EHsc")
endif()

include_directories("${MINIZ_INCLUDE_DIR}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common/svn_version_default")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(libopenmpt_SRC
common/AudioCriticalSection.cpp
common/Logging.cpp
common/Profiler.cpp
common/misc_util.cpp
common/mptPathString.cpp
common/mptString.cpp
common/serialization_utils.cpp
common/stdafx.cpp
common/typedefs.cpp
common/version.cpp
soundlib/Dither.cpp
soundlib/Dlsbank.cpp
soundlib/Fastmix.cpp
soundlib/ITCompression.cpp
soundlib/ITTools.cpp
soundlib/Load_669.cpp
soundlib/Load_amf.cpp
soundlib/Load_ams.cpp
soundlib/Load_dbm.cpp
soundlib/Load_digi.cpp
soundlib/Load_dmf.cpp
soundlib/Load_dsm.cpp
soundlib/Load_far.cpp
soundlib/Load_gdm.cpp
soundlib/Load_imf.cpp
soundlib/Load_it.cpp
soundlib/Load_itp.cpp
soundlib/Load_mdl.cpp
soundlib/Load_med.cpp
soundlib/Load_mid.cpp
soundlib/Load_mo3.cpp
soundlib/Load_mod.cpp
soundlib/Load_mt2.cpp
soundlib/Load_mtm.cpp
soundlib/Load_okt.cpp
soundlib/Load_psm.cpp
soundlib/Load_ptm.cpp
soundlib/Load_s3m.cpp
soundlib/Load_stm.cpp
soundlib/Load_ult.cpp
soundlib/Load_umx.cpp
soundlib/Load_wav.cpp
soundlib/Load_xm.cpp
soundlib/MIDIEvents.cpp
soundlib/MIDIMacros.cpp
soundlib/Message.cpp
soundlib/MixerLoops.cpp
soundlib/MixerSettings.cpp
soundlib/Mmcmp.cpp
soundlib/ModChannel.cpp
soundlib/ModInstrument.cpp
soundlib/ModSample.cpp
soundlib/ModSequence.cpp
soundlib/RowVisitor.cpp
soundlib/S3MTools.cpp
soundlib/SampleFormats.cpp
soundlib/SampleIO.cpp
soundlib/Snd_flt.cpp
soundlib/Snd_fx.cpp
soundlib/Sndfile.cpp
soundlib/Sndmix.cpp
soundlib/SoundFilePlayConfig.cpp
soundlib/Tables.cpp
soundlib/Tagging.cpp
soundlib/WAVTools.cpp
soundlib/WindowedFIR.cpp
soundlib/XMTools.cpp
soundlib/load_j2b.cpp
soundlib/mod_specifications.cpp
soundlib/modcommand.cpp
soundlib/modsmp_ctrl.cpp
soundlib/pattern.cpp
soundlib/patternContainer.cpp
soundlib/tuning.cpp
soundlib/tuningCollection.cpp
soundlib/tuningbase.cpp
libopenmpt/libopenmpt_c.cpp
libopenmpt/libopenmpt_cxx.cpp
libopenmpt/libopenmpt_ext.cpp
libopenmpt/libopenmpt_impl.cpp
)


add_library(openmpt ${libopenmpt_SRC})
target_link_libraries(openmpt miniz)

