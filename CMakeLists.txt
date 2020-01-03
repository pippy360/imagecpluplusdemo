cmake_minimum_required(VERSION 2.6.4)

project (demo)

add_subdirectory("googletest/googletest/")

add_executable(hello hello.cpp)

target_include_directories(hello PUBLIC "." "googletest/googletest/include/")

target_link_libraries(hello gtest gtest_main)

add_dependencies(hello gtest_main)

target_compile_options(hello PUBLIC "-g")

