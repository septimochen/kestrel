cmake_minimum_required(VERSION 3.30)

project(kestrel
    VERSION 0.1.0
    LANGUAGES CXX
)

set(KESTREL_CXX_STANDARD "23" CACHE STRING "C++ language standard: 23 or experimental 26")
set_property(CACHE KESTREL_CXX_STANDARD PROPERTY STRINGS 23 26)
if(NOT KESTREL_CXX_STANDARD MATCHES "^(23|26)$")
    message(FATAL_ERROR "KESTREL_CXX_STANDARD must be 23 or 26")
endif()
set(CMAKE_CXX_STANDARD ${KESTREL_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(kestrel_lib
    src/board.cpp
    src/move.cpp
    src/movegen.cpp
    src/perft.cpp
    src/search.cpp
)

target_compile_features(kestrel_lib PUBLIC cxx_std_${KESTREL_CXX_STANDARD})

target_include_directories(kestrel_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(kestrel_lib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

add_executable(kestrel src/main.cpp)
target_link_libraries(kestrel PRIVATE kestrel_lib)

enable_testing()

add_executable(kestrel_tests
    tests/perft_tests.cpp
)

target_link_libraries(kestrel_tests PRIVATE kestrel_lib)

add_test(NAME kestrel_perft COMMAND kestrel_tests)

option(KESTREL_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(KESTREL_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Sanitizers require Clang or GCC")
    endif()
    target_compile_options(kestrel_lib PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(kestrel_lib PUBLIC -fsanitize=address,undefined)
endif()

add_executable(kestrel_board_tests tests/board_tests.cpp)
target_link_libraries(kestrel_board_tests PRIVATE kestrel_lib)
add_test(NAME kestrel_board COMMAND kestrel_board_tests)
add_test(NAME kestrel_cli_invalid COMMAND kestrel perft -1)
set_tests_properties(kestrel_cli_invalid PROPERTIES WILL_FAIL TRUE)

# Keep experiments separate: a compiler may support the core but lack a feature here.
option(KESTREL_CPP_EXPERIMENTS "Build modern C++ feature experiments" OFF)
if(KESTREL_CPP_EXPERIMENTS)
    add_executable(kestrel_cpp_features examples/modern_cpp.cpp)
    target_link_libraries(kestrel_cpp_features PRIVATE kestrel_lib)
    add_test(NAME kestrel_cpp_features COMMAND kestrel_cpp_features)
endif()

foreach(case IN ITEMS malformed trailing overflow)
    if(case STREQUAL "malformed")
        set(depth "abc")
    elseif(case STREQUAL "trailing")
        set(depth "3junk")
    else()
        set(depth "999999999999999999999999")
    endif()
    add_test(NAME kestrel_cli_${case} COMMAND kestrel perft ${depth})
    set_tests_properties(kestrel_cli_${case} PROPERTIES WILL_FAIL TRUE)
endforeach()
add_test(NAME kestrel_cli_command COMMAND kestrel unknown)
add_test(NAME kestrel_cli_arity COMMAND kestrel perft 1 extra)
set_tests_properties(kestrel_cli_command kestrel_cli_arity PROPERTIES WILL_FAIL TRUE)
