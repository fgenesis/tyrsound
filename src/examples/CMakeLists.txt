
include_directories("${TYRSOUND_INCLUDE_DIR}")

add_executable(tyrplay tyrplay.cpp)
target_link_libraries(tyrplay tyrsound)

add_executable(tyrpipe tyrpipe.c)
target_link_libraries(tyrpipe tyrsound)

add_executable(tyrwav tyrwav.cpp)
target_link_libraries(tyrwav tyrsound)

add_executable(bytebeat bytebeat.c)
target_link_libraries(bytebeat tyrsound)

add_executable(bytebeat2 bytebeat2.c)
target_link_libraries(bytebeat2 tyrsound)

add_executable(test test.cpp)
target_link_libraries(test tyrsound)
