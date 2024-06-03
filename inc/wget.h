/***

	wget.h

	As an alternative to the sockets code, here are functions
	that act as wrappers to Wget.

	Copyright (c) 2010-2022 C. M. Street.

***/
#ifndef ___WGET_H
#define ___WGET_H

/*** Set the location (path) of the wget executable. Takes ownership of the passed-in ptr loc. Set to NULL to use
	the default location. ***/
extern void set_wget_location(const char* loc);

/*** Use wget to read a file from the Internet. ***/
extern char* WgetFile(const char *url, bool timeout, bool silent, bool again, u32* flen = nullptr, bool check_cert = false);

/*** As above, with cookies support. ***/
extern char* WgetFileCookies(const char *url, bool timeout, bool silent, char *cookies, bool again, u32* flen = nullptr, bool check_cert = false);

/*** Some helper functions we might as well export. ***/
extern char* temp_file_name(const char* extension);
extern char* temp_file_name(const char* prefix, const char* extension);
extern char* load_file_mem(const char* fname, u32* filelen_out = nullptr);
extern const char* get_wget_location(void);

/* A function that works on both native and WASM builds. This will fetch the requested URI into a RamFile. */
extern RamFile* FetchURI(const char* URI, bool check_cert = false);
extern RamFile* FetchURI(const std::string& URI,  bool check_cert = false);

/* Fetch a URI, and then write it to the specified filename. Returns true iff success. */
extern bool FetchURIToFile(const char* URI, const char* out_path, bool check_cert = false);
extern bool FetchURIToFile(const std::string& URI, const std::string& out_path, bool check_cert = false);

#endif  // __WGET_H
/*** end wget.h ***/
