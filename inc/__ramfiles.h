/***
	__ramfiles.h

	RAM-based file I/O

	Advantages of RAM files:

	* The file is available in memory in its entirety
	on open. Random-access is cheap and quick. Searches
	and many other operations are also easy.

	* RAM files are inherently read-write. The behavior of
	write operations at the read location can be changed
	as desired: the write could simply overwrite whatever
	is at the read location, the write could insert the
	new data before the read location, or writes can be
	directed at the end of the file (as appends) regardless
	of read location.

	* No distinction between binary and text mode. Newlines
	of '\n', '\n\r', and '\r\n' form are all recognized as
	newlines by functions that care about newlines.

	* RAM files can be compressed and decompressed
	completely transparently.

	Disadvantages of RAM files:

	* Very large files will eat up a lot of RAM, and most
	platforms will be unable to open a file 2 GB in size
	or larger.

	* No incremental writes (at least currently) -- you have
	to manually flush or close the file to write data to disk,
	at which point the entire file is written.

	C. M. Street

***/
#ifndef RAMFILES_H
#define RAMFILES_H

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
#define	RAMFILE_READ			(RAMFILE_READONLY)

typedef struct _ramfile {
	char* fname;		// the file name
	scratchpad* sp;		// the file contents
	char* readp;		// the current read position
	uint32_t options;	// options
} ramfile;

/***
	Opens a RAM file with the specified options. Returns
	the RAM file on success, or NULL on failure.
***/
extern ramfile* ramfile_open(const char* fname, uint32_t options);

/***
	Opens a static RAM file with the specified options. Returns
	the RAM file on success, or NULL on failure.
***/
extern ramfile* ramfile_open_static(char* data, uint dlen, uint32_t options);

/***
	Closes a specified RAM file. 
***/
extern void ramfile_close(ramfile* rf);

/***
	Sets the read/write pointer back to the beginning of the RAM file.
***/
extern void ramfile_rewind(ramfile* rf);

/***
	Sets the read/write pointer to the specified position relative to the beginning of the RAM file: 0 points to the first byte.
	Returns true on success.  On failure, does not change the read/write pointer.
***/
extern bool ramfile_seek(ramfile* rf, u64 pos);

/***
	Sets the read/write pointer to the specified position relative to the end of the RAM file: 0 points to the end of file, 1 points
	to the last byte in the file.
	Returns true on success.  On failure, does not change the read/write pointer.
***/
extern bool ramfile_seek_from_end(ramfile* rf, u64 pos);

/***
	Sets the read/write pointer to the specified position relative to the current position.
	Returns true on success.  On failure, does not change the read/write pointer.
***/
extern bool ramfile_seek_from_here(ramfile* rf, i64 pos);

/***
	The ramfile's length.
***/
extern u32 ramfile_len(const ramfile* rf);

/***
	Set the ramfile's (path) name.
***/
extern u32 ramfile_setname(ramfile* rf, const char* newname);

/***
	Forces a write to disk for the specified RAM file.
***/
extern void ramfile_flush(const ramfile* rf);

/***
	Reads the next byte from a RAM file. Returns
	(-1) on EOF.
***/
extern int ramfile_getc(ramfile* rf);

/***
	Reads the next (newline-terminated) line from a RAM file.
	Returns 0 on success, (-1) on EOF.
***/
extern int ramfile_getline(ramfile* rf, scratchpad* sp);

/***
	Writes a byte to the RAM file with the current write
	options. Returns 0 on success.
***/
extern int ramfile_putc(ramfile* rf, char c);

/***
	Writes a string to the RAM file with the current write
	options. Returns 0 on success.
***/
extern int ramfile_puts(ramfile* rf, const char* str);

/***
	Puts a sequence of bytes in memory to the RAM file.
***/
int ramfile_putmem(ramfile* rf, const char* data, u32 nbytes);

/***
	Writes the contents of a scratchpad to the RAM file with
	the current write options. Returns 0 on success.
***/
extern int ramfile_putsp(ramfile* rf, const scratchpad* sp);

/***
	Was this RAM file compressed?
***/
extern bool ramfile_compressed(const ramfile* rf);

/***
	Turn the current option on.

	If the option is a write mode, we mask off the current write mode.
***/
extern void ramfile_option_on(ramfile* rf, uint32_t opt);

/***
	Turn the current option off.

	If no write modes are specified, we set the default write mode (overwrite at read position).
***/
extern void ramfile_option_off(ramfile* rf, uint32_t opt);

/*** Return the ramfile's data buffer. ***/
extern char* ramfile_buf(ramfile* rf);

#ifdef	CODEHAPPY_CPP
/*** Class wrapper for ramfile ***/
class RamFile
{
public:
	RamFile();
	RamFile(const char* fname, u32 options);
	~RamFile();
	
	bool open(const char* fname, uint32_t options);
	bool open_static(char* data, uint dlen, uint32_t options);
	void close(void);
	void rewind(void);
	u32 len(void) const;
	u32 setname(const char* newname);
	void flush(void);
	int getc(void);
	int getline(scratchpad* sp);
	int putc(char c);
	int puts(const char* str);
	int putsp(const scratchpad* sp);
	bool compressed(void) const;
	void option_on(u32 opt);
	void option_off(u32 opt);

private:
	ramfile* rf_;
};

#endif // CODEHAPPY_CPP

#endif  // RAMFILES_H
/* end __ramfiles.h */
