LZHAM 
=============

<p>Lossless Data Compression Codec</p>
<p>License: MIT</p>
<p>Copyright (c) 2009-2015 Richard Geldreich, Jr. <richgel99@gmail.com></p>

<p>LZHAM is a lossless data compression codec written in C/C++ with a compression ratio similar to LZMA but 1.5x-8x faster decompression speed. It officially supports Linux x86/x64, Windows x86/x64, 
OSX, and iOS, with Android support on the way.</p>

<h3>Introduction</h3>

<p>LZHAM is a lossless (LZ based) data compression codec optimized for particularly fast decompression at very high compression ratios with a zlib compatible API. 
It's been developed over a period of 3 years and alpha versions have already shipped in many products. (The alpha is here: https://code.google.com/p/lzham/)
LZHAM's decompressor is slower than zlib's, but generally much faster than LZMA's, with a compression ratio that is typically within a few percent of LZMA's and sometimes better.</p>

<p>LZHAM's compressor is intended for offline use, but it is tested alongside the decompressor on mobile devices and is usable on the faster settings.</p>

<p>LZHAM's decompressor currently has a higher cost to initialize than LZMA, so the threshold where LZHAM is typically faster vs. LZMA decompression is between 1000-13,000 of 
*compressed* output bytes, depending on the platform. It is not a good small block compressor: it likes large (15KB, preferably 50KB) blocks.
LZHAM has simple support for patch files (delta compression), but this is a side benefit of its design, not its primary use case. Internally it supports LZ matches up 
to ~64KB and very large dictionaries (up to .5 GB).</p>

<p>LZHAM may be valuable to you if you compress data offline and distribute it to many customers, care about read/download times, and decompression speed/low CPU+power use 
are important to you.</p>

<h3>Platforms/Compiler Support</h3>

LZHAM currently officially supports x86/x64 Linux, iOS, OSX, and Windows x86/x64. Android support is coming next.
It should be easy to retarget by modifying the macros in lzham_core.h. Early alphas were tested on Xbox 360 (PPC, big endian).
LZHAM has optional support for multithreaded compression. It supports gcc built-ins or Windows API's for atomic ops. For threading, it supports OSX 
specific pthreads, generic phthreads, or Windows API's.
For compilers, I've tested with gcc, clang, and MSVC 2008 and 2010.

<h3>API</h3>

LZHAM supports streaming or memory to memory compression/decompression. See include/lzham.h. LZHAM can be linked statically or dynamically, just study the 
headers and the lzhamtest project. 
On Linux/OSX, it's only been tested with static linking so far.

LZHAM also supports a usable subset of the zlib API with extensions, either include/zlib.h or #define LZHAM_DEFINE_ZLIB_API and use include/lzham.h.

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
https://lkml.org/lkml/2003/2/26/158
http://stackoverflow.com/questions/2958633/gcc-strict-aliasing-and-horror-stories

It might work fine, I don't know yet. This is usually not a problem with MSVC, which defaults to strict aliasing being off.

<h3>ANSI C/C++</h3>

LZHAM supports compiling as plain vanilla ANSI C/C++. To see how the codec configures itself check out lzham_core.h and search for "LZHAM_ANSI_CPLUSPLUS". 
All platform specific stuff (unaligned loads, threading, atomic ops, etc.) should be disabled when this macro is defined. Note, the compressor doesn't use threads 
or atomic operations when built this way so it's going to be pretty slow. (The compressor was built from the ground up to be threaded.)

<h3>Known Problems</h3>

<p>LZHAM's decompressor is like a drag racer that needs time to get up to speed. LZHAM is not intended or optimized to be used on "small" blocks of data (less 
than ~10,000 bytes of *compressed* data on desktops, or around 1,000-5,000 on iOS). If your usage case involves calling the codec over and over with tiny blocks 
than use LZMA, LZ4, Deflate, etc.</p>

<p>The decompressor still takes too long to init vs. LZMA. On iOS the cost is not that bad, but on desktop the cost is high. I have reduced the startup cost vs. the 
alpha but there's still work to do.</p>

<p>The compressor is slower than I would like, and doesn't scale as well as it could. I added a reinit() method to make it initialize faster, but it's not a speed demon. 
My focus has been on ratio and decompression speed.</p>
