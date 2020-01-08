cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_STANDARD 17)

project (imagecppdemo)

add_subdirectory("googletest/googletest/")

add_executable(runtests shapeNormalise.cpp testIntegrationValues.cpp)

target_include_directories(runtests PUBLIC "." "googletest/googletest/include/")

target_link_libraries(runtests gtest gtest_main)

add_dependencies(runtests gtest_main)

target_compile_options(runtests PUBLIC "-g")

