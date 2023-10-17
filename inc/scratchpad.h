/***

	scratchpad.h

	Scratchpad objects.

	Need a dynamic memory buffer to hold string input from untrusted sources (file, user input)? Don't know what
	size you need ahead of time? Create a scratchpad object to handle it for you.

	These are used throughout libcodehappy for various purposes (keyboard buffers, ramfile data, etc.)

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef _SCRATCHPAD_H_
#define _SCRATCHPAD_H_

class Scratchpad {
public:
	/* New scratchpad with no allocated storage. */
	Scratchpad();

	/* New scratchpad with nbytes allocated storage. */
	Scratchpad(u32 nbytes);

	/* Destructor. */
	~Scratchpad();

	/* Free the scratchpad data. */
	void free(void);

	/* Return the buffer, which is zero-terminated (can be used as a dynamic C string). */
	u8* buffer(void)  const { return buf; }
	char* c_str(void) const { return (char *)buf; }

	/* Return the number of bytes with data in the scratchpad. */
	u32 length(void) const { return cend - buf; }

	/* Return the number of bytes allocated for the scratchpad buffer. */
	u32 allocated(void) const { return ialloc; }

	/* Add a single character to the scratchpad. Returns 0 on success, 1 on failure (probably OOM.) */
	int addc(u8 ch);

	/* Insert a single character at a position (buffer offset). Data beyond this position in the buffer is moved forward.
		Returns 0 on success, 1 on failure (probably OOM.) */
	int insertch(u8 ch, u32 pos);

	/* Deletes nb bytes at the specified position. Data beyond this position is moved back. */
	int delbytes(u32 nb, u32 pos);
	
	/* Deletes nb bytes from the end of the buffer. */
	int delfromend(u32 nb);

	/* Concatenate scratchpad buffer with the passed-in C string. */
	int strcat(const char* str);
	int strcat(const char* str, u32 len);

	/* Concatenate the passed-in memory buffer to the scratchpad. */
	int memcat(const u8* mem, u32 nbytes);

	/* Concatenate two scratchpads together. */
	int concat(const Scratchpad* sp);

	/* Update the scratchpad -- only use this if you've been fiddling with the buffer contents as a zero-terminated string. */
	void update_str(void);

	/* Reallocate the scratchpad buffer to contain at least nbytes bytes of data. */
	int realloc(u32 nbytes);

	/* Reallocate the scratchpad buffer to be exactly the size required to contain its data. */
	int compact(void);

	/* Copies a string into the scratchpad buffer, replacing what was there. */
	int strcpy(const char* str);
	int strncpy(const char* str, u32 nchar);

	/* Returns and removes the first character in this scratchpad.
           Returns (-1) if nothing's in the scratchpad. */
	int getch(void);

	/* Swaps the contents of two scratchpads. */
	void swap(Scratchpad* sp);

	/* Return the buffer, and relinquish ownership of it. */
	u8* relinquish_buffer(void);

	/* Add little-endian 16/32 bit values. */
	void addle_u16(u16 val);
	void addle_u32(u32 val);
	void addle_u64(u64 val);
	void addle_16(i16 val);
	void addle_32(i32 val);
	void addle_64(i64 val);

	/* Empties the scratchpad (but does not free allocated memory.) */
	void clear();

private:
	/* Helper function for strcat() */
	int doublestr(void);

	/* Gives the scratchpad a buffer. Note that the scratchpad doesn't take ownership. Most
	   consumers should never do this; it's fiddly and hacky. Only friend classes get to. */
	void give_static_buffer(u8* buf_in, u32 buflen);

	u32 ialloc;
	u8* buf;
	u8* cend;
	u8* bend;

	friend class RamFile;
};

#endif  // _SCRATCHPAD_H_
