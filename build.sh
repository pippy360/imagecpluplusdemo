emcc -I./externalDependencies/ -I/usr/local/opt/opencv/include/opencv4/ ./src/main.cc ./src/img_hash_opencv_module/phash.cpp ./src/img_hash_opencv_module/PHash_Fast.cpp ./src/shapeNormalise.cpp  -std=c++17 ./lib/libopencv* -o index.html
