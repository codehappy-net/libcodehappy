echo "*** Begin native with SDL 1.2 C++ library make (clean)"
rm *.o
rm bin/libcodehappys.a

# these from sdl-config output (TODO: could/should just run the script to get them, but I also should go SDL2, so...)
SDL_LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lSDL -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -lSDL_mixer"
SDL_COMPILE_FLAGS="-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"

echo "*** Build external C libraries (sqlite, pdcurses on Windows, ggml on CPU)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Wno-unused-result -Iinc inc/external/sqlite3.c $SDL_COMPILE_FLAGS -o sqlite3.o

GGML_GPP_ARGS="-fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -Wno-cast-qual -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
GGML_GCC_ARGS="-fPIC -DNDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"


gcc -I. -O3 -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/common.cpp -o common.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/console.cpp -o console.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/build-info.cpp -o build-info.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
gcc -I. -O3 -std=c11 $GGML_GCC_ARGS -c inc/external/ggml/ggml-quants.c -o ggml-quants.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/train.cpp -o train.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/llava.cpp -o llava.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/clip.cpp -o clip.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/json-schema-to-grammar.cpp -o json-schema-to-grammar.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/ngram-cache.cpp -o ngram-cache.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/sgemm.cpp -o sgemm.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/unicode.cpp -o unicode.o
g++ -I. -O3 -std=c++11 $GGML_GPP_ARGS -c inc/external/ggml/unicode-data.cpp -o unicode-data.o

echo "*** Build embedded fonts/patches"
if [ -f "bin/embed_s.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_s.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL -DALL_FONTS $SDL_COMPILE_FLAGS src/embed.cpp -o embed_s.o
cp embed_s.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE_SDL -DALL_FONTS $SDL_COMPILE_FLAGS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappys.a *.o

echo "*** Build some demo apps"
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL $SDL_COMPILE_FLAGS examples/counter.cpp -o counter.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/colrname.cpp $SDL_COMPILE_FLAGS -o colrname.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -c examples/midi.cpp $SDL_COMPILE_FLAGS -o midi.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -c examples/toy.cpp $SDL_COMPILE_FLAGS -o toy.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -c examples/mp3.cpp $SDL_COMPILE_FLAGS -o mp3.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -c examples/paint.cpp $SDL_COMPILE_FLAGS -o paint.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -c examples/clip.cpp $SDL_COMPILE_FLAGS -o clip.o
g++ -O3 -flto -fuse-linker-plugin -m64 -Iinc -DCODEHAPPY_NATIVE_SDL -c examples/tetris.cpp $SDL_COMPILE_FLAGS -o tetris.o
g++ -O3 -flto -fuse-linker-plugin -m64 counter.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o counter
g++ -O3 -flto -fuse-linker-plugin -m64 colrname.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o colrname
g++ -O3 -flto -fuse-linker-plugin -m64 midi.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o midi
g++ -O3 -flto -fuse-linker-plugin -m64 toy.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o toy
g++ -O3 -flto -fuse-linker-plugin -m64 mp3.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o mp3
g++ -O3 -flto -fuse-linker-plugin -m64 paint.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o paint
g++ -O3 -flto -fuse-linker-plugin -m64 clip.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o clip
g++ -O3 -flto -fuse-linker-plugin -m64 tetris.o bin/libcodehappys.a $SDL_LINKER_FLAGS -o tetris

echo "*** Cleanup"
rm *.o
