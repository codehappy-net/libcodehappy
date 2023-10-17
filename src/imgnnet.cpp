/***

	imgnnet.cpp

	The ImgNNet class, which can predict pixels in a bitmap based on nearby pixels. 
	Can be used as a "magic eraser", an inpainter/superresolution network, as a fun image 
	filter, a despeckler/noise reducer, to extend images, for machine vision, image 
	compression, colorization, etc.

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"
#include <thread>

TrainData::TrainData() {
	citer = 0;
	retry = 0;
	err_in = 0.;
	err_out = 0.;
	lrate1 = 0.;
	lrate = 0.;
	lrate_eff = 0.;
	fname_idx = STRINDEX_INVALID;
	flip = false;
	img_w = 0;
	img_h = 0;
}

void TrainData::out_to_ramfile(RamFile* rf, u32 version) const {
	/* Version: 1 has img_w/img_h, 0 does not. */
	rf->putu32(citer);
	rf->putu32(retry);
	rf->putdouble(err_in);
	rf->putdouble(err_out);
	rf->putdouble(lrate1);
	rf->putdouble(lrate);
	rf->putdouble(lrate_eff);
	rf->putbool(flip);
	rf->putu32(fname_idx);
	tim_start.out_ramfile(rf);
	tim_end.out_ramfile(rf);
	if (version > 0) {
		rf->putu32(img_w);
		rf->putu32(img_h);
	}
}

void TrainData::dump(StringTable& st, VerboseStream& vs) const {
	if (STRINDEX_INVALID == fname_idx) {
		vs << "Training data [Filename invalid]\n";
	} else {
		vs << "Training data [" << st.string(fname_idx) << "]\n";
	}
	vs << "\tTraining began: " << fmt_datetime_shell_long(&tim_start) << "\n";
	vs << "\tTraining ended: " << fmt_datetime_shell_long(&tim_end) << "\n";
	vs << "\tError fn in   : " << err_in << "\n";
	vs << "\tError fn out  : " << err_out << "\n";
	vs << "\tLearn rate in : " << lrate1 << "\n";
	vs << "\tLearn rate out: " << lrate << "\n";
	vs << "\tLearn rate eff: ";
	if (lrate_eff <= 0.)
		vs << "n/a\n";
	else
		vs << lrate_eff << "\n";
	vs << "\tFlip          : " << flip << "\n";
	if (img_w * img_h > 0)
		vs << "\tImage dimen.  : " << img_w << " x " << img_h << "\n";
}

void TrainData::read_from_ramfile(RamFile* rf, u32 version) {
	/* Version: 1 has img_w/img_h, 0 does not. */
	citer = rf->getu32();
	retry = rf->getu32();
	err_in = rf->getdouble();
	err_out = rf->getdouble();
	lrate1 = rf->getdouble();
	lrate = rf->getdouble();
	lrate_eff = rf->getdouble();
	fname_idx = STRINDEX_INVALID;
	flip = rf->getbool();
	fname_idx = rf->getu32();
	tim_start.in_ramfile(rf);
	tim_end.in_ramfile(rf);
	if (version > 0) {
		img_w = rf->getu32();
		img_h = rf->getu32();
	}
}

ValidationSetData::ValidationSetData() {
	clear();
}

void ValidationSetData::clear() {
	nf = 0;
	fs.clear();
	off.clear();
	inp.clear();
	out.clear();
}

void ValidationSetData::in_from_ramfile(RamFile* rf) {
	u32 e;
	clear();
	nf = rf->getu32();
	if (0 == nf) {
		return;
	}
	rad = rf->getu32();
	for (e = 0; e < nf; ++e) {
		fs.push_back(rf->getu32());
		off.push_back(rf->getu32());
	}
	rf->getsp(inp);
	rf->getsp(out);
}

void ValidationSetData::out_to_ramfile(RamFile* rf) const {
	u32 e;
	rf->putu32(nf);
	if (0 == nf) {
		return;
	}
	rf->putu32(rad);
	for (e = 0; e < nf; ++e) {
		rf->putu32(fs[e]);
		rf->putu32(off[e]);
	}
	rf->putsp(inp);
	rf->putsp(out);
}

void ValidationSetData::dump(StringTable& st, VerboseStream& vs) const {
	vs << "Validation set contains " << nf << " files.\n";
	for (u32 e = 0; e < nf; ++e) {
		if (STRINDEX_INVALID == fs[e]) {
			vs << "Validation set data [Filename invalid]\n";
		} else {
			vs << "Validation set data [" << st.string(fs[e]) << "]\n";
		}
		vs << "\tOffset of data : " << off[e] << "\n";
	}
	vs << "Size of input data  (bytes): " << inp.length() << "\n";
	vs << "Size of output data (bytes): " << out.length() << "\n"; 
}

ImgBrush::ImgBrush(u32 r) {
	u32 no;	/* unused */
	ImgNNet::inout_from_radius(r, 0, ni, no);
	double vr = RandDouble(-0.1, 0.1), vg = RandDouble(-0.1, 0.1), vb = RandDouble(-0.1, 0.1);
	in = new double [ni];
	velocity_color = new double [ni];
	in[0] = RandDouble(0., 1.);
	in[1] = RandDouble(0., 1.);
	in[2] = RandDouble(0., 1.);
	velocity_color[0] = vr;
	velocity_color[1] = vg;
	velocity_color[2] = vb;
	for (u32 e = 3; e < ni; e += 3) {
		velocity_color[e] = velocity_color[e - 3] + RandDouble(-0.002, 0.002);
		velocity_color[e + 1] = velocity_color[e - 2] + RandDouble(-0.002, 0.002);
		velocity_color[e + 2] = velocity_color[e - 1] + RandDouble(-0.002, 0.002);
		in[e] = in[e - 3] + RandDouble(-0.04, 0.04);
		in[e + 1] = in[e - 2] + RandDouble(-0.04, 0.04);
		in[e + 2] = in[e - 1] + RandDouble(-0.04, 0.04);
		in[e] = CLAMP(in[e], 0., 1.);
		in[e + 1] = CLAMP(in[e + 1], 0., 1.);
		in[e + 2] = CLAMP(in[e + 2], 0., 1.);
	}
	do {
		velocity_brush[0] = RandDouble(-4., 4.);
	} while (std::abs(velocity_brush[0]) < 0.5);
	do {
		velocity_brush[1] = RandDouble(-4., 4.);
	} while (std::abs(velocity_brush[1]) < 0.5);
	x = 0.;
	y = 0.;
	rad = r;
}

void ImgBrush::brush_update(u32 w, u32 h) {
	x += velocity_brush[0];
	if (x < 0.) {
		x = 0;
		velocity_brush[0] = -velocity_brush[0];
	} else if (x >= double(w) - 0.5) {
		x = (double)(w - 1);
		velocity_brush[0] = -velocity_brush[0];
	}
	y += velocity_brush[1];
	if (y < 0.) {
		y = 0;
		velocity_brush[1] = -velocity_brush[1];
	} else if (y >= double(h) - 0.5) {
		y = (double)(h - 1);
		velocity_brush[1] = -velocity_brush[1];
	}
	for (u32 e = 0; e < ni; ++e) {
		in[e] += velocity_color[e];
		if (in[e] > 1.0) {
			velocity_color[e] = -velocity_color[e];
			in[e] = 1.0 - (in[e] - 1.0);
		} else if (in[e] < 0.0) {
			velocity_color[e] = -velocity_color[e];
			in[e] = -in[e];
		}
	}
}

void ImgBrush::set_from_predictions(PredictAccum& pa) {
	double* disc = in;
	int iy = (int)floor(y + 0.5), ix = (int)floor(x + 0.5);
	int xx, yy;

	for (yy = iy - ((int)rad); yy <= iy + ((int)rad); ++yy) {
		for (xx = ix - ((int)rad); xx <= ix + ((int)rad); ++xx) {
			int ds = (ix - xx) * (ix - xx) + (iy - yy) * (iy - yy);
			if (ds <= (int)(rad * rad)) {
				u32 np = pa.get_num_predictions_lock(xx, yy);
				if (np > 0) {
					RGBOut ro;
					pa.get_avg_prediction_lock(xx, yy, ro);
					*(disc++) = ro.r;
					*(disc++) = ro.g;
					*(disc++) = ro.b;
				} else {
					disc += 3;
				}
			}
		}
	}
}

ImgBrush::~ImgBrush() {
	if (in != nullptr)
		delete in;
	if (velocity_color != nullptr)
		delete velocity_color;
}

u32 color_distance(RGBColor c1, RGBColor c2) {
	int ret = 0;
	ret += std::abs((int)RGB_RED(c1) - (int)RGB_RED(c2));
	ret += std::abs((int)RGB_GREEN(c1) - (int)RGB_GREEN(c2));
	ret += std::abs((int)RGB_BLUE(c1) - (int)RGB_BLUE(c2));
	return (u32)ret;
}

double channel_intensity(SBitmap* bmp, u32 ch, int x, int y) {
	u32 v = 0;
	switch (ch) {
	case CHANNEL_RED:
		v = bmp->get_red(x, y);
		break;
	case CHANNEL_GREEN:
		v = bmp->get_green(x, y);
		break;
	case CHANNEL_BLUE:
		v = bmp->get_blue(x, y);
		break;
	case CHANNEL_ALPHA:
		v = bmp->get_alpha(x, y);
		break;
	}
	return double(v) / 255.0;
}

double gray_intensity(SBitmap* bmp, int x, int y) {
	double intsy;
	intsy = channel_intensity(bmp, CHANNEL_RED, x, y);
	intsy += channel_intensity(bmp, CHANNEL_GREEN, x, y);
	intsy += channel_intensity(bmp, CHANNEL_BLUE, x, y);
	intsy /= 3.0;
	return intsy;
}

bool file_is_text(const char* pathname) {
	char buf[128];
	if (!FileExists(pathname))
		return false;
	u32 fl = filelen(pathname);
	if (fl == 0)
		return false;

	/* Check the first 128 bytes of the file; if it's all printable characters, call it good. */
	fl = std::min((unsigned long)fl, 128UL);
	FILE* f = fopen(pathname, "rb");
	fread(buf, 1, fl, f);
	fclose(f);
	for (u32 i = 0; i < fl; ++i) {
		if (!isprint(buf[i]))
			return false;
	}
	return true;
}

const char* timepr(u64 mills) {
	static char st[4][32];
	static u32 idx = 0;
	char* ret;
	int min = mills / 60000;
	int sec = mills / 1000;
	int mil = int(mills - sec * 1000);
	sec -= min * 60;
	ret = st[idx++];
	if (idx > 3)
		idx = 0;
	sprintf(ret, "%02d:%02d.%03d", min, sec, mil);
	return ret;
}

static genann* gn_read_from_file(const char* fname) {
	FILE* f = fopen(fname, "r");
	genann* ret = genann_read(f);
	fclose(f);
	return ret;
}

static void gn_out_to_file(genann* nnet, const char* fname) {
	FILE* f = fopen(fname, "w");
	genann_write(nnet, f);
	fclose(f);
}

/*** genann and kann ramfile I/O ***/

static genann* gn_read_from_ramfile(RamFile* rf) {
	/* Perform a genann read from a RamFile (which can be compressed.) */
	i32 inputs, hidden_layers, hidden, outputs;
	inputs = rf->get32();
	hidden_layers = rf->get32();
	hidden = rf->get32();
	outputs = rf->get32();
	genann* ann = genann_init(inputs, hidden_layers, hidden, outputs);
	NOT_NULL_OR_RETURN(ann, ann);

	for (int i = 0; i < ann->total_weights; ++i) {
		ann->weight[i] = rf->getdouble();
	}

	return ann;
}

static void gn_out_to_ramfile(RamFile* rf, genann* nnet) {
	/* Perform a genann write to a RamFile (which can and will be compressed.) */
	rf->put32(nnet->inputs);
	rf->put32(nnet->hidden_layers);
	rf->put32(nnet->hidden);
	rf->put32(nnet->outputs);
	for (int i = 0; i < nnet->total_weights; ++i) {
		rf->putdouble(nnet->weight[i]);
	}
}

static void kad1_out_to_ramfile(RamFile* rf, const kad_node_t* p) {
	rf->put32(p->ext_label);
	rf->putu32(p->ext_flag);
	rf->putc(p->flag);
	rf->put32(p->n_child);
	if (p->n_child) {
		i32 j, pre = p->pre? p->pre->tmp : -1;
		rf->putu16(p->op);
		for (j = 0; j < p->n_child; ++j)
			rf->put32(p->child[j]->tmp);
		rf->put32(pre);
		rf->put32(p->ptr_size);
		if (p->ptr_size > 0 && p->ptr)
			rf->putmem((const char*)(p->ptr), p->ptr_size);
	} else {
		rf->putc(p->n_d);
		for (int e = 0; e < p->n_d; ++e)
			rf->put32(p->d[e]);
	}
}

static void kad_out_to_ramfile(RamFile* rf, int n_node, kad_node_t **node) {
	i32 i, k = n_node;
	rf->put32(k);
	for (i = 0; i < n_node; ++i)
		node[i]->tmp = i;
	for (i = 0; i < n_node; ++i)
		kad1_out_to_ramfile(rf, node[i]);
	for (i = 0; i < n_node; ++i)
		node[i]->tmp = 0;
}

static void kann_out_to_ramfile(RamFile* rf, kann_t* nnet) {	
	kann_set_batch_size(nnet, 1);
	kad_out_to_ramfile(rf, nnet->n, nnet->v);
	rf->putmem((const char*)nnet->x, sizeof(float) * kann_size_var(nnet));
	rf->putmem((const char*)nnet->c, sizeof(float) * kann_size_const(nnet));
}

static kad_node_t *kad1_from_ramfile(RamFile* rf, kad_node_t **node) {
	kad_node_t *p;
	p = (kad_node_t*)calloc(1, sizeof(kad_node_t));
	p->ext_label = rf->get32();
	p->ext_flag = rf->getu32();
	p->flag = (u8)rf->getc();
	p->n_child = rf->get32();
	if (p->n_child) {
		int32_t j, k;
		p->child = (kad_node_t**)calloc(p->n_child, sizeof(kad_node_t*));
		p->op = rf->getu16();
		for (j = 0; j < p->n_child; ++j) {
			k = rf->get32();
			p->child[j] = node ? node[k] : 0;
		}
		k = rf->get32();
		if (k >= 0)
			p->pre = node[k];
		p->ptr_size = rf->get32();
		if (p->ptr_size > 0) {
			p->ptr = malloc(p->ptr_size);
			rf->getmem((u8*)p->ptr, p->ptr_size);
		}
	} else {
		p->n_d = (u8)rf->getc();
		if (p->n_d) {
			for (int e = 0; e < p->n_d; ++e)
				p->d[e] = rf->get32();
		}
	}
	return p;
}

static kad_node_t **kad_read_from_ramfile(RamFile* rf, int *_n_node) {
	i32 i, n_node;
	kad_node_t **node;
	n_node = rf->get32();
	node = (kad_node_t**)malloc(n_node * sizeof(kad_node_t*));
	NOT_NULL_OR_RETURN(node, node);
	for (i = 0; i < n_node; ++i) {
		kad_node_t *p;
		p = node[i] = kad1_from_ramfile(rf, node);
		if (p->n_child) {
			kad_op_list[p->op](p, KAD_ALLOC);
			kad_op_list[p->op](p, KAD_SYNC_DIM);
		}
	}
	*_n_node = n_node;
	kad_mark_back(n_node, node);
	return node;
}

static kann_t *kann_read_from_ramfile(RamFile* rf) {
	kann_t *ret;
	int n_var, n_const;

	ret = (kann_t*)calloc(1, sizeof(kann_t));
	NOT_NULL_OR_RETURN(ret, ret);
	ret->v = kad_read_from_ramfile(rf, &ret->n);
	n_var = kad_size_var(ret->n, ret->v);
	n_const = kad_size_const(ret->n, ret->v);
	ret->x = (float*)malloc(n_var * sizeof(float));
	ret->g = (float*)calloc(n_var, sizeof(float));
	ret->c = (float*)malloc(n_const * sizeof(float));
	rf->getmem((u8*)ret->x, sizeof(float) * n_var);
	rf->getmem((u8*)ret->c, sizeof(float) * n_const);
	kad_ext_sync(ret->n, ret->v, ret->x, ret->g, ret->c);

	return ret;
}

const u32 FILENAME_INVALID = 0xfffffffful;
const u32 NNET_FORMAT_GENANN = 0;
const u32 NNET_FORMAT_KANN = 1;

static bool nearest_neighbor(SBitmap* bin, SBitmap* berase, int x, int y, RGBColor& c_out) {
	RGBColor n[8], avg;
	u32 cn = 0, bs, dm;
	int dx, dy;
	int r = 0, g = 0, b = 0;

	for (dy = -1; dy <= 1; ++dy) {
		for (dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0)
				continue;
			if (!pixel_ok(berase, x + dx, y + dy))
				continue;
			if (berase->get_red(x + dx, y + dy) != 0)
				continue;
			n[cn] = bin->get_pixel(x + dx, y + dy);
			r += RGB_RED(n[cn]);
			g += RGB_GREEN(n[cn]);
			b += RGB_BLUE(n[cn]);
			++cn;
		}
	}

	if (0 == cn)
		return false;
	c_out = n[0];
	if (1 == cn)
		return true;

	r /= cn;
	g /= cn;
	b /= cn;
	avg = MAKE_RGB(r, g, b);
	bs = color_distance(c_out, avg);
	for (dx = 1; dx < cn; ++dx) {
		dm = color_distance(n[dx], avg);
		if (dm < bs) {
			bs = dm;
			c_out = n[dx];
		}
	}

	return true;
}

/* Compare against replacing missing pixels with the best average pixel (or last valid, that failing). */
static double error_bmp_best_neighbor_pixel(SBitmap* o, SBitmap* e) {
	RGBColor last = RGB_GRAY(127);
	u64 ret = 0;
	u32 c = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1, cn;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				if (nearest_neighbor(o, e, x, y, cn))
					ret += color_distance(c1, cn);
				else
					ret += color_distance(c1, last);
				c += 3;
			} else {
				last = c1;
			}
		}
	}
	return double(ret) / double(c);
}

/* Compare against replacing missing pixels with the last valid pixel. */
static double error_bmp_last_pixel(SBitmap* o, SBitmap* e) {
	RGBColor last = RGB_GRAY(127);
	u64 ret = 0;
	u32 c = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				ret += color_distance(c1, last);
				c += 3;
			} else {
				last = c1;
			}
		}
	}
	return double(ret) / double(c);
}

/* Compare against the average color of erased pixels. */
static double error_bmp_avg(SBitmap* o, SBitmap* e) {
	u64 ret = 0;
	u64 r = 0, g = 0, b = 0;
	u32 ce = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				++ce;
				r += RGB_RED(c1);
				g += RGB_GREEN(c1);
				b += RGB_BLUE(c1);
			}
		}
	}
	if (0 == ce)
		return 0ULL;
	r /= ce;
	g /= ce;
	b /= ce;
	RGBColor ca = RGB_NO_CHECK(r, g, b);
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			if (e->get_red(x, y) > 0) {
				RGBColor c1;
				c1 = o->get_pixel(x, y);
				ret += color_distance(c1, ca);
			}
		}
	}

	return double(ret) / double(ce * 3);
}

static double error_bmp(SBitmap* b1, SBitmap* b2, SBitmap* e) {
	u64 ret = 0;
	u32 ce = 0;
	for (int y = 0; y < b1->height(); ++y) {
		for (int x = 0; x < b1->width(); ++x) {
			RGBColor c1, c2;
			if (e->get_red(x, y) > 0)
				ce += 3;
			c1 = b1->get_pixel(x, y);
			c2 = b2->get_pixel(x, y);
			ret += color_distance(c1, c2);
		}
	}
	return double(ret) / double(ce);
}

RGBOut RGBOut::operator+=(const RGBOut& rhs) {
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	return *this;
}

RGBOut RGBOut::operator+(const RGBOut& rhs) {
	RGBOut ret;
	ret.r = r + rhs.r;
	ret.g = g + rhs.g;
	ret.b = b + rhs.b;
	return ret;
}

RGBOut RGBOut::operator*(double rhs) {
	RGBOut ret;
	ret.r = r * rhs;
	ret.g = g * rhs;
	ret.b = b * rhs;
	return ret;
}

/*** Operator implementations for the VerboseStream. ***/

#define VSOP(__TYPE__) \
	VerboseStream& VerboseStream::operator<<(const __TYPE__ i) { \
		if (is_quiet())	return *this;                              \
		*o << i;                                                   \
		return *this;                                              \
	}

VSOP(std::string&);
VSOP(char*);
VSOP(int);
VSOP(unsigned int);
VSOP(int64_t);
VSOP(uint64_t);
VSOP(float);
VSOP(double);
#ifdef __GNUC__
VSOP(long double);
#endif
VSOP(unsigned char);
VSOP(signed char);
VSOP(int16_t);
VSOP(uint16_t);
VSOP(void * const);
#undef VSOP

VerboseStream& VerboseStream::operator<<(const bool b) {
	if (is_quiet())
		return *this;
	*o << (b ? "true" : "false");
	return *this;
}

void PredictAccum::add_prediction(int x, int y, RGBOut& predict) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		predictions[pr] = std::make_pair(predict, 1L);
		return;
	}
	auto p = predictions[pr];
	p.first += predict;
	p.second++;
	predictions[pr] = p;
}

void PredictAccum::add_prediction(int x, int y, RGBOut& predict, int weight) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		predictions[pr] = std::make_pair(predict, weight);
		return;
	}
	auto p = predictions[pr];
	p.first += (predict * double(weight));
	p.second += weight;
	if (p.second > (1 << 26)) {
		p.first = p.first * (1.0 / 1024.0);
		p.second /= 1024;
	}
	predictions[pr] = p;
}

void PredictAccum::get_avg_prediction(int x, int y, RGBOut& p) {
	u32 c = get_num_predictions(x, y);
	if (0 == c) {
		p.r = 0.;
		p.g = 0.;
		p.b = 0.;
		return;
	}
	get_total_prediction(x, y, p);
	p.r /= (double)c;
	p.g /= (double)c;
	p.b /= (double)c;
	p.r = CLAMP(p.r, 0.0, 1.0);
	p.g = CLAMP(p.g, 0.0, 1.0);
	p.b = CLAMP(p.b, 0.0, 1.0);
}

void PredictAccum::get_total_prediction(int x, int y, RGBOut& p) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		p.r = 0.;
		p.g = 0.;
		p.b = 0.;
		return;
	}
	p = predictions[pr].first;
	return;
}

u32 PredictAccum::get_num_predictions(int x, int y) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end())
		return 0;
	return predictions[pr].second;
}

void PredictAccum::reset() {
	predictions.clear();
}

void PredictAccum::fold_in(PredictAccum& pa) {
	for (auto& e : pa.predictions) {
		auto& pt = e.first;
		auto& pred_pr = e.second;
		if (predictions.find(pt) == predictions.end()) {
			predictions[pt] = pred_pr;
		} else {
			auto p = predictions[pt];
			p.first += pred_pr.first;
			p.second += pred_pr.second;
			predictions[pt] = p;
		}
	}
}

/* Initialize an empty ImgNNet. */
ImgNNet::ImgNNet() {
	vs = VerboseStream(true);
	set_default_parameters();
	nnet = nullptr;
	nnet_best = nullptr;
	in = nullptr;
	out = nullptr;
	pw = nullptr;
}

/* Initialize a fresh image neural network with the specified radius. */
ImgNNet::ImgNNet(bool verbose, u32 radius, u32 hidden_layers) {
	vs = VerboseStream(verbose);
	set_default_parameters();
	ImgNNet::inout_from_radius(radius, 0, ni, no);
	d = radius;
	nnet = nnet_init(ni, hidden_layers, neurons, no);
	nnet_best = nnet;
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];
	pw = nullptr;
	vs << "Inputs: " << ni << ", outputs: " << no << ", hidden layers: " << hidden_layers << "\n";
}

/* As above, but allows setting of neurons as well. */
ImgNNet::ImgNNet(bool verbose, u32 radius, u32 nneurons, u32 hidden_layers) {
	vs = VerboseStream(verbose);
	set_default_parameters();
	neurons = nneurons;
	ImgNNet::inout_from_radius(radius, 0, ni, no);
	d = radius;
	nnet = nnet_init(ni, hidden_layers, neurons, no);
	nnet_best = nnet;
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];
	pw = nullptr;
	vs << "Inputs: " << ni << ", outputs: " << no << ", hidden layers: " << hidden_layers << "\n";
}

/* As above, but allows setting the library used (genann or kann) and identity training flag. */
ImgNNet::ImgNNet(bool verbose, u32 radius, u32 nneurons, u32 hidden_layers, bool idtrain, u32 library) {
	vs = VerboseStream(verbose);
	set_default_parameters();
	neurons = nneurons;
	ImgNNet::inout_from_radius(radius, 0, ni, no);
	d = radius;
	if (library > NNET_FORMAT_KANN) {
		vs << "Error: maximum library format permissable is " << NNET_FORMAT_KANN << ".\n";
		return;
	}
	nnet_ver = library;
	identity = idtrain;
	nnet = nnet_init(ni, hidden_layers, neurons, no);
	nnet_best = nnet;
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];
	pw = nullptr;
	vs << "Inputs: " << ni << ", outputs: " << no << ", hidden layers: " << hidden_layers << "\n";
}

/* Create a colorization neural network. */
ImgNNet::ImgNNet(bool verbose, u32 rad, u32 r2, u32 nneurons, u32 hidden_layers, bool idtrain, u32 library) {
	vs = VerboseStream(verbose);
	set_default_parameters();
	colorize = true;
	neurons = nneurons;
	assert(r2 > 0 && r2 <= rad);
	ImgNNet::inout_from_radius(rad, r2, ni, no);
	d = rad;
	d2 = r2;
	if (library > NNET_FORMAT_KANN) {
		vs << "Error: maximum library format permissable is " << NNET_FORMAT_KANN << ".\n";
		return;
	}
	nnet_ver = library;
	identity = idtrain;
	nnet = nnet_init(ni, hidden_layers, neurons, no);
	nnet_best = nnet;
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];
	pw = nullptr;
	vs << "Inputs: " << ni << ", outputs: " << no << ", hidden layers: " << hidden_layers << "\n";
}

/* Load an existing neural network from file. */
ImgNNet::ImgNNet(bool verbose, const char* pathname) {
	vs = VerboseStream(verbose);
	pw = nullptr;
	read_from_file(pathname);
}

ImgNNet::ImgNNet(bool verbose, const char* pathname, u32 radius, u32 hidden_layers) {
	vs = VerboseStream(verbose);
	vs << "Loading the neural network...\n";
	pw = nullptr;
	if (FileExists(pathname)) {
		read_from_file(pathname);
	} else {
		vs << "(Neural net doesn't exist, creating.)\n";
		set_default_parameters();
		ImgNNet::inout_from_radius(radius, 0, ni, no);
		d = radius;
		nnet = nnet_init(ni, hidden_layers, neurons, no);
		nnet_best = nnet;
		pw = nullptr;
		in = new double [ni];
		out = new double [no];		
		fin = new float [ni];
		fout = new float [no];
		vs << "Inputs: " << ni << ", outputs: " << no << ", hidden layers: " << hidden_layers << "\n";
	}
}

const u32 BATCH_TRAIN_SIZE = 1000;

ImgNNet::~ImgNNet() {
	if (!is_null(nnet))
		nnet_free(nnet);
	if (!is_null(nnet_best) && nnet_best != nnet)
		nnet_free(nnet_best);
	if (out != nullptr)
		delete [] out;
	if (in != nullptr)
		delete [] in;
	if (batch_in != nullptr) {
		for (u32 e = 0; e < BATCH_TRAIN_SIZE; ++e) {
			delete [] batch_in[e];
			delete [] batch_out[e];
		}
		delete [] batch_in;
		delete [] batch_out;
	}
}

/* Save the neural network to file. */
void ImgNNet::out_to_file(const char* pathname) {
#ifdef USE_GENANN_SAVE_ONLY
	gn_out_to_file(nnet_best, pathname);
#else
	out_to_ramfile(pathname);
#endif
}

/* Outputs the neural network to disk, if there's a filename. */
void ImgNNet::update_on_disk() {
	if (nn_fname.empty()) {
		vs << "Neural net filename empty, not updating on disk.\n";
		return;
	}
	out_to_file(nn_fname.c_str());
	vs << "Neural network update successful.\n";
}

const i32 IMGNNET_MAGIC = 10891234;
const u32 VERSION_MIN = 1000000;

void ImgNNet::out_to_ramfile(const char* pathname) {
	if (FileExists(pathname)) {
		remove(pathname);
	}
	RamFile rf;
	if (rf.open(pathname, RAMFILE_COMPRESS | RAMFILE_CREATE_IF_MISSING)) {
		vs << "Unable to open " << pathname << "!\n";
		return;
	}

	rf.put32(IMGNNET_MAGIC);
	rf.putu32(d);
	rf.putu32(ni);
	rf.putu32(no);
	rf.putu32(neurons);
	rf.put32((i32)space);
	rf.putdouble(lrate);
	rf.putbool(use_neighbors);
	rf.putu32(max_neighbors);
	rf.putu32(max_neighbor_pass);
	rf.putbool(vs.is_verbose());
	rf.putbool(flip);
	rf.putu32(max_size);
	rf.putu32(max_iter);
	rf.putu32(max_retry);
	rf.putu32(mp);
	rf.putbool(out_erased);
	rf.putbool(out_neighb);
	// *** We used to just call gn_out_to_ramfile(), but now we output an entire version data block to support 
	// more formats for both neural nets and TrainData, as well as colorization neural networks. ***
	rf.putu32(nnet_ver + VERSION_MIN);
	nnet_out_to_ramfile(&rf);
	rf.putbool(colorize);
	rf.putbool(identity);
	if (colorize)
		rf.putc((u8)d2);
	for (int e = 0; e < 64; ++e)
		rf.putc(0);
	// *** Version block ends. ***
	st.persist(&rf);
	traindata_to_ramfile(&rf);
	vsd.out_to_ramfile(&rf);
	// This final u32 is essentially a version number, for easier format extensibility.
	rf.putu32(0);

	rf.close();
}

/* Load the neural network from file. */
void ImgNNet::read_from_file(const char* pathname) {
	if (file_is_text(pathname))
		read_from_genann_file(pathname);
	else
		read_ramfile_format(pathname);
}

void ImgNNet::read_from_genann_file(const char* pathname) {
	nnet = gn_read_from_file(pathname);
	no = ((genann *)nnet)->outputs;
	ni = ((genann *)nnet)->inputs;
	u32 x = 0;
	radius_from_inout(this->d, x, false);
	if (0 == d) {
		vs << "Unknown neural network structure.\n";
		return;
	} else {
		vs << "Loaded neural network with window radius " << d << ", " << ni << " inputs, " << no << " outputs.\n";
	}
	nnet_best = nnet;
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];

	/* Other parameters to defaults. */
	set_default_parameters();
	nn_fname = pathname;
	vsd.rad = d;
}

void ImgNNet::read_ramfile_format(const char* pathname) {
	RamFile rf;
	u32 i, o;
	if (rf.open(pathname, RAMFILE_READONLY)) {
		vs << "Unable to open " << pathname << "!\n";
		return;
	}

	if (rf.get32() != IMGNNET_MAGIC) {
		vs << "Error! " << pathname << " is not a valid imgnnet file!\n";
		return;
	}

	set_default_parameters();
	d = rf.getu32();
	ni = rf.getu32();
	no = rf.getu32();
	neurons = rf.getu32();
	space = (colorspace)rf.get32();
	lrate = rf.getdouble();
	clrate = lrate;
	use_neighbors = rf.getbool();
	max_neighbors = rf.getu32();
	max_neighbor_pass = rf.getu32();
	if (rf.getbool())
		vs.verbose();
	else
		vs.quiet();
	flip = rf.getbool();
	max_size = rf.getu32();
	max_iter = rf.getu32();
	max_retry = rf.getu32();
	mp = rf.getu32();
	out_erased = rf.getbool();
	out_neighb = rf.getbool();

	if (rf.peeku32() < VERSION_MIN) {
		// No version number, this is straight-up a genann neural network.
		nnet_best = (void *)gn_read_from_ramfile(&rf);
		if (is_null(nnet_best)) {
			vs << "Error loading the neural network.\n";
			return;
		}
		nnet = nnet_best;
		verdata = false;
		colorize = false;
		identity = false;
		nnet_ver = NNET_FORMAT_GENANN;
	} else {
		// Read the version data block.
		nnet_ver = rf.getu32() - VERSION_MIN;
		verdata = true;
		nnet_best = nnet_from_ramfile(&rf);
		nnet = nnet_best;
		colorize = rf.getbool();
		identity = rf.getbool();
		if (colorize)
			d2 = (u32)rf.getc();
		// Block of 64 bytes. This is reserved for future feature expansion.
		for (int e = 0; e < 64; ++e)
			ignore_return_val(rf.getc());
	}

	inout_from_radius(d, d2, i, o);
	if (ni != i || no != o) {
		vs << "Error! Unrecognized neural net structure.\n";
		return;
	}
	in = new double [ni];
	out = new double [no];
	fin = new float [ni];
	fout = new float [no];

	if (nnet_ver == NNET_FORMAT_GENANN) {
		ship_assert(((genann *)nnet)->outputs == no && ((genann *)nnet)->inputs == ni);
	}
	st.load(&rf);
	traindata_from_ramfile(&rf);
	vsd.in_from_ramfile(&rf);
	vsd.rad = d;
	// we ignore the version number, for now
	// u32 version = rf.getu32();
	nn_fname = pathname;

	rf.close();
}

void ImgNNet::dump() {
	vs << "Filename        : " << nn_fname << "\n";
	vs << "Input radius    : " << d << "\n";
	vs << "# inputs        : " << ni << "\n";
	vs << "# outputs       : " << no << "\n";
	vs << "Neurons/hidden  : " << neurons << "\n";
	if (nnet_ver == NNET_FORMAT_GENANN) {
		vs << "# hidden layers : " << ((genann *)nnet_best)->hidden_layers << "\n";
		vs << "# weights total : " << ((genann *)nnet_best)->total_weights << "\n";
		vs << "# neurons total : " << ((genann *)nnet_best)->total_neurons << "\n";
	}
	vs << "Colorspace      : " << colorspace_name(space) << "\n";
	vs << "Learning rate   : " << lrate << "\n";
	vs << "Use neighbors   : " << use_neighbors << "\n";
	vs << "Max neighbors   : " << max_neighbors << "\n";
	vs << "Neighbor passes : " << max_neighbor_pass << "\n";
	vs << "Training flipped: " << flip << "\n";
	vs << "Max image size  : " << max_size << "\n";
	vs << "Max iterations  : " << max_iter << "\n";
	vs << "Max retries     : " << max_retry << "\n";
	vs << "Min predictions : " << mp << "\n";
	vs << "Output erased   : " << out_erased << "\n";
	vs << "Output neighbor : " << out_neighb << "\n";
	vs << "Neural net fmt  : " << (nnet_ver == NNET_FORMAT_GENANN ? "genann" : "kann") << "\n";
	vs << "Colorize        : " << colorize << "\n";
	vs << "Identity train. : " << identity << "\n";
	if (colorize)
		vs << "Output radius   : " << d2 << "\n";

	vs << "=== Contents of string table ===\n";
	for (u32 idx = 0; idx < st.cstr(); ++idx) {
		const char* str = st.string(idx);
		vs << "[" << idx << "]: " << str << "\n";
	}

	vs << "----> Contents of TrainVec <----\n";
	for (const auto& e : train_data) {
		const auto& v = e.second;
		for (const auto& td : v) {
			td.dump(st, vs);
		}
	}

	vsd.dump(st, vs);

	if (nnet_ver == NNET_FORMAT_GENANN) {
		vs << "*** First ten neural net weights follow:\n";
		for (int e = 0; e < 10 && e < ((genann *)nnet_best)->total_weights; ++e) {
			vs << "\t" << ((genann *)nnet_best)->weight[e] << "\n";
		}
	}
}

/* Helper function: set default parameters when we load from a bare genann file or create a new nnet. */
void ImgNNet::set_default_parameters(void) {
	space = colorspace_rgb;
	train_flips(false);
	max_size = 800;
	min_predict_low();
	pass = 0;
	lrate = 0.005;
	clrate = 0.005;
	neurons = 1200;
	use_neighbors = false;
	max_neighbors = 6;
	max_neighbor_pass = 8;
	max_iter = 3;
	max_retry = 1;
	fn_index = FILENAME_INVALID;
	nn_fname.clear();
	out_erased = true;
	out_neighb = true;
	maxmt = 1;
	verdata = false;
	colorize = false;
	identity = false;
	nnet_ver = NNET_FORMAT_GENANN;
	batch_in = nullptr;
	batch_out = nullptr;
	batch_i = 0;
	d2 = 0;
}

const u32 MAXIMGNNT_THREADS = 64;

void ImgNNet::set_max_threads(u32 v) {
	u32 nt = std::thread::hardware_concurrency();
	nt = CLAMP(nt, 1, MAXIMGNNT_THREADS);
	v = CLAMP(v, 1, nt);
	maxmt = v;
	vs << "Maximum number of concurrent evaluation threads set to " << v << ".\n";
}

/* Train the neural network on an image. The output structure (if non-null)
   will be filled with information about the training. */
void ImgNNet::train_on_image(SBitmap* bmp, TrainData* traindata_out) {
	// Resize the image if too big.
	SBitmap* bmpuse = bmp;
	bool willflip;
	u32 big_side = std::max(bmp->width(), bmp->height());
	if (max_size > 0 && big_side > max_size)
		bmpuse = bmp->scale_rational(max_size, big_side);

	vs << "Training the neural network... (image dimensions " << bmpuse->width() << " x " << bmpuse->height() << ", " << bmpuse->width() * bmpuse->height() << " pixels)\n";
	willflip = flip && RandBool();
	if (willflip) {
		// We train on a horizontally flipped version of the image.
		vs << "Training on the horizontally mirrored image.\n";
		if (bmpuse == bmp) {
			bmpuse = new SBitmap(bmp->width(), bmp->height());
			bmp->blit(bmpuse, SPoint(0, 0));
		}
		bmpuse->flip_horiz();
	}
	train_on_image_core(bmpuse, traindata_out, willflip);

	if (bmpuse != bmp)
		delete bmpuse;
}

void ImgNNet::train_on_image(const char* pathname, TrainData* traindata_out) {
	SBitmap* bmp = SBitmap::load_bmp(pathname);
	if (is_null(bmp)) {
		vs.printf("Error loading bitmap %s.\n", pathname);
		return;
	}
	fn_index = st.index(pathname);
	train_on_image(bmp, traindata_out);
	fn_index = FILENAME_INVALID;
	delete bmp;
}

void ImgNNet::fill_train_inout(SBitmap* bmp, int x, int y, u32& ci, u32& co) {
	fill_train_inout(bmp, x, y, ci, co, in, out);
}

void ImgNNet::fill_train_inout(SBitmap* bmp, int x, int y, u32& ci, u32& co, double* vin, double* vout) {
	int dx, dy, ds;
	if (colorize) {
		/* Colorization. */
		ship_assert(d2 > 0 && d2 <= d);
		for (dy = -((int)(d)); dy <= (int)(d); ++dy) {
			for (dx = -((int)(d)); dx <= (int)(d); ++dx) {
				RGBColor c = bmp->get_pixel(x + dx, y + dy);
				int hh, ss, vv;
				RGB_HSV(RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c), &hh, &ss, &vv);
				ds = (dy * dy) + (dx * dx);
				if (ds <= (int)(d * d)) {
					/* Within d pixels, one of the inputs. */
					vin[ci++] = double(vv) / 255.; // gray_intensity(bmp, x + dx, y + dy);
				}
				if (ds <= (int)(d2 * d2)) {
					/* Within d2 pixels, one of the outputs. */
					vout[co++] = double(hh) / 255.;
					vout[co++] = double(ss) / 255.;
#if 0
					vout[co++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
					vout[co++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
					vout[co++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
#endif
				}
			}
		}
	} else {
		/* Pixel prediction. */
		for (dy = -((int)(d + 1)); dy <= (int)(d + 1); ++dy) {
			for (dx = -((int)(d + 1)); dx <= (int)(d + 1); ++dx) {
				ds = (dy * dy) + (dx * dx);
				if (ds <= (int)(d * d)) {
					/* Within d pixels, one of the inputs. */
					vin[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
					vin[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
					vin[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
				} else if (ds <= (int)(d+1)*(int)(d+1)) {
					/* Within the (d+1)-perimeter, one of the outputs. */
					vout[co++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
					vout[co++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
					vout[co++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
				}
			}
		}
	}
}

void ImgNNet::fill_train_in(SBitmap* bmp, int x, int y, u32& ci, double* tin) {
	int dx, dy, ds;
	if (colorize) {
		/* Colorization. */
		ship_assert(d2 > 0 && d2 <= d);
		for (dy = -((int)(d)); dy <= (int)(d); ++dy) {
			for (dx = -((int)(d)); dx <= (int)(d); ++dx) {
				ds = (dy * dy) + (dx * dx);
				if (ds <= (int)(d * d)) {
					/* Within d pixels, one of the inputs. */
					RGBColor c = bmp->get_pixel(x + dx, y + dy);
					int hh, ss, vv;
					RGB_HSV(RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c), &hh, &ss, &vv);
					tin[ci++] = double(vv) / 255.;
					// tin[ci++] = gray_intensity(bmp, x + dx, y + dy);
				}
			}
		}
	} else {
		/* Pixel prediction. */
		for (dy = -((int)(d + 1)); dy <= (int)(d + 1); ++dy) {
			for (dx = -((int)(d + 1)); dx <= (int)(d + 1); ++dx) {
				ds = (dy * dy) + (dx * dx);
				if (ds <= (int)(d * d)) {
					/* Within d pixels, one of the inputs. */
					tin[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
					tin[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
					tin[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
				}
			}
		}
	}
}

void ImgNNet::train_on_image_core(SBitmap* bmp, TrainData* traindata_out, bool isflipped) {
	TrainData td;
	int x, y;
	u32 ci, co;
	double n_err, err_next;

	td.citer = 1;
	td.retry = 0;
	td.err_in = iterative_error(bmp);
	td.err_out = td.err_in;
	// TODO: set clrate from existing training data, not always from default rate.
	clrate = lrate;
	td.lrate1 = clrate;
	td.lrate_eff = 0.0;
	n_err = td.err_in;
	td.fname_idx = fn_index;
	set_validation_pixels();
	get_current_datetime(&td.tim_start);
	td.flip = isflipped;
	td.img_w = bmp->width();
	td.img_h = bmp->height();

	// The iterative training loop.
	forever {

	/* First we do the training. */

	// Initialization.
	nnet = nnet_copy(nnet_best);
	set_training_pixels();
	batch_train_begin();
	sw.start();

	// The training loop.
	for (y = (d + 1); y < bmp->height() - (int)(d + 2); ++y) {
		if ((y & 1) == 0) {
			vs.printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp->height() - (d + 1), 
				   timepr(sw.stop(UNIT_MILLISECOND)),
				   timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - (d + 1) - y)) / ((y == 0) ? 1 : y)));
		}
		for (x = (d + 1); x < bmp->width() - (int)(d + 2); ++x) {
			ci = 0;
			co = 0;
			if (!is_in_training(x, y)) {
				continue;
			}
			fill_train_inout(bmp, x, y, ci, co);
			ship_assert(ci == ni);
			ship_assert(co == no);
			batch_train(nnet, in, out, clrate);
			// If identity training is mixed in, we do an iteration 4% of the time.
			if (identity && !colorize && RandPercent(4)) {
				train_identity_iter();
			}
		}
	}
	batch_train_end(nnet, clrate);
	vs.printf("Completed in [%s]                         \n", timepr(sw.stop(UNIT_MILLISECOND)));
	sw.start();

	/* Now verify the training. */
	err_next = iterative_error(bmp);
	if (err_next < n_err) {
		ship_assert(nnet != nnet_best);
		nnet_free(nnet_best);
		nnet_best = nnet;
		n_err = err_next;
		if (!nn_fname.empty()) {
			vs << "Error improved! Saving the neural network to file.\n";
			out_to_file(nn_fname.c_str());
		} else {
			vs << "Error improved! Updating the neural network.\n";
		}
		td.err_out = n_err;
		td.lrate = clrate;
		td.lrate_eff = clrate;
		if (td.citer >= max_iter) {
			vs << "Maximum number of iterations performed.\n";
			break;
		}
	} else {
		ship_assert(nnet != nnet_best);
		nnet_free(nnet);
		nnet = nnet_best;
		td.lrate = clrate;
		if (td.retry >= max_retry) {
			vs << "Error did not improve, and we have reached the maximal number of retries.\n";
			break;
		} else {
			vs << "Error did not improve: lowering the learning rate and retrying iteration.\n";
			clrate /= 2.0;
			++td.retry;
			continue;
		}
		break;
	}

	if (1 == td.citer) {
		clrate *= 0.50;
		vs << "First iteration done; setting learning rate to " << clrate << ".\n";
	}

	++td.citer;
	vs << "Starting iteration #" << td.citer << "...\n";

	}  // forever (iterative training loop)

	/* Finish this training iteration. */
	get_current_datetime(&td.tim_end);
	// Save the training data, and set traindata_out if it exists.
	train_data[fn_index].push_back(td);
	if (!is_null(traindata_out)) {
		*traindata_out = td;
	}
#ifndef USE_GENANN_SAVE_ONLY
	if (!nn_fname.empty()) {
		vs << "Saving the neural network with training data to file.\n";
		out_to_file(nn_fname.c_str());
	}
#endif

	return;
}

/* Predict the portions of the bitmap specified by 'erased' (black pixel = not erased, any
   other pixel = erased). You can use monochrome 1bpp and 'erased' is a literal bitmap. */
SBitmap* ImgNNet::predict_from_missing(SBitmap* bmp, SBitmap* erased) {
	if (colorize) {
		return nullptr;
	}
	if (maxmt > 1) {
		return predict_from_missing_mt(bmp, erased);
	}
	SBitmap* bmpcopy = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ecopy = new SBitmap(bmp->width(), bmp->height());
	u32 min_pred = 4;
	Stopwatch swp;
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(erased, erased);
	NOT_NULL_OR_RETURN(bmpcopy, bmpcopy);
	NOT_NULL_OR_RETURN(ecopy, ecopy);
	ret->clear();
	bmp->blit(bmpcopy, SPoint(0, 0));
	erased->blit(ecopy, SPoint(0, 0));

	vs << "Predicting erased portions of bitmap on " << (d + 1) << "-perimeter.\n";
	pass = 0;
	swp.start();

	count_erased(ecopy);
	if (pw) {
		pw->nerased_in = ce_st;
		pw->nerased = ce_st;
		pw->ret = ret;
		pw->erase = ecopy;
		pw->done = false;
		pw->pass = 1;
		pw->ace = -1.;
	}

	while (predict_pass_from_missing(bmpcopy, ecopy, ret))
		no_op;

	if (pw) {
		pw->done = true;
		pw->erase = nullptr;
		pw->ace = error_bmp(bmp, ret, erased);
	}
	fillsettings fs;
	fs.size = 8;
	fs.background = RGB_GRAY(192);
	fs.foreground = RGB_GRAY(128);
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (erased->get_red(x, y) != 0)
				bmpcopy->put_pixel(x, y, checkerboard_pattern(x, y, (void*)&fs));
		}
	}
	vs << "Error between original and erased   : " << error_bmp(bmp, bmpcopy, erased) << "\n";
	vs << "Error between original and avg.pixel: " << error_bmp_avg(bmp, erased) << "\n";
	vs << "Error between original and lastpixel: " << error_bmp_last_pixel(bmp, erased) << "\n";
	vs << "Error between original and neighbors: " << error_bmp_best_neighbor_pixel(bmp, erased) << "\n";
	vs << "Error between original and predicted: " << error_bmp(bmp, ret, erased) << "\n";

	if (out_erased) {
		bmpcopy->save_bmp("predict_in.png");
		vs << "The erased bitmap is saved as predict_in.png\n";
	}
	if (out_neighb) {
		get_best_neighbors_bmp(bmpcopy, erased);
		bmpcopy->save_bmp("predict_in2.png");
		vs << "The 'best match neighbors' prediction bitmap is saved as 'predict_ibn.png'\n";
	}

	vs.printf("Prediction complete [%s]\n", timepr(swp.stop(UNIT_MILLISECOND)));

	return ret;
}

/* This is glorious and everything, but I don't want the entire bitmap returned, just the
   predicted portions. If the predicted portion is a rectangle, you'll just get that. If
   it's not (i.e. there's non-predicted pixels within its boundaries) you'll get the whole
   bounding rect, but the non-predicted pixels will be black with full alpha transparency
   (i.e. with blit_blend() you can just slam them onto another bitmap.) */
SBitmap* ImgNNet::predicted_from_missing(SBitmap* bmp, SBitmap* erased) {
	if (colorize) {
		return nullptr;
	}
	int x1 = 999999, x2 = 0, y1 = 999999, y2 = 0;
	SBitmap* p = predict_from_missing(bmp, erased);

	for (int y = 0; y < erased->height(); ++y) {
		for (int x = 0; x < erased->width(); ++x) {
			if (erased->get_red(x, y) == 0)
				continue;
			x1 = std::min(x1, x);
			x2 = std::max(x2, x);
			y1 = std::min(y1, y);
			y2 = std::max(y2, y);
		}
	}
	if (x2 < x1 || y2 < y1)
		return nullptr;

	SBitmap* ret = new SBitmap(x2 - x1 + 1, y2 - y1 + 1);
	NOT_NULL_OR_RETURN(ret, ret);
	// SBitmap::clear() fills the alpha channel with full opacity, mainly because the default primitives may ignore alpha
	ret->clear();
	for (int y = y1; y <= y2; ++y) {
		for (int x = x1; x <= x2; ++x) {
			if (erased->get_red(x, y) == 0) {
				ret->set_alpha(x - x1, y - y1, ALPHA_TRANSPARENT);
			} else {
				ret->put_pixel(x - x1, y - y1, p->get_pixel(x, y));
			}
		}
	}
	delete p;
	return ret;
}

/* Return an appropriate erased bitmap for the passed-in bitmap and erase type. */
SBitmap* ImgNNet::get_erased_bmp(SBitmap* bmp, EraseType et) {
	SBitmap* e = new SBitmap(bmp->width(), bmp->height());
	e->clear();
	switch (et) {
	case ERASE_CENTER:
		e->rect_fill(SPoint(40, POINT_PERCENT, 40, POINT_PERCENT), SPoint(60, POINT_PERCENT, 60, POINT_PERCENT), C_WHITE);
		break;

	case ERASE_DICE:
		e->rect_fill(SPoint(40, POINT_PERCENT, 40, POINT_PERCENT), SPoint(60, POINT_PERCENT, 60, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(0, 0), SPoint(20, POINT_PERCENT, 20, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(80, POINT_PERCENT, 0, POINT_PERCENT), SPoint(100, POINT_PERCENT, 20, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(0, POINT_PERCENT, 80, POINT_PERCENT), SPoint(20, POINT_PERCENT, 100, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(80, POINT_PERCENT, 80, POINT_PERCENT), SPoint(100, POINT_PERCENT, 100, POINT_PERCENT), C_WHITE);
		break;

	case ERASE_BANDS:
		for (int x = 3; x < e->width(); x += 14)
			e->vline(x, 0, e->height(), C_WHITE);
		for (int y = 5; y < e->height(); y += 14)
			e->hline(0, e->width(), y, C_WHITE);
		break;

	case ERASE_RANDOM_RECTS:
		for (int c = 0; c < 40; ) {
			SCoord co;
			e->random_rect(&co);
			if (co.dx() * co.dy() > 4000)
				continue;
			e->rect_fill(co, C_WHITE);
			++c;
		}
		break;

	case ERASE_STATIC:
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 19) == 0)
					e->put_pixel(x, y, C_WHITE);
			}
		}
		break;

	case ERASE_STATIC2:
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 39) == 0)
					e->rect_fill(x, y, x + 1, y + 1, C_WHITE);
			}
		}
		break;

	case ERASE_STATIC3:
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 19) == 0)
					e->rect_fill(x, y, x + 1, y + 1, C_WHITE);
			}
		}
		break;

	case ERASE_HALFSAW:
		for (int x = 0; x < 10; ++x) {
			e->line(SPoint(x * 10, POINT_PERCENT, 55, POINT_PERCENT), SPoint(x * 10 + 5, POINT_PERCENT, 45, POINT_PERCENT), C_WHITE);
			e->line(SPoint(x * 10 + 5, POINT_PERCENT, 45, POINT_PERCENT), SPoint(x * 10 + 10, POINT_PERCENT, 55, POINT_PERCENT), C_WHITE);
		}
		e->floodfill(SPoint(50, POINT_PERCENT, 90, POINT_PERCENT), C_WHITE);
		break;

	case ERASE_THICK_LINES:
		for (int x = 0; x < 50; ++x) {
			float r = (float)RandDouble(2.0, 5.0);
			SPoint p1, p2;
			e->random_pixel(&p1);
			e->random_pixel(&p2);
			e->aathickline(p1, p2, r, C_WHITE);
		}
		break;
	}
	return e;
}

/* Helper function: predict from erased. */
bool ImgNNet::predict_pass_from_missing(SBitmap* bin, SBitmap* berase, SBitmap* bout) {
	PredictAccum pa;
	bool ret = false;
	double const* pout;
	int x, y, dx, dy, ds;
	u32 ci, co, ce = 0, cneighbors;
	RGBOut ov;

	ship_assert(!colorize);
	++pass;
	if (pw)
		pw->pass = pass;
	vs << "Pass " << pass << "... ";

	for (y = d; y < bin->height() - d; ++y) {
		for (x = d; x < bin->width() - d; ++x) {
			bool any_erased = false;
			for (dy = -((int)(d + 1)); dy <= (int)(d+1) && !any_erased; ++dy) {
				for (dx = -((int)(d + 1)); dx <= (int)(d+1) && !any_erased; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds > (int)(d+1)*(int)(d+1) || ds <= (int)(d*d))
						continue;
					if (!pixel_ok(berase, x + dx, y + dy))
						continue;
					if (berase->get_red(x + dx, y + dy) != 0)
						any_erased = true;
				}
			}
			if (!any_erased)
				continue;

			/* At least one pixel on our (d+1)-perimeter was erased; let's calculate our predictions. */
			ci = 0;
			cneighbors = 0;
			for (dy = -((int)d); dy <= ((int)d); ++dy) {
				for (dx = -((int)d); dx <= ((int)d); ++dx) {
					if (!pixel_ok(berase, x + dx, y + dy))
						continue;
					ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d * d)) {
						/* Within d pixels, one of the inputs. */
						if (berase->get_red(x + dx, y + dy) != 0) {
							/* If we're allowed to use neighbors, we might still be able to make this prediction. */
							if (!use_neighbors || pass > max_neighbor_pass)
								continue;
							RGBColor cn;
							bool gotn = nearest_neighbor(bin, berase, x + dx, x + dy, cn);
							if (!gotn)
								continue;
							/* Won't you be my neighbor? */
							in[ci++] = double(RGB_RED(cn)) / 255.0;
							in[ci++] = double(RGB_GREEN(cn)) / 255.0;
							in[ci++] = double(RGB_BLUE(cn)) / 255.0;
							++cneighbors;
							continue;
						}
						in[ci++] = channel_intensity(bin, CHANNEL_RED, x + dx, y + dy);
						in[ci++] = channel_intensity(bin, CHANNEL_GREEN, x + dx, y + dy);
						in[ci++] = channel_intensity(bin, CHANNEL_BLUE, x + dx, y + dy);
					}
				}
			}
			if (ci < ni) {
				/* Not enough valid inputs, must have some erased with no neighbors permitted. Skip. */
				continue;
			}
			if (use_neighbors && cneighbors > max_neighbors) {
				/* Too many neighbors? That's not very Mr. Rogers! */
				continue;
			}
			/* Inputs filled, now calculate outputs and populate predictions for (d+1)-perimeter. */
			co = 0;
			ship_assert(ci == ni);
			pout = nnet_run(nnet, in);
			for (dy = -((int)(d+1)); dy <= ((int)(d+1)); ++dy) {
				for (dx = -((int)(d+1)); dx <= ((int)(d+1)); ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d*d)) {
						/* Part of the (d+1)-perimeter. */
						ov.r = pout[co++];
						ov.g = pout[co++];
						ov.b = pout[co++];
						pa.add_prediction(x + dx, y + dy, ov);
						if (pw != nullptr && pw->pa != nullptr) {
							pw->pa->add_prediction(x + dx, y + dy, ov);
						}
					}
				}
			}
			ship_assert(co == no);
		}
	}

	/* Now fill in any erased pixel with at least mp predictions. */
	for (y = 0; y < bin->height(); ++y) {
		for (x = 0; x < bin->width(); ++x) {
			if (berase->get_red(x, y) == 0) {
				/* Not erased. If this is the first pass, it's OK to set the pixel.
				   Successive passes shouldn't do it because we change berase. */
				if (1 == pass) {
					bout->put_pixel(x, y, bin->get_pixel(x, y));
				}
				continue;
			}
			++ce;
			if (pa.get_num_predictions(x, y) < mp)
				continue;
			pa.get_avg_prediction(x, y, ov);
			bout->set_red(x, y, (u32)floor(ov.r * 255. + 0.5));
			bout->set_green(x, y, (u32)floor(ov.g * 255. + 0.5));
			bout->set_blue(x, y, (u32)floor(ov.b * 255. + 0.5));
			bin->set_red(x, y, (u32)floor(ov.r * 255. + 0.5));
			bin->set_green(x, y, (u32)floor(ov.g * 255. + 0.5));
			bin->set_blue(x, y, (u32)floor(ov.b * 255. + 0.5));
			/* Mark this pixel as no longer erased. */
			berase->put_pixel(x, y, C_BLACK);
			ret = true;
			--ce;
		}
	}

	ce_n = ce;
	if (pw)
		pw->nerased = ce_n;
	vs.printf("(%d/%d pixels remain erased, %s) [ETA %s]     \r", ce_n, ce_st,
		   timepr(sw.stop(UNIT_MILLISECOND)),
		   timepr((sw.stop(UNIT_MILLISECOND) * ce_n) / ((ce_st == ce_n) ? 1 : (ce_st - ce_n))));
	if (!ret)
		vs << "\n";

	return ret;
}

/* Helper function: predict from erased (multi-threaded version) */
void ImgNNet::predict_pass_from_missing_mt_t(SBitmap* bin, PredictPassThreadData* pptd) {
	bool ret = false;
	double const* pout;
	int x, y, dx, dy, ds;
	u32 ci, co, ce = 0;
	RGBOut ov;

	ship_assert(!colorize);
	if (pptd->pass == 0) {
		pptd->in = new double [ni];
	}
	++pptd->pass;
	pptd->done = false;

	for (y = d; y < bin->height() - d; ++y) {
		for (x = d; x < bin->width() - d; ++x) {
			bool any_erased = false;
			if ((u32(x + y) % pptd->nth) != pptd->ith)
				continue;
			for (dy = -((int)(d + 1)); dy <= (int)(d+1) && !any_erased; ++dy) {
				for (dx = -((int)(d + 1)); dx <= (int)(d+1) && !any_erased; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds > (int)(d+1)*(int)(d+1) || ds <= (int)(d*d))
						continue;
					if (!pixel_ok(pptd->erase, x + dx, y + dy))
						continue;
					if (pptd->erase->get_red(x + dx, y + dy) != 0)
						any_erased = true;
				}
			}
			if (!any_erased)
				continue;

			/* At least one pixel on our (d+1)-perimeter was erased; let's calculate our predictions. */
			ci = 0;
			for (dy = -((int)d); dy <= ((int)d); ++dy) {
				for (dx = -((int)d); dx <= ((int)d); ++dx) {
					if (!pixel_ok(pptd->erase, x + dx, y + dy))
						continue;
					ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d * d)) {
						/* Within d pixels, one of the inputs. */
						if (pptd->erase->get_red(x + dx, y + dy) != 0) {
							continue;
						}
						pptd->in[ci++] = channel_intensity(bin, CHANNEL_RED, x + dx, y + dy);
						pptd->in[ci++] = channel_intensity(bin, CHANNEL_GREEN, x + dx, y + dy);
						pptd->in[ci++] = channel_intensity(bin, CHANNEL_BLUE, x + dx, y + dy);
					}
				}
			}
			if (ci < ni) {
				continue;
			}
			/* Inputs filled, now calculate outputs and populate predictions for (d+1)-perimeter. */
			co = 0;
			ship_assert(ci == ni);
			pout = nnet_run(pptd->nnet, pptd->in, pptd->ith);
			for (dy = -((int)(d+1)); dy <= ((int)(d+1)); ++dy) {
				for (dx = -((int)(d+1)); dx <= ((int)(d+1)); ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d*d)) {
						/* Part of the (d+1)-perimeter. */
						ov.r = pout[co++];
						ov.g = pout[co++];
						ov.b = pout[co++];
						pptd->pa.add_prediction(x + dx, y + dy, ov);
					}
				}
			}
			ship_assert(co == no);
		}
	}

	pptd->done = true;
}

/* The driver function for multi-threaded missing pixel prediction. */
SBitmap* ImgNNet::predict_from_missing_mt(SBitmap* bmp, SBitmap* erased) {
	SBitmap* bmpcopy = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ecopy = new SBitmap(bmp->width(), bmp->height());
	u32 min_pred = 4, ce;
	Stopwatch swp;
	std::thread* th[MAXIMGNNT_THREADS];
	PredictPassThreadData pptd[MAXIMGNNT_THREADS];
	PredictAccum pa;
	RGBOut ro;
	u32 nt = maxmt;
	int e;
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(erased, erased);
	NOT_NULL_OR_RETURN(bmpcopy, bmpcopy);
	NOT_NULL_OR_RETURN(ecopy, ecopy);
	ret->clear();
	bmp->blit(bmpcopy, SPoint(0, 0));
	erased->blit(ecopy, SPoint(0, 0));
	nt = CLAMP(nt, 1, MAXIMGNNT_THREADS);
	ship_assert(!colorize);

	vs << "Predicting erased portions of bitmap on " << (d + 1) << "-perimeter, on " << nt << " threads.\n";
	pass = 0;
	swp.start();
	count_erased(ecopy);
	for (e = 0; e < nt; ++e) {
		pptd[e].ith = e;
		pptd[e].nth = nt;
		pptd[e].pass = 0;
		pptd[e].erase = ecopy;
		if (e == 0) {
			pptd[e].nnet = nnet;
		} else {
			pptd[e].nnet = nnet_copy(nnet);
		}
		pptd[e].in = nullptr;
		pptd[e].done = false;
	}

	bool again = true;
	while (again) {
		++pass;
		// Prepare each pass. First, update the prediction window, if there is one.
		if (pw) {
			pw->nerased_in = ce_st;
			pw->nerased = ce_n;
			pw->ret = ret;
			pw->erase = ecopy;
			pw->done = false;
			pw->pass = pass;
			pw->ace = -1.;
		}

		// Then begin thread execution for this pass.
		vs << "Pass " << pass << "... ";
		for (e = 0; e < nt; ++e) {
			pptd[e].done = false;
			auto fn = std::bind(&ImgNNet::predict_pass_from_missing_mt_t, this, bmpcopy, &pptd[e]);
			th[e] = new std::thread(fn);
		}

		// Wait for execution to finish.
		do {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			again = false;
			for (e = 0; e < nt && !again; ++e) {
				if (!pptd[e].done)
					again = true;
			}
		} while (again);

		// Some cleanup.
		for (e = 0; e < nt; ++e) {
			if (th[e]->joinable()) {
				th[e]->join();
			}
			delete th[e];
		}

		// Accumulate the threads' predictions.
		pa.reset();
		for (e = 0; e < nt; ++e) {
			pa.fold_in(pptd[e].pa);
			if (pw != nullptr && pw->pa != nullptr) {
				pw->pa->fold_in(pptd[e].pa);
			}
			pptd[e].pa.reset();
		}

		// Fill in the newly-unerased pixels, and update our count.
		// (Note that again's value is guaranteed to be false here, we don't need to reset it.)
		ce = 0;
		for (int y = 0; y < bmpcopy->height(); ++y) {
			for (int x = 0; x < bmpcopy->width(); ++x) {
				if (ecopy->get_red(x, y) == 0) {
					/* Not erased. If this is the first pass, go ahead and set the pixel. */
					if (1 == pass) {
						ret->put_pixel(x, y, bmpcopy->get_pixel(x, y));
					}
					continue;
				}
				++ce;
				if (pa.get_num_predictions(x, y) < mp)
					continue;
				pa.get_avg_prediction(x, y, ro);
				ret->set_red(x, y, (u32)floor(ro.r * 255. + 0.5));
				ret->set_green(x, y, (u32)floor(ro.g * 255. + 0.5));
				ret->set_blue(x, y, (u32)floor(ro.b * 255. + 0.5));
				bmpcopy->set_red(x, y, (u32)floor(ro.r * 255. + 0.5));
				bmpcopy->set_green(x, y, (u32)floor(ro.g * 255. + 0.5));
				bmpcopy->set_blue(x, y, (u32)floor(ro.b * 255. + 0.5));
				/* Mark this pixel as no longer erased. */
				ecopy->put_pixel(x, y, C_BLACK);
				again = true;
				--ce;
			}
		}
		ce_n = ce;
		if (0 == ce)
			again = false;

		// Give a status update, if we're verbose anyway.
		vs.printf("(%d/%d pixels remain erased, %s) [ETA %s]     \r", ce_n, ce_st,
			   timepr(swp.stop(UNIT_MILLISECOND)),
			   timepr((swp.stop(UNIT_MILLISECOND) * ce_n) / ((ce_st == ce_n) ? 1 : (ce_st - ce_n))));
	}

	vs << "\n";
	for (e = 0; e < nt; ++e) {
		if (pptd[e].nnet != nnet) {
			nnet_free(pptd[e].nnet);
		}
		if (pptd[e].in != nullptr) {
			delete [] pptd[e].in;
		}
	}
	if (pw) {
		pw->done = true;
		pw->nerased = ce_n;
		pw->erase = nullptr;
		pw->ace = error_bmp(bmp, ret, erased);
	}

	fillsettings fs;
	fs.size = 8;
	fs.background = RGB_GRAY(192);
	fs.foreground = RGB_GRAY(128);
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (erased->get_red(x, y) != 0)
				bmpcopy->put_pixel(x, y, checkerboard_pattern(x, y, (void*)&fs));
		}
	}

	vs << "Error between original and erased   : " << error_bmp(bmp, bmpcopy, erased) << "\n";
	vs << "Error between original and avg.pixel: " << error_bmp_avg(bmp, erased) << "\n";
	vs << "Error between original and lastpixel: " << error_bmp_last_pixel(bmp, erased) << "\n";
	vs << "Error between original and neighbors: " << error_bmp_best_neighbor_pixel(bmp, erased) << "\n";
	vs << "Error between original and predicted: " << error_bmp(bmp, ret, erased) << "\n";

	if (out_erased) {
		bmpcopy->save_bmp("predict_in.png");
		vs << "The erased bitmap is saved as 'predict_in.png'\n";
	}
	if (out_neighb) {
		get_best_neighbors_bmp(bmpcopy, erased);
		bmpcopy->save_bmp("predict_in2.png");
		vs << "The 'best match neighbors' prediction bitmap is saved as 'predict_in2.png'\n";
	}

	vs.printf("Prediction complete [%s]\n", timepr(swp.stop(UNIT_MILLISECOND)));

	return ret;
}

/* Create a "best neighbors" reconstruction of the missing pixels. (Draws directly on bmp.) */
void ImgNNet::get_best_neighbors_bmp(SBitmap* bmp, SBitmap* erase) {
	RGBColor last = RGB_GRAY(128);
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (erase->get_red(x, y) == 0) {
				last = bmp->get_pixel(x, y);
				continue;
			}
			RGBColor neighbor = last;
			ignore_return_val(nearest_neighbor(bmp, erase, x, y, neighbor));
			bmp->put_pixel(x, y, neighbor);
		}
	}
}

/* Make a prediction based on the specified pixel of the bitmap, and add it to the PredictAccum. */
void ImgNNet::prediction_for_pixel(SBitmap* bmp, int x, int y, PredictAccum& pa) {
	int dx, dy, ds;
	u32 ci, co;
	double const * pout;
	RGBOut ov;

	ci = 0;
	for (dy = -((int)d); dy <= ((int)d); ++dy) {
		for (dx = -((int)d); dx <= ((int)d); ++dx) {
			if (!pixel_ok(bmp, x + dx, y + dy))
				return;
			ds = (dy * dy) + (dx * dx);
			if (ds <= (int)(d * d)) {
				/* Within d pixels, one of the inputs. */
				in[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
				in[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
				in[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
			}
		}
	}

	/* Now calculate outputs. */
	co = 0;
	ship_assert(ci == ni);
	pout = nnet_run(nnet, in);
	for (dy = -((int)(d+1)); dy <= ((int)(d+1)); ++dy) {
		for (dx = -((int)(d+1)); dx <= ((int)(d+1)); ++dx) {
			ds = (dy * dy) + (dx * dx);
			if (ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d*d)) {
				ov.r = pout[co++];
				ov.g = pout[co++];
				ov.b = pout[co++];
				pa.add_prediction(x + dx, y + dy, ov);
			}
		}
	}
	ship_assert(co == no);
}

/* Make predictions for the passed in ImgBrush; put the result in the passed in PredictAccum. */
void ImgNNet::prediction_for_brush(ImgBrush* ib, PredictAccum& pa) {
	if (colorize)
		return;
	int xx = (int)floor(ib->x + 0.5), yy = (int)floor(ib->y + 0.5);
	double const * pout;
	u32 ci = 0, co = 0;
	int x, y;

#if 1
	/* Combine the colors of the brush with any predictions already made. */
	for (y = yy - ((int)d); y <= yy + ((int)d); ++y) {
		for (x = xx - ((int)d); x <= xx + ((int)d); ++x) {
			int ds = (x - xx) * (x - xx) + (y - yy) * (y - yy);
			if (ds <= (int)(d * d)) {
				u32 np = pa.get_num_predictions(x, y);
				if (0 == np) {
					in[ci] = ib->in[ci];
					++ci;
					in[ci] = ib->in[ci];
					++ci;
					in[ci] = ib->in[ci];
					++ci;
				} else {
					RGBOut ro, r2;
					pa.get_total_prediction(x, y, ro);
					r2.r = ib->in[ci] * double(np);
					r2.g = ib->in[ci + 1] * double(np);
					r2.b = ib->in[ci + 2] * double(np);
					ro += r2;
					ro = ro * (1.0 / double(np + np));
					in[ci++] = ro.r;
					in[ci++] = ro.g;
					in[ci++] = ro.b;
				}
			}
		}
	}
	ship_assert(ci == ni);

	pout = nnet_run(nnet, in);
#else
	pout = nnet_run(nnet, ib->in);
#endif

	for (y = yy - ((int)(d + 1)); y <= yy + ((int)(d + 1)); ++y) {
		for (x = xx - ((int)(d + 1)); x <= xx + ((int)(d + 1)); ++x) {
			int ds = (x - xx) * (x - xx) + (y - yy) * (y - yy);
			if (ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d*d)) {
				RGBOut ov;
				ov.r = pout[co++];
				ov.g = pout[co++];
				ov.b = pout[co++];
				pa.lock();
				int np = pa.get_num_predictions(x, y);
				if (0 == np)
					np = 1;
				pa.add_prediction(x, y, ov, np);
				pa.unlock();
			}
		}
	}
	ship_assert(co == no);
}

/* Generate a predicted bitmap that is (weight1) parts bmp1 and (1 - weight1) parts bmp2. */
SBitmap* ImgNNet::generate_weighted_prediction(SBitmap* bmp1, SBitmap* bmp2, double weight1) {
	SBitmap* bin = new SBitmap(bmp1->width(), bmp2->height());
	int i = (int)(floor(1000. * weight1 + 0.5));
	assert(bmp1->width() == bmp2->width() && bmp1->height() == bmp2->height());
	NOT_NULL_OR_RETURN(bin, bin);
	sw.start();
	/* TODO: Best to do it this way, or by combining two PredictAccums? */
	/* How about if for each pixel prediction, we place bmp1's value at center, the weighted pixel values at the
	   perimeter, and interpolate between? */
	/* Or how about if we randomly choose between bmp1 and bmp2 (based on weight probability) when populating inputs,
	   and repeat pixel prediction a few times? */
	PredictAccum pa;
	for (int y = d; y < bmp1->height() - (int)d; y += 4) {
		vs.printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp1->height() - 1,
			   timepr(sw.stop(UNIT_MILLISECOND)),
			   timepr((sw.stop(UNIT_MILLISECOND) * (bmp1->height() - 1 - y)) / ((y == 0) ? 1 : y)));
		for (int x = d; x < bmp1->width() - (int)d; x += 4) {
			for (int pp = 0; pp < 4; ++pp) {
				for (int by = y - (int)d; by <= y + (int)d; ++by) {
					for (int bx = x - (int)d; bx <= x + (int)d; ++bx) {
						if (RandDouble(0., 1.) <= weight1)
							bin->put_pixel(bx, by, bmp1->get_pixel(bx, by));
						else
							bin->put_pixel(bx, by, bmp2->get_pixel(bx, by));
					}
				}
				prediction_for_pixel(bin, x, y, pa);
			}
		}
	}

	SBitmap* ret = new SBitmap(bmp1->width() / 4, bmp1->height() / 4);
	for (int y = 0; y < ret->height(); ++y) {
		for (int x = 0; x < ret->width(); ++x) {
			RGBOut ro, ro2;
			u32 cp = 0;
			ro.r = 0.;
			ro.g = 0.;
			ro.b = 0.;
			for (int ny = y * 4; ny <= y * 4 + 3; ++ny) {
				for (int nx = x * 4; nx <= x * 4 + 3; ++nx) {
					cp += pa.get_num_predictions(nx, ny);
					pa.get_total_prediction(nx, ny, ro2);
					ro += ro2;
				}
			}
			if (0 == cp) {
				ret->put_pixel(x, y, C_BLACK);
			} else {
				ro.r /= double(cp);
				ro.g /= double(cp);
				ro.b /= double(cp);
				ret->set_red(x, y, (u32)floor(ro.r * 255. + 0.5));
				ret->set_green(x, y, (u32)floor(ro.g * 255. + 0.5));
				ret->set_blue(x, y, (u32)floor(ro.b * 255. + 0.5));
			}
		}
	}
	return ret;

#if 0
	for (int y = 0; y < bmp1->height(); ++y) {
		for (int x = 0; x < bmp1->width(); ++x) {
			RGBColor c1, c2, ci;
			int r, g, b;
			c1 = bmp1->get_pixel(x, y);
			c2 = bmp2->get_pixel(x, y);
			interpolate_rgbcolor(c1, c2, &ci, i);	
			bin->put_pixel(x, y, ci);
		}
	}
	PredictAccum pa;
	for (int y = 0; y < bmp1->height(); ++y) {
		vs.printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp1->height() - 1,
			   timepr(sw.stop(UNIT_MILLISECOND)),
			   timepr((sw.stop(UNIT_MILLISECOND) * (bmp1->height() - 1 - y)) / ((y == 0) ? 1 : y)));
		for (int x = 0; x < bmp1->width(); ++x) {
			prediction_for_pixel(bin, x, y, pa);
		}
	}

	for (int y = 0; y < bmp1->height(); ++y) {
		for (int x = 0; x < bmp1->width(); ++x) {
			u32 c = pa.get_num_predictions(x, y);
			if (c > 0) {
				RGBOut ro;
				pa.get_avg_prediction(x, y, ro);
				bin->set_red(x, y, (u32)floor(ro.r * 255. + 0.5));
				bin->set_green(x, y, (u32)floor(ro.g * 255. + 0.5));
				bin->set_blue(x, y, (u32)floor(ro.b * 255. + 0.5));
			}
		}
	}
	return bin;
#endif
}

/* Helper function: Determine radius from the number of inputs/outputs, or vice versa. */
void ImgNNet::radius_from_inout(u32& d_out, u32& d2_out, bool coloriz) const {
	u32 i, o;
	for (d_out = 1; ; ++d_out) {
		if (coloriz) {
			for (d2_out = 1; d2_out <= d_out; ++d2_out) {
				ImgNNet::inout_from_radius(d_out, d2_out, i, o);
				if (i == ni && o == no)
					return;
				if (i > ni || o > no)
					break;
			}
			if (d2_out == 1)
				return;
		} else {
			ImgNNet::inout_from_radius(d_out, 0, i, o);
			if (i == ni && o == no)
				return;
			if (i > ni || o > no)
				break;
		}
	}
	d_out = 0;
	d2_out = 0;
}

void ImgNNet::inout_from_radius(u32 radius, u32 rd2, u32& in_o, u32& out_o) {
	int x, y;
	in_o = 0;
	out_o = 0;
	if (rd2 > 0) {
		for (x = -((int)(radius)); x <= (int)(radius); ++x) {
			for (y = -((int)(radius)); y <= (int)(radius); ++y) {
				u32 ds = (u32)(x * x) + (u32)(y * y);
				if (ds <= radius * radius)
					++in_o;
				if (ds <= rd2 * rd2)
					++out_o;
			}
		}
		// Two outputs for hue and saturation.
		out_o *= 2;
	} else {
		for (x = -((int)(radius + 1)); x <= (int)(radius + 1); ++x) {
			for (y = -((int)(radius + 1)); y <= (int)(radius + 1); ++y) {
				u32 ds = (u32)(x * x) + (u32)(y * y);
				if (ds <= radius * radius)
					++in_o;
				if (ds > radius * radius && ds <= (radius + 1) * (radius + 1))
					++out_o;
			}
		}
		// One I/O for each chroma channel.
		in_o *= 3;
		out_o *= 3;
	}
}

/* Helper function: do the dx/dy offsets represent a pixel in the training or validation sets? */
bool ImgNNet::is_in_training(int dx, int dy) const {
	u32 x4, y4;
	// TODO: more than one pixel per block?
	x4 = (training & 0x03);
	y4 = (training >> 2) & 0x03;
	return ((dx & 3) == x4) && ((dy & 3) == y4);
}

bool ImgNNet::is_in_validation(int dx, int dy) const {
	u32 x4, y4;
	// TODO: more than one pixel per block?
	x4 = (validation & 0x03);
	y4 = (validation >> 2) & 0x03;
	return ((dx & 3) == x4) && ((dy & 3) == y4);
}

/* Helper function: set the validation and training pixels randomly at the start of a pass. Although the 
   choice of dx/dy belonging to the sets is changed at the start of each training pass, the validation 
   and training pixel sets are ALWAYS disjoint for a given image, across passes or even across trainings. */
void ImgNNet::set_training_pixels(void) {
	// TODO: more than one pixel per block?
	do {
		training = RandU32();
		training &= 15;
	} while (training == 0);
}

void ImgNNet::set_validation_pixels(void) {
	// TODO: set these randomly per-image?
	validation = 0;
}

/* Helper: Calculate the average component error of the neural network's prediction of the passed-in bitmap. 
   Uses the validation pixel set. */
double ImgNNet::iterative_error(SBitmap* bmp) {
	int x, y;
	u32 ci, co;
#define	CALC_ERROR(x)	{ int ca = (int)(x); \
			  double v = pout[co++]; \
			  v = CLAMP(v, 0., 1.); \
			  int vi = (int)floor(v * 255. + 0.5); \
			  comp_error += std::abs(vi - ca); \
			  cd++; }
	i64 comp_error = 0, cd = 0;
	const double* pout;

	// TODO: Multithreaded iterative error doesn't work with KANN -- presumably kann_clone()
	// isn't threadsafe. Copy the neural network outside of the thread worker function?
	// Note that multithreaded prediction works.
	if (maxmt > 1 && nnet_ver != NNET_FORMAT_KANN) {
		return iterative_error_mt(bmp);
	}
	vs << "Calculating error...\n";
	for (y = ((int)d + 1); y < bmp->height() - ((int)d + 2); ++y) {
		if ((y & 7) == 0) {
			vs.printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp->height() - (d + 1), 
				   timepr(sw.stop(UNIT_MILLISECOND)),
				   timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - (d + 1) - y)) / ((y == 0) ? 1 : y)));
		}
		for (x = (d + 1); x < bmp->width() - (int)(d + 2); ++x) {
			int dx, dy, ds;
			ci = 0;
			co = 0;
			if (!is_in_validation(x, y)) {
				continue;
			}

			fill_train_in(bmp, x, y, ci, in);
			ship_assert(ci == ni);

			pout = nnet_run(nnet, in);
			for (dy = -((int)(d+1)); dy <= (int)(d+1); ++dy) {
				for (dx = -((int)(d+1)); dx <= (int)(d+1); ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (!colorize && ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d * d)) {
						/* Within the (d+1)-perimeter, one of the outputs. */
						CALC_ERROR(bmp->get_red(x + dx, y + dy));
						CALC_ERROR(bmp->get_green(x + dx, y + dy));
						CALC_ERROR(bmp->get_blue(x + dx, y + dy));
					} else if (colorize && ds <= (int)(d2 * d2)) {
						/* Within the d2-circle, one of the outputs. */
						RGBColor c = bmp->get_pixel(x + dx, y + dy);
						int hh, ss, vv;
						int rr, gg, bb;
						double h_p, s_p;
						RGB_HSV(RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c), &hh, &ss, &vv);
						h_p = pout[co++];
						s_p = pout[co++];
						hh = (int)floor(h_p * 255. + 0.5);
						ss = (int)floor(s_p * 255. + 0.5);
						HSV_RGB(hh, ss, vv, &rr, &gg, &bb);
						comp_error += color_distance(MAKE_RGB(rr, gg, bb), c);
						cd += 3;
#if 0
						CALC_ERROR(bmp->get_red(x + dx, y + dy));
						CALC_ERROR(bmp->get_green(x + dx, y + dy));
						CALC_ERROR(bmp->get_blue(x + dx, y + dy));
#endif
					}
				}
			}
			ship_assert(co == no);
		}
	}
	double retval = double(comp_error) / double(cd);
	vs << "Done in [" << timepr(sw.stop(UNIT_MILLISECOND)) << "] / Total error: " << comp_error << "                \n";
	vs << "Error per component: " << retval << "\n";
	return retval;
#undef	CALC_ERROR
}

/* Mutex for neural net copy. */
static std::mutex __nncopymtx;

/* The per-thread function for iterative error calculation. */
void ImgNNet::iterative_error_mt_t(SBitmap* bmp, u32 ith, u32 nth, ErrorThreadData* etd) {
	int x, y;
	u32 ci, co;
#define	CALC_ERROR(x)	{ int ca = (int)(x); \
			  double v = pout[co++]; \
			  v = CLAMP(v, 0., 1.); \
			  int vi = (int)floor(v * 255. + 0.5); \
			  etd->lock(); \
			  etd->comp_error += std::abs(vi - ca); \
			  etd->cd++; \
			  etd->unlock(); }
	double* tin;
	const double* pout;
	void* nnet_u = nullptr;
	u32 cv = 0;

	assert(nnet_ver != NNET_FORMAT_KANN);
	tin = new double [ni];
	if (0 == ith) {
		nnet_u = nnet;
	} else {
		__nncopymtx.lock();
		nnet_u = nnet_copy(nnet);
		__nncopymtx.unlock();
	}

	for (y = ((int)d + 1); y < bmp->height() - ((int)d + 2); ++y) {
		etd->lock();
		etd->progress = (u32)y;
		etd->unlock();
		for (x = (d + 1); x < bmp->width() - (int)(d + 2); ++x) {
			int dx, dy, ds;
			ci = 0;
			co = 0;
			if (!is_in_validation(x, y)) {
				continue;
			}
			++cv;
			if ((cv % nth) != ith) {
				continue;
			}

			fill_train_in(bmp, x, y, ci, tin);
			ship_assert(ci == ni);

			pout = nnet_run(nnet_u, tin, ith);
			for (dy = -((int)(d+1)); dy <= (int)(d+1); ++dy) {
				for (dx = -((int)(d+1)); dx <= (int)(d+1); ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (!colorize && ds <= (int)(d+1)*(int)(d+1) && ds > (int)(d * d)) {
						/* Within the (d+1)-perimeter, one of the outputs. */
						CALC_ERROR(bmp->get_red(x + dx, y + dy));
						CALC_ERROR(bmp->get_green(x + dx, y + dy));
						CALC_ERROR(bmp->get_blue(x + dx, y + dy));
					} else if (colorize && ds <= (int)(d2 * d2)) {
						/* Within the d2-circle, one of the outputs. */
						RGBColor c = bmp->get_pixel(x + dx, y + dy);
						int hh, ss, vv;
						int rr, gg, bb;
						double h_p, s_p;
						RGB_HSV(RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c), &hh, &ss, &vv);
						h_p = pout[co++];
						s_p = pout[co++];
						hh = (int)floor(h_p * 255. + 0.5);
						ss = (int)floor(s_p * 255. + 0.5);
						HSV_RGB(hh, ss, vv, &rr, &gg, &bb);
						etd->lock();
						etd->comp_error += color_distance(MAKE_RGB(rr, gg, bb), c);
						etd->cd += 3;
						etd->unlock();
#if 0
						CALC_ERROR(bmp->get_red(x + dx, y + dy));
						CALC_ERROR(bmp->get_green(x + dx, y + dy));
						CALC_ERROR(bmp->get_blue(x + dx, y + dy));
#endif
					}
				}
			}
			ship_assert(co == no);
		}
	}

	delete [] tin;
	if (nnet_u != nnet) {
		nnet_free(nnet_u);
	}
	etd->lock();
	etd->done = true;
	etd->unlock();
	return;
#undef	CALC_ERROR
}

double ImgNNet::iterative_error_mt(SBitmap* bmp) {
	std::thread* th[MAXIMGNNT_THREADS];
	ErrorThreadData etd[MAXIMGNNT_THREADS];
	u32 nt = maxmt;
	u32 e, lp = 0;

	assert(nnet_ver != NNET_FORMAT_KANN);
	nt = CLAMP(nt, 1, MAXIMGNNT_THREADS);
	vs << "Calculating error on " << nt << " threads...\n";
	for (e = 0; e < nt; ++e) {
		etd[e].progress = 0;
		etd[e].comp_error = 0;
		etd[e].cd = 0;
		etd[e].done = false;
		auto fn = std::bind(&ImgNNet::iterative_error_mt_t, this, bmp, e, nt, &etd[e]);
		th[e] = new std::thread(fn);
	}

	forever {
		bool done = true;
		u32 np = INT32_MAX;
		for (e = 0; e < nt; ++e) {
			etd[e].lock();
			np = std::min(np, etd[e].progress);
			done = done && etd[e].done;
			etd[e].unlock();
		}
		if (done) {
			break;
		}
		if (lp == np) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		lp = np;
		vs.printf("Row %d of %d (%s) [ETA: %s]...\r", lp, bmp->height() - (d + 1), 
			   timepr(sw.stop(UNIT_MILLISECOND)),
			   timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - (d + 1) - lp)) / ((lp == 0) ? 1 : lp)));
	}

	// Clean up the loose threads.
	for (e = 0; e < nt; ++e) {
		if (th[e]->joinable()) {
			th[e]->join();
		}
		delete th[e];
	}

	i64 tce = 0, tcd = 0;
	for (e = 0; e < nt; ++e) {
		tcd += etd[e].cd;
		tce += etd[e].comp_error;
	}
	double retval = double(tce) / double(tcd);
	vs << "Done in [" << timepr(sw.stop(UNIT_MILLISECOND)) << "] / Total error: " << tce << "                \n";
	vs << "Error per component: " << retval << "\n";
	return retval;
}

void ImgNNet::traindata_to_ramfile(RamFile* rf) const {
	rf->putu32(train_data.size());
	for (const auto& e : train_data) {
		const auto& v = e.second;
		rf->putu32(e.first);
		rf->putu32(v.size());
		for (const auto& td : v) {
			// We always write out a version data block in .rfn format now, so pass 1 for the version.
			td.out_to_ramfile(rf, 1);
		}
	}
}

void ImgNNet::traindata_from_ramfile(RamFile* rf) {
	u32 sz, vsz, idx;
	sz = rf->getu32();
	for (u32 e = 0; e < sz; ++e) {
		idx = rf->getu32();
		vsz = rf->getu32();
		for (u32 f = 0; f < vsz; ++f) {
			TrainData td;
			td.read_from_ramfile(rf, verdata ? 1 : 0);
			train_data[idx].push_back(td);
		}
	}
}

bool ImgNNet::valid() const {
	if (is_null(nnet) || is_null(nnet_best))
		return false;
	if (is_null(in) || is_null(out))
		return false;
	if (nnet_ver > NNET_FORMAT_KANN)
		return false;
	if (nnet_ver == NNET_FORMAT_GENANN) {
		if (ni != ((genann *)nnet)->inputs)
			return false;
		if (no != ((genann *)nnet)->outputs)
			return false;
	}
	if (colorize && (d2 > d || d2 == 0))
		return false;
	u32 i, o;
	ImgNNet::inout_from_radius(d, d2, i, o);
	if (i != ni || o != no)
		return false;
	return true;
}

void ImgNNet::strip_traindata() {
	train_data.clear();
	st.clear();
	vsd.clear();
}

void ImgNNet::strip_validation() {
	vsd.clear();
}

void ImgNNet::traindata_to_csv(const char* fname) {
	std::ofstream o;
	o.open(fname);
	o << "StrId,FileName,Flip,ErrIn,ErrOut,LRateIn,LRateOut,LRateEff,TimeStart,TimeEnd,DurationSec,ImgW,ImgH\n";
	for (const auto& e : train_data) {
		const auto& v = e.second;
		for (const auto& td : v) {
			o << td.fname_idx << ",";
			if (td.fname_idx == STRINDEX_INVALID)
				o << "n/a,";
			else
				o << "\"" << st.string(td.fname_idx) << "\",";
			o << (td.flip ? "Y," : "N,");
			o << td.err_in << "," << td.err_out << "," << td.lrate1 << "," << td.lrate << "," << td.lrate_eff << ",";
			o << fmt_datetime_shell(&td.tim_start) << ",";
			o << fmt_datetime_shell(&td.tim_end) << "," << seconds_difference_dt(&td.tim_start, &td.tim_end) << "," << td.img_w << "," << td.img_h << "\n";
		}
	}
	o.close();
	o.clear();
}

void ImgNNet::count_erased(SBitmap* bmp) {
	ce_st = 0;
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (bmp->get_red(x, y) != 0)
				++ce_st;
		}
	}
	ce_n = ce_st;
}

/*** Produce a neural net whole image 'filter'. The image is divided into 5x5 blocks, and four
     erase restores are done in sequence:

	.#.#.    #.#.#    .....    .....
	.....    .....    .#.#.    #.#.#
	#.#.#    .#.#.    .....    .....
	.....    .....    .#.#.    #.#.#
	.#.#.    #.#.#    .....    .....
***/
SBitmap* ImgNNet::image_5x5_filter(SBitmap* bmp) {
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* erased = new SBitmap(bmp->width(), bmp->height());
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(erased, erased);
	ret->clear();

	vs << "Applying neural net filter -- first pass!\n";
	erased->clear();
#define ERASE_FILL(p1, p2)	erased->rect_fill(SPoint(p1, POINT_PERCENT, p2, POINT_PERCENT), SPoint(p1 + 20, POINT_PERCENT, p2 + 20, POINT_PERCENT), C_WHITE);
	ERASE_FILL(20, 0);
	ERASE_FILL(60, 0);
	ERASE_FILL(0, 40);
	ERASE_FILL(40, 40);
	ERASE_FILL(80, 40);
	ERASE_FILL(20, 80);
	ERASE_FILL(60, 80);
	count_erased(erased);
	while (predict_pass_from_missing(bmp, erased, ret))
		no_op;

	vs << "Applying neural net filter -- second pass!\n";
	erased->clear();
	ERASE_FILL(0, 0);
	ERASE_FILL(40, 0);
	ERASE_FILL(80, 0);
	ERASE_FILL(20, 40);
	ERASE_FILL(60, 40);
	ERASE_FILL(0, 80);
	ERASE_FILL(40, 80);
	ERASE_FILL(80, 80);
	count_erased(erased);
	while (predict_pass_from_missing(bmp, erased, ret))
		no_op;

	vs << "Applying neural net filter -- third pass!\n";
	erased->clear();
	ERASE_FILL(20, 20);
	ERASE_FILL(60, 20);
	ERASE_FILL(20, 60);
	ERASE_FILL(60, 60);
	count_erased(erased);
	while (predict_pass_from_missing(bmp, erased, ret))
		no_op;

	vs << "Applying neural net filter -- fourth pass!\n";
	erased->clear();
	ERASE_FILL(0, 20);
	ERASE_FILL(40, 20);
	ERASE_FILL(80, 20);
	ERASE_FILL(0, 60);
	ERASE_FILL(40, 60);
	ERASE_FILL(80, 60);
	count_erased(erased);
	while (predict_pass_from_missing(bmp, erased, ret))
		no_op;

	delete erased;
	return ret;
#undef ERASE_FILL
}

SBitmap* ImgNNet::image_nxn_filter(SBitmap* bmp, u32 n) {
	SBitmap* erased = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* p;
	NOT_NULL_OR_RETURN(erased, erased);
	NOT_NULL_OR_RETURN(ret, ret);
	int x, y;
	int xx, yy, xl = 0, yl = 0;

	ret->clear();
	for (y = 0; y < (int)n; ++y) {
		yy = yl;
		yl = (bmp->height() * (y + 1)) / n;
		if (yl == yy)
			continue;
		for (x = 0; x < (int)n; ++x) {
			xx = xl;
			xl = (bmp->width() * (x + 1)) / n;
			if (xl == xx)
				continue;

			vs << "Generating tile (" << x << ", " << y << ")...\n";
			erased->clear();
			erased->rect_fill(SPoint(xx, yy), SPoint(xl - 1, yl - 1), C_WHITE);			
			p = predicted_from_missing(bmp, erased);
			p->blit(ret, SPoint(xx, yy));
			ret->save_bmp("filter.png");
			vs << "Output to 'filter.png'.\n";
		}
		xl = 0;
	}

	return ret;
}

void ImgNNet::add_validation_image(const char* pname) {
	/* Validation set test cases are just long strings of neural net inputs, followed
	   by the expected neural net outputs. This creates a new validation test case based
	   on an input validation set image. */
	SBitmap* bmp = SBitmap::load_bmp(pname);
	const u32 step = d * 2;
	if (is_null(bmp)) {
		vs << "Error reading image file '" << pname << "'.\n";
		return;
	}
	u32 pn_idx = st.index(pname);
	for (auto i : vsd.fs) {
		if (i == pn_idx) {
			vs << "Pathname " << pname << " is already in the validation set!\n";
			return;
		}
	}
	double* vin;
	double* vout;
	u32 estin = (bmp->height() / step + 2) * (bmp->width() / step + 2);
	u32 ci = 0, co = 0;
	vin = new double [estin * ni];
	vout = new double [estin * no];
	for (int y = d + 1; y < (int)(bmp->height() - d) - 2; y += step) {
		for (int x = d + 1; x < (int)(bmp->width() - d) - 2; x += step) {
			u32 cip = ci, cop = co;
			fill_train_inout(bmp, x, y, ci, co, vin, vout);
			cip = ci - cip;
			cop = co - cop;
#if 0
			int dx, dy;
			for (dy = -((int)(d + 1)); dy <= (int)(d + 1); ++dy) {
				for (dx = -((int)(d + 1)); dx <= (int)(d + 1); ++dx) {
					int ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d * d)) {
						/* Within d pixels, one of the inputs. */
						vin[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
						vin[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
						vin[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
						cip += 3;
					} else if (ds <= (int)(d+1)*(int)(d+1)) {
						/* Within the (d+1)-perimeter, one of the outputs. */
						vout[co++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
						vout[co++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
						vout[co++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
						cop += 3;
					}
				}
			}
#endif
			ship_assert(cip == ni && cop == no && ci <= estin * ni && co <= estin * ni);
		}
	}

	const u32 size_input = ni * sizeof(double), size_output = no * sizeof(double);
	vsd.fs.push_back(pn_idx);
	ship_assert(integer_multiple(vsd.inp.length(), size_input));
	ship_assert(integer_multiple(vsd.out.length(), size_output));
	ship_assert((vsd.inp.length() / size_input) == (vsd.out.length() / size_output));
	vsd.off.push_back(vsd.inp.length() / size_input);
	vsd.inp.memcat((u8*) vin, sizeof(double) * ci);
	vsd.out.memcat((u8*) vout, sizeof(double) * co);
	vsd.nf++;
	ship_assert(vsd.nf == vsd.fs.size() && vsd.nf == vsd.off.size());
	vs << ci << " inputs (" << (ci / ni) << " points) and " << co << " outputs added to validation set.\n";

	delete [] vin;
	delete [] vout;
}

static const char* validation_set_name(int d, bool c) {
	static char vsn[32];
	sprintf(vsn, "validation.set.%d%s", d, (c ? "c" : ""));
	return vsn;
}

double ImgNNet::validation_set_run() {
	std::thread* th[MAXIMGNNT_THREADS];
	double err[MAXIMGNNT_THREADS] = { 0.0 };
	double errt = 0.;
	u32 total_pts = 0;
	double* allin = (double *)vsd.inp.buffer();
	double* allout = (double *)vsd.out.buffer();
	u32 thuse = maxmt;
	ship_assert(vsd.rad == d);

	if (nnet_ver == NNET_FORMAT_KANN) {
		// Don't run multithreaded on KANN (see comment in iterative_error(): kann_clone() is not threadsafe.)
		thuse = 1;
	}
	sw.start();

	if (vsd.nf == 0 && FileExists(validation_set_name((int)d, colorize))) {
		RamFile rf;
		StringTable stv;
		vs << "No validation set in file, attempting to read default validation set.\n";
		rf.open(validation_set_name((int)d, colorize), RAMFILE_READONLY);
		vsd.in_from_ramfile(&rf);
		stv.load(&rf);
		rf.close();
		if (vsd.rad != d) {
			vs << "Validation set input radius doesn't match this neural network.\n";
			vsd.clear();
			vsd.rad = d;
			return -1.;
		}
		for (int e = 0; e < vsd.nf; ++e) {
			if (vsd.fs[e] == STRINDEX_INVALID)
				continue;
			const char* str = stv.string(vsd.fs[e]);
			vsd.fs[e] = st.index(str);
		}
		allin = (double *)vsd.inp.buffer();
		allout = (double *)vsd.out.buffer();
		vs << "Successfully read default validation set.\n";
	}

	for (int e = 0; e < vsd.nf; ++e) {
		if (STRINDEX_INVALID == vsd.fs[e]) {
			vs << "Running validation set test case #" << e + 1 << "\n";
		} else {
			vs << "Running validation set test case #" << e + 1 << " '" << st.string(vsd.fs[e]) << "'\n";
		}
		double* start_in = allin + (ni * vsd.off[e]);
		double* start_out = allout + (no * vsd.off[e]);
		double* end_in;
		if (e + 1 >= vsd.off.size()) {
			end_in = (double*)(vsd.inp.buffer() + vsd.inp.length());
		} else {
			end_in = allin + (ni * vsd.off[e + 1]);
		}
		ship_assert(integer_multiple(end_in - start_in, ni));
		u32 npts = (end_in - start_in) / ni;
		total_pts += npts;
		for (int t = 0; t < thuse; ++t)
			err[t] = 0.;

		if (thuse > 1) {
			/* Perform the validation multi-threaded. */
			int t;
			assert(nnet_ver != NNET_FORMAT_KANN);
			for (t = 0; t < thuse; ++t) {
				void* nnet_u;
				if (0 == t) {
					nnet_u = nnet;
				} else {
					nnet_u = nnet_copy(nnet);
				}
				auto fn = std::bind(&ImgNNet::validation_eval_mt_t, this, nnet_u, start_in, start_out, npts, t, thuse, &err[t]);
				th[t] = new std::thread(fn);
			}
			for (t = 0; t < thuse; ++t) {
				if (th[t]->joinable()) {
					th[t]->join();
				}
				delete th[t];
			}
			for (t = 1; t < thuse; ++t) {
				err[0] += err[t];
			}
		} else {
			/* Single threaded evaluation. */
			validation_eval_mt_t(nnet, start_in, start_out, npts, 0, 1, &err[0]);
		}
		errt += err[0];
		vs << "\tTotal component error over this test case  : " << err[0] * 255. << "\n";
		vs << "\tAverage component error over this test case: " << (err[0] * 255.) / double(u64(npts) * no * 3) << "\n";
		vs << "\tComponent error (cumulative so far)        : " << (errt * 255.) << "\n";
		vs << "\tAverage component error (this run so far)  : " << (errt * 255.) / double(u64(total_pts) * no * 3) << "\n";
	}

	vs.printf("Validation suite complete [%s]\n", timepr(sw.stop(UNIT_MILLISECOND)));

	return (errt * 255.) / double(u64(total_pts) * no * 3);
}

void ImgNNet::validation_eval_mt_t(void* nnet_use, double* inp, double* outp, u32 np, u32 ith, u32 nth, double* err) {
	/* All our inputs and outputs are in a nice row; makes for easy evaluation. */
	double* ie = inp + (np * ni);
	double const* pout;
	inp += (ith * ni);
	outp += (ith * no);
	while (inp < ie) {
		pout = nnet_run(nnet_use, inp, ith);
		for (u32 e = 0; e < no; ++e) {
			(*err) += std::abs(outp[e] - pout[e]);
		}
		inp += nth * ni;
		outp += nth * no;
	}
	if (nnet != nnet_use) {
		nnet_free(nnet_use);
	}
}

/* Output the validation set data as a default validation set. */
void ImgNNet::out_validation_set() {
	RamFile rf;
	if (FileExists(validation_set_name((int)d, colorize))) {
		remove(validation_set_name((int)d, colorize));
	}
	if (rf.open(validation_set_name((int)d, colorize), RAMFILE_COMPRESS | RAMFILE_CREATE_IF_MISSING)) {
		vs << "Unable to create validation set!\n";
		return;
	}
	vsd.out_to_ramfile(&rf);
	st.persist(&rf);
	rf.close();
	vs << "Validation set output to file 'validation.set." << d << (colorize ? "c" : "") << "'.\n";
}

/* Disk training: ensure the neural network knows the identity mapping. */
void ImgNNet::train_identity(u32 niterations) {
	if (colorize)
		return;	// No-op for these types of neural nets.
	vs << "Training the identity mapping, " << niterations << " iterations...\n";
	sw.start();
	batch_train_begin();
	for (u32 e = 0; e < niterations; ++e) {
		train_identity_iter();
	}
	batch_train_end(nnet, clrate);
	vs << "Done in [" << timepr(sw.stop(UNIT_MILLISECOND)) << "]\n";
}

/* NB: Assumes as a precondition that we're inside a batch_train_begin(). */
void ImgNNet::train_identity_iter(void) {
	double r, g, b;
	u32 ci, co;
	r = RandDouble(0., 1.);
	g = RandDouble(0., 1.);
	b = RandDouble(0., 1.);
	/* Inputs identically (r, g, b) */
	ci = 0;
	while (ci < ni) {
		in[ci++] = r;
		in[ci++] = g;
		in[ci++] = b;
	}
	/* Outputs also identically (r, g, b) */
	co = 0;
	while (co < no) {
		out[co++] = r;
		out[co++] = g;
		out[co++] = b;
	}
		
	/* Train on the identity mapping. */
	batch_train(nnet, in, out, clrate);
}

/* Train on the identity mapping as long as accuracy improves. */
void ImgNNet::train_identity_optimize(void) {
	double err_best;
	int repeat = 0;
	bool persist_needed = false;

	vs << "Calculating initial error...\n";
	ship_assert(nnet == nnet_best);
	err_best = validation_set_run();
	vs << "Initial error: " << err_best << "\n";
	nnet = nnet_copy(nnet_best);
	forever {
		double err;
		train_identity(2500);
		err = validation_set_run();
		vs << "New error: " << err << "\n";
		if (err < err_best) {
			vs << "Error improved! Continuing identity training.\n";
			ship_assert(nnet != nnet_best);
			nnet_free(nnet_best);
			nnet_best = nnet_copy(nnet);
			err_best = err;
			repeat = 0;
			persist_needed = true;
		} else {
			if (repeat < 1) {
				vs << "Error did not improve, retrying...\n";
				++repeat;
			} else {
				vs << "Error did not improve after repetition, finishing training.\n";
				ship_assert(nnet != nnet_best);
				nnet_free(nnet);
				nnet = nnet_best;
				break;
			}
		}
	}
	ship_assert(nnet == nnet_best);
	if (persist_needed) {
		update_on_disk();
	}
}

/* Train another ImgNNet, of the same radius, with our outputs. May yield a better
   starting point for a	new nnet than random weights. */
void ImgNNet::train_new_nnet(ImgNNet& to_train, u32 niter) {
	double const* pout;
	if (d != to_train.d) {
		vs << "Error: input neural network not the same radius.\n";
		return;
	}
	vs << "Training random inputs, " << niter << " iterations...\n";
	sw.start();
	batch_train_begin();
	for (u32 e = 0; e < niter; ++e) {
		u32 ci, co;
		/* Inputs random */
		ci = 0;
		while (ci < ni) {
			in[ci++] = RandDouble(0.0, 1.0);
		}
		/* Fill outputs from our neural net */
		pout = nnet_run(nnet, in);
		co = 0;
		while (co < no) {
			out[co] = pout[co];
			co++;
		}
		
		/* Train on the outputs. */
		batch_train(to_train.nnet, in, out, clrate);
	}
	batch_train_end(to_train.nnet, clrate);
	vs << "Done in [" << timepr(sw.stop(UNIT_MILLISECOND)) << "]\n";
}

/* Translate inputs/outputs from doubles to floats or vice versa. */
void ImgNNet::in_to_fin(double const* in_) {
	for (u32 e = 0; e < ni; ++e) {
		fin[e] = (float)in_[e];
	}
}

void ImgNNet::fout_to_out(float const* fout_) {
	for (u32 e = 0; e < no; ++e) {
		out[e] = (double)fout_[e];
	}
}

void ImgNNet::out_to_fout(double const* out_) {
	for (u32 e = 0; e < no; ++e) {
		fout[e] = (float)out_[e];
	}
}

/* The helper functions: these are layers on top of the genann/kann libraries. */
void* ImgNNet::nnet_copy(void* nnet_) {
	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		return (void *)genann_copy((genann *)nnet_);
	case NNET_FORMAT_KANN:
		return (void *)kann_clone((kann_t *)nnet_, 1);
	}
	return nullptr;
}

void ImgNNet::nnet_train(void *nnet_, double* in_, double* out_, double clrate_) {
	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		genann_train((genann *)nnet_, in_, out_, clrate_);
		break;
	case NNET_FORMAT_KANN:
		in_to_fin(in_);
		out_to_fout(out_);
		kann_train_fnn1((kann_t *)nnet_, clrate_, 64 /* mini_size? */, 2 /* max epoch */, 10 /* max_drop_streak? */, 0.1f /* frac_val? */, 1, &fin, &fout);
		break;
	}
}

void ImgNNet::nnet_free(void* nnet_) {
	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		genann_free((genann *)nnet_);
		break;
	case NNET_FORMAT_KANN:
		kann_delete((kann_t *)nnet_);
		break;
	}
}

void* ImgNNet::nnet_from_ramfile(RamFile* rf) {
	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		// genann format.
		return (void *)gn_read_from_ramfile(rf);
	case NNET_FORMAT_KANN:
		// kann format.
		kann_verbose = 0;
		return (void *)kann_read_from_ramfile(rf);
	default:
		// unknown?
		vs << "Error loading the neural network: unknown format " << nnet_ver << ".\n";
		break;
	}
	return nullptr;
}

void ImgNNet::nnet_out_to_ramfile(RamFile* rf) {
	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		gn_out_to_ramfile(rf, (genann *)nnet_best);
		break;
	case NNET_FORMAT_KANN:
		kann_out_to_ramfile(rf, (kann_t *)nnet_best);
		break;
	}
}

void* ImgNNet::nnet_init(u32 n_in, u32 hidden_layers, u32 neurons, u32 n_out) {
	kad_node_t *t;
	int e;

	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		return (void *)genann_init(n_in, hidden_layers, neurons, n_out);

	case NNET_FORMAT_KANN:
		// Build up the model.
		t = kann_layer_input(n_in);
#ifdef IMGNNET_CONVOL
		/* TODO: neural net graph that includes a convolution layer? */
		if (convol) {
			// Insert a 2-D convolution layer.
			t = kad_relu(kann_layer_conv2d(t, 4, 3, 3, 1, 0, 0, 0));
		}
#endif
		for (e = 0; e < hidden_layers; ++e) {
			t = kann_layer_dropout(kad_relu(kann_layer_dense(t, neurons)), 0.0f);
		}
		t = kann_layer_cost(t, n_out, KANN_C_CEB); // KANN_C_CEM);
		kann_verbose = 0;
		return (void *)kann_new(t, 0);
	}
	return nullptr;
}

double const* ImgNNet::nnet_run(void* nnet_, double const* in_, u32 ithread) {
	static float* fi[MAXIMGNNT_THREADS] = { nullptr };
	static double* da[MAXIMGNNT_THREADS] = { nullptr };
	static u32 fa = 0;
	const float* fo;

	// Return value float buffer, for kann format.
	if (nnet_ver != NNET_FORMAT_GENANN && no > fa) {
		fa = no;
		for (u32 e = 0; e < MAXIMGNNT_THREADS; ++e) {
			da[e] = new double [no];
			fi[e] = new float [ni];
		}
	}

	switch (nnet_ver) {
	case NNET_FORMAT_GENANN:
		return genann_run((genann *)nnet_, in_);

	case NNET_FORMAT_KANN:
		for (u32 e = 0; e < ni; ++e)
			fi[ithread][e] = (float)in_[e];
		fo = kann_apply1((kann_t *)nnet_, fi[ithread]);
		for (u32 e = 0; e < no; ++e)
			da[ithread][e] = (double)fo[e];
		return da[ithread];
	}
	return nullptr;
}

void ImgNNet::batch_train_begin(void) {
	if (nnet_ver != NNET_FORMAT_KANN)
		return;
	batch_i = 0;
	if (is_null(batch_in)) {
		batch_in = new float * [BATCH_TRAIN_SIZE];
		batch_out = new float * [BATCH_TRAIN_SIZE];
		for (u32 e = 0; e < BATCH_TRAIN_SIZE; ++e) {
			batch_in[e] = new float [ni];
			batch_out[e] = new float [no];
		}
	}
}

void ImgNNet::batch_train(void* nnet_, double* in_, double* out_, double clrate_) {
	if (nnet_ver != NNET_FORMAT_KANN) {
		nnet_train(nnet_, in_, out_, clrate_);
		return;
	}

	in_to_fin(in_);
	out_to_fout(out_);
	memcpy(batch_in[batch_i], fin, sizeof(float) * ni);
	memcpy(batch_out[batch_i], fout, sizeof(float) * no);

	++batch_i;
	if (batch_i >= BATCH_TRAIN_SIZE) {
		kann_train_fnn1((kann_t *)nnet_, clrate_, 64 /* mini_size? */, 10 /* max epoch */, 10 /* max_drop_streak? */, 0.1f /* validation fraction */, batch_i, batch_in, batch_out);
		batch_i = 0;
	}
}

void ImgNNet::batch_train_end(void* nnet_, double clrate_) {
	if (nnet_ver != NNET_FORMAT_KANN)
		return;
	if (batch_i > 0) {
		kann_train_fnn1((kann_t *)nnet_, clrate_, 64 /* mini_size? */, 10 /* max epoch */, 10 /* max_drop_streak? */, 0.1f /* validation fraction */, batch_i, batch_in, batch_out);
	}
	batch_i = 0;
}

/* Colorize a bitmap. (If the bitmap isn't grayscale, it's converted internally and then colorized.) */
SBitmap* ImgNNet::colorize_bitmap(SBitmap* bmp) {
	SBitmap* gray = new SBitmap(bmp->width(), bmp->height(), BITMAP_GRAYSCALE);
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	Stopwatch swp;
	int x, y;
	for (y = 0; y < bmp->height(); ++y)
		for (x = 0; x < bmp->width(); ++x) {
			u32 i = RGBColorGrayscaleLevel(bmp->get_pixel(x, y));
			gray->put_pixel(x, y, RGB_GRAY(i));
		}

	std::thread* th[MAXIMGNNT_THREADS];
	ColorizationThreadData ctd[MAXIMGNNT_THREADS];
	PredictAccum pa;
	RGBOut ro;
	u32 nt = maxmt;
	int e;
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(gray, gray);
	ret->clear();
	nt = CLAMP(nt, 1, MAXIMGNNT_THREADS);
	ship_assert(colorize);

	vs << "Predicting RGB color of bitmap in " << (d2) << "-circle, on " << nt << " threads.\n";
	swp.start();

	for (e = 0; e < nt; ++e) {
		ctd[e].ith = e;
		ctd[e].nth = nt;
		if (e == 0) {
			ctd[e].nnet = nnet;
		} else {
			ctd[e].nnet = nnet_copy(nnet);
		}
		ctd[e].in = nullptr;
		ctd[e].done = false;
		ctd[e].row = 0;
	}

	for (e = 0; e < nt; ++e) {
		auto fn = std::bind(&ImgNNet::colorize_mt, this, gray, &ctd[e]);
		th[e] = new std::thread(fn);
	}

	// Wait for execution to finish.
	bool again;
	do {
		int row = bmp->height();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		again = false;
		for (e = 0; e < nt; ++e) {
			row = std::min(row, ctd[e].row);
			if (!ctd[e].done)
				again = true;
		}
		vs.printf("Row %d of %d (%s) [ETA: %s]...\r", row, bmp->height() - (d + 1), 
			   timepr(sw.stop(UNIT_MILLISECOND)),
			   timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - (d + 1) - row)) / ((row == 0) ? 1 : row)));
	} while (again);
	vs << "\n";

	// Some cleanup.
	for (e = 0; e < nt; ++e) {
		if (th[e]->joinable()) {
			th[e]->join();
		}
		delete th[e];
	}

	// Accumulate the threads' predictions.
	pa.reset();
	for (e = 0; e < nt; ++e) {
		pa.fold_in(ctd[e].pa);
		ctd[e].pa.reset();
	}

	// Fill in the pixels.
	for (y = 0; y < ret->height(); ++y) {
		for (x = 0; x < ret->width(); ++x) {
#if 0
			double intense = gray_intensity(bmp, x, y) * 3.0;
#endif
			if (pa.get_num_predictions(x, y) == 0) {
				/* No predictions? Just use the input. */
				ret->put_pixel(x, y, gray->get_pixel(x, y));
				continue;
			}
			RGBColor c = gray->get_pixel(x, y);
			int hh, ss, vv;
			int rr, gg, bb;
			RGB_HSV(RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c), &hh, &ss, &vv);
			pa.get_avg_prediction(x, y, ro);
			HSV_RGB((int)floor(ro.r * 255. + 0.5),
				(int)floor(ro.g * 255. + 0.5),
				vv, &rr, &gg, &bb);
			ret->put_pixel(x, y, MAKE_RGB(rr, gg, bb));
#if 0
			/* Adjust the prediction so the gray intensity equals the input bitmap. */
			if (ro.r + ro.g + ro.b > 0.) {
				double ratio = intense / (ro.r + ro.g + ro.b);
				ro.r *= ratio;
				ro.g *= ratio;
				ro.b *= ratio;
				CLAMP(ro.r, 0., 1.);
				CLAMP(ro.g, 0., 1.);
				CLAMP(ro.b, 0., 1.);
			}
			ret->set_red(x, y, (u32)floor(ro.r * 255. + 0.5));
			ret->set_green(x, y, (u32)floor(ro.g * 255. + 0.5));
			ret->set_blue(x, y, (u32)floor(ro.b * 255. + 0.5));
#endif
		}
	}

	for (e = 0; e < nt; ++e) {
		if (ctd[e].nnet != nnet) {
			nnet_free(ctd[e].nnet);
		}
		if (ctd[e].in != nullptr) {
			delete [] ctd[e].in;
		}
	}

	if (out_erased) {
		gray->save_bmp("predict_in.png");
		vs << "The grayscale bitmap is saved as 'predict_in.png'\n";
	}

	vs.printf("Prediction complete [%s]\n", timepr(swp.stop(UNIT_MILLISECOND)));

	delete gray;
	return ret;
}

void ImgNNet::colorize_mt(SBitmap* bmp, ColorizationThreadData* ctd) {
	double const* pout;
	int x, y, dx, dy, ds;
	u32 ci, co;
	RGBOut ov;

	ship_assert(colorize);
	if (is_null(ctd->in)) {
		ctd->in = new double [ni];
	}
	ctd->done = false;

	for (y = d; y < bmp->height() - d; ++y) {
		for (x = d; x < bmp->width() - d; ++x) {
			if ((u32(x + y) % ctd->nth) != ctd->ith)
				continue;

			ci = 0;
			fill_train_in(bmp, x, y, ci, ctd->in);
			if (ci < ni)
				continue;

			co = 0;
			ship_assert(ci == ni);
			pout = nnet_run(ctd->nnet, ctd->in, ctd->ith);
			for (dy = -((int)(d2)); dy <= ((int)(d2)); ++dy) {
				for (dx = -((int)(d2)); dx <= ((int)(d2)); ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= (int)(d2*d2)) {
						ov.r = pout[co++];	// hue
						ov.g = pout[co++];	// saturation
						ov.b = 0.0;		// value (get from input bitmap)
#if 0
						ov.r = pout[co++];
						ov.g = pout[co++];
						ov.b = pout[co++];
#endif
						ctd->pa.add_prediction(x + dx, y + dy, ov);
					}
				}
			}
			ship_assert(co == no);
		}
		ctd->row = y;
	}

	ctd->done = true;
}

/* Image discriminator implementation follows. */
ImgDiscrim::ImgDiscrim(const char* imgnnet, bool verbose, u32 nneurons, u32 hidden_layers) {
	inet.read_from_file(imgnnet);
	inet_path = new char [strlen(imgnnet) + 1];
	strcpy(inet_path, imgnnet);
	rad = inet.radius();
	ImgNNet::inout_from_radius(rad, 0, ni, no);
	ni = std::max(ni, no);
	ni = std::max(ni, inputs_from_radius(rad));
	in = new float [ni];
	out = new float [no];
	init(hidden_layers, nneurons);
}

ImgDiscrim::ImgDiscrim() {
	inet_path = nullptr;
	in = nullptr;
	out = nullptr;
	nnet = nullptr;
	bin = nullptr;
	bout = nullptr;
	bt = 0;
	ni = 0;
	no = 0;
	rad = 0;
}

ImgDiscrim::~ImgDiscrim() {
	free();
}

void ImgDiscrim::init(u32 hl, u32 n) {
	kad_node_t *t;
	u32 e;
	// Build up the model.
	t = kann_layer_input(inputs_from_radius(rad));
	for (e = 0; e < hl; ++e) {
		t = kann_layer_dropout(kad_relu(kann_layer_dense(t, n)), 0.0f);
	}
	t = kann_layer_cost(t, 1, KANN_C_CEB);
	kann_verbose = 0;
	nnet = kann_new(t, 0);
	bin = nullptr;
	bout = nullptr;
	bt = 0;
}

void ImgDiscrim::free() {
	if (!is_null(inet_path))
		delete [] inet_path;
	if (!is_null(in))
		delete [] in;
	if (!is_null(out))
		delete [] out;
	if (!is_null(nnet))
		kann_delete(nnet);
	if (!is_null(bin))
		delete [] bin;
	if (!is_null(bout))
		delete [] bout;
	inet_path = nullptr;
	in = nullptr;
	out = nullptr;
	nnet = nullptr;
	bin = nullptr;
	bout = nullptr;
	bt = 0;
}

void ImgDiscrim::persist_to_file(const char* path) {
	if (is_null(nnet))
		return;
	if (FileExists(path)) {
		remove(path);
	}
	RamFile rf;
	if (rf.open(path, RAMFILE_COMPRESS | RAMFILE_CREATE_IF_MISSING)) {
		vs << "Unable to open " << path << "!\n";
		return;
	}
	persist(&rf);
}

void ImgDiscrim::persist(RamFile* rf) {
	if (is_null(nnet))
		return;

	rf->put32(IMGNNET_MAGIC);
	rf->puts(inet_path);
	rf->putu32(rad);
	rf->putu32(ni);
	rf->putu32(no);
	rf->putbool(vs.is_verbose());
	kann_out_to_ramfile(rf, nnet);
}

void ImgDiscrim::load_from_file(const char* path) {
	RamFile rf;
	if (rf.open(path, RAMFILE_READONLY)) {
		vs << "Unable to open " << path << "!\n";
		return;
	}
	load(&rf);
}

void ImgDiscrim::load(RamFile* rf) {
	if (rf->get32() != IMGNNET_MAGIC) {
		return;
	}
	free();
	inet_path = new char [1024];
	rf->gets(inet_path, 1024);
	inet.read_from_file(inet_path);
	rad = rf->getu32();
	ni = rf->getu32();
	no = rf->getu32();
	bool v = rf->getbool();
	if (v)
		vs.verbose();
	else
		vs.quiet();	
	nnet = kann_read_from_ramfile(rf);
	in = new float [ni];
	out = new float [no];
	bin = nullptr;
	bout = nullptr;
	bt = 0;
	vs << "Discriminator loaded successfully.\n";
}

void ImgDiscrim::train_on_image(const char* path) {
	SBitmap* bmp = SBitmap::load_bmp(path);
	if (is_null(bmp)) {
		vs << "Unable to load '" << path << "' as an image file!\n";
		return;
	}
	train_on_image(bmp);
	delete bmp;
}

void ImgDiscrim::train_on_image(SBitmap* bmp) {
	int x, x1, y;
	x1 = rad + 1 + (RandU32() & 15);
	y = rad + 1 + (RandU32() & 15);

	while (y < (int)bmp->height() - (int)(rad + 2)) {
		vs << "Row " << y << " of " << bmp->height() << "...\r";
		x = x1;
		while (x < (int)bmp->width() - (int)(rad + 2)) {
			train_img_point(bmp, x, y);
			x += 16;
		}
		y += 16;
	}
	vs << "Image training done.        \n";
	batch_end();
}

void ImgDiscrim::train_img_point(SBitmap* bmp, int x, int y) {
	int xx, yy;
	const int r = (int)rad + 1;
	float* p;
	u32 ci;

	/* Train image perimeter -> 1 */
	p = in;
	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
#ifdef OLD_WAY
			if (ds > (r-1)*(r-1) && ds <= r * r) {
#else	// We include everything in the (rad+1)-square, less the rad-circle, now, not just the (rad+1)-perimeter.
			if (ds > (r-1)*(r-1)) {
#endif
				*(p++) = (float)channel_intensity(bmp, CHANNEL_RED, xx, yy);
				*(p++) = (float)channel_intensity(bmp, CHANNEL_GREEN, xx, yy);
				*(p++) = (float)channel_intensity(bmp, CHANNEL_BLUE, xx, yy);
			}
		}
	}
	out[0] = 1.;
	batch_train();
	ci = p - in;

	/* Now run the ImgNNet on the r-disc. */
	double* disc = inet.in;
	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
			if (ds <= (int)(rad * rad)) {
				*(disc++) = channel_intensity(bmp, CHANNEL_RED, xx, yy);
				*(disc++) = channel_intensity(bmp, CHANNEL_GREEN, xx, yy);
				*(disc++) = channel_intensity(bmp, CHANNEL_BLUE, xx, yy);
			}
		}
	}
	double const* pout = inet.nnet_run(inet.nnet, inet.in);

	/* Train predicted image perimeter -> 0 */
#ifdef OLD_WAY
	for (u32 e = 0; e < ci; ++e) {
		in[e] = (float)pout[e];
	}
#else
	p = in;
	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
			if (ds > (r-1)*(r-1) && ds <= r * r) {
				// in the perimeter, use the ImgNNet prediction
				*(p++) = (float)(*(pout++));
				*(p++) = (float)(*(pout++));
				*(p++) = (float)(*(pout++));
			} else if (ds > (r-1)*(r-1)) {
				// in the r-square but not the r-perimeter or (r-1)-circle, use the image pixel
				*(p++) = channel_intensity(bmp, CHANNEL_RED, xx, yy);
				*(p++) = channel_intensity(bmp, CHANNEL_GREEN, xx, yy);
				*(p++) = channel_intensity(bmp, CHANNEL_BLUE, xx, yy);
			}
		}
	}
	ship_assert((p - in) == ci);
#endif
	out[0] = 0.;
	batch_train();
}

void ImgDiscrim::batch_train() {
	u32 e;
	if (nullptr == bin) {
		bin = new float * [16384];
		bout = new float * [16384];
		for (e = 0; e < 16384; ++e) {
			bin[e] = new float [ni];
			bout[e] = new float [1];
		}
	}
	if (16384 == bt) {
		batch_end();
	}
	memcpy(bin[bt], in, sizeof(float) * no);
	memcpy(bout[bt], out, sizeof(float));
	++bt;
}

void ImgDiscrim::batch_end() {
	if (bt > 0) {
		kann_train_fnn1(nnet, 0.005, 64 /* mini_size? */, 10 /* max epoch */, 10 /* max_drop_streak? */, 0.1f /* validation fraction */, bt, bin, bout);
	}
	bt = 0;
}

float ImgDiscrim::loss_est(const char* path) {
	SBitmap* bmp = SBitmap::load_bmp(path);
	if (is_null(bmp)) {
		vs << "Unable to load '" << path << "' as an image file!\n";
		return -1.;
	}
	float ret = loss_est(bmp);
	delete bmp;
	return ret;
}

float ImgDiscrim::loss_est(SBitmap* bmp) {
	float loss_t = 0.0;
	const int LOSS_POINTS_IMAGE = 32;
	for (int e = 0; e < LOSS_POINTS_IMAGE; ++e) {
		int x, y;
		x = (int)RandU32Range(rad + 1, bmp->width() - (rad + 2));
		y = (int)RandU32Range(rad + 1, bmp->height() - (rad + 2));
		loss_t += loss_point(bmp, x, y);
	}
	return loss_t / float(LOSS_POINTS_IMAGE);
}

float ImgDiscrim::loss_point(SBitmap* bmp, int x, int y) {
	float ev = eval(bmp, x, y);
	return 1.0 - ev;
}

float ImgDiscrim::eval(SBitmap* bmp, int x, int y) {
	int xx, yy;
	float* p = in;
	const int r = (int)rad + 1;

	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
#ifdef OLD_WAY
			if (!(ds > (r-1)*(r-1) && ds <= r * r))
				continue;
#else
			if (ds <= (r-1)*(r-1))
				continue;
#endif
			if (!pixel_ok(bmp, xx, yy)) {
				*(p++) = 0.5;
				*(p++) = 0.5;
				*(p++) = 0.5;
			} else {
				*(p++) = (float)channel_intensity(bmp, CHANNEL_RED, xx, yy);
				*(p++) = (float)channel_intensity(bmp, CHANNEL_GREEN, xx, yy);
				*(p++) = (float)channel_intensity(bmp, CHANNEL_BLUE, xx, yy);
			}
		}
	}
	const float* pout = kann_apply1(nnet, in);
	return pout[0];
}

float ImgDiscrim::eval(ImgBrush* ib, PredictAccum& pa) {
	RGBOut ro;
	int xx, yy;
	float* p = in;
	const int r = (int)rad + 1;
	float p_in, p_out;
	int x = (int)floor(ib->x + 0.5), y = (int)floor(ib->y + 0.5);

	/* First calculate image probability for predictions on the perimeter. */
	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
#ifdef OLD_WAY
			if (ds > (r-1)*(r-1) && ds <= r * r) {
#else
			if (ds > (r-1)*(r-1)) {
#endif
				pa.get_avg_prediction_lock(xx, yy, ro);
				*(p++) = (float)ro.r;
				*(p++) = (float)ro.g;
				*(p++) = (float)ro.b;
			}
		}
	}
	p_in = kann_apply1(nnet, in)[0];

	/* Combine the colors of the brush with any predictions already made, then run the
	   ImgNNet to get the generated perimeter. */
	double* disc = inet.in;
	u32 ci = 0;
	for (yy = y - ((int)rad); yy <= y + ((int)rad); ++yy) {
		for (xx = x - ((int)rad); xx <= x + ((int)rad); ++xx) {
			int ds = (x - xx) * (x - xx) + (y - yy) * (y - yy);
			if (ds <= (int)(rad * rad)) {
				u32 np = pa.get_num_predictions_lock(xx, yy);
				if (0 == np) {
					*(disc++) = ib->in[ci++];
					*(disc++) = ib->in[ci++];
					*(disc++) = ib->in[ci++];
				} else {
					RGBOut r2;
					pa.get_avg_prediction_lock(xx, yy, ro);
					r2.r = ib->in[ci++];
					r2.g = ib->in[ci++];
					r2.b = ib->in[ci++];
					ro += r2;
					ro = ro * 0.5;
					*(disc++) = ro.r;
					*(disc++) = ro.g;
					*(disc++) = ro.b;
				}
			}
		}
	}
	double const* discout = inet.nnet_run(inet.nnet, inet.in);
	u32 co = 0;

	/* Now calculate probability for the predictions, with the new ImgBrush in. */
	p = in;
	for (yy = y - r; yy <= y + r; ++yy) {
		for (xx = x - r; xx <= x + r; ++xx) {
			int ds = (xx - x) * (xx - x) + (yy - y) * (yy - y);
			if (ds > (r-1)*(r-1) && ds <= r * r) {
				RGBOut ov;
				u32 np = pa.get_num_predictions_lock(xx, yy);
				if (0 == np) {
					ro.r = discout[co++];
					ro.g = discout[co++];
					ro.b = discout[co++];
				} else {
					pa.get_avg_prediction_lock(xx, yy, ro);
					ov.r = discout[co++];
					ov.g = discout[co++];
					ov.b = discout[co++];
					ro += ov;
					ro = ro * 0.5;
				}
				*(p++) = (float)ro.r;
				*(p++) = (float)ro.g;
				*(p++) = (float)ro.b;
			}
#ifndef OLD_WAY
			 else if (ds > (r-1)*(r-1)) {
				pa.get_avg_prediction_lock(xx, yy, ro);
				*(p++) = (float)ro.r;
				*(p++) = (float)ro.g;
				*(p++) = (float)ro.b;
			}
#endif
		}
	}
	p_out = kann_apply1(nnet, in)[0];

	/* Positive return value ~= "looks more like an original image from the training set" */
	return p_out - p_in;
}

u32 ImgDiscrim::inputs_from_radius(u32 r) {
	/* We now include everything in the (rad+1)-square not inside the rad-circle as
	   inputs to the discriminator. */
	const int rr = (int)r + 1;
	u32 ret = 0;
	int xx, yy;
	for (yy = -rr; yy <= rr; ++yy) {
		for (xx = -rr; xx <= rr; ++xx) {
			int ds = xx * xx + yy * yy;
			if (ds > int(r * r))
				++ret;
		}
	}
	return ret;
}

/*** end imgnnet.cpp ***/
