/***

	binary_embed.h

	Transform a binary file into a C source file that can be compiled into an executable.

	Copyright (c) 2014-2022, C. M. Street

***/
#ifndef _BINARY_EMBED_H_
#define _BINARY_EMBED_H_

extern u32 flength(const char* pname);
extern int binary_to_c_src(const char* infile, const char* outfile);

#endif  // _BINARY_EMBED_H_
/* end binary_embed.h */
