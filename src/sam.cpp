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
}

void bitmap_mask::move(bitmap_mask& swap_to) {
	swap_to.bmp = bmp;
	swap_to.x_min = x_min;
	swap_to.x_max = x_max;
	swap_to.y_min = y_min;
	swap_to.y_max = y_max;
	swap_to.iou = iou;
	swap_to.stability_score = stability_score;
	bmp = nullptr;
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
	// N.B. trivial destructor: segmenting code handles the free
}

SegmentAnything::SegmentAnything(const std::string& model_path) {
	m_path = model_path;
	rng_seed = -1;
	nthreads = -1;
}

SegmentAnything::~SegmentAnything() {
}

bitmap_masks* SegmentAnything::segment_point(SBitmap* bmp, int x, int y) {
	return sam_mask_segment(bmp, "", m_path, (float) x, (float) y, rng_seed, nthreads);
}

bitmap_masks* SegmentAnything::segment_point(const std::string& img_path, int x, int y) {
	return sam_mask_segment(nullptr, img_path, m_path, (float) x, (float) y, rng_seed, nthreads);
}

bitmap_masks* SegmentAnything::segment_point(SBitmap* bmp, const SPoint& p) {
	return sam_mask_segment(bmp, "", m_path, (float) p.X(bmp), (float) p.Y(bmp), rng_seed, nthreads);
}

bitmap_masks* SegmentAnything::segment_point(const std::string& img_path, const SPoint& p) {
	return sam_mask_segment(nullptr, img_path, m_path, (float) p.X(), (float) p.Y(), rng_seed, nthreads);
}

static float proportion_covered(const SBitmap* bmp) {
	float ret;
	int cw = 0;
	for (int f = 0; f < bmp->height(); ++f) {
		for (int e = 0; e < bmp->width(); ++e) {
			if (bmp->get_pixel(e, f) == C_WHITE)
				++cw;
		}
	}

	ret = (float) cw;
	ret /= (float) (bmp->height() * bmp->width());
	return ret;
}

bitmap_masks* SegmentAnything::segment_image_auto(const std::string& img_path, float pct, bool echo_progress) {
	SBitmap* bmp = SBitmap::load_bmp(img_path);
	bitmap_masks* ret;
	if (is_null(bmp))
		return nullptr;
	ret = segment_image_auto(bmp, pct, echo_progress);
	delete bmp;
	return ret;
}

bitmap_masks* SegmentAnything::segment_image_auto(SBitmap* bmp, float pct, bool echo_progress) {
	/***

		first, get the best mask for each point on a 3x3 evenly spaced and centered grid on the image
		sort by (iou * stability_score), and in order add any that don't i'sect with previous matches (and are above min threshold)

	***/
	const int MAX_MATCHES = 256;
	bitmap_mask all[MAX_MATCHES];
	bitmap_mask matches[9];
	bitmap_masks* coll;
	SBitmap* mask;
	int nall = 0, nf = 0;
	float prop_img;
	const float MIN_SCORE_THRESHOLD = 0.40;

	while (rng_seed < 0) {
		rng_seed = RandI32();
	}

	mask = new SBitmap(bmp->width(), bmp->height(), BITMAP_MONO);

	coll = segment_point(bmp, SPoint(25, POINT_PERCENT, 25, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[0]);
	coll = segment_point(bmp, SPoint(50, POINT_PERCENT, 25, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[1]);
	coll = segment_point(bmp, SPoint(75, POINT_PERCENT, 25, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[2]);
	coll = segment_point(bmp, SPoint(25, POINT_PERCENT, 50, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[3]);
	coll = segment_point(bmp, SPoint(50, POINT_PERCENT, 50, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[4]);
	coll = segment_point(bmp, SPoint(75, POINT_PERCENT, 50, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[5]);
	coll = segment_point(bmp, SPoint(25, POINT_PERCENT, 75, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[6]);
	coll = segment_point(bmp, SPoint(50, POINT_PERCENT, 75, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[7]);
	coll = segment_point(bmp, SPoint(75, POINT_PERCENT, 75, POINT_PERCENT));
	bitmap_mask_best_match(coll, matches[8]);

	for (int e = 0; e < 9; ++e) {
		float sc1, sc2;
		sc1 = matches[e].iou * matches[e].stability_score;
		for (int f = e + 1; f < 9; ++f) {
			sc2 = matches[f].iou * matches[f].stability_score;
			if (sc2 > sc1) {
				bitmap_mask cp;
				matches[f].move(cp);
				matches[e].move(matches[f]);
				cp.move(matches[e]);
				--e;
				break;
			}
		}
	}

#ifdef CODEHAPPY_DEBUG
	echo_progress = true;
#endif
	matches[0].move(all[nall]);
	monobmp_mask_union(all[nall].bmp, mask);
	++nall;
	if (echo_progress) {		
		prop_img = proportion_covered(mask);
		std::cout << "Proportion masked: " << prop_img << "\n";
	}

	for (int i = 1; i < 9; ++i) {
		if (matches[i].bmp == nullptr || monobmp_mask_isect(matches[i].bmp, mask))
			continue;
		if (matches[i].iou * matches[i].stability_score < MIN_SCORE_THRESHOLD)
			break;
		matches[i].move(all[nall]);
		monobmp_mask_union(all[nall].bmp, mask);
		++nall;
		if (echo_progress) {
			prop_img = proportion_covered(mask);
			std::cout << "Proportion masked: " << prop_img << "\n";
		}
	}

	for (int i = 0; i < 9; ++i) {
		if (matches[i].bmp != nullptr)
			delete matches[i].bmp;
		matches[i].bmp = nullptr;
	}

	/*** now, continue by segmenting at chosen points, until we reach the desired proportion of coverage ***/
	DetRand dr((u32) rng_seed);
	int failure_thres = 10;
	pct *= 0.01;
	prop_img = proportion_covered(mask);

	while (prop_img < pct && nall < MAX_MATCHES && nf < failure_thres) {
		SPoint p(dr.RandU32Range(0, mask->width() - 1), dr.RandU32Range(0, mask->height() - 1));
		if (mask->get_pixel(p) == C_WHITE)
			continue;

		coll = segment_point(bmp, p);
		bitmap_mask_best_match(coll, matches[0], mask);
		if (matches[0].iou * matches[0].stability_score < MIN_SCORE_THRESHOLD) {
			++nf;
			if (matches[0].bmp != nullptr)
				delete matches[0].bmp;
			matches[0].bmp = nullptr;
			continue;
		}
		if (matches[0].bmp == nullptr) {
			++nf;
			continue;
		}
		nf = 0;

		matches[0].move(all[nall]);
		monobmp_mask_union(all[nall].bmp, mask);
		++nall;

		prop_img = proportion_covered(mask);
		if (echo_progress)
			std::cout << "Proportion masked: " << prop_img << "\n";
		if (prop_img > 0.4)
			failure_thres = 8;
		if (prop_img > 0.6)
			failure_thres = 6;
		if (prop_img > 0.8)
			failure_thres = 4;
	}

	delete mask;

	coll = new bitmap_masks;
	coll->nmasks = nall;
	coll->nx = bmp->width();
	coll->ny = bmp->height();
	coll->masks = new bitmap_mask [nall];
	for (int e = 0; e < nall; ++e) {
		all[e].move(coll->masks[e]);
	}

	return coll;
}

void bitmap_mask_best_match(bitmap_masks* coll, bitmap_mask& mask_out, SBitmap* mask) {
	float bs = 0.0;
	int bi = 0;
	if (0 == coll->nmasks) {
		delete coll;
		mask_out.bmp = nullptr;
		mask_out.iou = 0.;
		mask_out.stability_score = 0.;
		return;
	}
	for (int e = 0; e < coll->nmasks; ++e) {
		float sc = coll->masks[e].iou * coll->masks[e].stability_score;
		if (monobmp_mask_isect(mask, coll->masks[e].bmp))
			continue;
		if (sc > 0. && sc > bs) {
			bi = e;
			bs = sc;
		}
	}

	coll->masks[bi].move(mask_out);
	if (bs <= 0.)
		mask_out.bmp = nullptr;
	delete coll;
}

/*** end sam.cpp ***/
