/***

	Very simple demo of exif info reading in libcodehappy.

	Chris Street, 2024.

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	ExifDictionary exifdata;
	ArgParse ap;
	std::string pathname, key, value;

	ap.ensure_args(argc, argv);
	if (ap.nonflag_args() == 0) {
		codehappy_cerr << "Usage: exifdemo [image file]\n";
		return 1;
	}

	ap.nonflag_arg(0, pathname);
	exifdata.read_exif_from_image_file(pathname);
	exifdata.iterate_dict_start();
	while (exifdata.iterate_dict_next(key, value)) {
		std::cout << key << ":\t" << value << "\n";
	}

	return 0;
}

