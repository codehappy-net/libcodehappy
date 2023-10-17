#!/bin/bash
echo "*** Begin native C++ library make (clean)"
rm *.o
rm bin/libcodehappy.a

echo "*** Build external C libraries (sqlite, pdcurses on Windows)"
gcc -O3 -flto -fuse-linker-plugin -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed.o .
else
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed.o
cp embed.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DALL_FONTS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappy.a *.o

echo "*** Build tests and examples"
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/compress.cpp -o compress.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testfont.cpp -o testfont.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/colors.cpp -o colors.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/256color.cpp -o 256color.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testwav.cpp -o testwav.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testpat.cpp -o testpat.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testrng.cpp -o testrng.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/sql.cpp -o sql.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testsat.cpp -o testsat.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/makefr.cpp -o makefr.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/sf2info.cpp -o sf2info.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/imgn.cpp -o imgn.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/midipart.cpp -o midipart.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/randbmp.cpp -o randbmp.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/polyn.cpp -o polyn.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/optn.cpp -o optn.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/atten.cpp -o atten.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/flip.cpp -o flip.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/latent.cpp -o latent.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/dataset.cpp -o dataset.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/gpt.cpp -o gpt.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/prompt.cpp -o prompt.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/laion.cpp -o laion.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/screencapsite.cpp -o screencapsite.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/astropix.cpp -o astropix.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/wikiart.cpp -o wikiart.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/lora.cpp -o lora.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/wikimedia.cpp -o wikimedia.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/quant.cpp -o quant.o
g++ -O3 -flto -fuse-linker-plugin -m64 -c -Iinc examples/newspapers.cpp -o newspapers.o
g++ -O3 -flto -fuse-linker-plugin -m64 compress.o bin/libcodehappy.a -lpthread -o compress
g++ -O3 -flto -fuse-linker-plugin -m64 testfont.o bin/libcodehappy.a -lpthread -o testfont
g++ -O3 -flto -fuse-linker-plugin -m64 colors.o bin/libcodehappy.a -lpthread -o colors
g++ -O3 -flto -fuse-linker-plugin -m64 256color.o bin/libcodehappy.a -lpthread -o 256color
g++ -O3 -flto -fuse-linker-plugin -m64 testwav.o bin/libcodehappy.a -lpthread -o testwav
g++ -O3 -flto -fuse-linker-plugin -m64 testpat.o bin/libcodehappy.a -lpthread -o testpat
g++ -O3 -flto -fuse-linker-plugin -m64 testrng.o bin/libcodehappy.a -lpthread -o testrng
g++ -O3 -flto -fuse-linker-plugin -m64 sql.o bin/libcodehappy.a -lpthread -ldl -o sql
g++ -O3 -flto -fuse-linker-plugin -m64 testsat.o bin/libcodehappy.a -lpthread -o testsat
g++ -O3 -flto -fuse-linker-plugin -m64 sf2info.o bin/libcodehappy.a -lpthread -o sf2info
g++ -O3 -flto -fuse-linker-plugin -m64 imgn.o bin/libcodehappy.a -lpthread -o imgn
g++ -O3 -flto -fuse-linker-plugin -m64 imgn.o bin/libcodehappy.a -lpthread -o imgnr
g++ -O3 -flto -fuse-linker-plugin -m64 midipart.o bin/libcodehappy.a -lpthread -o midipart
g++ -O3 -flto -fuse-linker-plugin -m64 randbmp.o bin/libcodehappy.a -lpthread -o randbmp
g++ -O3 -flto -fuse-linker-plugin -m64 polyn.o bin/libcodehappy.a -lpthread -o polyn
g++ -O3 -flto -fuse-linker-plugin -m64 optn.o bin/libcodehappy.a -lpthread -o optn
g++ -O3 -flto -fuse-linker-plugin -m64 atten.o bin/libcodehappy.a -lpthread -o atten
g++ -O3 -flto -fuse-linker-plugin -m64 flip.o bin/libcodehappy.a -lpthread -o flip
g++ -O3 -flto -fuse-linker-plugin -m64 latent.o bin/libcodehappy.a -lpthread -o latent
g++ -O3 -flto -fuse-linker-plugin -m64 makefr.o bin/libcodehappy.a -lpthread -o makefr
g++ -O3 -flto -fuse-linker-plugin -m64 dataset.o bin/libcodehappy.a -lpthread -o dataset
g++ -O3 -flto -fuse-linker-plugin -m64 gpt.o bin/libcodehappy.a -lpthread -o gpt
g++ -O3 -flto -fuse-linker-plugin -m64 prompt.o bin/libcodehappy.a -o prompt
g++ -O3 -flto -fuse-linker-plugin -m64 laion.o bin/libcodehappy.a -o laion
g++ -O3 -flto -fuse-linker-plugin -m64 screencapsite.o bin/libcodehappy.a -o screencapsite
g++ -O3 -flto -fuse-linker-plugin -m64 astropix.o bin/libcodehappy.a -o astropix
g++ -O3 -flto -fuse-linker-plugin -m64 wikiart.o bin/libcodehappy.a -o wikiart
g++ -O3 -flto -fuse-linker-plugin -m64 lora.o bin/libcodehappy.a -o lora
g++ -O3 -flto -fuse-linker-plugin -m64 wikimedia.o bin/libcodehappy.a -o wikimedia
g++ -O3 -flto -fuse-linker-plugin -m64 quant.o bin/libcodehappy.a -o quant
g++ -O3 -flto -fuse-linker-plugin -m64 newspapers.o bin/libcodehappy.a -o newspapers

echo "*** Cleanup"
rm *.o
