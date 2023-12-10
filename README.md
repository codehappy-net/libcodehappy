## libcodehappy

*"Make hay while the sun shines."*

Yet another cross-platform library for building useful, portable applications in C++. Includes libraries written by me as well as a collection of
useful libraries by others released under permissive licenses (MIT, CC0, zlib.) This is currently my C++ developer toolkit: much of 
the software released in my GitHub depends on this library.

### Good things about it:

- You can build Windows, Linux, or WASM apps that run in-browser from the same source code base. Supports Win64 native, Linux native, or WebAssembly apps, terminal or windowed, in C++11, with minimal boilerplate. Good for tools, games, etc. Tested in Windows 7/8/10/11 and Ubuntu 20.04.
- Lots of functionality, convenience, and syntactic sugar features provided. Really. A lot. A true tinkerer's toybox: 2-D drawing, GUI controls, waveform synthesis, various non-linear optimization libraries including genetic optimization and ANNs, large language model inference on CPU or with GPU acceleration, etc.
- Performance for most features somewhere between acceptable and world-beating. Any modern compiler with link-time optimizations does good things with the code.
- Build process is simple.
- Native apps have no external dependencies except pthread and the standard library, GUI native apps need only SDL and SDL_mixer additionally, WASM apps use Emscripten and its built-in SDL implementation. SDL binaries included for Win64, for Linux you'll need to install via apt-get. GPU accelerated apps need NVIDIA's CUDA developer toolkit.
- Mostly easy to read, APIs are pretty simple and flexible, and it feels good to write code in, at least for me.

### Bad things about it:

- It's sort of an everything library that's evolved over years, with a fair bit of obsolete or niche code (you need support for Gravis patches or ShiftJIS?), and some duplicated functionality (the C++ stdlib has expanded a great deal since this library began.)
- No library-scope namespace, so it might conflict with other large frameworks.
- Build process is simple because it's, after preprocessing, a ginormous single file library, plus a large module for embedded data (fonts, GUI vector graphics, etc.) As long as you aren't changing the embedded data, it's not a slow build though.
- Documentation currently is mostly through source code comments and the example apps.
- The SDL used is version 1.2, it probably should be updated to use 2.0. There are other embedded libraries that might benefit from an update.
- It badly needs at least a good set of unit tests; all the commonly-used classes are pretty solid, but there are codepaths that are not well-tested.
- Every UI control is owned and drawn by the framework; OS accessibility tools aren't going to work automatically. There should be a good cross-platform C++ solution for this that isn't "use an extremely heavy full-featured framework like Qt", but ATM no so you'll have to write platform-specific code for accessibility features for a truly production-ready app. All the pieces are here to write a frontend for Nuklear or something like that, but I haven't gotten to it.
- Much of the code from my old C99 library hasn't been ported into this one yet, largely because I haven't used it in apps yet. (Lazy evaluation works for better performance in real life, too!) 

### Author credits and license

Code not otherwise marked is by me, Chris Street. That code is made available under MIT license: the software is presented as is, without warranty of any kind, but may be used in other projects as long as the copyright notice is included in all copies or substantial portions of the software.

Authors of other bits of code and the associated licensing include:

- Sean Barrett and the contributors to the wonderful STB header libraries (MIT or CC0)
- D. Richard Hipp, author of the triumphant SQLite database library (CC0)
- Georgi Gerganov and all the contributors to GGML and llama.cpp: the future is here! (MIT)
- Kim Walisch and ridiculous_fish, authors of the quiet-but-useful libdivide (zlib)
- Sergio Gonzalez, author of the JPEG encoder (CC0)
- Mikko Mononen, SVG parser (MIT)
- Bernhard Schelling, author of the soundfont (MIT) and MIDI parser (zlib)
- the Dana-Farber Cancer Institute and Broad Institute for the kann neural networks library (MIT)
- Rich Geldreich, zlib compression and decompression (CC0)
- Marc Alexander Lehmann, LZF compression and decompression (MIT)
- Tom St Denis, various number theoretic functions and SHA family hashes (CC0)
- Yusuke Suzuki, for console-colors (MIT)
- Bob Jenkins, Spooky Hash (CC0)
- leejet, for Stable Diffusion 1.x/2.x LDM inference in ggml (MIT)
- Dr. Tim Ferguson, for codecs for various old video formats (CC with attribution)
- Guillaume Chereau, portable file dialogs and quaternion library (MIT)
- Arian Rezazadeh, clipboard support library (MIT)
- skeskinen, for BERT inference and embeddings support in ggml (MIT)
- all PDCurses authors and contributors, for ANSI console support on Windows (CC0)
- Lewis Van Winkle, for Student's T score/regularized incomplete beta function (zlib) and the GENANN neural networks library (MIT)
- Paul Schlyter, for various astronomical functions including sunrise/sunset (CC0)
- Titus Wormer, Levenshtein distance (MIT)
- Simon Howard, Bloom filter library (MIT)
- Joseph Adams, diff patch library (MIT)
- Ahmed Samy, extended CPUID functions (MIT)
- Public domain contributions by Donald E. Knuth, Henry S. Warren Jr., Timothy B. Terriberry, Rusty Russell, Michelangelo Jones, Raymond Gardner, David Ahl, Bob Stout, Ross Cottrell, David Harmon, Maynard Hogg, Jeffrey Foy, Thad Smith III, S. E. Margison, Bob Jarvis, Gilles Kohl, Jerry Coffin, Ed Bernal, Alexander Peslyak, Steve Reid, Henry Spencer, Paul Bartrum (all CC0)

### Build process:

The make* scripts in the root directory build the library and example apps, .sh for Linux, .bat for Windows. You may need to change the values of the environment variables SDL_LINKER_FLAGS, SDL_COMPILER_FLAGS, CUDA_INC_DIRS, or CUDA_LIBRARIES. The name of the script tells you the kind of build:

- *maken*: Native build (for terminal/console apps), release, output is libcodehappy.a
- *makend*: Native build (for terminal/console apps), debug, output is libcodehappyd.a
- *makens*: Native + SDL build (for windowed apps or apps using SDL audio), release, output is libcodehappys.a
- *makensd*: Native + SDL build (for windowed apps or apps using SDL audio), debug, output is libcodehappysd.a
- *makecuda*: Native build (for terminal/console apps) with CUDA support, release, output is libcodehappycuda.a
- *makecudad*: Native build (for terminal/console apps) with CUDA support, debug, output is libcodehappycudad.a
- *makecudas*: Native + SDL build (for windowed apps or apps using SDL audio) with CUDA support, release, output is libcodehappycudas.a
- *makecudasd*: Native + SDL build (for windowed apps or apps using SDL audio) with CUDA support, debug, output is libcodehappycudasd.a
- *makewasm*: WebAssembly + Emscripten build (for in-browser apps), no WebGPU acceleration yet! :-), output is libcodehappy.bc.

