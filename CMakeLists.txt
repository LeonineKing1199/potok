cmake_minimum_required(VERSION 3.21)
project(
  Potok 
  VERSION      0.1.0 
  DESCRIPTION "HTTP/2 parser and serializer targetting Boost.Asio"
  HOMEPAGE_URL https://github.com/LeonineKing1199/potok
  LANGUAGES    CXX
)

find_package(Boost 1.76.0 REQUIRED)

add_library(potok INTERFACE)
target_include_directories(potok INTERFACE include)
target_link_libraries(potok INTERFACE Boost::headers)

add_subdirectory(tests)
