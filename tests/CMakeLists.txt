find_package(fmt 8.0.0 CONFIG REQUIRED)

add_library(Catch2WithMain catch_amalgamated.cpp)

function(potok_add_test filename)
  cmake_path(SET testpath "${filename}")
  cmake_path(GET testpath STEM stem)
  add_executable("${stem}" "${filename}")
  target_link_libraries("${stem}" PRIVATE potok Catch2WithMain fmt::fmt)
  add_test(NAME "${stem}" COMMAND "${stem}")
endfunction()

potok_add_test(hpack_integer_representation.cpp)
