#!/bin/bash
echo "*** Begin native CUDA with SDL 1.2 C++ library make (clean)"
rm *.o
rm bin/libcodehappycudas.a

# these from sdl-config output (TODO: could/should just run the script to get them, but I also should go SDL2, so...)
SDL_LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lSDL -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -lSDL_mixer"
SDL_COMPILE_FLAGS="-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"

echo "*** Build external C libraries (sqlite)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c $SDL_COMPILE_FLAGS -o sqlite3.o

echo "*** Build ggml library with CUDA support"
gcc -I. -O3 -std=c11 -fPIC -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/common.cpp -o common.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/console.cpp -o console.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -O3 -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -DNDEBUG -D_FORTIFY_SOURCE=2 -O2 -isystem /usr/local/anaconda3/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  $SDL_COMPILE_FLAGS -c -o k_quants.o inc/external/ggml/k_quants.c
nvcc --forward-unknown-to-host-compiler -use_fast_math -arch=native -DGGML_CUDA_DMMV_X=32 -DGGML_CUDA_MMV_Y=1 -DK_QUANTS_PER_ITERATION=2 -DGGML_CUDA_MMQ_Y=64 -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable $SDL_COMPILE_FLAGS -c inc/external/ggml/ggml-cuda.cu -o ggml-cuda.o
gcc -I. -O3 -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  $SDL_COMPILE_FLAGS  -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o


echo "*** Build embedded fonts/patches."
if [ -f "bin/embed.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL -DCODEHAPPY_CUDA -DALL_FONTS $SDL_COMPILE_FLAGS src/embed.cpp -o embed.o
cp embed.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE_SDL -DCODEHAPPY_CUDA -DALL_FONTS $SDL_COMPILE_FLAGS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappycudas.a *.o

echo "*** Build tests and examples"

echo "*** Cleanup"
rm *.o
