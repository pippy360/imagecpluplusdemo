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
        src/utils.hpp src/mainImageProcessingFunctions.hpp
#        src/webasmMain.cpp
)

target_include_directories(runtests PUBLIC "." ./externalDependencies googletest/googletest/include/ ${JPEG_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

#link_directories(/usr/local/opt/libomp/lib/)

target_link_libraries(runtests gtest gtest_main hiredis ${JPEG_LIBRARIES} ${OpenCV_LIBS})

add_dependencies(runtests gtest_main hiredis)

#Openmp settings
#set( CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++" )
#set( CMAKE_CXX_FLAGS "-fopenmp -L/usr/local/Cellar/libomp/9.0.0/lib/" )

#webasm settings
#set( CMAKE_CXX_COMPILER "/Users/tmurphy/git/image/emsdk/upstream/emscripten/emcc" )
#set( CMAKE_CXX_FLAGS "-O3 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"cwrap\"]'  -o hello.html" )

#target_compile_options(runtests PUBLIC "-Ofast")
target_compile_options(runtests PUBLIC "-g")

