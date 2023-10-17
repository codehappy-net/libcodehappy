/***

	bitfile.cpp

	Bitfile implementation: bitwise I/O for files and memory buffers.

	Copyright (c) 2014-2022 C. M. Street

***/

/*** helper function: read a byte from the source file or memory buffer ***/
static int __bitfile_getc(bitfile* bf) {
	if (BF_IS_RAM(bf)) {
		if (bf->buf < bf->bufe) {
			++bf->buf;
			return *(unsigned char *)(bf->buf - 1);
		}
		return(-1);	/* end-of-buffer */
	}
	// else
	return(fgetc(bf->f));
}

/*** helper function: write a byte to the destination file or memory buffer from the bitfile's accumulator ***/
static void __bitfile_putc(bitfile* bf) {
	if (BF_IS_RAM(bf)) {
		if (bf->buf < bf->bufe) {
			*bf->buf = (char)(bf->acc);
			++bf->buf;
		}
		// TODO: we currently have silent failure when a RAM bitfile reaches the end of its buffer. No security risk, but potentially annoying.
	} else {
		fputc(bf->acc, bf->f);
	}
}

/*** Opens a disk file to read/write using bit I/O. ***/
void bitfile_open(bitfile* bf, const char* fname, bool is_write) {
	bf->f = fopen(fname, (is_write ? "wb" : "rb"));
	bf->buf = NULL;
	bf->bufe = NULL;
	bf->bufs = NULL;
	bf->acc = 0UL;
	bf->acc_c = 0UL;
	bf->opt = (is_write ? BITFILE_IS_WRITE : BITFILE_NONE);
}

/*** Opens a bitfile that will read or write to a memory buffer. ***/
void bitfile_open_mem(bitfile* bf, char* buf, u32 bufsize, bool is_write) {
	bf->f = NULL;
	bf->buf = bf->bufs = buf;
	bf->bufe = buf + bufsize;
	bf->acc = 0UL;
	bf->acc_c = 0UL;
	bf->opt = (is_write ? BITFILE_IS_WRITE : BITFILE_NONE) | BITFILE_IS_RAM;
}

/*** Closes the bitfile. ***/
void bitfile_close(bitfile* bf) {
	/* flush the last byte if needed */
	if (BF_IS_WRITE(bf) && bf->acc_c != 0UL) {
		__bitfile_putc(bf);
		bf->acc = 0UL;
		bf->acc_c = 0UL;
	}
	if (!BF_IS_RAM(bf))
		fclose(bf->f);
}

/*** Writes a single bit to the bitfile. ***/
void bitfile_writebit(bitfile* bf, bool on) {
	if (!BF_IS_WRITE(bf))
		return;
	bf->acc += bf->acc;
	if (on)
		bf->acc |= 1UL;
	bf->acc_c++;
	if (8 == bf->acc_c) {
		__bitfile_putc(bf);
		bf->acc = 0UL;
		bf->acc_c = 0UL;
	}
}

/*** Writes a single byte to the bitfile. ***/
void bitfile_writebyte(bitfile* bf, u32 byte) {
	int t = 128UL;
	while (t) {
		bitfile_writebit(bf, ((byte & t) != 0));
		t >>= 1;
	}
}

/*** Writes a 32-bit unsigned integer to the bitfile. ***/
void bitfile_write32(bitfile* bf, u32 val) {
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
}

/*** Writes a 64-bit unsigned integer to the bitfile. ***/
extern void bitfile_write64(bitfile* bf, u64 val) {
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
	val >>= 8;
	bitfile_writebyte(bf, val & 0xff);
}

/*** Reads a bit from the bitfile. Returns -1 on EOF. ***/
int bitfile_readbit(bitfile* bf) {
	int ret;
	if (0UL == bf->acc_c)
		{
		bf->acc = __bitfile_getc(bf);
		if (bf->acc < 0)		/* EOF */
			return(bf->acc);
		bf->acc_c = 128UL;
		}
	ret = ((bf->acc & bf->acc_c) ? 1 : 0);
	bf->acc_c >>= 1;
	return(ret);
}

/*** Reads an (unsigned) byte from the bitfile. Returns -1 on EOF. ***/
int bitfile_readbyte(bitfile* bf) {
	int t = 128UL;
	int ret = 0UL;
	while (t) {
		int b;
		b = bitfile_readbit(bf);
		if (b < 0)
			return(b);
		if (b)
			ret |= t;
		t >>= 1;
	}
	return(ret);
}

/*** Reads a 32 bit integer from the bitfile. Sets the is_eof flag to true on EOF, if is_eof is non-NULL. ***/
u32 bitfile_read32(bitfile* bf, bool* is_eof) {
	int r;
	u32 ret = 0UL;
	
	if (not_null(is_eof))
		*is_eof = false;
	r = bitfile_readbyte(bf);
	if (unlikely(r < 0)) {
LEOF:		if (not_null(is_eof))
			*is_eof = true;
		return(-1);
	}
	ret = r;

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 8);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 16);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 24);

	return(ret);
}

/*** Reads a 64 bit integer from the bitfile. Sets the is_eof flag to true on EOF, if is_eof is non-NULL. ***/
u64 bitfile_read64(bitfile* bf, bool* is_eof) {
	u64 ret = 0UL;
	u64 r;
		
	if (not_null(is_eof))
		*is_eof = false;
	r = bitfile_readbyte(bf);
	if (unlikely(r < 0)) {
LEOF:		if (not_null(is_eof))
			*is_eof = true;
		return(-1);
	}
	ret = r;
	
	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 8);
	
	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 16);
	
	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 24);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 32);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 40);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 48);

	r = bitfile_readbyte(bf);
	if (unlikely(r < 0))	goto LEOF;
	ret |= (r << 56);
	
	return(ret);
}

/*** Class wrapper for bitfile ***/

BitFile::BitFile() {
	this->init_ = false;
}

BitFile::BitFile(const char* filename, bool is_write) {
	this->open(filename, is_write);
}

BitFile::~BitFile() {
	this->close();
}

void BitFile::open(const char* fname, bool is_write) {
	bitfile_open(&bf_, fname, is_write);
	init_ = true;
}

void BitFile::open_mem(char* buf, u32 bufsize, bool is_write) {
	bitfile_open_mem(&bf_, buf, bufsize, is_write);
	init_ = true;
}

void BitFile::close(void) {
	if (!init_)
		return;
	
	bitfile_close(&bf_);
	init_ = false;
}

void BitFile::write(char byte) {
	bitfile_writebyte(&bf_, byte);
}

void BitFile::write(u32 ui32) {
	bitfile_write32(&bf_, ui32);
}

void BitFile::write(u64 ui64) {
	bitfile_write64(&bf_, ui64);
}

void BitFile::writebit(bool on) {
	bitfile_writebit(&bf_, on);
}

void BitFile::writebyte(u32 byte) {
	bitfile_writebyte(&bf_, byte);
}

void BitFile::write32(u32 val) {
	bitfile_write32(&bf_, val);
}

void BitFile::write64(u64 val) {
	bitfile_write64(&bf_, val);
}

int BitFile::readbit(void) {
	if (!init_)
		return(-1);
	return bitfile_readbit(&bf_);
}

int BitFile::readbyte(void) {
	if (!init_)
		return(-1);
	return bitfile_readbyte(&bf_);
}

u32 BitFile::read32(bool* is_eof) {
	if (!init_) {
		if (not_null(is_eof))
			*is_eof = true;
		return(0UL);
	}
	return bitfile_read32(&bf_, is_eof);
}

u64 BitFile::read64(bool* is_eof) {
	if (!init_) {
		if (not_null(is_eof))
			*is_eof = true;
		return(0UL);
	}
	return bitfile_read64(&bf_, is_eof);
}

/*** end bitfile.cpp ***/