/***
	bitfile.h

	Bit-level I/O for disk files or memory buffers.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifndef BITFILE_H
#define BITFILE_H

/*** Bitfile options. ***/

#define	BITFILE_NONE		0
#define	BITFILE_IS_WRITE	1
#define	BITFILE_IS_RAM		2

#define	BF_IS_WRITE(bf)	((bf)->opt & BITFILE_IS_WRITE)
#define	BF_IS_RAM(bf)	(zo1((bf)->opt & BITFILE_IS_RAM))

/*** The bitfile structure. ***/

struct bitfile {
	FILE* 	f;		/* disk file associated with this bitfile, if any */
	char*	buf;	/* in RAM bitfiles, this always points to the next read/write location */
	char*	bufe;	/* in RAM bitfiles, this always points to the end of the memory buffer */
	char*	bufs;	/* in RAM bitfiles, this always points to the start of the original memory buffer */
	u32		acc;	/* accumulator */
	u32		acc_c;	/* count of bits in the accumulator */
	u32		opt;	/* the bitfile's option flags */
};

/*** Opens a disk file to read/write using bit I/O. ***/
extern void bitfile_open(bitfile* bf, const char* fname, bool is_write);

/*** Opens a bitfile that will read or write to a memory buffer. ***/
extern void bitfile_open_mem(bitfile* bf, char* buf, u32 bufsize, bool is_write);

/*** Closes the bitfile. ***/
extern void bitfile_close(bitfile* bf);

/*** Writes a single bit to the bitfile. ***/
extern void bitfile_writebit(bitfile* bf, bool on);

/*** Writes a single byte to the bitfile. ***/
extern void bitfile_writebyte(bitfile* bf, u32 byte);

/*** Writes a 32-bit unsigned integer to the bitfile. ***/
extern void bitfile_write32(bitfile* bf, u32 val);

/*** Writes a 64-bit unsigned integer to the bitfile. ***/
extern void bitfile_write64(bitfile* bf, u64 val);

/*** Reads a bit from the bitfile. Returns -1 on EOF. ***/
extern int bitfile_readbit(bitfile* bf);

/*** Reads an (unsigned) byte from the bitfile. Returns -1 on EOF. ***/
extern int bitfile_readbyte(bitfile* bf);

/*** Reads a 32 bit integer from the bitfile. Sets the is_eof flag to true on EOF, if is_eof is non-NULL. ***/
extern u32 bitfile_read32(bitfile* bf, bool* is_eof);

/*** Reads a 64 bit integer from the bitfile. Sets the is_eof flag to true on EOF, if is_eof is non-NULL. ***/
extern u64 bitfile_read64(bitfile* bf, bool* is_eof);

/* Class wrapper interface for bitfile */

class BitFile
{
public:
	BitFile();
	BitFile(const char* filename, bool is_write);
	~BitFile();
	
	void open(const char* fname, bool is_write);
	void open_mem(char* buf, u32 bufsize, bool is_write);
	void close(void);
	void write(char byte);
	void write(u32 ui32);
	void write(u64 ui64);
	void writebit(bool on);
	void writebyte(u32 byte);
	void write32(u32 val);
	void write64(u64 val);
	int readbit(void);
	int readbyte(void);
	u32 read32(bool* is_eof);
	u64 read64(bool* is_eof);
	
private:
	bitfile bf_;
	bool init_;
};

#endif  // BITFILE_H
/* end bitfile.h */