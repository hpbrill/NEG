cmake_minimum_required(VERSION 3.16)
project(NEG LANGUAGES CXX)

add_library(neg_core src/game.cpp)
target_include_directories(neg_core PUBLIC include)
target_compile_features(neg_core PUBLIC cxx_std_17)
set_target_properties(neg_core PROPERTIES CXX_EXTENSIONS OFF)
add_executable(NegInit src/main.cpp)
target_link_libraries(NegInit PRIVATE neg_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(neg_tests tests/game_tests.cpp)
    target_link_libraries(neg_tests PRIVATE neg_core)
    add_test(NAME neg_core_tests COMMAND neg_tests)
    add_test(NAME neg_cli_tests COMMAND ${CMAKE_COMMAND}
        -DNEG_PROGRAM=$<TARGET_FILE:NegInit>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_tests.cmake)
endif()
