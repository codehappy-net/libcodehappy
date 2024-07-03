@echo off
REM Win64 batch make script
echo *** Begin native C++ library make (clean)
del *.o /q
del bin\libcodehappyd.a /q

echo *** Build external C libraries (sqlite, pdcurses on Windows, ggml on CPU)
gcc -g -Wa,-mbig-obj  -m64 -c -Iinc inc/external/sqlite3.c -o sqlite3.o

set GGML_GPP_ARGS=-fPIC -DDEBUG -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function -Wno-multichar -Wno-cast-qual -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
set GGML_GCC_ARGS=-fPIC -DDEBUG -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wmissing-prototypes -pthread -march=native -mtune=native -DGGML_USE_K_QUANTS -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable

gcc -I. -g -Wa,-mbig-obj -std=c11 %GGML_GCC_ARGS% -c inc/external/ggml/ggml.c -o ggml.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/llama.cpp -o llama.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/common.cpp -o common.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/console.cpp -o console.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/build-info.cpp -o build-info.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/grammar-parser.cpp -o grammar-parser.o
gcc -I. -g -Wa,-mbig-obj -std=c11 %GGML_GCC_ARGS% -c inc/external/ggml/ggml-alloc.c -o ggml-alloc.o
gcc -I. -g -Wa,-mbig-obj -std=c11 %GGML_GCC_ARGS% -c inc/external/ggml/ggml-backend.c -o ggml-backend.o
gcc -I. -g -Wa,-mbig-obj -std=c11 %GGML_GCC_ARGS% -c inc/external/ggml/ggml-quants.c -o ggml-quants.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/sampling.cpp -o sampling.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/train.cpp -o train.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/llava.cpp -o llava.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/clip.cpp -o clip.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/json-schema-to-grammar.cpp -o json-schema-to-grammar.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/ngram-cache.cpp -o ngram-cache.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/sgemm.cpp -o sgemm.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/unicode.cpp -o unicode.o
g++ -I. -g -Wa,-mbig-obj -std=c++11 %GGML_GPP_ARGS% -c inc/external/ggml/unicode-data.cpp -o unicode-data.o

echo *** Build embedded fonts/patches.
set embed_built=0
if exist bin\embed_d.o (
echo (Using cached version from previous build.)
copy bin\embed_d.o .
) else (
g++ -std=c++11 -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE src/embed.cpp -o embed_d.o
copy embed_d.o bin
set embed_built=1
)

echo *** Build the C++ library (nice single file build)
g++ -std=c++11 -g -Wa,-mbig-obj -m64 -c -Iinc -Wno-unused-result -DCODEHAPPY_NATIVE -Wno-deprecated-declarations src/libcodehappy.cpp -o libcodehappy.o

echo *** Create the library archive
gcc-ar rcs bin/libcodehappyd.a *.o

echo *** Build tests and examples
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/compress.cpp -o compress.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testfont.cpp -o testfont.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/colors.cpp -o colors.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/256color.cpp -o 256color.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testwav.cpp -o testwav.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testpat.cpp -o testpat.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/sql.cpp -o sql.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc -DCODEHAPPY_NATIVE examples/testsat.cpp -o testsat.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/sf2info.cpp -o sf2info.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/midipart.cpp -o midipart.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/randbmp.cpp -o randbmp.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/polyn.cpp -o polyn.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/optn.cpp -o optn.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/astropix.cpp -o astropix.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/wikiart.cpp -o wikiart.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/lora.cpp -o lora.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/wikimedia.cpp -o wikimedia.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/quant.cpp -o quant.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/newspapers.cpp -o newspapers.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/sd.cpp -o sd.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/ttfembed.cpp -o ttfembed.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/bertembed.cpp -o bertembed.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/bert2stream.cpp -o bert2stream.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/chat.cpp -o chat.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/sam-img.cpp -o sam-img.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/llava.cpp -o llava.o
g++ -g -Wa,-mbig-obj -m64 -c -Iinc examples/exifdemo.cpp -o exifdemo.o
g++ -g -Wa,-mbig-obj -m64 compress.o bin/libcodehappyd.a -lpthread -o compress
g++ -g -Wa,-mbig-obj -m64 testfont.o bin/libcodehappyd.a -lpthread -o testfont
g++ -g -Wa,-mbig-obj -m64 colors.o bin/libcodehappyd.a -lpthread -o colors
g++ -g -Wa,-mbig-obj -m64 256color.o bin/libcodehappyd.a -lpthread -o 256color
g++ -g -Wa,-mbig-obj -m64 testwav.o bin/libcodehappyd.a -lpthread -o testwav
g++ -g -Wa,-mbig-obj -m64 testpat.o bin/libcodehappyd.a -lpthread -o testpat
g++ -g -Wa,-mbig-obj -m64 sql.o bin/libcodehappyd.a -lpthread -ldl -o sql
g++ -g -Wa,-mbig-obj -m64 testsat.o bin/libcodehappyd.a -lpthread -o testsat
g++ -g -Wa,-mbig-obj -m64 sf2info.o bin/libcodehappyd.a -lpthread -o sf2info
g++ -g -Wa,-mbig-obj -m64 midipart.o bin/libcodehappyd.a -lpthread -o midipart
g++ -g -Wa,-mbig-obj -m64 randbmp.o bin/libcodehappyd.a -lpthread -o randbmp
g++ -g -Wa,-mbig-obj -m64 polyn.o bin/libcodehappyd.a -lpthread -o polyn
g++ -g -Wa,-mbig-obj -m64 optn.o bin/libcodehappyd.a -lpthread -o optn
g++ -g -Wa,-mbig-obj -m64 astropix.o bin/libcodehappyd.a -lpthread -o astropix
g++ -g -Wa,-mbig-obj -m64 wikiart.o bin/libcodehappyd.a -lpthread -o wikiart
g++ -g -Wa,-mbig-obj -m64 lora.o bin/libcodehappyd.a -lpthread -o lora
g++ -g -Wa,-mbig-obj -m64 wikimedia.o bin/libcodehappyd.a -lpthread -o wikimedia
g++ -g -Wa,-mbig-obj -m64 quant.o bin/libcodehappyd.a -lpthread -o quant
g++ -g -Wa,-mbig-obj -m64 newspapers.o bin/libcodehappyd.a -lpthread -o newspapers
g++ -g -Wa,-mbig-obj -m64 sd.o bin/libcodehappyd.a -lpthread -o sd-cpu
g++ -g -Wa,-mbig-obj -m64 ttfembed.o bin/libcodehappyd.a -lpthread -o ttfembed
g++ -g -Wa,-mbig-obj -m64 bertembed.o bin/libcodehappyd.a -lpthread -o bertembed-cpu
g++ -g -Wa,-mbig-obj -m64 bert2stream.o bin/libcodehappyd.a -lpthread -o bert2stream
g++ -g -Wa,-mbig-obj -m64 chat.o bin/libcodehappyd.a -lpthread -o chat-cpu
g++ -g -Wa,-mbig-obj -m64 sam-img.o bin/libcodehappyd.a -lpthread -o sam-img
g++ -g -Wa,-mbig-obj -m64 llava.o bin/libcodehappyd.a -lpthread -o llava-cpu
g++ -g -Wa,-mbig-obj -m64 exifdemo.o bin/libcodehappyd.a -lpthread -o exifdemo

echo *** Cleanup
del *.o
