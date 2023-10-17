/***

	files.h

	Cross-platform file system access. These functions manipulate, find, search, or query the attributes 
	of disk files.

	Copyright (c) 2006-2022 C. M. Street.

***/
#ifndef __CODEHAPPY_FILES
#define __CODEHAPPY_FILES

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*** File length functions. ***/
extern u32 filelen(FILE* f);
extern u32 filelen(const char* fname);
extern u32 filelen(const std::string& fname);
extern u32 flength(const char *fname);
extern u64 flength_64(const char *fname);
#define	flength_32(fname)	flength(fname)

/*** Does the named file exist? ***/
extern bool FileExists(const char *fname);
extern bool FileExists(const std::string& fname);

/*** functions to save the current and change the directory ***/
// TODO: do these functions work on Linux?
extern void SaveCurDir(void);
extern void RestoreCurDir(void);
extern void ChangeDir(const char* newdir);
/*** save and change the current directory, working with a stack -- these nest ***/
extern void PushCurDir(void);
extern void PopCurDir(void);
extern size_t DirStackSize(void);

/*** Returns the filename without its extension. ***/
extern const char* strip_filename_extension(const char* fname);

/*** Returns whether or not the file's extension is ext. Note: case-insensitive. ***/
extern bool has_extension(const char* fname, const char* ext);
extern bool has_extension(const std::string& fname, const char* ext);

/*** Change the extension of the filename. ***/
extern const char* change_filename_extension(const char* fname, const char* new_ext);
extern std::string change_filename_extension(const std::string& fname, const char* new_ext);

#ifdef CODEHAPPY_WINDOWS
/*** Does the specified directory exist? (Currently a Windows-only function.) ***/
extern bool DirectoryExists(LPCTSTR szPath);
#endif

/*** Find and open the named file on the environment path with the given options. Returns NULL if not found. ***/
extern FILE* FindFileOnPath(char *fname, const char* opt);

/*** "found file information" type for the find_*_file() functions ***/
#ifdef CODEHAPPY_MSFT
/*** For MSVC we use the Microsoft functions _findfirst64() and friends ***/
typedef	struct __findfile	{
	struct _finddata64_t	data;
	int						handle;
	} FINDFILE;

typedef	FINDFILE*
	FINDFILEHANDLE;
#else
/*** For other compilers we try the POSIX opendir(), etc. ***/
typedef	struct __findfile {
	DIR*				dir;
	struct dirent*		entry;
	const char*			match;
	} FINDFILE;

typedef FINDFILE*
		FINDFILEHANDLE;
#endif

/*** file times that may be of interest ***/
#define	FILETIME_CREATION		1
#define	FILETIME_ACCESS			2
#define	FILETIME_WRITE			3
#define	FILETIME_STATUS_CHANGE	FILETIME_CREATION

/*** file attribute flags ***/
#define	ATTRIB_HIDDEN		0x02
#define	ATTRIB_SYSTEM		0x04
#define	ATTRIB_SUBDIRECTORY	0x10
#define	ATTRIB_READONLY		0x01
#define	ATTRIB_ARCHIVE		0x20

/*** Portable-ish versions of Microsoft C's _findfirst()/_findnext() follow -- search the current path for files matching given properties. ***/

/* Call this function to start the search -- pass it the glob pattern that the filename must match. Pattern can include the
	wildcards '?' and '*'. Will return NULL if there is no first result, else it returns a handle that represents the first file matched. */
extern FINDFILEHANDLE find_first_file(const char* filespec);

/* Call this function to continue the search -- will return NULL if there are no further matches. */
extern FINDFILEHANDLE find_next_file(FINDFILEHANDLE ffhandle);

/* Call this function when you are done and want to free the handle. Note that if the search terminates normally (i.e., all matching
	files have been cycled through and find_next_file() returns NULL) the handle is automatically closed by the library, so you only 
	need to call this if you want to stop mid-search. */
extern void find_file_close_handle(FINDFILEHANDLE ffhandle);

/* Call this function to determine if the find-file handle is valid. */
extern bool find_file_handle_valid(FINDFILEHANDLE ffhandle);

/* Use this function to read the filename from the find-file handle. */
extern const char* filename_from_handle(FINDFILEHANDLE handle);

/* Use this function to read the file size from the handle. */
extern u64 file_size_from_handle(FINDFILEHANDLE handle);

/* Call this function to get attribute flags for the found file. The ATTRIB_* bits defined above will be set as appropriate. */
extern u32 file_attributes_from_handle(FINDFILEHANDLE ffhandle);

/* Get the specified file time. Use the FILETIME_* constants defined above to specify which file time you are interested in. */
extern time_t file_time_from_handle(FINDFILEHANDLE handle, int which_time);

/* Normalize the file path. Relative paths become absolute paths: "." and ".." are resolved, backslashes become forward slashes, and 
	we prepend the current directory path if necessary. Returns the allocated full path on success. */
extern char* fixpath(const char* in_path);

/* Is the passed-in pathname relative or absolute? */
extern bool path_is_relative(const char* pathname);
extern bool path_is_relative(const std::string& pathname);

/* File mutexes: when you want multiple libcodehappy processes to be able to write to a file but don't want them to stomp 
   each other. Each process touching the file must grab a FILEMUTEX_HANDLE first. */
typedef void* FILEMUTEX_HANDLE;

/* Locks the file and returns a handle to the mutex. If the file is already locked, this function waits for it to unlock. */
extern FILEMUTEX_HANDLE GetMutexOnFile(const char* fname);

/* Attempts to lock the file for exclusive access, however, if the file is already locked, immediately return nullptr. */
extern FILEMUTEX_HANDLE TryMutexOnFile(const char* fname);

/* Releases the passed in file mutex. The mutex handle, after being released, is no longer valid. */
extern void ReleaseMutexOnFile(FILEMUTEX_HANDLE mutex);

/* Given a directory/folder name and a file name, combine to make a whole pathname. */
extern void make_pathname(const std::string& folder, const std::string& fname, std::string& path_out);

/* Get the filename from a pathname. */
extern const char* filename_from_path(const char* path);

/* Convert any slashes/backslashes in the passed path to the native format. */
extern void regularize_slashes(char* path);

#endif  // __CODEHAPPY_FILES
/*** end files.h ***/
