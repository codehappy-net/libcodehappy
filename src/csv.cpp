/***

	csv.cpp

	Library for reading and writing CSV files.

	Includes C++ class CsvFile.

	Now for libcodehappy!

	Copyright (c) 2022 C. M. Street

***/

#include "libcodehappy.h"

/*** Take a guess as to whether the string is UTF-32 ***/
bool string_is_utf32(const char* buf, u32 buflen) {
	int cz = 0;
	int ct = min_int(buflen, 400);
	const char* bufe = buf + ct;
	if (ct < 4)
		return(false);
	while (buf < bufe) {
		if (iszero(*buf))
			cz++;
		++buf;
	}

	/* if at least 1/3 the buffer is zeros this is probably UTF-32 */
	return (cz + cz + cz > ct);
}

/*** Helper function: return the end of the data in the cell (ASCII) ***/
char* cell_data_end_ascii(char* buf) {
	char* w = buf;
	bool esc = false;
	while (*w) {
		if (*w == '\"')
			{
			if (*(w + 1) == '\"')
				{
				w += 2;
				continue;
				}
			esc = !esc;
			}
		if (*w == ',' && !esc)
			break;
		if (*w == '\n' && !esc)
			break;
		if (*w == '\r' && !esc)
			break;
		++w;
		continue;
	}
	return(w);
}

/*** Helper function: return the data type of the cell (ASCII) ***/
static int cell_datatype_ascii(char* buf) {
	char* w = buf;
	int ce = 0;
	int cs = 0;
	int cp = 0;
	int cd = 0;
	int cch = 0;
	bool digit_seen = false;

	while (*w) {
		if (*w == '\"')
			return CSV_STR;
		if (*w == ',')
			break;
		++cch;
		// TODO: 'e' is commented out, need to make floating-point recognition more robust
		if (!isspace(*w) && !isdigit(*w) && (*w != '-' && *w != '+' && *w != '.' /* && tolower(*w) != 'e' */))
			return CSV_STR;
		if (isdigit(*w))
			{
			++cd;
			digit_seen = true;
			}
		if (*w == '.')
			++cp;
		if (tolower(*w) == 'e')
			++ce;
		if (*w == '+' || *w == '-')
			{
			// Interior plus or minus signs are OK for numbers only if we know this is scientific notation.
			if (digit_seen && (ce == 0))
				return CSV_STR;
			++cs;
			}
		++w;
	}

	if (ce > 1 || ce > 1 || cs > 1 || cd == 0)
		return CSV_STR;

	if (iszero(cch))
		return CSV_VOID;

	if (ce == 0 && cp == 0)
		// TODO: check for numeric value greater than 2^63 here
		return CSV_INT;

	// TODO: check for a string like ".+E2"

	return CSV_DOUBLE;
}

/*** Helper function: read CSV in UTF-32 encoding ***/
static void csv_read_utf32(csvdata* data, ustring buf, bool has_headers) {
	// TODO: implement UTF-32 CSV support
}

/*** Helper function: Un-escape a CSV cell string inplace in 8-bit encoding ***/
static void csv_unescape_ascii(char* str) {
	char* w;
	w = str;
	forever {
		w = strchr(w, '\"');
		if (is_null(w))
			break;
		OUR_MEMCPY(w, w + 1, strlen(w));
		if (*w == '\"')
			++w;
	}
}

/*** Helper function: read CSV headers in 8-bit encoding ***/
static char* csv_read_headers_ascii(csvdata* data, char* buf) {
	stru hdr;
	char*w, *w2;

	w = buf;
	forever {
		char sv;
		
		w2 = cell_data_end_ascii(w);
		sv = *w2;
		*w2 = 0;

		hdr.ascii = NEW_STR(strlen(w) + 1);
		strcpy(hdr.ascii, w);
		csv_unescape_ascii(hdr.ascii);

		darray_append(data->hdrdata, hdr);

		w = w2;
		*w = sv;
		if (*w == ',')
			++w;
		else
			w = advance_past_whitespace(w);
		if (sv == 0 || sv == '\n' || sv == '\r')
			break;
	}

	return(w);
}

/*** Helper function: read CSV cell in 8-bit encoding ***/
static char* csv_read_cell_ascii(char* buf, csvcell** cell) {
	bool esc = false;
	Scratchpad* sp;
	
	if (!(*buf)) {
		*cell = NULL;
		return(buf);
	}
	if (*buf == '\n' || *buf == '\r') {
		buf = advance_past_whitespace(buf);
		*cell = NULL;
		return(buf);
	}

	(*cell) = NEW(csvcell);

	if (*buf == ',') {
		(*cell)->typ = CSV_VOID;
		++buf;
		return(buf);
	}

	(*cell)->typ = cell_datatype_ascii(buf);

	switch ((*cell)->typ) {
	case CSV_VOID:
LComma:
		forever {
			if (*buf == ',' || *buf == '\n' || *buf == '\r') {
				++buf;
				break;
			}
			if (!(*buf))
				break;
			++buf;
		}
		break;
	case CSV_STR:
		sp = new Scratchpad;
		if (unlikely(is_null(sp))) {
			delete *cell;
			*cell = NULL;
			break;
		}
		forever {
			if (!esc && ((*buf) == ',' || *buf == '\n' || *buf == '\r')) {
				++buf;
				break;
			}
			if (!(*buf))
				break;
			if (*buf == '\"') {
				++buf;
				if (*buf != '\"') {
					esc = !esc;
					continue;
				}
			}
			sp->addc(*buf);
			++buf;
		}
		(*cell)->dat.s = (char*)sp->relinquish_buffer();
		delete sp;
		/* Don't call csv_unescape_ascii() since the quotes were unescaped above. */
		break;
		// TODO: strtof/strtoll?
	case CSV_DOUBLE:
		(*cell)->dat.f = atof(buf);
		goto LComma;
	case CSV_INT:
		(*cell)->dat.i = atoll(buf);
		goto LComma;
		}

	return(buf);
}

/*** Helper function: read CSV row in 8-bit encoding ***/
static char* csv_read_row_ascii(csvdata* data, char* buf) {
	csvrow* rw = NULL;
	csvcell* cell;

	do {
		buf = csv_read_cell_ascii(buf, &cell);
		if (not_null(cell)) {
			if (is_null(rw))
				{
				rw = NEW(csvrow);
				darray_init(rw->data);
				darray_append(data->rowdata, rw);
				}
			darray_append(rw->data, cell);
		}
	} while (not_null(cell));

	if (!(*buf))
		return(NULL);

	return(buf);
}

/*** Helper function: read CSV in 8-bit encoding ***/
static void csv_read_ascii(csvdata* data, char* buf, bool has_headers) {
	int crw = 0;

	if (has_headers)
		buf = csv_read_headers_ascii(data, buf);

	do {
		buf = csv_read_row_ascii(data, buf);
	} while (not_null(buf));
}

/*** Read a CSV file, return the data. ***/
csvdata* csv_read(const char* filename, bool has_headers) {
	RamFile rf(filename, RAMFILE_READ);
	csvdata* data;
	
	data = NEW(csvdata);
	darray_init(data->hdrdata);
	darray_init(data->rowdata);
	data->maxcols = 0UL;
	data->headers = has_headers;
	data->utf32 = string_is_utf32((const char*)rf.buffer(), rf.length());
	if (data->utf32)
		csv_read_utf32(data, (ustring)rf.buffer(), has_headers);
	else
		csv_read_ascii(data, (char*)rf.buffer(), has_headers);

	return(data);
}

/*** Helper function: get the specified row ***/
static csvrow* csv_get_row(csvdata* data, u32 rw) {
	NOT_NULL_OR_RETURN(data, NULL);
	if (rw >= darray_size(data->rowdata))
		return(NULL);
	return darray_item(data->rowdata, rw);
}

/*** Get the data at a specific row and column of the CSV. ***/
csvcell* csv_cell(csvdata* data, u32 row, u32 col) {
	csvrow* rw = csv_get_row(data, row);
	NOT_NULL_OR_RETURN(rw, NULL);
	if (col >= darray_size(rw->data))
		return(NULL);
	return darray_item(rw->data, col);
}


/*** Helper function: write a new line in ASCII. File was opened as binary since we may need to write
	UTF32. Under UNIX this is just \n, Windows it's \r\n. ***/
static void csv_write_newline(FILE* f) {
#ifdef CODEHAPPY_WINDOWS
	fputc('\r', f);
	fputc('\n', f);
#else
	fputc('\n', f);
#endif
}

/*** Helper function: write a CSV-escaped string, 8-bit encoding. ***/
static void csv_write_string_ascii(FILE* f, char* str) {
	bool need_esc = false;
	char* w;
	
	// let's say we need to quote the string if it contains commas or quotation marks.
	w = str;
	while (*w) {
		if (*w == '\"' || *w == ',')
			{
			need_esc = true;
			break;
			}
		++w;
	}
	if (need_esc)
		fputc('\"', f);

	// write the CSV-escaped string
	w = str;
	forever {
		if (!(*w))
			break;
		if (*w == '\"')
			fputc('\"', f);
		fputc(*w, f);
		++w;
	}

	if (need_esc)
		fputc('\"', f);
}

/*** Helper function: write a CSV-escaped string, UTF-32 encoding. ***/
static void csv_write_string_utf32(FILE* f, ustring str) {
	// TODO: implement UTF-32 CSV write.
}

/*** Helper function: write the CSV as UTF-32. ***/
static void csv_write_utf32(FILE* f, csvdata* data) {
	// TODO: implement UTF-32 write.
}

/*** Helper function: write a CSV cell in 8-bit encoding. ***/
static void csv_write_cell_ascii(FILE* f, csvcell* cell) {
	NOT_NULL_OR_RETURN_VOID(cell);
	switch (cell->typ)
		{
	case CSV_VOID:
		return;
	case CSV_INT:
		fprintf(f, "%lld", (long long)cell->dat.i);
		break;
	case CSV_DOUBLE:
		fprintf(f, "%f", cell->dat.f);
		break;
	case CSV_STR:
		csv_write_string_ascii(f, cell->dat.s);
		break;
		}
}

/*** Helper function: write a CSV cell in UTF-32 encoding. ***/
static void csv_write_cell_utf32(FILE* f, csvcell* cell) {
	// TODO: implement UTF-32 write.
}

/*** Helper function: write the CSV as 8-bit encoding (like ASCII). ***/
static void csv_write_ascii(FILE* ff, csvdata* data) {
	int e, f;
	
	if (data->headers) {
		for (e = 0; e < csv_nhdr(data); ++e) {
			if (!iszero(e))
				fputc(',', ff);
			csv_write_string_ascii(ff, darray_item(data->hdrdata, e).ascii);
		}
		csv_write_newline(ff);
	}

	for (e = 0; ; ++e) {
		csvrow* row = csv_get_row(data, e);
		if (is_null(row))
			break;
		for (f = 0; f < csv_ncol(data, e); ++f)
			{
			csvcell *cell;
			if (!iszero(f))
				fputc(',', ff);
			cell = csv_cell(data, e, f);
			csv_write_cell_ascii(ff, cell);
			}
		csv_write_newline(ff);
	}
}

/*** Write the CSV data to file. ***/
void csv_write(const char* filename, csvdata* data) {
	FILE* f;

	f = fopen(filename, "wb");
	NOT_NULL_OR_RETURN_VOID(f);
	if (data->utf32)
		csv_write_utf32(f, data);
	else
		csv_write_ascii(f, data);
	fclose(f);
}

/*** Helper function: return a pointer to the allocated csvcell ***/
static csvcell** csv_get_cell_address(csvdata* data, u32 row, u32 col) {
	csvrow* rw = csv_get_row(data, row);
	NOT_NULL_OR_RETURN(rw, NULL);
	if (col >= darray_size(rw->data))
		return(NULL);
	return &darray_item(rw->data, col);
}

/*** Get the number of rows ***/
uint csv_nrw(csvdata* data) {
	NOT_NULL_OR_RETURN(data, 0UL);
	return darray_size(data->rowdata);
}

/*** Get the number of columns in a given row ***/
uint csv_ncol(csvdata* data, u32 rw) {
	csvrow* row;
	NOT_NULL_OR_RETURN(data, 0UL);
	if (rw >= csv_nrw(data))
		return(0UL);
	row = csv_get_row(data, rw);
	return darray_size(row->data);
}

/*** Get the number of header columns ***/
uint csv_nhdr(csvdata* data) {
	NOT_NULL_OR_RETURN(data, 0UL);
	if (!data->headers)
		return(0UL);
	return darray_size(data->hdrdata);
}

/*** Helper function: free data associated with a CSV cell ***/
static void csv_free_cell(csvcell* cell, bool unicode) {
	NOT_NULL_OR_RETURN_VOID(cell);
	if (CSV_STR == cell->typ) {
		if (unicode)
			delete [] cell->dat.u;
		else
			delete [] cell->dat.s;
	}
	delete cell;
}

/*** Free all the data associated with the CSV file ***/
void csv_free(csvdata* data) {
	int e, f;

	if (data->headers) {
		for (e = 0; e < darray_size(data->hdrdata); ++e)
			{
			stru s = darray_item(data->hdrdata, e);
			if (data->utf32)
				delete [] s.utf32;
			else
				delete [] s.ascii;
			}
		darray_free(data->hdrdata);
		darray_init(data->hdrdata);
		data->headers = false;
	}

	for (e = 0; ; ++e) {
		csvrow* row = csv_get_row(data, e);
		if (is_null(row))
			break;
		for (f = 0; f < darray_size(row->data); ++f)
			{
			csvcell* cell = darray_item(row->data, f);
			csv_free_cell(cell, data->utf32);
			}
	}
	darray_free(data->rowdata);
	darray_init(data->rowdata);
	
	delete data;
}

/*** Create new CSV data. ***/
csvdata* csv_new(bool is_unicode) {
	csvdata* data = NEW(csvdata);
	NOT_NULL_OR_RETURN(data, NULL);
	darray_init(data->hdrdata);
	darray_init(data->rowdata);
	data->headers = false;
	data->utf32 = is_unicode;
	data->maxcols = 0UL;
	return(data);
}

/*** Add a new allocated row in the CSV data; returns the row index. ***/
u32 csv_new_row(csvdata* data) {
	csvrow* row = NEW(csvrow);
	int i = darray_size(data->rowdata);
	darray_init(row->data);
	darray_append(data->rowdata, row);
	return(i);
}

/*** Find the column with the given header. Case-insensitive. Returns (-1) if the column was not found,
	or the CSV has no headers. In both flavors, 8-bit encoding and UTF-32 ***/
int csv_find_header_ascii(csvdata* data, const char* str) {
	int e;
	NOT_NULL_OR_RETURN(data, -1);
	if (!data->headers)
		return(-1);
	if (data->utf32)
		{
		ustring ustr = cstr2ustr(str);
		int ret = csv_find_header_utf32(data, ustr);
		delete [] ustr;
		return(ret);
		}
	for (e = 0; e < csv_nhdr(data); ++e)
		if (!__stricmp(str, darray_item(data->hdrdata, e).ascii))
			return(e);
	return(-1);
}

int csv_find_header_utf32(csvdata* data, ustring str) {
	int e;
	NOT_NULL_OR_RETURN(data, -1);
	if (!data->headers)
		return(-1);
	if (!data->utf32)
		{
		char* cstr = ustr2str(str, -1, NULL);
		int ret = csv_find_header_ascii(data, cstr);
		delete [] cstr;
		return(ret);
		}
	for (e = 0; e < csv_nhdr(data); ++e)
		if (!ustricmp(str, darray_item(data->hdrdata, e).utf32))
			return(e);
	return(-1);
}

/*** Add allocated data at the specified row and column. ***/
void csv_set_cell(csvdata* data, u32 row, u32 col, csvcell* cell) {
	csvrow* rw;
	csvcell **cellat;
	// ensure that the given row and column exists
	while (row >= darray_size(data->rowdata))
		csv_new_row(data);
	rw = darray_item(data->rowdata, row);
	while (col >= darray_size(rw->data))
		{
		csvcell* cell = NEW(csvcell);
		NOT_NULL_OR_RETURN_VOID(cell);
		cell->typ = CSV_VOID;
		cell->dat.s = NULL;
		darray_append(rw->data, cell);
		}
	cellat = csv_get_cell_address(data, row, col);
	csv_free_cell(*cellat, data->utf32);
	*cellat = cell;
}

/*** Set an integer value at the specified row and column. ***/
void csv_set_int(csvdata* data, u32 row, u32 col, i64 val) {
	csvcell* cell = NEW(csvcell);
	NOT_NULL_OR_RETURN_VOID(cell);
	cell->typ = CSV_INT;
	cell->dat.i = val;
	csv_set_cell(data, row, col, cell);
}

/*** Set a string value (8-bit) at the specified row and column. If the CSV is UTF-32 it's converted. ***/
void csv_set_ascii_str(csvdata* data, u32 row, u32 col, char* str) {
	csvcell* cell = NEW(csvcell);
	NOT_NULL_OR_RETURN_VOID(cell);
	if (data->utf32)
		{
		ustring ustr = cstr2ustr(str);
		csv_set_ustr(data, row, col, ustr);
		delete [] ustr;
		return;
		}
	cell->typ = CSV_STR;
	cell->dat.s = OUR_STRDUP(str);
	csv_set_cell(data, row, col, cell);
}

/*** Set a string value (UTF-32) at the specified row and column. If the CSV is 8-bit it's converted. ***/
void csv_set_ustr(csvdata* data, u32 row, u32 col, ustring str) {
	csvcell* cell = NEW(csvcell);
	NOT_NULL_OR_RETURN_VOID(cell);
	if (!data->utf32)
		{
		char* cstr = ustr2str(str, -1, NULL);
		csv_set_ascii_str(data, row, col, cstr);
		delete [] cstr;
		return;
		}
	cell->typ = CSV_STR;
	cell->dat.u = ustrdup(str);
	csv_set_cell(data, row, col, cell);
}

/*** Set a floating point value at the specified row and column. ***/
void csv_set_double(csvdata* data, u32 row, u32 col, double val) {
	csvcell* cell = NEW(csvcell);
	NOT_NULL_OR_RETURN_VOID(cell);
	cell->typ = CSV_DOUBLE;
	cell->dat.f = val;
	csv_set_cell(data, row, col, cell);
}

/*** Empty the cell at the specified row and column. ***/
void csv_set_void(csvdata* data, u32 row, u32 col) {
	csvcell* cell = NEW(csvcell);
	NOT_NULL_OR_RETURN_VOID(cell);
	cell->typ = CSV_VOID;
	cell->dat.s = NULL;
	csv_set_cell(data, row, col, cell);
}

/*** Set a header value. ***/
void csv_set_header_ascii(csvdata* data, u32 col, char* str) {
	// TODO: implement set headers
}

/*** Set a header value. ***/
void csv_set_header_ustr(csvdata* data, u32 col, ustring str) {
	// TODO: implement set headers
}

/*** C++ wrapper ***/
CsvFile::CsvFile() {
	data_ = NULL;
}

CsvFile::CsvFile(const char* filename, bool has_headers) {
	open(filename, has_headers);
}

CsvFile::CsvFile(bool is_unicode) {
	data_ = csv_new(is_unicode);
}

CsvFile::~CsvFile() {
	csv_free(data_);
	data_ = NULL;
}

bool CsvFile::open(const char* filename, bool has_headers) {
	if (not_null(data_))
		csv_free(data_);
	data_ = csv_read(filename, has_headers);
	return(not_null(data_));
}

uint CsvFile::nrws(void) const {
	NOT_NULL_OR_RETURN(data_, 0UL);
	return darray_size(data_->rowdata);
}

uint CsvFile::ncols(u32 rw) const {
	return csv_ncol(data_, rw);
}

uint CsvFile::nhdrs(void) const {
	return csv_nhdr(data_);
}

void CsvFile::write(const char* filename) {
	csv_write(filename, data_);
}

csvcell* CsvFile::cell(u32 rw, u32 col) {
	return csv_cell(data_, rw, col);
}

u32 CsvFile::new_row(void) {
	return csv_new_row(data_);
}

int CsvFile::find_header(const char* str) {
	return csv_find_header_ascii(data_, str);
}

int CsvFile::find_header(ustring str) {
	return csv_find_header_utf32(data_, str);
}

void CsvFile::set(u32 rw, u32 col, i64 val) {
	csv_set_int(data_, rw, col, val);
}

void CsvFile::set(u32 rw, u32 col, char* val) {
	csv_set_ascii_str(data_, rw, col, val);
}

void CsvFile::set(u32 rw, u32 col, ustring val) {
	csv_set_ustr(data_, rw, col, val);
}

void CsvFile::set(u32 rw, u32 col, double val) {
	csv_set_double(data_, rw, col, val);
}

void CsvFile::set(u32 rw, u32 col) {
	csv_set_void(data_, rw, col);
}

void CsvFile::set_header(u32 col, char* str) {
	csv_set_header_ascii(data_, col, str);
}

void CsvFile::set_header(u32 col, ustring str) {
	csv_set_header_ustr(data_, col, str);
}

/*** end __csv.c ***/
