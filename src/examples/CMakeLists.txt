
include_directories("${TYRSOUND_INCLUDE_DIR}")

add_executable(tyrplay tyrplay.cpp)
target_link_libraries(tyrplay tyrsound)

add_executable(tyrwav tyrwav.cpp)
target_link_libraries(tyrwav tyrsound)

add_executable(bytebeat bytebeat.cpp)
target_link_libraries(bytebeat tyrsound)

add_executable(test test.cpp)
target_link_libraries(test tyrsound)
