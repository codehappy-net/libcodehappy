#!/bin/bash
echo "*** Begin native C++ library make (DEBUG BUILD, clean)"
rm *.o
rm bin/libcodehappyd.a

echo "*** Build external C libraries (sqlite, pdcurses on Windows)"
gcc -g -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

echo "*** Build embedded fonts/patches."
if [ -f "bin/embed_d.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_d.o .
else
g++ -std=c++11 -g -m64 -c -Iinc -DCODEHAPPY_NATIVE -DALL_FONTS src/embed.cpp -o embed_d.o
cp embed_d.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -g -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -DALL_FONTS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappyd.a *.o

echo "*** Cleanup"
rm *.o
