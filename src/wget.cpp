/***

	wget.cpp

	As an alternative to the sockets code, here are functions that act as wrappers 
	to Wget. Can provide cookies, so might be easier to adapt to certain applications. 
	Also permits HTTPS, proxies, and does not require linking in any external sockets 
	libraries.

	Also gives FetchURI(), which uses Wget on native builds and calls the appropriate
	libcodehappy function for WASM builds. 

	Copyright (c) 2010-2024 Chris Street

***/
#include "libcodehappy.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define	PURL	'\"'

static bool __rinit = false;

char* load_file_mem(const char* fname, u32* filelen_out) {
	FILE* f;
	char* w;
	u32 len = filelen(fname);
	if (!is_null(filelen_out))
		*filelen_out = len;
	if (len <= 0)
		return nullptr;
	f = fopen(fname, "rb");
	if (is_null(f))
		return nullptr;
	w = new char [len + 1];
	fread(w, 1, len, f);
	fclose(f);
	w[len] = '\000';
	return w;
}

char* temp_file_name(const char* extension) {
	char* name;
	if (!__rinit) {
		srand(time(NULL));
		__rinit = true;
	}
	name = new char [32];
	forever {
		sprintf(name, "tmp%04d%04d%s", rand() % 10000, rand() % 10000, ((extension == nullptr) ? "" : extension));
		if (!FileExists(name))
			break;
	}
	return name;
}

char* temp_file_name(const char* prefix, const char* extension) {
	char* name;
	if (!__rinit) {
		srand(time(NULL));
		__rinit = true;
	}
	int ialloc = strlen(prefix) + strlen(extension) + 10;
	ialloc = std::max(256, ialloc);
	name = new char [ialloc];
	forever {
		sprintf(name, "%s%04d%04d%s", prefix, rand() % 10000, rand() % 10000, ((extension == nullptr) ? "" : extension));
		if (!FileExists(name))
			break;
	}
	return name;
}

static void DoublePercent(const char *u, char *d) {
	int e;
	int f;

	e = 0;
	f = 0;
	while (u[e]) {
		d[f] = u[e];
		f++;
		if (u[e] == '%') {
			d[f] = u[e];
			f++;
		}
		e++;
	}
	d[f] = '\000';

}

#ifdef CODEHAPPY_WINDOWS
#define	SILENT		" >nul"
#define	BATCH_XTN	".bat"
static char default_wget_loc[] = "C:\\wget\\wget.exe";
#else
#define	SILENT		" >/dev/null"
#define	BATCH_XTN	".sh"
static char default_wget_loc[] = "wget";
#endif

static const char* user_wget_loc = NULL;

const char* get_wget_location(void) {
	if (not_null(user_wget_loc))
		return(user_wget_loc);
	return (default_wget_loc);
}

void set_wget_location(const char* loc) {
	user_wget_loc = loc;
}

char* WgetFileCookies(const char *url, bool timeout, bool silent, char *cookies, bool again, u32* filelen_out, bool check_cert) {
	/* made much more robust in the face of network connections that may occasionally drop -- CS, 20100609 */
	char *durl;
	char *cmd;
	char* fname;
	char* bname;
	char* sname;
	bool tryagain;
	int att;
	const int max_attempts = 3;
	FILE *f;
	u32 flen;
	unsigned char *status;
	char* ret;
	std::string c_cert = (check_cert ? "" : "--no-check-certificate");

	att = 0;
	durl = new char [strlen(url) * 2 + 1];
	NOT_NULL_OR_RETURN(durl, durl);
	cmd = new char [strlen(url) * 2 + 1024];
	if (is_null(cmd)) {
		delete durl;
		return nullptr;
	}

	do {
#ifdef CODEHAPPY_WINDOWS	
	DoublePercent(url, durl);
#else
	strcpy(durl, url);
#endif	

	fname = temp_file_name(nullptr);
	bname = temp_file_name(BATCH_XTN);
	sname = temp_file_name(nullptr);

#define WGET_CH_USER_AGENT "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0"

#ifdef CODEHAPPY_WINDOWS
	f = fopen(bname, "w");
	fprintf(f,
		"%s --user-agent=\"" WGET_CH_USER_AGENT "\" %s %s%s %s %s -O %s %c%s%c 1>> %s 2>&1\n",
		get_wget_location(),
		c_cert.c_str(),
		(cookies != NULL) ? "--load-cookies " : "",
		(cookies != NULL) ? cookies : "",
		timeout ? "-T5" : "",
		silent ? "-q" : "",
		fname,
		PURL,
		durl,
		PURL,
		sname);
	fclose(f);

	sprintf(cmd, "%s%s", bname, silent ? SILENT : "");
	system(cmd);

	remove(bname);
#else
	sprintf(cmd,
		"%s --user-agent=\"" WGET_CH_USER_AGENT "\" %s %s%s %s %s -O %s %c%s%c > %s%s",
		get_wget_location(),
		c_cert.c_str(),
		(cookies != NULL) ? "--load-cookies " : "",
		(cookies != NULL) ? cookies : "",
		timeout ? "-T5" : "",
		silent ? "-q" : "",
		fname,
		PURL,
		durl,
		PURL,
		sname,
		silent ? " 2>/dev/null" : "");
	system(cmd);
#endif	

	delete [] bname;
	tryagain = again;

	// read the status
	flen = filelen(sname);
	f = fopen(sname, "rb");
	NOT_NULL_OR_RETURN(f, nullptr);
	status = new unsigned char[flen + 1];
	fread(status, 1, flen, f);
	status[flen] = 0;
	fclose(f);
	if (status && strstr((char *)status, "failed: Unknown host")) {
		// we have lost the connection. Wait 10 seconds and then try again.
		sleep(10);
		tryagain = true;
	} else {
		tryagain = false;
	}
	if (status)
		delete [] status;
	remove(sname);
	delete [] sname;

	if (!tryagain)
		ret = load_file_mem(fname, filelen_out);
	remove(fname);
	delete [] fname;
	att++;
	} while (tryagain && att < max_attempts);

	delete durl;
	delete cmd;

	return(ret);
}

char* WgetFile(const char *url, bool timeout, bool silent, bool again, u32* flen, bool check_cert) {
	return WgetFileCookies(url, timeout, silent, NULL, again, flen, check_cert);
}

/* A function that works on both native and WASM builds. This will fetch the 
	requested URI into a RamFile. */
RamFile* FetchURI(const char* URI, bool check_cert) {
	RamFile* ret;
#ifdef CODEHAPPY_NATIVE
	// Native builds use Wget.
	u32 flen;
	char* buf = WgetFile(URI, true, true, false, &flen, check_cert);
	NOT_NULL_OR_RETURN(buf, nullptr);
	ret = new RamFile;
	ret->open_static(buf, flen, RAMFILE_DEFAULT);
#else
	// WebAssembly builds use codehappy_URI_fetch().
	ret = codehappy_URI_fetch(URI);
#endif
	return ret;
}

RamFile* FetchURI(const std::string& URI, bool check_cert) {
	return FetchURI(URI.c_str(), check_cert);
}

bool FetchURIToFile(const char* URI, const char* out_path, bool check_cert) {
	RamFile* rf = FetchURI(URI, check_cert);
	if (is_null(rf) || is_null(rf->buffer()))
		return false;
	rf->write_to_file(out_path);
	delete rf;
	return true;
}

bool FetchURIToFile(const std::string& URI, const std::string& out_path, bool check_cert) {
	return FetchURIToFile(URI.c_str(), out_path.c_str(), check_cert);
}

/*** end wget.cpp ***/
