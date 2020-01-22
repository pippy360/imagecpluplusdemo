cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_STANDARD 17)

project (imagecppdemo)

find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory("./externalDependencies/googletest/googletest/")
add_subdirectory("./externalDependencies/hiredis/")

add_executable(
        runtests
        src/shapeNormalise.cpp
#        src/testIntegrationValues.cpp
#        src/canny.cpp
#        src/canny.h
        src/main.cc
        src/img_hash_opencv_module/phash.cpp
        src/img_hash_opencv_module/PHash_Fast.cpp
        src/utils.hpp src/mainImageProcessingFunctions.hpp)

target_include_directories(runtests PUBLIC "." ./externalDependencies googletest/googletest/include/ ${JPEG_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

target_link_libraries(runtests gtest gtest_main hiredis ${JPEG_LIBRARIES} ${OpenCV_LIBS})

add_dependencies(runtests gtest_main hiredis)

#target_compile_options(runtests PUBLIC "-Ofast")
target_compile_options(runtests PUBLIC "-g")

