cmake_minimum_required(VERSION 3.20)

project(kestrel
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(kestrel_lib
    src/board.cpp
    src/move.cpp
    src/movegen.cpp
    src/perft.cpp
    src/search.cpp
)

target_include_directories(kestrel_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(kestrel_lib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_executable(kestrel src/main.cpp)
target_link_libraries(kestrel PRIVATE kestrel_lib)

enable_testing()

add_executable(kestrel_tests
    tests/perft_tests.cpp
)

target_link_libraries(kestrel_tests PRIVATE kestrel_lib)

add_test(NAME kestrel_perft COMMAND kestrel_tests)
