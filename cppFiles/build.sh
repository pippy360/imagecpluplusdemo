#emcc -I./externalDependencies/ -I/usr/local/opt/opencv/include/opencv4/ \
# ./src/webasmMain.cpp ./src/img_hash_opencv_module/phash.cpp ./src/img_hash_opencv_module/PHash_Fast.cpp \
#./src/shapeNormalise.cpp  -std=c++17 ./lib/libopencv* -o ../webFiles/wasmLibs/cppdemo.js \
# -O3 -s WASM=1  -s DISABLE_EXCEPTION_CATCHING=2   -s TOTAL_MEMORY=149880832  -s ASSERTIONS=1  -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' --bind   
make -f Makefile.wasm 

