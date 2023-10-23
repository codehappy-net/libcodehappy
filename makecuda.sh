#!/bin/bash
echo "*** Begin native CUDA C++ library make (clean)"
rm *.o
rm bin/libcodehappycuda.a

CUDA_INC_DIRS="-I/usr/local/cuda/include -I/opt/cuda/include -I/targets/x86_64-linux/include"
CUDA_LIBRARIES="-lcublas -lculibos -lcudart -lcublasLt -lpthread -ldl -lrt -L/usr/local/cuda/lib64 -L/opt/cuda/lib64 -L/targets/x86_64-linux/lib"

echo "*** Build external C libraries (sqlite)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

echo "*** Build ggml library with CUDA support"
gcc -I. -O3 -std=c11 -fPIC -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/common.cpp -o common.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/console.cpp -o console.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -O3 -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 $CUDA_INC_DIRS -DNDEBUG -D_FORTIFY_SOURCE=2 -O2 -isystem /usr/local/anaconda3/include -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  -c -o k_quants.o inc/external/ggml/k_quants.c
nvcc --forward-unknown-to-host-compiler -use_fast_math -arch=native -DGGML_CUDA_DMMV_X=32 -DGGML_CUDA_MMV_Y=1 -DK_QUANTS_PER_ITERATION=2 -DGGML_CUDA_MMQ_Y=64 -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/ggml-cuda.cu -o ggml-cuda.o
gcc -I. -O3 -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable   -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -O3 -std=c11   -fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable   -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -I./examples -O3 -std=c++11 -fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CUBLAS $CUDA_INC_DIRS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -c inc/external/ggml/train.cpp -o train.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed.o
cp embed.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DCODEHAPPY_CUDA -DALL_FONTS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappycuda.a *.o

echo "*** Build tests and examples"
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamatok.cpp -o llamatok.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamagen.cpp -o llamagen.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamaisn.cpp -o llamaisn.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamasummarize.cpp -o llamasummarize.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/lang.cpp -o lang.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/chat.cpp -o chat.o
g++ -O3 -flto -fPIC -pthread -m64 $CUDA_INC_DIRS -Iinc -c examples/llamaembed.cpp -o llamaembed.o
g++ -O3 -flto -m64 llamatok.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamatok
g++ -O3 -flto -m64 llamagen.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamagen
g++ -O3 -flto -m64 llamaisn.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamaisn
g++ -O3 -flto -m64 llamasummarize.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o  llamasummarize
g++ -O3 -flto -m64 lang.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o lang
g++ -O3 -flto -m64 chat.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o chat
g++ -O3 -flto -m64 llamaembed.o bin/libcodehappycuda.a $CUDA_LIBRARIES -o llamaembed

echo "*** Cleanup"
rm *.o
