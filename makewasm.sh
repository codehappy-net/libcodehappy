#!/bin/bash
echo "*** Begin native C++ library make (clean, Emscripten + WebAssembly)"
rm *.o
rm bin/libcodehappy.bc
rm bin/sqlite3.bc

echo "*** Build external C libraries (sqlite, pdcurses on Windows)"
emcc -std=c11 -O3 -flto -Iinc -DCODEHAPPY_WASM -sUSE_SDL -c inc/external/sqlite3.c -o bin/sqlite3.bc

echo "*** Build the C++ library (nice single file build)"
emcc -std=c++11 -O3 -flto -Iinc -sUSE_SDL -DALL_FONTS -DINCLUDE_EMBED -DCODEHAPPY_WASM -c -Wno-deprecated-register -Wno-parentheses -Wno-switch src/libcodehappy.cpp -o bin/libcodehappy.bc

echo "*** Build example apps"
emcc -O3 -flto -std=c++11 -sUSE_SDL -DCODEHAPPY_WASM -Iinc examples/tetris.cpp bin/libcodehappy.bc -o tetris.js --preload-file assets-tetris/ -s ALLOW_MEMORY_GROWTH=1
