find_package(fmt 8.0.0 CONFIG REQUIRED)

add_library(Catch2WithMain catch_amalgamated.cpp)

set(POTOK_MEMCHECK_COMMAND "valgrind")
set(POTOK_MEMCHECK_ARGS "--leak-check=full --error-exitcode=1")

function(potok_add_test filename)
  set(memcheck_command "${POTOK_MEMCHECK_COMMAND} ${POTOK_MEMCHECK_ARGS}")
  separate_arguments(memcheck_command)

  cmake_path(SET testpath "${filename}")
  cmake_path(GET testpath STEM stem)
  add_executable("${stem}" "${filename}")
  target_link_libraries("${stem}" PRIVATE potok Catch2WithMain fmt::fmt)

  add_test(NAME "${stem}" COMMAND ${memcheck_command} ./${stem})
endfunction()

potok_add_test(hpack_common.cpp)
potok_add_test(hpack_encode_integer.cpp)
potok_add_test(hpack_decode_integer.cpp)
potok_add_test(huffman_decode.cpp)
