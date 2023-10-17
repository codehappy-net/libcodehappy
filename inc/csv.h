/***

	csv.h

	Functions for reading and writing CSV files for libcodehappy.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef	__CSV_H
#define	__CSV_H

/*** CSV cell data types. ***/

// cell is empty, contains no data
#define	CSV_VOID	0
// cell has string data (either ASCII/8-bit strings or UTF-32, depending on the file format)
#define	CSV_STR		1
// cell has floating-point data
#define	CSV_DOUBLE	2
// cell has integer data
#define	CSV_INT		3

struct csvcell {
	int typ;
	union
		{
		double	f;
		i64		i;
		char*	s;
		ustring	u;
		} dat;
};

struct csvrow {
	darray(csvcell*)	data;
};

union stru {
	char*	ascii;
	ustring	utf32;
};

struct csvdata {
	bool 				headers;
	bool				utf32;
	darray(stru)		hdrdata;
	darray(csvrow*)		rowdata;
	// TODO: maxcols currently isn't used for anything
	u32					maxcols;
};

/*** Read a CSV file, return the data. ***/
extern csvdata* csv_read(const char* filename, bool has_headers);

/*** Write the CSV data to file. ***/
extern void csv_write(const char* filename, csvdata* data);

/*** Get the number of rows ***/
extern uint csv_nrw(csvdata* data);

/*** Get the number of columns in a given row ***/
extern uint csv_ncol(csvdata* data, u32 rw);

/*** Get the number of header columns ***/
extern uint csv_nhdr(csvdata* data);

/*** Get the data at a specific row and column of the CSV. ***/
extern csvcell* csv_cell(csvdata* data, u32 row, u32 col);

/*** Free all the data associated with the CSV file ***/
extern void csv_free(csvdata* data);

/*** Create new CSV data. ***/
extern csvdata* csv_new(bool is_unicode);

/*** Add a new allocated row in the CSV data; returns the row index. ***/
extern u32 csv_new_row(csvdata* data);

/*** Find the column with the given header. Case-insensitive. Returns (-1) if the column was not found,
	or the CSV has no headers. In both flavors, 8-bit encoding and UTF-32 ***/
extern int csv_find_header_ascii(csvdata* data, const char* str);
extern int csv_find_header_utf32(csvdata* data, ustring str);

/*** Add allocated data at the specified row and column. ***/
extern void csv_set_cell(csvdata* data, u32 row, u32 col, csvcell* cell);

/*** Set an integer value at the specified row and column. ***/
extern void csv_set_int(csvdata* data, u32 row, u32 col, i64 val);

/*** Set a string value at the specified row and column. If the CSV is UTF-32 it's converted. ***/
extern void csv_set_ascii_str(csvdata* data, u32 row, u32 col, char* str);

/*** Set a string value (UTF-32) at the specified row and column. If the CSV is ASCII it's converted. ***/
extern void csv_set_ustr(csvdata* data, u32 row, u32 col, ustring str);

/*** Set a floating point value at the specified row and column. ***/
extern void csv_set_double(csvdata* data, u32 row, u32 col, double val);

/*** Empty the cell at the specified row and column. ***/
extern void csv_set_void(csvdata* data, u32 row, u32 col);

/*** Set a header value. ***/
extern void csv_set_header_ascii(csvdata* data, u32 col, char* str);

/*** Set a header value. ***/
extern void csv_set_header_ustr(csvdata* data, u32 col, ustring str);

/*** Take a guess as to whether the string is UTF-32 ***/
extern bool string_is_utf32(const char* buf, u32 buflen);

/*** C++ wrapper ***/
class CsvFile {
public:
	CsvFile();
	/*** Load CSV from file. ***/
	CsvFile(const char* filename, bool has_headers);
	/*** Create a new CSV file. UTF-32 encoding is used iff is_unicode is true. ***/
	CsvFile(bool is_unicode);
	~CsvFile();

	/*** Open the specified CSV file ***/
	bool open(const char* filename, bool has_headers);

	/*** Return the number of rows in the currently loaded CSV ***/
	uint nrws(void) const;

	/*** Return the number of columns in the specified row of the currently loaded CSV ***/
	uint ncols(u32 rw) const;

	/*** Return the number of header columns in the currently loaded CSV. ***/
	uint nhdrs(void) const;

	/*** Write the CSV data to file. ***/
	void write(const char* filename);

	/*** Get the data at a specific row and column of the CSV. ***/
	csvcell* cell(u32 rw, u32 col);

	/*** Add a new row to the bottom of the CSV. Returns the row index. ***/
	u32 new_row(void);

	/*** Find the column with the given header, (-1) if not found. ***/
	int find_header(const char* str);
	int find_header(ustring str);

	/*** Set data at the specified row and column. ***/
	void set(u32 rw, u32 col, i64 val);
	void set(u32 rw, u32 col, char* val);	// strings will be converted if they don't match the CSV encoding.
	void set(u32 rw, u32 col, ustring val);
	void set(u32 rw, u32 col, double val);
	void set(u32 rw, u32 col);	// ensures that the cell is created, but it contains no data

	/*** Set header at the specified column. ***/
	void set_header(u32 col, char* str);
	void set_header(u32 col, ustring str);

private:
	csvdata* data_;
};

#endif  // __CSV_H
/* end csv.h */
