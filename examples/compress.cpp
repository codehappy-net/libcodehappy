/***
	compress.cpp

	A simple compression tool, using RAM files. Showing off the power of
	the library a little.

	C. M. Street
***/

#include "libcodehappy.h"

void change_extension(char* fname, const char* new_ext) {
	char *w = strchr(fname, '.');
	if (not_null(w))
		*w = '\000';
	strcat(fname, new_ext);
	return;
}

#define	BUFFER_SIZE	2048

int main(int argc, char **argv) {
	char newname[BUFFER_SIZE];
	RamFile open_file;

	if (argc < 2) {
		printf("Call: compress [input]\n");
		printf(" - If file is not compressed, new file with extension .compress is created\n");
		printf(" - If file is compressed, it is uncompressed with extension .decompress\n");
		return(0);
	}

	if (strlen(argv[1]) >= BUFFER_SIZE - 12) {	// leave space for the extensions
		printf("File name too long? What sort of input is this? Do you live on the moon?\n");
		return(1);
	}

	if (open_file.open(argv[1], RAMFILE_COMPRESS)) {
		printf("Error opening input file %s!\n", argv[1]);
		return(1);
	}

	strcpy(newname, argv[1]);
	if (open_file.compressed()) {
		open_file.option_off(RAMFILE_COMPRESS);
		change_extension(newname, ".decompress");
	} else {
		change_extension(newname, ".compress");
	}

	open_file.setname(newname);

	/* if we aren't read-only, we auto-flush to disk on close */
	open_file.close();

	return(0);
}

