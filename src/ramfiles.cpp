/***

	ramfiles.cpp

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

#include "libcodehappy.h"

#define	RAMFILE_WRITE_OPTIONS		(RAMFILE_WRITE_APPEND | RAMFILE_WRITE_OVERWRITE | RAMFILE_WRITE_INSERT)

/*** this is set by the RAM file code internally, to indicate a file that was found to be compressed ***/
#define	RAMFILE_WAS_COMPRESSED		256

/*** the RAM file is made from an embedded RAM file; we didn't allocate the scratchpad buffer ***/
#define	RAMFILE_EMBEDDED		512

/*** magic numbers to designate compressed ramfiles ***/
static unsigned char __magic_compress_ramfiles[12] =
	{'R', 'A', 'M', '\004', 0x10, 0x89, 0x09, 0x19, 0x80, 0x09, 0x11, 0x58};

#define VERSION_LZ	1
#define VERSION_ZLIB	2

static int compress_version_from_magic(char* buf) {
	if (!strncmp(buf, (char *)__magic_compress_ramfiles, sizeof(__magic_compress_ramfiles))) {
		return VERSION_LZ;
	}
	if (strncmp(buf, "RAM", 3)) {
		return 0;
	}
	if (buf[3] == 0x7f) {
		if (!strncmp(buf + 4, (char *)(&__magic_compress_ramfiles[4]), sizeof(__magic_compress_ramfiles) - 4)) {
			return VERSION_ZLIB;
		}
	}
	return 0;
}

static void write_magic_version(FILE* f, int version) {
	switch (version) {
	default:
	case VERSION_LZ:
		fwrite(__magic_compress_ramfiles, sizeof(char), sizeof(__magic_compress_ramfiles), f);
		break;
	case VERSION_ZLIB:
		fwrite(__magic_compress_ramfiles, sizeof(char), 3, f);
		fputc(0x7f, f);
		fwrite(__magic_compress_ramfiles + 4, sizeof(char), sizeof(__magic_compress_ramfiles) - 4, f);
		break;
	}
}

RamFile::RamFile() {
	fname = nullptr;
	readp = nullptr;
	options = 0UL;
}

RamFile::RamFile(const char* fn, u32 opt) {
	fname = nullptr;
	readp = nullptr;
	options = 0UL;
	open(fn, opt);
}

RamFile::RamFile(const std::string& fn, u32 opt) {
	fname = nullptr;
	readp = nullptr;
	options = 0UL;
	open(fn, opt);
}

RamFile::RamFile(const EmbeddedRamFile erf) {
	const u64* bufst = &erf[1];
	u32 len = u32(erf[0]);
	fname = nullptr;
	options = RAMFILE_READONLY | RAMFILE_EMBEDDED;
	sp.clear();
	sp.memcat((u8 *)bufst, len);
	readp = sp.buffer();
	decompress();
}

RamFile::~RamFile() {
	close();
}

int RamFile::open(const std::string& fn, u32 opt) {
	return open(fn.c_str(), opt);
}

int RamFile::open(const char* fn, u32 opt) {
	FILE* f;
	bool exists = FileExists(fn);
	u32 flen;

	if (fname != nullptr || sp.length() > 0) {
		close();
	}
	
	options = opt;
	if (!exists && falsity(options & RAMFILE_CREATE_IF_MISSING))
		return(1);
	if ((options & (RAMFILE_WRITE_OPTIONS)) == 0)
		options |= RAMFILE_WRITE_OVERWRITE;

	/* must have exactly one of the write options set  */
	switch (options & RAMFILE_WRITE_OPTIONS)
		{
	case RAMFILE_WRITE_OVERWRITE:
	case RAMFILE_WRITE_APPEND:
	case RAMFILE_WRITE_INSERT:
		break;
	default:
		return 1;
		}
	
	fname = NEW_STR(strlen(fn));
	if (is_null(fname))
		return(1);
	strcpy(fname, fn);

	if (exists) {
		f = fopen(fname, "rb");
		flen = filelen(f);
		if (sp.realloc(flen + 2)) {
			fclose(f);
			delete [] fname;
			return 1;
		}
		fread(sp.buffer(), sizeof(char), flen, f);
		sp.cend = sp.buf + flen;
		*sp.cend = '\000';
		fclose(f);
	} else {
		sp.realloc(32);
	}

	readp = sp.buffer();

	if ((options & RAMFILE_IGNORE_COMPRESSION) == 0) {
		/* if this ramfile was compressed by us, decompress the buffer */
		decompress();
	}
	return(0);
}

void RamFile::decompress(void) {
	int version = compress_version_from_magic((char *)sp.buf);
	if (version > 0) {
		Scratchpad* sp2;
		u32 needed;
		u8* compress_data_start;

		options |= RAMFILE_WAS_COMPRESSED;
		options |= RAMFILE_COMPRESS;
			
		needed = *((u32 *)(sp.buf + sizeof(__magic_compress_ramfiles)));
		needed = LE32_TO_CPU(needed);
		compress_data_start = (sp.buf + sizeof(__magic_compress_ramfiles) + sizeof(u32));

		sp2 = new Scratchpad(needed);
		switch (version) {
		case VERSION_LZ:
		default:
			if (is_null(sp2) ||
				lzf_decompress(compress_data_start, length() - (sizeof(__magic_compress_ramfiles) + sizeof(uint32_t)), sp2->buf, needed) !=
				needed) {
LDecompErr:			delete sp2;
				sp.free();
				if (not_null(fname))
					delete [] fname;
				return;
			}
			break;

		case VERSION_ZLIB:
			if (is_null(sp2))
				goto LDecompErr;
			if (mz_uncompress((unsigned char*)sp2->buf, (mz_ulong *)&needed, 
					  (const unsigned char*)compress_data_start, (mz_ulong)(length() - (sizeof(__magic_compress_ramfiles) + sizeof(uint32_t)))) != MZ_OK) {
				goto LDecompErr;
			}
			break;
		}

		sp2->cend = sp2->buf + needed;

		sp.swap(sp2);
		delete sp2;
		readp = sp.buffer();
	}
}

void RamFile::close(void)
{
	flush();
	sp.free();
	if (not_null(fname))
		delete [] fname;
	fname = nullptr;
	readp = nullptr;
}

void RamFile::rewind(void) {
	readp = sp.buffer();
}

void RamFile::truncate() {
	sp.clear();
	rewind();
}

bool RamFile::seek(u64 pos) {
	if (sp.buffer() == nullptr)
		return false;
	if (pos >= length())
		return(false);

	readp = sp.buf + pos;
	return(true);
}

bool RamFile::seek_from_end(u64 pos) {
	u8* nreadp;

	if (is_null(sp.buffer()))
		return false;
	
	nreadp = sp.cend - pos;
	if (nreadp < sp.buf)
		return(false);

	readp = nreadp;
	return(true);
}

bool RamFile::seek_from_here(i64 pos) {
	u8* nreadp;
	
	if (is_null(sp.buffer()))
		return (false);

	nreadp = readp + pos;
	if (nreadp < sp.buf)
		return(false);
	if (nreadp >= sp.cend)
		return(false);

	readp = nreadp;
	return(true);
}


u32 RamFile::length(void) const {
	if (sp.buffer() == nullptr)
		return 0UL;
	return (sp.cend - sp.buf);
}

void RamFile::flush(void)
{
	FILE* f;
	if (is_null(fname))
		return;

	if ((options & (RAMFILE_READONLY | RAMFILE_STATIC)) != 0)
		return;

	if ((options & RAMFILE_COMPRESS) != 0)
		{
		/* compress the buffer and write it out */
		/* if we fail to allocate a buffer for the compressed data, or the data doesn't compress,
			just write it out raw. */
		char* compress_buffer, *cb2;
		u32 cbuf_len = length() - sizeof(__magic_compress_ramfiles) - sizeof(uint32_t) - 1;
		unsigned int out_len;
		u32 endian;
		int version = 0;
		
		compress_buffer = new char [cbuf_len];
		if (is_null(compress_buffer))
			goto LRaw;
		out_len = lzf_compress(sp.buf, length(), compress_buffer, cbuf_len);
		if (out_len > 0) {
			version = VERSION_LZ;
		}

		/* Try zLib compression, too. */
		cb2 = new char [cbuf_len];
		if (!is_null(cb2)) {
			mz_ulong dest_len = cbuf_len;
			if (mz_compress((unsigned char*)cb2, &dest_len, (const unsigned char*)sp.buf, (mz_ulong)length()) == MZ_OK) {
				if (version == 0 || dest_len < out_len) {
					delete [] compress_buffer;
					compress_buffer = cb2;
					cb2 = nullptr;
					out_len = dest_len;
					version = VERSION_ZLIB;
				}
			}
		}
		if (!is_null(cb2)) {
			delete [] cb2;
		}
		if (0 == version) {
			/* Neither LZF nor zLib successfully compressed the buffer. */
			delete [] compress_buffer;
			goto LRaw;
		}

		/* output the compressed payload. Write the length of the uncompressed data as a little-endian 32 bit integer. */
		f = fopen(fname, "wb");
		write_magic_version(f, version);
		endian = CPU_TO_LE32(length());
		fwrite(&endian, sizeof(endian), 1, f);
		fwrite(compress_buffer, sizeof(char), out_len, f);
		fclose(f);
		delete [] compress_buffer;
	} else {
LRaw:		/* write out the raw buffer */
		f = fopen(fname, "wb");
		fwrite(sp.buf, sizeof(char), sp.cend - sp.buf, f);
		fclose(f);
		}
}

void RamFile::open_static(char* data, uint dlen, uint32_t opt) {
	/* static RAM files should have specific set of options */
	options = opt;
	options |= RAMFILE_STATIC;
	options &= (~(RAMFILE_COMPRESS | RAMFILE_WRITE_APPEND | RAMFILE_WRITE_INSERT));
	options |= RAMFILE_IGNORE_COMPRESSION;
	options |= RAMFILE_WRITE_OVERWRITE;
	
	fname = nullptr;

	// The Scratchpad object takes ownership of the memory buffer.
	sp.free();
	sp.buf = (u8*)data;
	sp.cend = (u8*)data + dlen;
	sp.bend = (u8*)data + dlen;
	sp.ialloc = dlen;
	readp = sp.buffer();
}

int RamFile::getc(void)
{
	unsigned int ret;

	if (is_null(readp))
		return(-1);
	
	if (readp >= sp.cend)
		return(-1);

	ret = *(unsigned char*)readp;
	++readp;
	return((int)ret);
}

/***
	Writes a byte to the RAM file with the current write
	options. Returns 0 on success.
***/
int RamFile::putc(int c)
{
	u32 offset;

	if (truth(options & RAMFILE_READONLY))
		return(1);

	/* never allow writing off the end of a static RAM file buffer */
	if (unlikely(truth(options & RAMFILE_STATIC) && readp >= sp.cend))
		return(1);

	c &= 0xff;

	switch (options & RAMFILE_WRITE_OPTIONS)
		{
	case RAMFILE_WRITE_OVERWRITE:
		if (likely(not_null(sp.cend) && readp < sp.cend)) {
			*(readp) = c;
			++readp;
		} else {
			u32 roff;
			if (is_null(sp.buf))
				roff = 0UL;
			else
				roff = readp - sp.buf;
			if (sp.addc(c) != 0)
				return(-1);
			readp = sp.buf + roff;
			++readp;
			}
		break;
	case RAMFILE_WRITE_APPEND:
		// note that a write in append mode doesn't change the read pointer
		if (is_null(sp.buf))
			offset = 0;
		else
			offset = readp - sp.buf;
		if (unlikely(sp.addc(c) != 0))
			return(-1);
		readp = sp.buf + offset + 1;
		break;
	case RAMFILE_WRITE_INSERT:
		// TODO: implement, this is the most expensive operation the naive way, but there are ways to make this efficient
		TBI();
		return(1);
		}

	return(0);
}

int RamFile::puts(const char* str)
{
	u32 offset;

	if (unlikely((options & RAMFILE_READONLY) != 0))
		return(1);

	switch (options & RAMFILE_WRITE_OPTIONS)
		{
	case RAMFILE_WRITE_OVERWRITE:
		while (*str) {
			if (putc(*str))
				return(-1);
			++str;
		}
		putc(0);
		break;
	case RAMFILE_WRITE_APPEND:
		offset = readp - sp.buf;
		if (sp.strcat(str) != 0)
			return(-1);
		readp = sp.buf + offset;
		break;
	case RAMFILE_WRITE_INSERT:
		// TODO: implement insert mode
		TBI();
		return(1);
		}

	return(0);
}

int RamFile::gets(char* str_dest, u32 buflen) {
	int c;
	const char* bufe = str_dest + buflen;
	zeromem(str_dest, buflen);
	while (str_dest < bufe) {
		c = getc();
		if (c <= 0)
			break;
		*str_dest = c;
		++str_dest;
	}
	str_dest[buflen - 1] = '\000';
	return 0;
}

int RamFile::putmem(const char* data, u32 nbytes)
{
	char *datap = (char *)data;
	const char* datae = data + nbytes;
	int ret;

	ret = 0;
	while (datap < datae) {
		ret = putc(*datap);
		if (ret)
			break;
		++datap;
	}

	return(ret);
}

int RamFile::getmem(u8* to, u32 nb) {
	u8* datae = to + nb;
	int ret;

	while (to < datae) {
		ret = getc();
		if (ret < 0)
			return ret;
		*to = u8(ret);
		++to;
	}

	return 0;
}

int RamFile::putsp(const Scratchpad& sp1)
{
	if (unlikely(truth(options & RAMFILE_READONLY)))
		return(1);

	putu32(sp1.cend - sp1.buf);
	if (sp1.cend - sp1.buf > 0) {
		putmem((char*)sp1.buf, sp1.cend - sp1.buf);
	}

	return(0);
}

int RamFile::getsp(Scratchpad& sp) {
	sp.free();
	u32 len = getu32();
	if (len == 0)
		return 0;
	sp.memcat(readp, len);
	readp += len;
	return 0;
}

u32 RamFile::setname(const char* newname)
{
	if (not_null(fname))
		delete [] fname;
	fname = new char [strlen(newname) + 1];
	if (unlikely(is_null(fname)))
		return(1);

	strcpy(fname, newname);
	return(0);
}

bool RamFile::compressed(void) const {
	return truth(options & RAMFILE_WAS_COMPRESSED);
}

void RamFile::option_on(u32 opt)
{
	// opt must be a single bit
	if (unlikely(!ispow2(opt)))
		return;
	
	// do not allow twiddling of internal states
	if (unlikely(RAMFILE_INTERNAL <= opt))
		return;

	// we must have only one write mode on at a time
	switch (opt) {
	case RAMFILE_WRITE_OVERWRITE:
	case RAMFILE_WRITE_APPEND:
	case RAMFILE_WRITE_INSERT:
		options &= (~(RAMFILE_WRITE_OPTIONS));
		break;
	}
	options |= opt;
}

void RamFile::option_off(u32 opt)
{
	// opt must be a single bit
	if (unlikely(!ispow2(opt)))
		return;

	// do not allow twiddling of internal states
	if (unlikely(RAMFILE_INTERNAL <= opt))
		return;

	options &= (~(opt));
	if ((options & RAMFILE_WRITE_OPTIONS) == 0)
		options |= RAMFILE_WRITE_OVERWRITE;

	// TODO: enforce flags for static RAM files
}

int RamFile::getline(Scratchpad& sp_out) {
	/* check for initial EOF */
	if (readp >= sp.cend)
		return(-1);
	
	u8* next_line, *ws;
	char sv;
	/* don't use advance_to_next_line() here, since there may be zero bytes in
		the scratchpad buffer. */
	// TODO: shouldn't there be a scratchpad_to_next_line() function?
	next_line = readp;
	if (readp >= sp.cend)
		return(-1);
	while (next_line < sp.cend)
		{
		if (*next_line == '\n') {
			++next_line;
			if (*next_line == '\r')
				++next_line;
			break;
		} else if (*next_line == '\r') {
			++next_line;
			if (*next_line == '\n')
				++next_line;
			break;
		}
		++next_line;
	}
	ws = next_line;
	--ws;
	while (ws >= readp && (*ws == '\n' || *ws == '\r'))
		--ws;
	++ws;
	sv = *ws;
	*ws = 0;

	sp_out.strcpy((const char*)readp);

	*ws = sv;
	readp = next_line;

	return(0);
}

u8* RamFile::buffer(void) const {
	NOT_NULL_OR_RETURN(sp.buf, NULL);
	return sp.buf;
}

u64 RamFile::get_u64(void) {
	u64 ret;
	NOT_NULL_OR_RETURN(sp.buf, 0ULL);
	while (!eof() && isspace(*readp))
		++readp;
	if (eof())
		return 0;
	ret = strtoull((char *)readp, nullptr, 10);
	while (!eof() && isdigit(*readp))
		++readp;
	while (!eof() && isspace(*readp))
		++readp;
	return ret;
}

bool RamFile::eof(void) const {
	return readp >= sp.cend;
}

u64 RamFile::getu64(void) {
	if (is_null(readp) || readp + 7 >= sp.cend)
		return 0;
	u64 ret = LE64_TO_CPU((*((u64*)readp)));
	readp += 8;
	return ret;
}

i64 RamFile::get64(void) {
	if (is_null(readp) || readp + 7 >= sp.cend)
		return 0;
	i64 ret = LE64_TO_CPU((*((i64*)readp)));
	readp += 8;
	return ret;
}

u32 RamFile::getu32(void) {
	if (is_null(readp) || readp + 3 >= sp.cend)
		return 0;
	u32 ret = LE32_TO_CPU((*((u32*)readp)));
	readp += 4;
	return ret;
}

i32 RamFile::get32(void) {
	if (is_null(readp) || readp + 3 >= sp.cend)
		return 0;
	i32 ret = LE32_TO_CPU((*((i32*)readp)));
	readp += 4;
	return ret;
}

u16 RamFile::getu16(void) {
	if (is_null(readp) || readp + 1 >= sp.cend)
		return 0;
	u16 ret = LE16_TO_CPU((*((u16*)readp)));
	readp += 2;
	return ret;
}

i16 RamFile::get16(void) {
	if (is_null(readp) || readp + 1 >= sp.cend)
		return 0;
	i16 ret = LE16_TO_CPU((*((i16*)readp)));
	readp += 2;
	return ret;
}

int RamFile::putu64(u64 u) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_u64(u);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(u64);
	return 0;
}

int RamFile::put64(i64 i) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_64(i);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(i64);
	return 0;
}

int RamFile::putu32(u32 u) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_u32(u);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(u32);
	return 0;
}

int RamFile::put32(i32 i) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_32(i);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(i32);
	return 0;
}

int RamFile::putu16(u16 u) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_u16(u);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(u16);
	return 0;
}

int RamFile::put16(i16 i) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.addle_16(i);
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(i16);
	return 0;
}

double RamFile::getdouble(void) {
	if (is_null(readp) || readp + sizeof(double) > sp.cend)
		return 0.;
	double ret = *((double*)readp);
	readp += sizeof(double);
	return ret;
}

float RamFile::getfloat(void) {
	if (is_null(readp) || readp + sizeof(float) > sp.cend)
		return 0.;
	float ret = *((float*)readp);
	readp += sizeof(float);
	return ret;
}

int RamFile::putdouble(double v) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.memcat((u8*)(&v), sizeof(double));
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(double);
	return 0;
}

int RamFile::putfloat(float v) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	u32 offset = readp - sp.buf;
	sp.memcat((u8*)(&v), sizeof(float));
	NOT_NULL_OR_RETURN(sp.buf, 1);
	readp = sp.buf + offset + sizeof(float);
	return 0;
}

bool RamFile::getbool(void) {
	int c = this->getc();
	return (c == 1);
}

int RamFile::putbool(bool v) {
	return this->putc(v ? 1 : 0);
}

int RamFile::putstring(const std::string& s) {
	if (truth(options & RAMFILE_READONLY))
		return(1);
	putu32(s.length());
	for (int e = 0; e < s.length(); ++e) {
		int i = ((unsigned char) s[e]);
		putc(i);
	}
	return 0;
}

int RamFile::putstring(const char* s) {
	if (is_null(s)) {
		putu32(0);
		return 0;
	}
	u32 len = strlen(s);
	putu32(len);
	for (u32 e = 0; e < len; ++e) {
		int i = ((unsigned char) s[e]);
		putc(i);
	}
	return 0;
}

std::string RamFile::getstring() {
	std::string ret;
	u32 len = getu32();
	for (u32 e = 0; e < len; ++e) {
		int c = getc();
		ret.push_back(char(c));
	}
	return ret;
}

void RamFile::embed_ramfile(const char* fname_out, const char* rfname) {
	// The embed is in CPU integer format (little endian on x86/x64/almost all Android.)
	std::ofstream o;
	u64* data, *datae;
	u64 data_last;
	u32 c = 0;
	bool extra;
	char buf[8] = { 0 };

	BUILD_ASSERT(sizeof(buf) <= sizeof(u64));

	data = (u64 *)sp.buffer();
	extra = (length() & 7) != 0;
	datae = data + (length() / sizeof(u64));

	if (extra) {
		char* u64e, *reale;
		reale = ((char*)data) + length();
		u64e = (char *)datae;
		while (u64e < reale) {
			buf[c++] = *u64e;
			++u64e;
		}
		data_last = *((u64 *)buf);
		c = 0;
	}

	o.open(fname_out);

	o << "/*** auto-generated code ***/\n";
	if (is_null(rfname))
		o << "const EmbeddedRamFile ramfile_embed = {\n";
	else
		o << "const EmbeddedRamFile " << rfname << " = {\n";
	o << length() << ", ";
	++c;
	while (data < datae) {
		o << (*data) << ", ";
		++c;
		if (c >= 10) {
			o << "\n";
			c = 0;
		}
		++data;
	}
	if (extra) {
		o << data_last << std::endl;
	}
	o << "};\n";

	o.close();
	o.clear();
}

/*** Peek functions. ***/
int RamFile::peek(void) const {
	int ret;
	if (is_null(readp) || readp >= sp.cend)
		return -1;
	ret = (int)(*(u8*)readp);
	return ret;
}

int RamFile::peeku16(void) const {
	int ret;
	if (is_null(readp) || readp + 1 >= sp.cend)
		return -1;
	ret = (int)(*(u16*)readp);
	return ret;
}

u32 RamFile::peeku32(void) const {
	u32 ret;
	if (is_null(readp) || readp + 3 >= sp.cend)
		return 0UL;
	ret = (*(u32*)readp);
	return ret;
}

void RamFile::write_to_file(const std::string& fname) const {
	write_to_file(fname.c_str());
}

void RamFile::write_to_file(const char* fname) const {
	FILE* f;
	f = fopen(fname, "wb");
	fwrite(sp.buf, 1, sp.cend - sp.buf, f);
	fclose(f);
}

u8* RamFile::relinquish_buffer() {
	flush();

	u8* ret = sp.relinquish_buffer();
	if (not_null(fname))
		delete [] fname;
	fname = nullptr;
	readp = nullptr;

	return ret;
}

/* end ramfiles.cpp */
