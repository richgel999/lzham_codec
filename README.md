LZHAM - Lossless Data Compression Codec
=============

<p>Copyright (c) 2009-2015 Richard Geldreich, Jr. - richgel99@gmail.com - MIT License</p>

<p>LZHAM is a lossless data compression codec written in C/C++ (specifically C++03), with a compression ratio similar to LZMA but with 1.5x-8x faster decompression speed. It officially supports Linux x86/x64, Windows x86/x64, 
OSX, and iOS, with Android support on the way.</p>

<p>Some slightly out of date API documentation is here (I'll be migrating this to github): https://code.google.com/p/lzham/wiki/API_Docs</p>

<h3>Introduction</h3>

<p>LZHAM is a lossless (LZ based) data compression codec optimized for particularly fast decompression at very high compression ratios with a zlib compatible API. 
It's been developed over a period of 3 years and alpha versions have already shipped in many products. (The alpha is here: https://code.google.com/p/lzham/)
LZHAM's decompressor is slower than zlib's, but generally much faster than LZMA's, with a compression ratio that is typically within a few percent of LZMA's and sometimes better.</p>

<p>LZHAM's compressor is intended for offline use, but it is tested alongside the decompressor on mobile devices and is usable on the faster settings.</p>

<p>LZHAM's decompressor currently has a higher cost to initialize than LZMA, so the threshold where LZHAM is typically faster vs. LZMA decompression is between 1000-13,000 of 
*compressed* output bytes, depending on the platform. It is not a good small block compressor: it likes large (10KB-15KB minimum) blocks.</p>

<p>LZHAM has simple support for patch files (delta compression), but this is a side benefit of its design, not its primary use case. Internally it supports LZ matches up 
to ~64KB and very large dictionaries (up to .5 GB).</p>

<p>LZHAM may be valuable to you if you compress data offline and distribute it to many customers, care about read/download times, and decompression speed/low CPU+power use 
are important to you.</p>

<p>I've been profiling LZHAM vs. LZMA and publishing the results on my blog: http://richg42.blogspot.com</p>

<p>Some independent benchmarks of the previous alpha versions: http://heartofcomp.altervista.org/MOC/MOCADE.htm, http://mattmahoney.net/dc/text.html</p>

<h3>Compressed Bitstream Compatibility</h3>

<p>v1.0's bitstream format is now locked in place, so any future v1.x releases will be backwards/forward compatible with compressed files 
written with v1.0. The only thing that could change this are critical bugfixes.</p>

<p>Note LZHAM v1.x bitstreams are NOT backwards compatible with any of the previous alpha versions on Google Code.</p>

<h3>Platforms/Compiler Support</h3>

LZHAM currently officially supports x86/x64 Linux, iOS, OSX, and Windows x86/x64. At one time the codec compiled and ran fine on Xbox 360 (PPC, big endian). Android support is coming next.
It should be easy to retarget by modifying the macros in lzham_core.h.</p>

<p>LZHAM has optional support for multithreaded compression. It supports gcc built-ins or MSVC intrinsics for atomic ops. For threading, it supports OSX 
specific Pthreads, generic Pthreads, or Windows API's.</p>

<p>For compilers, I've tested with gcc, clang, and MSVC 2008, 2010, and 2013. In previous alphas I also compiled with TDM-GCC x64.</p>

<h3>API</h3>

LZHAM supports streaming or memory to memory compression/decompression. See include/lzham.h. LZHAM can be linked statically or dynamically, just study the 
headers and the lzhamtest project. 
On Linux/OSX, it's only been tested with static linking so far.

LZHAM also supports a usable subset of the zlib API with extensions, either include/zlib.h or #define LZHAM_DEFINE_ZLIB_API and use include/lzham.h.

<h3>Usage Tips</h3>

* Always try to use the smallest dictionary size that makes sense for the file or block you are compressing, i.e. don't use a 128MB dictionary for a 15KB file. The codec
doesn't automatically choose for you because in streaming scenarios it has no idea how large the file or block will be.
* The larger the dictionary, the more RAM is required during compression and decompression. I would avoid using more than 8-16MB dictionaries on iOS.
* For faster decompression, prefer "unbuffered" decompression mode vs. buffered decompression (avoids a dictionary alloc and extra memcpy()'s), and disable adler-32 checking. Also, use the built-in LZHAM API's, not the
zlib-style API's for fastest decompression.
* Experiment with the "m_table_update_rate" compression/decompression parameter. This setting trades off a small amount of ratio for faster decompression.
Note the m_table_update_rate decompression parameter MUST match the setting used during compression (same for the dictionary size). It's up to you to store this info somehow.
* Avoid using LZHAM on small *compressed* blocks, where small is 1KB-10KB compressed bytes depending on the platform. LZHAM's decompressor is only faster than LZMA's beyond the small block threshold.
Optimizing LZHAM's decompressor to reduce its startup time relative to LZMA is a high priority.
* For best compression (I've seen up to ~4% better), enable the compressor's "extreme" parser, which is much slower but finds cheaper paths through a much denser parse graph.
Note the extreme parser can greatly slow down on files containing large amounts of repeated data/strings, but it is guaranteed to finish.
* The compressor's m_level parameter can make a big impact on compression speed. Level 0 (LZHAM_COMP_LEVEL_FASTEST) uses a much simpler greedy parser, and the other levels use 
near-optimal parsing with different heuristic settings.
* Check out the compressor/decompressor reinit() API's, which are useful if you'll be compressing or decompressing many times. Using the reinit() API's is a lot cheaper than fully 
initializing/deinitializing the entire codec every time.
* LZHAM's compressor is no speed demon. It's usually slower than LZMA's, sometimes by a wide (~2x slower or so) margin. In "extreme" parsing mode, it can be many times slower. 
This codec was designed with offline compression in mind.
* One significant difference between LZMA and LZHAM is how uncompressible files are handled. LZMA usually expands uncompressible files, and its decompressor can bog down and run extremely 
slowly on uncompressible data. LZHAM internally detects when each 512KB block is uncompressible and stores these blocks as uncompressed bytes instead. 
LZHAM's literal decoding is significantly faster than LZMA's, so the more plain literals in the output stream, the faster LZHAM's decompressor runs vs. LZMA's.
* General advice (applies to LZMA and other codecs too): If you are compressing large amounts of serialized game assets, sort the serialized data by asset type and compress the whole thing as a single large "solid" block of data.
Don't compress each individual asset, this will kill your ratio and have a higher decompression startup cost. If you need random access, consider compressing the assets lumped 
together into groups of a few hundred kilobytes (or whatever) each.
* LZHAM is a raw codec. It doesn't include any sort of preprocessing: EXE rel to abs jump transformation, audio predictors, etc. That's up to you
to do, before compression.

<h3>Codec Test App</h3>

lzhamtest_x86/x64 is a simple command line test program that uses the LZHAM codec to compress/decompress single files. 
lzhamtest is not intended as a file archiver or end user tool, it's just a simple testbed.

-- Usage examples:

- Compress single file "source_filename" to "compressed_filename":
	lzhamtest_x64 c source_filename compressed_filename
	
- Decompress single file "compressed_filename" to "decompressed_filename":
    lzhamtest_x64 d compressed_filename decompressed_filename

- Compress single file "source_filename" to "compressed_filename", then verify the compressed file decompresses properly to the source file:
	lzhamtest_x64 -v c source_filename compressed_filename

- Recursively compress all files under specified directory and verify that each file decompresses properly:
	lzhamtest_x64 -v a c:\source_path
	
-- Options	
	
- Set dictionary size used during compressed to 1MB (2^20):
	lzhamtest_x64 -d20 c source_filename compressed_filename
	
Valid dictionary sizes are [15,26] for x86, and [15,29] for x64. (See LZHAM_MIN_DICT_SIZE_LOG2, etc. defines in include/lzham.h.)
The x86 version defaults to 64MB (26), and the x64 version defaults to 256MB (28). I wouldn't recommend setting the dictionary size to 
512MB unless your machine has more than 4GB of physical memory.

- Set compression level to fastest:
	lzhamtest_x64 -m0 c source_filename compressed_filename
	
- Set compression level to uber (the default):
	lzhamtest_x64 -m4 c source_filename compressed_filename
	
- For best possible compression, use -d29 to enable the largest dictionary size (512MB) and the -x option which enables more rigorous (but ~4X slower!) parsing:
	lzhamtest_x64 -d29 -x -m4 c source_filename compressed_filename

See lzhamtest_x86/x64.exe's help text for more command line parameters.

<h3>Compiling LZHAM</h3>

- Linux: Use "cmake ." then "make". The cmake script only supports Linux at the moment. (Sorry, working on build systems is a drag.)
- OSX/iOS: Use the included XCode project. (NOTE: I haven't merged this over yet. It's coming!)
- Windows: Use the included VS 2010 project

IMPORTANT: With clang or gcc compile LZHAM with "No strict aliasing" ENABLED: -fno-strict-aliasing

I DO NOT test or develop the codec with strict aliasing:
* https://lkml.org/lkml/2003/2/26/158
* http://stackoverflow.com/questions/2958633/gcc-strict-aliasing-and-horror-stories

It might work fine, I don't know yet. This is usually not a problem with MSVC, which defaults to strict aliasing being off.

<h3>ANSI C/C++</h3>

LZHAM supports compiling as plain vanilla ANSI C/C++. To see how the codec configures itself check out lzham_core.h and search for "LZHAM_ANSI_CPLUSPLUS". 
All platform specific stuff (unaligned loads, threading, atomic ops, etc.) should be disabled when this macro is defined. Note, the compressor doesn't use threads 
or atomic operations when built this way so it's going to be pretty slow. (The compressor was built from the ground up to be threaded.)

<h3>Known Problems</h3>

<p>LZHAM's decompressor is like a drag racer that needs time to get up to speed. LZHAM is not intended or optimized to be used on "small" blocks of data (less 
than ~10,000 bytes of *compressed* data on desktops, or around 1,000-5,000 on iOS). If your usage case involves calling the codec over and over with tiny blocks 
then LZMA, LZ4, Deflate, etc. are probably better choices.</p>

<p>The decompressor still takes too long to init vs. LZMA. On iOS the cost is not that bad, but on desktop the cost is high. I have reduced the startup cost vs. the 
alpha but there's still work to do.</p>

<p>The compressor is slower than I would like, and doesn't scale as well as it could. I added a reinit() method to make it initialize faster, but it's not a speed demon. 
My focus has been on ratio and decompression speed.</p>

<p>I use tabs=3 spaces, but I think some actual tabs got in the code. I need to run the sources through ClangFormat or whatever.</p>

<h3>Special Thanks</h3>

<p>Thanks to everyone at the http://encode.ru forums. I read these forums as a lurker before working on LZHAM, and I studied every LZ related 
post I could get my hands on. Especially anything related to LZ optimal parsing, which still seems like a black art. LZHAM was my way of 
learning how to implement optimal parsing (and you can see this if you study the progress I made in the early alphas on Google Code).</p>

<p>Also, thanks to Igor Pavlov, the original creator of LZMA and 7zip, for advancing the start of the art in LZ compression.</p>

<h3>Development History</h3>

I wrote my first LZ codecs in the early 90's, first in 16-bit asm under DOS, then in C. I eventually reverse engineered PKZIP's Deflate, by studying PKWare's appnote.txt 
and single stepping through PKUNZIP in Borland's Turbo Debugger to fill in the missing bits. This was years before the Infozip project, so I was on my own. The results of 
this work eventually resulted in a really fast Deflate compatible codec that I developed before and after I went to work at Microsoft. They shipped this codec (part of Ensemble's "eslib") in Age of 
Empires 1/2, Halo Wars, Forza 2, Halo 3, and probably other stuff. I probably had the fastest Deflate compatible decompressor available specifically optimized for Xbox 360.

Eventually, Microsoft laid us off, and they switched future titles to their higher ratio LZX codec. After leaving Microsoft I spent a year researching fast Huffman and 
arithmetic decoding techniques for a new codec that could beat LZX. 

I eventually wrote LZHAM at home during the night after starting at Valve, in bits and pieces as time permitted. I knew I couldn't afford to do much arithmetic decoding, because I was targeting the
codec for Xbox 360 CPU which had a slow non-pipelined integer multiplier unit. So I knew most of the decoded symbols had to be Huffman coded. The entropy coders were first, followed by the match accelerator, then a flexible parser, 
then a near-optimal parser (first just using plain Dijkstra's algorithm and greatly optimizing/simplifying that). I refined this framework over a couple weeks to compete against LZX, then LZMA. 

Competing against LZMA using mostly Huffman coding was a surprisingly difficult task. I tuned every aspect of the models and entropy coders to have the 
highest compression gain without sacrificing too much decompression throughput, which involved a lot of experimentation, coding instrumentation, and statistical analysis. It was like manually path finding through a multidimensional maze. 
I tried a number of coding techniques which just were not competitive against LZMA, or were just too slow or not easily scalable to multiple threads.

Eventually, I refactored the codec and released the first alpha on Google Code in 2009, then continued on with the alphas for 7-8 more alpha releases. I optimized the parser by moving away from Dijkstra, added an "extreme" parsing mode 
which can exploit locally suboptimal LZ decisions if they result in a better overall ratio, added zlib compatibility, and wrote examples. v1.0 was released in Jan. 2013 after spending another few weeks further profiling 
and optimizing the codec to run well on mobile CPU's.
