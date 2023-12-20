echo "*** Begin native build with SDL 1.2 C++ library make (DEBUG BUILD, clean)"
rm *.o
rm bin/libcodehappysd.a

# these from sdl-config output (TODO: could/should just run the script to get them, but I also should go SDL2, so...)
SDL_LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lSDL -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -lSDL_mixer"
SDL_COMPILE_FLAGS="-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"

echo "*** Build external C libraries (sqlite, pdcurses on Windows, ggml on CPU)"
gcc -g -m64 -c -Wno-unused-result -Iinc inc/external/sqlite3.c $SDL_COMPILE_FLAGS -o sqlite3.o

GGML_GPP_ARGS="-fPIC -DCODEHAPPY_DEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -march=nocona -mtune=haswell -ftree-vectorize -fstack-protector-strong -fno-plt -ffunction-sections -DGGML_USE_K_QUANTS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_GCC_ARGS="-fPIC -DCODEHAPPY_DEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -march=nocona -mtune=haswell -ftree-vectorize -fstack-protector-strong -fno-plt -ffunction-sections -DGGML_USE_K_QUANTS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"

gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/common.cpp -o common.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/console.cpp -o console.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/build-info.cpp -o build-info.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
gcc -I. -g -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-quants.c -o ggml-quants.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -g -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/train.cpp -o train.o

echo "*** Build embedded fonts/patches"
if [ -f "bin/embed_sd.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_sd.o .
else
g++ -std=c++11 -g -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL -DALL_FONTS $SDL_COMPILE_FLAGS src/embed.cpp -o embed_sd.o
cp embed_sd.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -g -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE_SDL -DCODEHAPPY_DEBUG -DALL_FONTS $SDL_COMPILE_FLAGS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappysd.a *.o

echo "*** Cleanup"
rm *.o
