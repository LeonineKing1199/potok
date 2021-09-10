cmake_minimum_required(VERSION 3.20..3.21)
project(
  Potok 
  VERSION      0.1.0 
  DESCRIPTION "HTTP/2 parser and serializer targetting Boost.Asio"
  HOMEPAGE_URL https://github.com/LeonineKing1199/potok
  LANGUAGES    CXX
)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "Minimum required C++ standard")
if (CMAKE_CXX_STANDARD LESS 17)
  message(FATAL_ERROR "Minimum required C++ standard is currently c++17")
endif()

find_package(Boost 1.76.0 REQUIRED)

add_library(potok)
target_include_directories(potok PUBLIC include)
target_link_libraries(potok PUBLIC Boost::headers)
target_sources(potok PUBLIC src/span.cpp)

include(CTest)
add_subdirectory(tests)
