/***

	sam.h

	Code for using the Segment Anything model (SAM) to generate masks for objects in an image.

	Copyright (c) 2023-2024, Chris Street

***/
#ifndef __SEGMENT_ANYTHING_CODEHAPPY
#define __SEGMENT_ANYTHING_CODEHAPPY

/* a single mask with associated data */
struct bitmap_mask {
	bitmap_mask();
	~bitmap_mask();
	SBitmap* bmp;
	/* bounding box */
	int x_min, x_max, y_min, y_max;
	/* iou prediction and stability score (intersections / unions) */
	float iou;
	float stability_score;
};

/* a collection of masks (stored as monochrome bitmaps.) */
struct bitmap_masks {
	bitmap_masks();
	~bitmap_masks();

	/* get one of the mask bitmaps by index */
	SBitmap* bmp(int idx) const;

	int nmasks;
	int nx, ny;
	bitmap_mask* masks;
};

/* Segment Anything class */

class SegmentAnything {
public:
	SegmentAnything(const std::string& model_path);
	~SegmentAnything();

	bitmap_masks* segment_point(SBitmap* bmp, int x, int y);
	bitmap_masks* segment_point(const std::string& img_path, int x, int y);

	void set_seed(int rng)	{ rng_seed = rng; }

private:
	std::string m_path;
	int rng_seed;
};

/* core function: this loads the model into RAM (if not already loaded) and performs a single mask operation on an image, which may be 
   passed in as a pathname or as a SBitmap. */
extern bitmap_masks* sam_mask_segment(const SBitmap* img_in, const std::string& img_path, const std::string& sam_model_path,
					float x, float y, int rng_seed);

#endif  // __SEGMENT_ANYTHING_CODEHAPPY
/* end sam.h */
