/***

	sam.cpp

	Code for using the Segment Anything model (SAM) to generate masks for objects in an image.

	Copyright (c) 2023-2024, Chris Street

***/

bitmap_masks::bitmap_masks() {
    this->nmasks = 0;
    this->nx = 0;
    this->ny = 0;
    this->masks = nullptr;
}

bitmap_masks::~bitmap_masks() {
	if (masks != nullptr) {
		delete [] masks;
	}
	masks = nullptr;
	nmasks = 0;
}

SBitmap* bitmap_masks::bmp(int idx) const {
	if (idx < 0 || idx >= nmasks)
		return nullptr;
	return masks[idx].bmp;
}

bitmap_mask::bitmap_mask() {
	bmp = nullptr;
}

bitmap_mask::~bitmap_mask() {
	if (bmp != nullptr)
		delete bmp;
	bmp = nullptr;
}

SegmentAnything::SegmentAnything(const std::string& model_path) {
	m_path = model_path;
	rng_seed = -1;
}

SegmentAnything::~SegmentAnything() {
}

bitmap_masks* SegmentAnything::segment_point(SBitmap* bmp, int x, int y) {
	return sam_mask_segment(bmp, "", m_path, (float) x, (float) y, rng_seed);
}

bitmap_masks* SegmentAnything::segment_point(const std::string& img_path, int x, int y) {
	return sam_mask_segment(nullptr, img_path, m_path, (float) x, (float) y, rng_seed);
}

/*** end sam.cpp ***/
