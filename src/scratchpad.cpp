/***

	scratchpad.cpp

	Scratchpad objects.

	Need a dynamic memory buffer to hold string input from untrusted sources (file, user input)? Don't know what
	size you need ahead of time? Create a scratchpad object to handle it for you.

	These are used throughout libcodehappy for various purposes (keyboard buffers, ramfile data, etc.)

	Copyright (c) 2014-2022 Chris Street

***/

#include "libcodehappy.h"

#define	MIN_ALLOC	32UL

Scratchpad::Scratchpad() {
	ialloc = 0;
	buf = cend = bend = nullptr;
}

Scratchpad::Scratchpad(u32 nbytes) {
	ialloc = 0;
	buf = cend = bend = nullptr;

	if (nbytes <= 0)
		return;
	if (nbytes < MIN_ALLOC)
		nbytes = MIN_ALLOC;

	buf = new u8 [nbytes];
	if (is_null(buf))
		return;
	ialloc = nbytes;
	cend = buf;
	bend = buf + nbytes;
}

Scratchpad::~Scratchpad() {
	free();
}

void Scratchpad::free(void) {
	if (buf != nullptr) {
		delete [] buf;
	}
	ialloc = 0;
	cend = bend = buf = nullptr;
}

int Scratchpad::addc(u8 ch) {
	if (is_null(buf)) {
		if (realloc(MIN_ALLOC))
			return(1);
	} else if (cend + 1 >= bend) {
		u32 newc = smallest_pow2_greater_than(ialloc);
		if (newc < MIN_ALLOC)
			newc = MIN_ALLOC;
		if (realloc(newc))
			return(1);
	}

	*cend = ch;
	++cend;
	*cend = '\000';

	return(0);
}

void Scratchpad::addle_u16(u16 val) {
	u16 v = CPU_TO_LE16(val);
	this->memcat((u8*)&v, (u32)sizeof(u16));
}

void Scratchpad::addle_u32(u32 val) {
	u32 v = CPU_TO_LE32(val);
	this->memcat((u8*)&v, (u32)sizeof(u32));
}

void Scratchpad::addle_u64(u64 val) {
	u64 v = CPU_TO_LE64(val);
	this->memcat((u8*)&v, (u32)sizeof(u64));
}

void Scratchpad::addle_16(i16 val) {
	i16 v = CPU_TO_LE16(val);
	this->memcat((u8*)&v, (u32)sizeof(i16));
}

void Scratchpad::addle_32(i32 val) {
	i32 v = CPU_TO_LE32(val);
	this->memcat((u8*)&v, (u32)sizeof(i32));
}

void Scratchpad::addle_64(i64 val) {
	i64 v = CPU_TO_LE64(val);
	this->memcat((u8*)&v, (u32)sizeof(i64));
}

int Scratchpad::insertch(u8 ch, u32 pos) {
	realloc(MIN_ALLOC);
	NOT_NULL_OR_RETURN(buf, 1);
	// ensure that we have enough room
	if (addc(0))
		return(1);
	if (pos > cend - buf)
		pos = cend - buf;
	if ((cend - (buf + pos)) + 1 > 0)
		memmove(buf + pos + 1, buf + pos, (cend - (buf + pos)) + 1);
	buf[pos] = ch;
	return(0);
}

int Scratchpad::delbytes(u32 nb, u32 pos) {
	NOT_NULL_OR_RETURN(buf, 1);
	if (iszero(nb))
		return 0;

	u8* from, *end;

	from = buf + pos;
	end = from + nb;

	if (from > cend)
		return 1;
	if (end > cend)
		return 1;

	memmove(from, end, (cend - end) + 1);
	cend -= nb;

	return 0;
}

int Scratchpad::delfromend(u32 nb) {
	NOT_NULL_OR_RETURN(buf, 1);
	if (iszero(nb))
		return 0;
	cend -= nb;
	if (cend < buf)
		cend = buf;
	*cend = '\000';
	return 0;
}

int Scratchpad::doublestr(void) {
	u32 sz = cend - buf;
	u8* savbuf;

	if (sz + sz + 1 < ialloc) {
		// don't need a realloc
		::strncpy((char *)cend, (char *)buf, sz);
		cend += sz;
		*cend = '\000';
		return(0);
	}

	savbuf = new u8 [ialloc + ialloc];
	if (is_null(savbuf))
		return(1);

	::strncpy((char*)savbuf, (char*)buf, sz);
	::strncpy((char*)savbuf + sz, (char*)buf, sz);
	savbuf[sz + sz] = '\000';

	delete [] buf;
	ialloc += ialloc;
	buf = savbuf;
	bend = savbuf + ialloc;
	cend = savbuf + sz + sz;

	return(0);
}

int Scratchpad::strcat(const char* str, u32 len) {
	u32 newc;

	if (is_null(buf)) {
		newc = smallest_pow2_greater_than(len);
		if (realloc(newc))
			return(1);
	} else if ((const u8*)str == buf) {
		return doublestr();
	} else if (cend + len + 1 >= bend) {
		newc = smallest_pow2_greater_than(ialloc + len + 1);
		if (realloc(newc))
			return(1);
	}

	::strcpy((char *)cend, str);
	cend += len;

	return(0);
}

int Scratchpad::strcat(const char* str) {
	return strcat(str, strlen(str));
}

int Scratchpad::memcat(const u8* mem, u32 nbytes) {
	u32 newc;
	if (is_null(buf)) {
		newc = smallest_pow2_greater_than(nbytes);
		if (realloc(newc))
			return(1);
		memcpy(buf, mem, nbytes);
		cend += nbytes;
		return(0);
	}
	if (cend + nbytes >= bend) {
		newc = smallest_pow2_greater_than(ialloc + nbytes);
		if (realloc(newc))
			return(1);
	}
	memcpy(cend, mem, nbytes);
	cend += nbytes;
	*cend = 0;
	return(0);
}

int Scratchpad::concat(const Scratchpad* sp) {
	if (nullptr == sp)
		return(0);	/* no-op */

	return this->memcat(sp->buffer(), sp->length());
}

void Scratchpad::update_str(void) {
	cend = buf + strlen((char *)buf);
}

int Scratchpad::realloc(u32 nbytes) {
	u32 offs;
	u8* newbuf;

	if (nbytes <= ialloc)
		return(0);

	if (is_null(buf))
		offs = 0;
	else
		offs = cend - buf;

	if (nbytes < MIN_ALLOC)
		nbytes = MIN_ALLOC;

	newbuf = new u8 [nbytes];
	NOT_NULL_OR_RETURN(newbuf, 1);
	if (!is_null(buf))
		memcpy(newbuf, buf, length());
	newbuf[length()] = '\000';


	if (!is_null(buf))
		delete [] buf;
	buf = newbuf;
	cend = newbuf + offs;
	bend = newbuf + nbytes;
	ialloc = nbytes;

	return(0);
}

int Scratchpad::compact(void) {
	u32 nc = length();
	u8 *newbuf;

	if (0 == nc) {
		cend = bend = nullptr;
		ialloc = 0;
		if (buf)
			delete [] buf;	
		buf = nullptr;
		return(0);
	}

	newbuf = new u8 [nc + 1];
	memcpy(newbuf, buf, nc + 1);
	delete [] buf;
	buf = newbuf;
	ialloc = nc + 1;
	cend = newbuf + nc;
	bend = newbuf + ialloc;

	return(0);
}

int Scratchpad::strcpy(const char* str) {
	const u32 needed = strlen(str) + 1;
	/* this is a no-op if we already have enough memory */
	if (realloc(needed) != 0)
		return(1);
	::strcpy((char *)buf, str);
	cend = buf + needed - 1;
	return(0);
}

int Scratchpad::strncpy(const char* str, u32 nchar) {
	if (realloc(nchar + 1) != 0)
		return(1);
	::strncpy((char *)buf, str, nchar);
	buf[nchar] = '\000';
	cend = buf + strlen((char *)buf);
	return(0);
}

int Scratchpad::getch(void) {
	int ret;
	int cpysize;
	
	NOT_NULL_OR_RETURN(buf, -1);
	if (cend == buf)
		return(-1);

	ret = buf[0];
	cpysize = (cend - buf) - 1;
	if (cpysize > 0)
		memcpy(buf, buf + 1, cpysize);
	--cend;

	return(ret);
}

void Scratchpad::swap(Scratchpad* sp) {
	u32 ia = ialloc;
	u8* b = buf;
	u8* ce = cend;
	u8* be = bend;
	if (is_null(sp))
		return;
	ialloc = sp->ialloc;
	buf = sp->buf;
	cend = sp->cend;
	bend = sp->bend;
	sp->ialloc = ia;
	sp->buf = b;
	sp->cend = ce;
	sp->bend = be;
}

u8* Scratchpad::relinquish_buffer(void) {
	u8* ret = buf;
	ialloc = 0;
	buf = cend = bend = nullptr;
	return(ret);
}

void Scratchpad::clear() {
	cend = buf;
}

void Scratchpad::give_static_buffer(u8* buf_in, u32 buflen) {
	buf = buf_in;
	ialloc = buflen;
	cend = buf;
	bend = buf + buflen;
}

/* end scratchpad.cpp */
