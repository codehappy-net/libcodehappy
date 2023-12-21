/***

	files.cpp

	Functions used for manipulating, finding, searching, or querying attributes of disk files.

	Copyright (c) 2006-2022 C. M. Street.

***/
#include "libcodehappy.h"
#include <thread>

#ifndef CODEHAPPY_WINDOWS
#define	_getcwd(X, Y)	getcwd(X, Y)
#endif  // !CODEHAPPY_WINDOWS

/*** semi-portable file length function ***/
u32 filelen(FILE* f) {
	NOT_NULL_OR_RETURN(f, 0);
#ifdef CODEHAPPY_MSFT
	return(_filelength(fileno(f)));
#else
	u32 ret;
	fseek(f, 0, SEEK_END);
	ret = ftell(f);
	rewind(f);
	return ret;
#endif
}

/*** as above, but takes a filename ***/
u32 filelen(const char* fname) {
	struct stat st;
	stat(fname, &st);
	return (u32)st.st_size;
}

u32 filelen(const std::string& fname) {
	return filelen(fname.c_str());
}

/*** note: won't work with files > 2GB ***/
u32 flength(const char *fname) {
	return filelen(fname);
}

/*** adapted to work with very large files ***/
u64 flength_64(const char *fname) {
#ifdef CODEHAPPY_MSFT
	FILE* f = fopen(fname, "rb");
	u64 ret;
	NOT_NULL_OR_RETURN(f, 0LL);
	ret = _filelengthi64(fileno(f));
	fclose(f);
	return(ret);
#else // non-MSVC compilers
	struct stat64 st;
	stat64(fname, &st);
	return (u64)st.st_size;
#endif
}

/*** Does the named file exist? ***/
bool FileExists(const char* fname) {
	struct stat info;
	if (stat(fname, &info) != 0)
		return false;
	return true;
}

bool FileExists(const std::string& fname) {
	return FileExists(fname.c_str());
}

/*** Returns the filename without its extension. ***/
/* Note: not thread-safe */
static char __fext[4096];
const char* strip_filename_extension(const char* fname) {
	const char* w = strchr(fname, '.');
	
	if (is_null(w))
		return(fname);
	if ((w - fname) > 4095)
		return(NULL);

	strncpy(__fext, fname, 4096);
	__fext[w - fname] = '\000';

	return(__fext);
}

bool has_extension(const char* fname, const char* ext) {
	const char* w = strchr(fname, '.');
	const char* cmpstr = ext;

	if (is_null(w))
		return(FEmpty(ext));

	w = fname + strlen(fname);
	while (*w != '.')
		--w;
	++w;
	if (*cmpstr == '.')
		++cmpstr;
#ifdef CODEHAPPY_WINDOWS
	// Case-insensitive on Windows platforms.
	if (__stricmp(w, cmpstr))
#else
	if (strcmp(w, cmpstr))
#endif
		return(false);

	return(true);
}

bool has_extension(const std::string& fname, const char* ext) {
	return has_extension(fname.c_str(), ext);
}

#define	F_BUFFER_SIZE	2048

/*** functions to save the current and change the directory ***/
static char __curdir[F_BUFFER_SIZE] = {0};
void SaveCurDir(void) {
	_getcwd(__curdir, F_BUFFER_SIZE);
}

void RestoreCurDir(void) {
	if (__curdir[0])
		chdir(__curdir);
}

void ChangeDir(const char* newdir) {
	chdir(newdir);
}

// declared this way to help IDE syntax parsing
#define TYPEX	darray(char*)
static TYPEX __direc_stack;
#undef TYPEX

/*** Like SaveCurDir() and RestoreCurDir(), but saves and restores the current directory to/from a stack. These can be nested. ***/
void PushCurDir(void) {
	char cdir[F_BUFFER_SIZE];
	_getcwd(cdir, F_BUFFER_SIZE);
	char* dir = strdup(cdir);
	darray_push(__direc_stack, dir);
}

void PopCurDir(void) {
	if (iszero(DirStackSize())) {
		assert(false);
		return;
	}

	char* dir;
	dir = __direc_stack[__direc_stack.size() - 1];
	__direc_stack.pop_back();
	chdir(dir);
	delete [] dir;
}

size_t DirStackSize(void) {
	return darray_size(__direc_stack);
}

// TODO: version of this fn that works on other platforms?
#ifdef CODEHAPPY_WINDOWS
bool DirectoryExists(LPCTSTR szPath) {
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
#endif

FILE* FindFileOnPath(char *fname, const char* opt) {
	FILE* ret;
	char *w;
	char *w2;
	char dirname[1024];

	// first, search the current path
	if (FileExists(fname)) {
		// easy 'nuff
		return fopen(fname, opt);
	}
	
	// now search the environment path
	SaveCurDir();

	char *envv;

	envv = getenv("PATH");
	if (envv) {
		w = envv;
		forever {
			w2 = strchr(w, ';');
			if (w2) {
				strncpy(dirname, w, w2 - w);
				dirname[w2 - w] = 0;
			} else {
				strcpy(dirname, w);
			}

			chdir(dirname);
			if (FileExists(fname)) {
				// there it is
				ret = fopen(fname, opt);
				RestoreCurDir();
				return(ret);
			}

			w = w2;
			if (!w)
				break;
			++w;
		}
	}

	RestoreCurDir();

	return(NULL);
}


/*** Portable-ish versions of findfirst/findnext -- search the current path for files matching given properties. ***/
FINDFILEHANDLE find_first_file(const char* filespec) {
	FINDFILE* ff = new FINDFILE;
	NOT_NULL_OR_RETURN(ff, ff);
#ifdef CODEHAPPY_MSFT
	// Using the MSVC function _findfirst64()
	ff->handle = _findfirst64(filespec, &ff->data);
	if (ff->handle == -1)
		return(NULL);
	return(ff);
#else
	// Using other compilers -- try opendir() from POSIX
	ff->dir = opendir(".");
	NOT_NULL_OR_RETURN(ff->dir, NULL);
	ff->match = strdup(filespec);
	NOT_NULL_OR_RETURN(ff->match, NULL);
	ff->entry = NULL;
	return find_next_file(ff);
#endif
}

FINDFILEHANDLE find_next_file(FINDFILEHANDLE ffhandle) {
#ifdef CODEHAPPY_MSFT
	// MSVC
	if (_findnext64(ffhandle->handle, &ffhandle->data)) {
		find_file_close_handle(ffhandle);
		return(NULL);
	}
	return(ffhandle);
#else
	// On other compilers, use readdir() and string_matches_pattern() to find the next matching file. 
	forever {
		/* Windows file names are case-insensitive, Unix-likes are case-sensitive; search appropriately. */
#ifdef CODEHAPPY_WINDOWS
		const bool case_sensitive = false;
#else
		const bool case_sensitive = true;
#endif
		const char* fname;
		ffhandle->entry = readdir(ffhandle->dir);
		NOT_NULL_OR_RETURN(ffhandle->entry, NULL);
		fname = filename_from_handle(ffhandle);
		if (string_matches_pattern(fname, ffhandle->match, case_sensitive))
			break;
	}
	return ffhandle;
#endif
}

void find_file_close_handle(FINDFILEHANDLE ffhandle) {
#ifdef CODEHAPPY_MSFT
	// MSVC
	NOT_NULL_OR_RETURN_VOID(ffhandle);
	if (ffhandle->handle != -1) {
		_findclose64(ffhandle->handle);
		ffhandle->handle = -1;
	}
#else
	// Other compilers
	NOT_NULL_OR_RETURN_VOID(ffhandle);
	NOT_NULL_OR_RETURN_VOID(ffhandle->dir);
	closedir(ffhandle->dir);
	ffhandle->dir = NULL;
#endif
	delete ffhandle;
}

const char* filename_from_handle(FINDFILEHANDLE handle) {
#ifdef CODEHAPPY_MSFT
	if (!find_file_handle_valid(handle))
		return NULL;
	return handle->data.name;
#else
	if (!find_file_handle_valid(handle) || is_null(handle->entry))
		return NULL;
	return handle->entry->d_name;
#endif
}

bool find_file_handle_valid(FINDFILEHANDLE ffhandle) {
#ifdef CODEHAPPY_MSFT
	NOT_NULL_OR_RETURN(ffhandle, false);
	return (ffhandle->handle != -1);
#else
	NOT_NULL_OR_RETURN(ffhandle, false);
	NOT_NULL_OR_RETURN(ffhandle->dir, false);
	return true;
#endif
}

u64 file_size_from_handle(FINDFILEHANDLE handle) {
#ifdef CODEHAPPY_MSFT
	if (!find_file_handle_valid(handle))
		return 0ULL;
	return handle->data.size;
#else
	const char* filename = filename_from_handle(handle);
	NOT_NULL_OR_RETURN(filename, 0ULL);
	return (u64)flength_64(filename);
#endif
}

u32 file_attributes_from_handle(FINDFILEHANDLE ffhandle) {
#ifdef CODEHAPPY_MSFT
	if (!find_file_handle_valid(ffhandle))
		return 0UL;
	return ffhandle->data.attrib;
#else
	struct stat st;
	const char* filename;
	u32 ret;
	if (!find_file_handle_valid(ffhandle))
		return 0UL;
	filename = filename_from_handle(ffhandle);
	if (is_null(filename))
		return 0UL;
	stat(filename, &st);
	/* now translate st_mode to our file attribute flags */
	ret = 0UL;
	if (filename[0] == '.')
		ret |= ATTRIB_HIDDEN;	// by default these files are hidden in bash
	if (falsity(st.st_mode & S_IWUSR))
		ret |= ATTRIB_READONLY;	// user does not have write permissions, so read-only
	// ATTRIB_SYSTEM is a Windows thing; "does user have read permissions?" is a reasonable substitute for non-Windows platforms
	if (falsity(st.st_mode & S_IRUSR))
		ret |= ATTRIB_SYSTEM;
	return ret;
#endif
}

time_t file_time_from_handle(FINDFILEHANDLE handle, int which_time) {
#ifdef CODEHAPPY_MSFT
	if (!find_file_handle_valid(handle))
		return (time_t)(-1L);
	switch (which_time)
		{
	case FILETIME_ACCESS:
		return handle->data.time_access;
	case FILETIME_CREATION:
		return handle->data.time_create;
	case FILETIME_WRITE:
		return handle->data.time_write;
		}
	return (time_t)(-1L);
#else
	struct stat st;
	const char* filename;
	if (!find_file_handle_valid(handle))
		return (time_t)(-1L);
	filename = filename_from_handle(handle);
	if (is_null(filename))
		return (time_t)(-1L);
	stat(filename, &st);
	switch (which_time)
		{
	case FILETIME_ACCESS:
		return st.st_atime;
	case FILETIME_CREATION:
		return st.st_ctime;
	case FILETIME_WRITE:
		return st.st_mtime;
		}
	return (time_t)(-1L);
#endif
}

/* Is the passed-in pathname relative or absolute? */
bool path_is_relative(const char* pathname) {
	NOT_NULL_OR_RETURN(pathname, true);
	
	switch (pathname[0]) {
	case '/':
	case '\\':
		return false;
	}

#ifdef CODEHAPPY_WINDOWS
	/* look for drive letters on Windows */
	if (strlen(pathname) >= 2 && isalpha(pathname[0]) && pathname[1] == ':')
		return false;
	// TODO: relative paths across drives? (i.e. C:../test)
#endif
	
	return true;
}

bool path_is_relative(const std::string& pathname) {
	return path_is_relative(pathname.c_str());
}

/*** Helper function: resolve single- and double-period directories in a path name ***/
static void __dot_directory(char* path) {
	char* w;
	int d;

	w = strstr(path, "/../");
	while (not_null(w)) {
		char* w2;

		w2 = w - 1;
		while (w2 >= path) {
			if (*w2 == '/')
				break;
			--w2;
		}

		if (w2 < path)
			break;

		if (*w2 == '/')
			memcpy(w2, w + 3, strlen(w) + 1);
		
		w = strstr(path, "/../");
	}

	/* remove single dot directories */
	strreplace(path, "/./", "/");
}

/* Normalize the file path. "." and ".." are resolved, backslashes become forward slashes, and prepend the current directory path.
	Returns the allocated full path on success. */
char* fixpath(const char* in_path) {
	Scratchpad sp;
	char cwd[256];
	char* path;

	// TODO: when this function returns the right thing for relative paths across drives in Windows, make sure this fn works with them too
	if (path_is_relative(in_path)) {
		/* the current working directory */
		getcwd(cwd, 256);
		sp.strcpy(cwd);
		if (*END_OF_STR(cwd) != '/')
			sp.addc('/');
	}
	
	sp.strcat(in_path);

	path = sp.c_str();
	strreplace(path, "\\", "/");
	strreplace(path, "//", "/");
	NOT_NULL_OR_RETURN(path, NULL);

	/* collapse single- and double-period directories */
	__dot_directory(path);

	return (char *)sp.relinquish_buffer();
}

#ifdef CODEHAPPY_WINDOWS
#define FOLDER_SLASH	"\\"
#else
#define FOLDER_SLASH	"/"
#endif

/* Given a directory/folder name and a file name, combine to make a whole pathname.
   TODO: Shouldn't I use the normalization functions here? */
void make_pathname(const std::string& folder, const std::string& fname, std::string& path_out) {
	if (folder.empty()) {
		path_out = fname;
		return;
	}
	int l = folder.c_str()[folder.length() - 1];
	if (l == '/' || l == '\\') {
		/* Has trailing slash. */
		path_out = folder + fname;
	} else {
		/* Add a trailing slash. */
		path_out = folder + FOLDER_SLASH;
		path_out += fname;
	}
}

/* File mutex. This structure is internal only; consumers only receive a handle to this struct. */
struct FileMutex {
	char* fname;
};

/* Get the mutex name for the passed-in filename. */
static const char* filemtx_name(const char* fname) {
	static char fmn[1024];
	strncpy(fmn, fname, 1019);
	fmn[1019] = '\000';
	strcat(fmn, ".mtx");
	return fmn;
}

/* Locks the file and returns a handle to the mutex. If the file is already locked, this function waits for it to unlock. */
FILEMUTEX_HANDLE GetMutexOnFile(const char* fname) {
	FILEMUTEX_HANDLE trym = TryMutexOnFile(fname);
	while (is_null(trym)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		trym = TryMutexOnFile(fname);
	}
	return trym;
}

/* Attempts to lock the file for exclusive access, however, if the file is already locked, immediately return nullptr. */
FILEMUTEX_HANDLE TryMutexOnFile(const char* fname) {
	const char* fn = filemtx_name(fname);
	if (FileExists(fn)) {
		return nullptr;
	}
	FILE* fo = fopen(fn, "w");
	fprintf(fo, "1\n");
	fclose(fo);

	FileMutex* ret = new FileMutex;
	ret->fname = new char [strlen(fname) + 1];
	strcpy(ret->fname, fname);
	return (FILEMUTEX_HANDLE)ret;
}

/* Releases the passed in mutex. The mutex handle, after being released, is no longer valid. */
void ReleaseMutexOnFile(FILEMUTEX_HANDLE mutex) {
	FileMutex* mtx = (FileMutex *)mutex;
	NOT_NULL_OR_RETURN_VOID(mtx);
	NOT_NULL_OR_RETURN_VOID(mtx->fname);
	const char* fn = filemtx_name(mtx->fname);
	if (FileExists(fn)) {
		remove(fn);
	}
	delete [] mtx->fname;
	delete mtx;
}

/* Get the filename from a pathname. */
const char* filename_from_path(const char* path) {
	const char* w = path + strlen(path);
	--w;
	while (w >= path && *w != '/' && *w != '\\') {
		--w;
	}
	if (w < path)
		return path;
	++w;
	return w;
}

/* Convert any slashes/backslashes in the passed path to the native format. */
void regularize_slashes(char* path) {
#ifdef CODEHAPPY_WINDOWS
	const char rep = '/', with = '\\';
#else
	const char rep = '\\', with = '/';
#endif
	while (*path) {
		if (*path == rep)
			*path = with;
		++path;
	}
}

const char* change_filename_extension(const char* fname, const char* new_ext) {
	strip_filename_extension(fname);
	if (is_null(new_ext))
		return __fext;
	if (strlen(__fext) + strlen(new_ext) + 1 >= 4096)
		return nullptr;
	if (new_ext[0] != '.')
		strcat(__fext, ".");
	strcat(__fext, new_ext);
	return __fext;
}

std::string change_filename_extension(const std::string& fname, const char* new_ext) {
	const char* nstr = change_filename_extension(fname.c_str(), new_ext);
	std::string str_ret = nstr;
	return str_ret;
}

void file_list_from_path(const char* path, std::vector<std::string>& files_out, bool recursive_search) {
	DIR* di = opendir(path);
	dirent* entry;
	bool is_dir;

	while (entry = readdir(di)) {
		std::string new_path;
#ifdef _DIRENT_HAVE_D_TYPE
        	if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK) {
			is_dir = (entry->d_type == DT_DIR);
	        } else
#endif
	        /* else... */	{
			struct stat stbuf;
			stat(entry->d_name, &stbuf);
			is_dir = S_ISDIR(stbuf.st_mode);
		}

		if (is_dir) {
			if (recursive_search && entry->d_name[0] != '.') {
				make_pathname(path, entry->d_name, new_path);
				file_list_from_path(new_path.c_str(), files_out, true);
			}
			continue;
		}

		make_pathname(path, entry->d_name, new_path);
		files_out.push_back(new_path);
	}
	closedir(di);
}

void file_list_from_path(const std::string& path, std::vector<std::string>& files_out, bool recursive_search) {
	file_list_from_path(path.c_str(), files_out, recursive_search);
}

/*** end files.cpp ***/
