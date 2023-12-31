#!/bin/bash
echo "*** Begin native C++ library make (clean)"
rm *.o
rm bin/libcodehappy.a

echo "*** Build external C libraries (sqlite, pdcurses on Windows, ggml on CPU)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

GGML_GPP_ARGS="-fPIC -DNDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable"
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

echo "*** Build embedded fonts/patches."
embed_built=0
if [ -f "bin/embed.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed.o
cp embed.o bin
embed_built=1
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DALL_FONTS -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappy.a *.o

echo "*** Build tests and examples"
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/compress.cpp -o compress.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testfont.cpp -o testfont.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/colors.cpp -o colors.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/256color.cpp -o 256color.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testwav.cpp -o testwav.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testpat.cpp -o testpat.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/sql.cpp -o sql.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testsat.cpp -o testsat.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/sf2info.cpp -o sf2info.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/midipart.cpp -o midipart.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/randbmp.cpp -o randbmp.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/polyn.cpp -o polyn.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/optn.cpp -o optn.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/astropix.cpp -o astropix.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/wikiart.cpp -o wikiart.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/lora.cpp -o lora.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/wikimedia.cpp -o wikimedia.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/quant.cpp -o quant.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/newspapers.cpp -o newspapers.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/sd.cpp -o sd.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/ttfembed.cpp -o ttfembed.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/bertembed.cpp -o bertembed.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/bert2stream.cpp -o bert2stream.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/chat.cpp -o chat.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/sam-img.cpp -o sam-img.o
g++ -O3 -flto -fuse-linker-plugin -m64 compress.o bin/libcodehappy.a -lpthread -o compress
g++ -O3 -flto -fuse-linker-plugin -m64 testfont.o bin/libcodehappy.a -lpthread -o testfont
g++ -O3 -flto -fuse-linker-plugin -m64 colors.o bin/libcodehappy.a -lpthread -o colors
g++ -O3 -flto -fuse-linker-plugin -m64 256color.o bin/libcodehappy.a -lpthread -o 256color
g++ -O3 -flto -fuse-linker-plugin -m64 testwav.o bin/libcodehappy.a -lpthread -o testwav
g++ -O3 -flto -fuse-linker-plugin -m64 testpat.o bin/libcodehappy.a -lpthread -o testpat
g++ -O3 -flto -fuse-linker-plugin -m64 sql.o bin/libcodehappy.a -lpthread -ldl -o sql
g++ -O3 -flto -fuse-linker-plugin -m64 testsat.o bin/libcodehappy.a -lpthread -o testsat
g++ -O3 -flto -fuse-linker-plugin -m64 sf2info.o bin/libcodehappy.a -lpthread -o sf2info
g++ -O3 -flto -fuse-linker-plugin -m64 midipart.o bin/libcodehappy.a -lpthread -o midipart
g++ -O3 -flto -fuse-linker-plugin -m64 randbmp.o bin/libcodehappy.a -lpthread -o randbmp
g++ -O3 -flto -fuse-linker-plugin -m64 polyn.o bin/libcodehappy.a -lpthread -o polyn
g++ -O3 -flto -fuse-linker-plugin -m64 optn.o bin/libcodehappy.a -lpthread -o optn
g++ -O3 -flto -fuse-linker-plugin -m64 astropix.o bin/libcodehappy.a -lpthread -o astropix
g++ -O3 -flto -fuse-linker-plugin -m64 wikiart.o bin/libcodehappy.a -lpthread -o wikiart
g++ -O3 -flto -fuse-linker-plugin -m64 lora.o bin/libcodehappy.a -lpthread -o lora
g++ -O3 -flto -fuse-linker-plugin -m64 wikimedia.o bin/libcodehappy.a -lpthread -o wikimedia
g++ -O3 -flto -fuse-linker-plugin -m64 quant.o bin/libcodehappy.a -lpthread -o quant
g++ -O3 -flto -fuse-linker-plugin -m64 newspapers.o bin/libcodehappy.a -lpthread -o newspapers
g++ -O3 -flto -fuse-linker-plugin -m64 sd.o bin/libcodehappy.a -lpthread -o sd-cpu
g++ -O3 -flto -fuse-linker-plugin -m64 ttfembed.o bin/libcodehappy.a -lpthread -o ttfembed
g++ -O3 -flto -fuse-linker-plugin -m64 bertembed.o bin/libcodehappy.a -lpthread -o bertembed-cpu
g++ -O3 -flto -fuse-linker-plugin -m64 bert2stream.o bin/libcodehappy.a -lpthread -o bert2stream
g++ -O3 -flto -fuse-linker-plugin -m64 chat.o bin/libcodehappy.a -lpthread -o chat-cpu
g++ -O3 -flto -fuse-linker-plugin -m64 sam-img.o bin/libcodehappy.a -lpthread -o sam-img
if [ $embed_built -eq 1 ]; then
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/fontsample.cpp -o fontsample.o
g++ -O3 -flto -fuse-linker-plugin -m64 fontsample.o bin/libcodehappy.a -lpthread -o fontsample
fi
echo "*** Cleanup"
rm *.o
