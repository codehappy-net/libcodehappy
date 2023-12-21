#!/bin/bash
echo "*** Begin native C++ library make (DEBUG BUILD, clean)"
rm *.o
rm bin/libcodehappyd.a

echo "*** Build external C libraries (sqlite, pdcurses on Windows, ggml on CPU)"
gcc -g -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

GGML_GPP_ARGS="-fPIC -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -DCODEHAPPY_DEBUG -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -march=nocona -mtune=haswell -ftree-vectorize -fstack-protector-strong -fno-plt -ffunction-sections -DGGML_USE_K_QUANTS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_GCC_ARGS="-fPIC -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -march=nocona -mtune=haswell -ftree-vectorize -fstack-protector-strong -fno-plt -ffunction-sections -DGGML_USE_K_QUANTS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -DCODEHAPPY_DEBUG -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"

gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/common.cpp -o common.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/console.cpp -o console.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/build-info.cpp -o build-info.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c -o k_quants.o inc/external/ggml/k_quants.c
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-quants.c -o ggml-quants.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/train.cpp -o train.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/llava.cpp -o llava.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/clip.cpp -o clip.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed_d.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_d.o .
else
g++ -std=c++11 -g -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed_d.o
cp embed_d.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -g -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DCODEHAPPY_DEBUG -DALL_FONTS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappyd.a *.o

echo "*** Cleanup"
rm *.o
