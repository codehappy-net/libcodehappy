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
	int x = 0, y = 0, pct = 90, nt = -1;
	bool segment_all = false;
	
	ap.add_argument("model", type_string, "the Segment Anything model path (.gguf format)");
	ap.add_argument("img", type_string, "path to the image file");
	ap.add_argument("x", type_int, "point on the x axis", &x);
	ap.add_argument("y", type_int, "point on the y axis", &y);
	ap.add_argument("all", type_none, "auto-segment the full image (up to pct% covered)", &segment_all);
	ap.add_argument("pct", type_int, "minimum percentage coverage desired for segment-all (default is 90)", &pct);
	ap.add_argument("threads", type_int, "number of concurrent threads for model inference", &nt);
	ap.ensure_args(argc, argv);
	ap.value_str("model", model);
	ap.value_str("img", image);

	if (model.empty() || image.empty()) {
		codehappy_cerr << "*** error: you must provide a path to a model and an input image\n";
		ap.show_help();
		return 1;
	}

	SegmentAnything sam(model);
	sam.set_nthreads(nt);
	bitmap_masks* bmp_masks;
	if (segment_all) {
		bmp_masks = sam.segment_image_auto(image, (float) pct, true);
	} else {
		bmp_masks = sam.segment_point(image, x, y);
	}

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
		sprintf(img_name, "mask%d.png", e + 1);
		bm.bmp->save_bmp(img_name);
		std::cout << "mask saved to " << img_name << "\n";
	}

	if (segment_all) {
		// output a colorful combined image with each segment
		SPalette pal(64);
		SBitmap* bmp = new SBitmap(bmp_masks->nx, bmp_masks->ny);
		int icol = 1;
		fill_ega_palette(&pal);
		for (int e = 0; e < bmp_masks->nmasks; ++e) {
			SBitmap* bmp_seg = bmp_masks->masks[e].bmp;
			RGBColor c = pal.clrs[icol++];
			if (64 == icol)
				icol = 1;
			for (int y = 0; y < (int) bmp->height(); ++y) {
				for (int x = 0; x < (int) bmp->width(); ++x) {
					if (bmp_seg->get_pixel(x, y) == C_WHITE)
						bmp->put_pixel(x, y, c);
				}
			}
		}
		bmp->save_bmp("mask-combined.png");
		std::cout << "combined mask saved to mask-combined.png\n";
	}

	return 0;
}

/* end sam-img.cpp */
