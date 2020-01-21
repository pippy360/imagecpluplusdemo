cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_STANDARD 17)

project (imagecppdemo)

find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory("googletest/googletest/")

add_executable(runtests shapeNormalise.cpp testIntegrationValues.cpp canny.cpp canny.h)

target_include_directories(runtests PUBLIC "." "googletest/googletest/include/" ${JPEG_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

target_link_libraries(runtests gtest gtest_main ${JPEG_LIBRARIES} ${OpenCV_LIBS})

add_dependencies(runtests gtest_main)

#target_compile_options(runtests PUBLIC "-Ofast")
target_compile_options(runtests PUBLIC "-g")

