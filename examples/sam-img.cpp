/***

	sam-img.cpp

	Quick command line demo of the Segment Anything inference

	2023, Chris Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string model, image;
	int x = 0, y = 0;
	
	ap.add_argument("model", type_string, "the Segment Anything model path (.gguf format)");
	ap.add_argument("img", type_string, "path to the image file");
	ap.add_argument("x", type_int, "point on the x axis", &x);
	ap.add_argument("y", type_int, "point on the y axis", &y);
	ap.ensure_args(argc, argv);
	ap.value_str("model", model);
	ap.value_str("img", image);

	if (model.empty() || image.empty()) {
		codehappy_cerr << "*** error: you must provide a path to a model and an input image\n";
		ap.show_help();
		return 1;
	}

	SegmentAnything sam(model);
	auto bmp_masks = sam.segment_point(image, x, y);

	if (is_null(bmp_masks)) {
		codehappy_cerr << "*** error segmenting the image!\n";
		return 2;
	}

	std::cout << bmp_masks->nmasks << " image masks.\n";
	for (int e = 0; e < bmp_masks->nmasks; ++e) {
		const bitmap_mask& bm = bmp_masks->masks[e];
		char img_name[32];
		std::cout << "Mask #" << (e + 1) << ":\n";
		std::cout << "\tbounding box: (" << bm.x_min << ", " << bm.y_min << ") - (" << bm.x_max << ", " << bm.y_max << ")\n";
		std::cout << "\tiou prediction: " << bm.iou << std::endl;
		std::cout << "\tstability score: " << bm.stability_score << std::endl;
		sprintf(img_name, "mask%d.png", e);
		bm.bmp->save_bmp(img_name);
		std::cout << "mask saved to " << img_name << "\n";
	}

	return 0;
}

/* end sam-img.cpp */
