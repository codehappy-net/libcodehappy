#!/bin/bash
echo "*** Begin native CUDA C++ library make (clean, debug)"
rm *.o
rm bin/libcodehappycudadebug.a

CUDA_INC_DIRS="-I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include"
CUDA_LIBRARIES="-lcublas -lculibos -lcudart -lcublasLt -lpthread -ldl -lrt -L/usr/local/cuda/lib64 -L/opt/cuda/lib64 -L/targets/x86_64-linux/lib"

echo "*** Build external C libraries (sqlite)"
gcc -g -c -Iinc inc/external/sqlite3.c -o sqlite3.o

echo "*** Build ggml library with CUDA support"
gcc -I. -g -std=c11 -fPIC -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -I./examples -g -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -I./examples -g -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/common.cpp -o common.o
g++ -I. -I./examples -g -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/console.cpp -o console.o
g++ -I. -I./examples -g -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -g -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 CUDA_INC_DIRS -DNDEBUG -D_FORTIFY_SOURCE=2 -O2 -isystem /usr/local/anaconda3/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  -c -o k_quants.o inc/external/ggml/k_quants.c
nvcc --forward-unknown-to-host-compiler -use_fast_math -arch=native -DGGML_CUDA_DMMV_X=32 -DGGML_CUDA_MMV_Y=1 -DK_QUANTS_PER_ITERATION=2 -DGGML_CUDA_MMQ_Y=64 -I. -I./examples -g -std=c++11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS CUDA_INC_DIRS -Wno-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/ggml-cuda.cu -o ggml-cuda.o
gcc -I. -g -std=c11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable   -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed_d.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_d.o .
else
g++ -std=c++11 -g -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed_d.o
cp embed_d.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -g -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DCODEHAPPY_CUDA -DALL_FONTS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappycudadebug.a *.o

echo "*** Build tests and examples"
g++ -g -fPIC -pthread -m64 CUDA_INC_DIRS -Iinc -c examples/llamatok.cpp -o llamatok.o
g++ -g -fPIC -pthread -m64 CUDA_INC_DIRS -Iinc -c examples/llamagen.cpp -o llamagen.o
g++ -g -fPIC -pthread -m64 CUDA_INC_DIRS -Iinc -c examples/chat.cpp -o chat.o
g++ -g -fPIC -pthread -m64 CUDA_INC_DIRS -Iinc -c examples/jeopardy-test.cpp -o jeopardy-test.o
g++ -g -fPIC -pthread -m64 CUDA_INC_DIRS -Iinc -c examples/llamaembed.cpp -o llamaembed.o
g++ -g -m64 llamatok.o bin/libcodehappycudadebug.a $CUDA_LIBRARIES -o  llamatok
g++ -g -m64 llamagen.o bin/libcodehappycudadebug.a $CUDA_LIBRARIES -o  llamagen
g++ -g -m64 chat.o bin/libcodehappycudadebug.a $CUDA_LIBRARIES -o  chat
g++ -g -m64 jeopardy-test.o bin/libcodehappycudadebug.a $CUDA_LIBRARIES -o jeopardy-test
g++ -g -m64 llamaembed.o bin/libcodehappycudadebug.a $CUDA_LIBRARIES -o llamaembed

echo "*** Cleanup"
rm *.o
