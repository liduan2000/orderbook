cmake_minimum_required(VERSION 3.14)
project(orderbook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(orderbook_lib ${SOURCES})

enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp)

add_executable(orderbook_test ${TEST_SOURCES})

target_link_libraries(orderbook_test PRIVATE orderbook_lib)

add_test(NAME OrderBookTests COMMAND orderbook_test)
