/***
	ramfiles.h

	RAM-based file I/O

	Advantages of RAM files:

	* The file is available in memory in its entirety on open. Random-access is cheap and quick. Searches
	and many other operations are also easy.

	* RAM files are inherently read-write. The behavior of write operations at the read location can be changed
	as desired: the write could simply overwrite whatever is at the read location, the write could insert the
	new data before the read location, or writes can be directed at the end of the file (as appends) regardless
	of read location.

	* No distinction between binary and text mode. Newlines of '\n', '\n\r', and '\r\n' form are all recognized as
	newlines by functions that care about newlines.

	* RAM files can be compressed and decompressed completely transparently.

	Disadvantages of RAM files:

	* Very large files will eat up a lot of RAM, and most platforms will be unable to open a file 2 GB in size
	or larger.

	* No incremental writes (at least currently) -- you have to manually flush or close the file to write data to disk,
	at which point the entire file is written.

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef _RAMFILES_H_
#define _RAMFILES_H_

/*** RAM file options -- can be specified on load and changed
	as desired; OR together the ones you want ***/

/*** on open, if a file of the specified name does not
	exist, create it empty. ***/
#define	RAMFILE_CREATE_IF_MISSING	1

/*** all write operations should append to the end of the file;
	ignore the read position ***/
#define RAMFILE_WRITE_APPEND		2

/*** write operations should simply overwrite existing data at the
	read position. Note that this is actually the default if
	none of the other RAMFILE_WRITE* options are given. ***/
#define	RAMFILE_WRITE_OVERWRITE		4

/*** write operations should insert before the read position ***/
#define RAMFILE_WRITE_INSERT		8

/*** if the file we're opening is compressed by us, don't decompress it;
	just open it as a regular binary file ***/
#define RAMFILE_IGNORE_COMPRESSION	16

/*** compress the RAM file's contents on write to disk ***/
#define RAMFILE_COMPRESS		32

/*** force the RAM file to be read only ***/
#define	RAMFILE_READONLY		64

/*** the RAM file's contents are actually a static (not allocated) buffer ***/
#define	RAMFILE_STATIC			128

/*** flags at and above this are for internal use only ***/
#define	RAMFILE_INTERNAL		128

/*** Some reasonable defaults. ***/
#define RAMFILE_DEFAULT			(RAMFILE_CREATE_IF_MISSING)
#define RAMFILE_DEFAULT_COMPRESS	(RAMFILE_CREATE_IF_MISSING | RAMFILE_COMPRESS)
#define	RAMFILE_READ			(RAMFILE_READONLY)

typedef u64 * EmbeddedRamFile;

class RamFile {
public:
	// Creates an empty ramfile.
	RamFile();
	// Creates a ramfile, and opens the specified file.
	RamFile(const char* fn, u32 opt);
	RamFile(const std::string& fn, u32 opt);
	// Load an embedded ramfile. This will always be read-only. May be compressed.
	RamFile(const EmbeddedRamFile erf);
	// Destructor.
	~RamFile();

	// Opens the specified pathname with the given options. Returns 0 on success.
	int open(const char* fn, u32 opt);
	int open(const std::string& fn, u32 opt);

	// Closes a specified ramfile. This also frees memory owned by the ramfile.
	void close(void);

	// Sets the read/write pointer back to the beginning of the RAM file.
	void rewind(void);

	// Sets the read/write pointer to the specified position relative to the beginning of the RAM file: 0 points
	// to the first byte. Returns true on success. On failure, does not change the read/write pointer.
	bool seek(u64 pos);

	// Sets the read/write pointer to the specified position relative to the end of the RAM file: 0 points to the
	// end of file, 1 points to the last byte in the file. Returns true on success.  On failure, does not change the
	// read/write pointer.
	bool seek_from_end(u64 pos);

	// Sets the read/write pointer to the specified position relative to the current position.
	// Returns true on success.  On failure, does not change the read/write pointer.
	bool seek_from_here(i64 pos);

	// The ramfile's length.
	u32 length(void) const;

	// Forces a write to disk for the ramfile.
	void flush(void);

	// Opens a static RAM file with the specified options. Returns 0 on success. Static RAM files encapsulate
	// memory buffers; they do not have an associated disk file. The scratchpad takes ownership of the buffer.
	void open_static(char* data, uint dlen, uint32_t options);

	// Reads the next byte from a RAM file. Returns (-1) on EOF.
	int getc(void);

	// Writes a byte to the RAM file with the current write options. Returns 0 on success.
	int putc(int c);

	// Writes a string to the RAM file with the current write options. Returns 0 on success.
	int puts(const char* str);

	// Read a string from the RAM file. Returns 0 on success.
	int gets(char* str_dest, u32 buflen);

	// Read an u64 (in text format) from the ramfile.
	u64 get_u64(void);

	// Peek functions. These return values at the current read pointer.
	// peek(): returns the unsigned char value at the read pointer, or -1 if EOF.
	int peek(void) const;
	// peeku16(): returns the unsigned short value at the read pointer, or -1 if EOF.
	int peeku16(void) const;
	// peeku32(): returns the unsigned long value at the read pointer. 0 if EOF, so watch out.
	u32 peeku32(void) const;

	// Read or write various size integers (in binary, little-endian) from the ramfile. 
	// The put() functions return 0 on success. On end-of-file the get() functions return 0.
	u64 getu64(void);
	i64 get64(void);
	u32 getu32(void);
	i32 get32(void);
	u16 getu16(void);
	i16 get16(void);
	int putu64(u64 u);
	int put64(i64 i);
	int putu32(u32 u);
	int put32(i32 i);
	int putu16(u16 u);
	int put16(i16 i);
	bool getbool(void);
	int putbool(bool v);

	// Also floating point values.
	double getdouble(void);
	float getfloat(void);
	int putdouble(double v);
	int putfloat(float v);

	// And C++ strings.
	int putstring(const std::string& s);
	std::string getstring();

	// Is the read pointer at the end of the ramfile?
	bool eof(void) const;

	// Puts a sequence of bytes in memory to the RAM file.
	int putmem(const char* data, u32 nbytes);

	// Gets a sequence of bytes in memory from the RAM file.
	int getmem(u8* to, u32 nb);

	// Writes the contents of a scratchpad to the RAM file with the current write options. Returns 0 on success.
	int putsp(const Scratchpad& sp1);

	// Fill the scratchpad from the RAM file. Returns 0 on success.
	int getsp(Scratchpad& sp);

	// Set the ramfile's pathname.
	u32 setname(const char* newname);

	// Was this RAM file compressed?
	bool compressed(void) const;

	// Turn the current option on. If the option is a write mode, we mask off the current write mode.
	void option_on(u32 opt);

	// Turn the current option off. If no write modes are specified, we set the default write mode (overwrite at read position).
	void option_off(u32 opt);

	// Read a line from the ramfile into the passed scratchpad.
	int getline(Scratchpad& sp_out);

	// Return the ramfile's data buffer.
	u8* buffer(void) const;

	// Write the RamFile out to a C/C++ source file as an EmbeddedRamFile.
	void embed_ramfile(const char* fname_out, const char* rfname = nullptr);

	// Truncate the ramfile.
	void truncate();

	// Write the contents of this RamFile to a specified file. (Can be used to write the contents of static RamFiles.)
	void write_to_file(const char* fname) const;
	void write_to_file(const std::string& fname) const;

	// closes the RamFile, relinquishes and returns the buffer.
	u8* relinquish_buffer();

	u8* read_ptr() const { return readp; }

private:
	// Helper function: decompress a compressed ramfile
	void decompress(void);

	char* fname;		// the file name
	Scratchpad sp;		// the file contents
	u8* readp;		// the current read position
	u32 options;		// options
};


#endif  // _RAMFILES_H_
