/***

	entropy.h

	An entropy encoder/compressor.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __ENTROPY_H
#define __ENTROPY_H

struct freqsymbol {
	u32	symbol_idx;
	u32	freq;
	u32	start;
	u32	total;
};

struct treenode {
	u32	lbound;
	u32	hbound;
	u32	symbol_idx;
	treenode *left;
	treenode *right;
};

/*** Compress fname_in into fname_out -- fname_out will be overwritten ***/
extern void entropy_compress_file(const char* fname_in, const char* fname_out);

/*** Output buffer must be allocated and passed in. Returns TRUE if compression was successful (fits inside
	the output buffer), FALSE otherwise. Compressed length comes back in compress_len. ***/
extern bool entropy_compress_membuf(char* buf_in, u32 input_len, char* buf_out, u32 output_len, u32* compress_len);

/*** Create a sorted array of 1-byte symbols for this file. Useful to count instances of characters, etc. ***/
extern freqsymbol* compile_symbol_freq_1(RamFile* rf);

/*** Decompress fname_in to fname_out -- fname_out will be overwritten ***/
extern void entropy_decompress_file(const char* fname_in, const char* fname_out);

/*** Decompresses an entropy-compressed memory buffer. Note that this allocates buf_out and returns the length in output_len. ***/
extern void entropy_decompress_membuf(char* buf_in, u32 input_len, char** buf_out, u32* output_len);

/*** Slower but much better compression for low-entropy data ***/
extern bool entropy_compress_file_full(const char* fname_in, const char* fname_out);

extern bool entropy_decompress_file_full(const char* fname_in, const char* fname_out);

#endif  // __ENTROPY_H
