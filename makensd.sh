echo "*** Begin native build with SDL 1.2 C++ library make (DEBUG BUILD, clean)"
rm *.o
rm bin/libcodehappysd.a

# these from sdl-config output (TODO: could/should just run the script to get them, but I also should go SDL2, so...)
SDL_LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lSDL -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -lSDL_mixer"
SDL_COMPILE_FLAGS="-I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"

echo "*** Build external C libraries (sqlite, pdcurses on Windows)"
gcc -g -m64 -c -Wno-unused-result -Iinc inc/external/sqlite3.c $SDL_COMPILE_FLAGS -o sqlite3.o

echo "*** Build embedded fonts/patches"
if [ -f "bin/embed_sd.o" ]; then
echo "(Using cached version from previous build.)"
cp bin/embed_sd.o .
else
g++ -std=c++11 -g -m64 -c -Iinc -DCODEHAPPY_NATIVE_SDL -DALL_FONTS $SDL_COMPILE_FLAGS src/embed.cpp -o embed_sd.o
cp embed_sd.o bin
fi

echo "*** Build the C++ library (nice single file build)"
g++ -std=c++11 -g -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE_SDL -DALL_FONTS $SDL_COMPILE_FLAGS src/libcodehappy.cpp -o libcodehappy.o

echo "*** Create the library archive"
gcc-ar rcs bin/libcodehappysd.a *.o

echo "*** Build some demo apps"
g++ -g -m64 -Iinc -c examples/copra.cpp $SDL_COMPILE_FLAGS -o copra.o
g++ -g -m64 copra.o bin/libcodehappysd.a $SDL_LINKER_FLAGS -o copra

echo "*** Cleanup"
rm *.o
