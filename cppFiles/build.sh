/Users/tmurphy/git/image/emsdk/upstream/emscripten/emcc -I./externalDependencies/ -I/usr/local/opt/opencv/include/opencv4/ \
./src/main.cc ./src/webasmMain.cpp ./src/img_hash_opencv_module/phash.cpp ./src/img_hash_opencv_module/PHash_Fast.cpp \
./src/shapeNormalise.cpp  -std=c++17 ./lib/libopencv* -o ../webFiles/wasmLibs/cppdemo.js \
 -O3 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]'


