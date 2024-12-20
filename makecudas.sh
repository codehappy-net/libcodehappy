#!/bin/bash
echo "*** Begin native CUDA with SDL 1.2 C++ library make (clean)"
rm *.o
rm bin/libcodehappycudas.a

# these from sdl-config output (TODO: could/should just run the script to get them, but I also should go SDL2, so...)
SDL_LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lSDL -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -lSDL_mixer"
SDL_COMPILE_FLAGS="-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"

CUDA_INC_DIRS="-I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include"
CUDA_LIBRARIES="-lcuda -lcublas -lculibos -lcudart -lcublasLt -lpthread -ldl -lrt -L/usr/local/cuda/lib64 -L/opt/cuda/lib64 -L/targets/x86_64-linux/lib"

GGML_GPP_ARGS="-fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -Wno-cast-qual -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -DSD_USE_CUBLAS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_GCC_ARGS="-fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -DSD_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_NVCC_ARGS="--forward-unknown-to-host-compiler -use_fast_math -arch=native -DGGML_CUDA_DMMV_X=32 -DGGML_CUDA_MMV_Y=1 -DK_QUANTS_PER_ITERATION=2 -DGGML_CUDA_MMQ_Y=64 -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUDA -DGGML_CUDA_USE_GRAPHS -Wno-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-cast-qual -diag-suppress 177"

echo "*** Build external C libraries (sqlite)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c $SDL_COMPILE_FLAGS -o sqlite3.o

echo "*** Build ggml library with CUDA support"
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
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL -DCODEHAPPY_CUDA -DALL_FONTS $SDL_COMPILE_FLAGS src/embed.cpp -o embed.o
cp embed.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE_SDL -DCODEHAPPY_CUDA -DALL_FONTS $SDL_COMPILE_FLAGS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappycudas.a *.o

echo "*** Build tests and examples"
g++ -O3 -flto -c -Iinc $SDL_COMPILE_FLAGS $CUDA_INC_DIRS examples/interpolator.cpp -o interpolator.o
g++ -O3 -flto interpolator.o bin/libcodehappycudas.a $CUDA_LIBRARIES $SDL_LINKER_FLAGS -o interpolator

echo "*** Cleanup"
rm *.o
