#!/bin/bash
echo "*** Begin native CUDA C++ library make (clean)"
rm *.o
rm bin/libcodehappycuda.a

CUDA_INC_DIRS="-I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include"
CUDA_LIBRARIES="-lcuda -lcublas -lculibos -lcudart -lcublasLt -lpthread -ldl -lrt -L/usr/local/cuda/lib64 -L/opt/cuda/lib64 -L/targets/x86_64-linux/lib"
GGML_GPP_ARGS="-fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -Wno-cast-qual -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -DSD_USE_CUBLAS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_GCC_ARGS="-fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -DSD_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_NVCC_ARGS="--forward-unknown-to-host-compiler -use_fast_math -arch=native -DGGML_CUDA_DMMV_X=32 -DGGML_CUDA_MMV_Y=1 -DK_QUANTS_PER_ITERATION=2 -DGGML_CUDA_MMQ_Y=64 -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -Wno-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-cast-qual -diag-suppress 177"

echo "*** Build external C libraries (sqlite)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

echo "*** Build ggml library with CUDA acceleration support"
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS $CUDA_INC_DIRS -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/common.cpp -o common.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/console.cpp -o console.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/build-info.cpp -o build-info.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
nvcc $CUDA_INC_DIRS $GGML_NVCC_ARGS -c inc/external/ggml/ggml-cuda.cu -o ggml-cuda.o
nvcc $CUDA_INC_DIRS $GGML_NVCC_ARGS -c inc/external/ggml/all.cu -o all.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS $CUDA_INC_DIRS -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS $CUDA_INC_DIRS -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS $CUDA_INC_DIRS -c inc/external/ggml/ggml-quants.c -o ggml-quants.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/train.cpp -o train.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/llava.cpp -o llava.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/clip.cpp -o clip.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/json-schema-to-grammar.cpp -o json-schema-to-grammar.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/ngram-cache.cpp -o ngram-cache.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/sgemm.cpp -o sgemm.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/unicode.cpp -o unicode.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS $CUDA_INC_DIRS -c inc/external/ggml/unicode-data.cpp -o unicode-data.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed.o
cp embed.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DCODEHAPPY_CUDA -DALL_FONTS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappycuda.a *.o

echo "*** Build tests and examples"
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamatok.cpp -o llamatok.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamagen.cpp -o llamagen.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamaisn.cpp -o llamaisn.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamasummarize.cpp -o llamasummarize.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/lang.cpp -o lang.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -DCODEHAPPY_CUDA -c examples/chat.cpp -o chat.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamaembed.cpp -o llamaembed.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/thegame.cpp -o thegame.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -DCODEHAPPY_CUDA -c examples/sd.cpp -o sd.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -DCODEHAPPY_CUDA -c examples/bertembed.cpp -o bertembed.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -DCODEHAPPY_CUDA -c examples/llava.cpp -o llava.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/sam-img.cpp -o sam-img.o
g++ -O3 -flto -m64 llamatok.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamatok
g++ -O3 -flto -m64 llamagen.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamagen
g++ -O3 -flto -m64 llamaisn.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamaisn
g++ -O3 -flto -m64 llamasummarize.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamasummarize
g++ -O3 -flto -m64 lang.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o lang
g++ -O3 -flto -m64 chat.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o chat
g++ -O3 -flto -m64 llamaembed.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o llamaembed
g++ -O3 -flto -m64 sd.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o sd-cuda
g++ -O3 -flto -m64 thegame.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o thegame
g++ -O3 -flto -m64 bertembed.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o bertembed
g++ -O3 -flto -m64 llava.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o llava

echo "*** Cleanup"
rm *.o
