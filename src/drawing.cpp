/***

	drawing.cpp

	Rich 2-D drawing functions for libcodehappy.

	Implementations of SBitmap, SPalette, SPoint, SCoord, and Font live here.

	Copyright (C) 2006-2022, Chris Street. 

***/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

#include "libcodehappy.h"

#include <unordered_map>

SPalette::SPalette() {
	ncolors = 0;
	clrs = nullptr;
}

SPalette::~SPalette() {
	if (nullptr != clrs)
		delete [] clrs;
}

SPalette::SPalette(u32 nclrs) {
	clrs = new RGBColor [nclrs];
	for (u32 e = 0; e < nclrs; ++e)
		clrs[e] = C_BLACK;
	if (nullptr != clrs)
		ncolors = nclrs;
}

u32 SPalette::index_from_rgb(RGBColor c) {
	u32 bd = 9999999, bi = 0;
	/* Best-fit to the palette... */
	for (u32 i = 0; i < ncolors; ++i) {
		int r, g, b, d;
		b = RGB_BLUE(c) - RGB_BLUE(clrs[i]);
		g = RGB_GREEN(c) - RGB_GREEN(clrs[i]);
		r = RGB_RED(c) - RGB_RED(clrs[i]);
		r *= r;
		g *= g;
		b *= b;
		d = r + g + b;
		if (d < bd) {
			bd = d;
			bi = i;
			if (0 == d) {
				/* Short-circuit on exact match. */
				return (bi);
			}
		}
	}
	return(bi);
}

void SPalette::to_ramfile(RamFile* rf) const {
	rf->putu32(ncolors);
	for (u32 e = 0; e < ncolors; ++e)
		rf->putu32((u32)clrs[e]);
}

void SPalette::from_ramfile(RamFile* rf) {
	ncolors = rf->getu32();
	clrs = new RGBColor [ncolors];
	for (u32 e = 0; e < ncolors; ++e)
		clrs[e] = rf->getu32();
}

SPoint::SPoint() {
	SPoint(0, 0);
}

SPoint::SPoint(int xx, int yy) {
	x = xx;
	y = yy;
	xt = POINT_PIXEL;
	yt = POINT_PIXEL;
}

SPoint::SPoint(int xx, PointType xxt, int yy, PointType yyt) {
	x = xx;
	xt = xxt;
	y = yy;
	yt = yyt;
}

int SPoint::X(const SBitmap* bmp) const {
	if (xt == POINT_PIXEL || is_null(bmp))
		return x;
	switch (xt) {
	default:
		break;
	case POINT_MILLES:
		return ((bmp->width() - 1) * x) / 1000;
	case POINT_CENTER:
		return (bmp->width() / 2) + x;
	case POINT_PIXEL_REV:
		return (bmp->width() - 1) - x;
	case POINT_PERCENT_REV:
		return ((bmp->width() - 1) * (100 - x)) / 100;
	case POINT_MILLES_REV:
		return ((bmp->width() - 1) * (1000 - x)) / 1000;
	}
	// POINT_PERCENT
	return ((bmp->width() - 1) * x) / 100;
}

int SPoint::Y(const SBitmap* bmp) const {
	if (yt == POINT_PIXEL || is_null(bmp))
		return y;
	switch (yt) {
	default:
		break;
	case POINT_MILLES:
		return ((bmp->height() - 1) * y) / 1000;
	case POINT_CENTER:
		return (bmp->height() / 2) + y;
	case POINT_PIXEL_REV:
		return (bmp->height() - 1) - y;
	case POINT_PERCENT_REV:
		return ((bmp->height() - 1) * (100 - y)) / 100;
	case POINT_MILLES_REV:
		return ((bmp->height() - 1) * (1000 - y)) / 1000;
	}
	// POINT_PERCENT
	return ((bmp->height() - 1) * y) / 100;
}

int SPoint::X(const SCoord& co, const SBitmap* bmp) const {
	switch (xt) {
	default:
		break;
	case POINT_PIXEL:
		return x + co.X1(bmp);
	case POINT_CENTER:
		return co.center_X(bmp) + x;
	case POINT_MILLES:
		return co.X1(bmp) + (co.width(bmp) * x) / 1000;
	case POINT_PIXEL_REV:
		return co.X2(bmp) - x;
	case POINT_PERCENT_REV:
		return co.X2(bmp) - (co.width(bmp) * x) / 100;
	case POINT_MILLES_REV:
		return co.X2(bmp) - (co.width(bmp) * x) / 1000;
	}
	// POINT_PERCENT
	return co.X1(bmp) + (co.width(bmp) * x) / 100;
}

int SPoint::Y(const SCoord& co, const SBitmap* bmp) const {
	switch (yt) {
	default:
		break;
	case POINT_PIXEL:
		return y + co.Y1(bmp);
	case POINT_CENTER:
		return co.center_Y(bmp) + y;
	case POINT_MILLES:
		return co.Y1(bmp) + (co.height(bmp) * y) / 1000;
	case POINT_PIXEL_REV:
		return co.Y2(bmp) - y;
	case POINT_PERCENT_REV:
		return co.Y2(bmp) - (co.height(bmp) * y) / 100;
	case POINT_MILLES_REV:
		return co.Y2(bmp) - (co.height(bmp) * y) / 1000;
	}
	// POINT_PERCENT
	return co.Y1(bmp) + (co.height(bmp) * y) / 100;
}

void SPoint::Flatten(const SBitmap* bmp) {
	if (xt != POINT_PIXEL) {
		x = X(bmp);
		xt = POINT_PIXEL;
	}
	if (yt != POINT_PIXEL) {
		y = Y(bmp);
		yt = POINT_PIXEL;
	}
}

void SPoint::Flatten(const SCoord& co, const SBitmap* bmp) {
	x = X(co, bmp);
	y = Y(co, bmp);
	xt = POINT_PIXEL;
	yt = POINT_PIXEL;
}

SPoint SPoint::operator+(const SPoint& rhs) const {
	SPoint ret = *this;
	if (xt == rhs.xt && yt == rhs.yt) {
		ret.x = x + rhs.x;
		ret.y = y + rhs.y;
	} else {
		SPoint cr = rhs;
		cr.Flatten();
		ret.Flatten();
		ret.x += cr.x;
		ret.y += cr.y;
	}
	return ret;
}

SPoint SPoint::operator-(const SPoint& rhs) const {
	SPoint ret = *this;
	if (xt == rhs.xt && yt == rhs.yt) {
		ret.x = x - rhs.x;
		ret.y = y - rhs.y;
	} else {
		SPoint cr = rhs;
		cr.Flatten();
		ret.Flatten();
		ret.x -= cr.x;
		ret.y -= cr.y;
	}
	return ret;
}

SPoint SPoint::operator+=(const SPoint& rhs) {
	if (xt == rhs.xt && yt == rhs.yt) {
		x += rhs.x;
		y += rhs.y;
	} else {
		SPoint cr = rhs;
		cr.Flatten();
		Flatten();
		x += cr.x;
		y += cr.y;
	}
	return *this;
}

SPoint SPoint::operator-=(const SPoint& rhs) {
	if (xt == rhs.xt && yt == rhs.yt) {
		x -= rhs.x;
		y -= rhs.y;
	} else {
		SPoint cr = rhs;
		cr.Flatten();
		Flatten();
		x -= cr.x;
		y -= cr.y;
	}
	return *this;
}

void SPoint::MatchTo(SPoint& p, SBitmap* bmp) {
	if (xt == POINT_PIXEL && p.xt != POINT_PIXEL) {
		p.x = p.X(bmp);
		p.xt = POINT_PIXEL;
	} else if (xt != POINT_PIXEL && p.xt == POINT_PIXEL) {
		x = X(bmp);
		xt = POINT_PIXEL;
	}
	if (yt == POINT_PIXEL && p.yt != POINT_PIXEL) {
		p.y = p.Y(bmp);
		p.yt = POINT_PIXEL;
	} else if (yt != POINT_PIXEL && p.yt == POINT_PIXEL) {
		y = Y(bmp);
		yt = POINT_PIXEL;
	}
}

void SPoint::to_ramfile(RamFile* rf) const {
	rf->put32(x);
	rf->put32((i32) xt);
	rf->put32(y);
	rf->put32((i32) yt);
}

void SPoint::from_ramfile(RamFile* rf) {
	x = rf->get32();
	xt = (PointType) rf->get32();
	y = rf->get32();
	yt = (PointType) rf->get32();
}

SCoord::SCoord() {
	SPoint p1_, p2_;
	SCoord(p1_, p2_);
}

SCoord::SCoord(const SPoint& p1_, const SPoint& p2_) {
	p1 = p1_;
	p2 = p2_;
}

SCoord::SCoord(int x1, int y1, int x2, int y2) {
	p1 = SPoint(x1, y1);
	p2 = SPoint(x2, y2);
}

SCoord::SCoord(int x1, PointType x1t, int y1, PointType y1t, int x2, PointType x2t, int y2, PointType y2t) {
	p1 = SPoint(x1, x1t, y1, y1t);
	p2 = SPoint(x2, x2t, y2, y2t);
}

int SCoord::X1(const SBitmap* bmp) const {
	return std::min(p1.X(bmp), p2.X(bmp));
}

int SCoord::X2(const SBitmap* bmp) const {
	return std::max(p1.X(bmp), p2.X(bmp));
}

int SCoord::Y1(const SBitmap* bmp) const {
	return std::min(p1.Y(bmp), p2.Y(bmp));
}

int SCoord::Y2(const SBitmap* bmp) const {
	return std::max(p1.Y(bmp), p2.Y(bmp));
}

bool SCoord::Contains(int x, int y, SBitmap* bmp) const {
	int x1 = p1.X(bmp), x2 = p2.X(bmp), y1 = p1.Y(bmp), y2 = p2.Y(bmp);
	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

bool SCoord::Contains(const SPoint& p, SBitmap* bmp) const {
	int x1 = p1.X(bmp), x2 = p2.X(bmp), y1 = p1.Y(bmp), y2 = p2.Y(bmp);
	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	int xx = p.X(bmp), yy = p.Y(bmp);
	return (xx >= x1 && xx <= x2 && yy >= y1 && yy <= y2);
}

bool SCoord::Isect(const SCoord& c2, SCoord& result, SBitmap* bmp) const {
	int x1[2], y1[2], x2[2], y2[2];
	bool ret = true;
	x1[0] = X1(bmp);
	x1[1] = c2.X1(bmp);
	x2[0] = X2(bmp);
	x2[1] = c2.X2(bmp);
	if (std::max(x1[0], x1[1]) > std::min(x2[0], x2[1]))
		ret = false;
	y1[0] = Y1(bmp);
	y1[1] = c2.Y1(bmp);
	y2[0] = Y2(bmp);
	y2[1] = c2.Y2(bmp);
	if (std::max(y1[0], y1[1]) > std::min(y2[0], y2[1]))
		ret = false;
	result = SCoord(std::max(x1[0], x1[1]), std::max(y1[0], y1[1]), std::min(x2[0], x2[1]), std::min(y2[0], y2[1]));
	return ret;
}

void SCoord::Flatten(const SBitmap* bmp) {
	p1.Flatten(bmp);
	p2.Flatten(bmp);
}

int SCoord::center_X(const SBitmap* bmp) const {
	return (X1(bmp) + X2(bmp)) / 2;
}

int SCoord::center_Y(const SBitmap* bmp) const {
	return (Y1(bmp) + Y2(bmp)) / 2;
}

int SCoord::dx(const SBitmap* bmp) const {
	return X2(bmp) - X1(bmp);
}

int SCoord::dy(const SBitmap* bmp) const {
	return Y2(bmp) - Y1(bmp);
}

void SCoord::min_height(u32 h, const SBitmap* bmp) {
	u32 oh = dy(bmp) + 1;
	if (oh < h) {
		SPoint pt1(X1(bmp), Y1(bmp)), pt2(X2(bmp), Y1(bmp) + h - 1);
		p1 = pt1;
		p2 = pt2;
	}
}

void SCoord::min_width(u32 w, const SBitmap* bmp) {
	u32 ow = dx(bmp) + 1;
	if (ow < w) {
		SPoint pt1(X1(bmp), Y1(bmp)), pt2(X1(bmp) + w - 1, Y2(bmp));
		p1 = pt1;
		p2 = pt2;
	}
}

SCoord SCoord::operator+(const SPoint& rhs) const {
	SCoord ret;
	ret.p1 = p1 + rhs;
	ret.p2 = p2 + rhs;
	return ret;
}

SCoord SCoord::operator-(const SPoint& rhs) const {
	SCoord ret;
	ret.p1 = p1 - rhs;
	ret.p2 = p2 - rhs;
	return ret;
}

SCoord SCoord::operator+=(const SPoint& rhs) {
	p1 += rhs;
	p2 += rhs;
	return *this;
}

SCoord SCoord::operator-=(const SPoint& rhs) {
	p1 -= rhs;
	p2 -= rhs;
	return *this;
}

SCoord SCoord::operator=(const SBitmap* rhs) {
	this->p1 = SPoint(0, 0);
	this->p2 = SPoint(rhs->width() - 1, rhs->height() - 1);
	return *this;
}

void SCoord::to_ramfile(RamFile* rf) const {
	p1.to_ramfile(rf);
	p2.to_ramfile(rf);
}

void SCoord::from_ramfile(RamFile* rf) {
	p1.from_ramfile(rf);
	p2.from_ramfile(rf);
}

SCoord operator-(const SPoint& lhs, const SCoord& rhs) {
	SCoord nr = rhs;
	nr.p1.x = -nr.p1.x;
	nr.p1.y = -nr.p1.y;
	nr.p2.x = -nr.p2.x;
	nr.p2.y = -nr.p2.y;
	return nr + lhs;
}

SCoord operator+(const SPoint& lhs, const SCoord& rhs) {
	return rhs + lhs;
}

SBitmap::SBitmap() {
	bits = nullptr;
	w = 0;

	h = 0;
	btype = BITMAP_INVALID;
	pal = nullptr;
	sbd = nullptr;
	put_pixel_fn = SBitmap::put_pixel_default;
	get_pixel_fn = SBitmap::get_pixel_default;
}

SBitmap::SBitmap(u32 width, u32 height) {
	bits = new u8 [width * height * sizeof(u32)];
	w = width;
	h = height;
	btype = BITMAP_DEFAULT;
	pal = nullptr;
	sbd = nullptr;
	put_pixel_fn = SBitmap::put_pixel_32bpp;
	get_pixel_fn = SBitmap::get_pixel_32bpp;
	clear();
}

SBitmap::SBitmap(u32 width, u32 height, BitmapType typ) {
	u32 a;
	w = width;
	h = height;
	btype = typ;
	pal = nullptr;
	sbd = nullptr;
	put_pixel_fn = SBitmap::put_pixel_default;
	get_pixel_fn = SBitmap::get_pixel_default;

	switch (typ) {
	case BITMAP_DEFAULT:
		bits = new u8 [width * height * sizeof(u32)];
		put_pixel_fn = SBitmap::put_pixel_32bpp;
		get_pixel_fn = SBitmap::get_pixel_32bpp;
		break;
	case BITMAP_MONO:
		a = ((width * height) >> 3) + 1;
		bits = new u8 [a];
		put_pixel_fn = SBitmap::put_pixel_1bpp;
		get_pixel_fn = SBitmap::get_pixel_1bpp;
		break;
	case BITMAP_PALETTE:
		bits = new u8 [width * height];
		pal = new SPalette(256);
		put_pixel_fn = SBitmap::put_pixel_8bpp;
		get_pixel_fn = SBitmap::get_pixel_8bpp;
		break;
	case BITMAP_GRAYSCALE:
		bits = new u8 [width * height];
		put_pixel_fn = SBitmap::put_pixel_grey;
		get_pixel_fn = SBitmap::get_pixel_grey;
		break;
	case BITMAP_SUBBITMAP:
		put_pixel_fn = SBitmap::put_pixel_subbmp;
		get_pixel_fn = SBitmap::get_pixel_subbmp;
		break;
	case BITMAP_16BITS:
		bits = new u8 [width * height * 2];
		put_pixel_fn = SBitmap::put_pixel_16bpp;
		get_pixel_fn = SBitmap::get_pixel_16bpp;
		break;
	case BITMAP_24BITS:
		bits = new u8 [width * height * 3];
		put_pixel_fn = SBitmap::put_pixel_24bpp;
		get_pixel_fn = SBitmap::get_pixel_24bpp;
		break;
	case BITMAP_DISPLAY_OWNED:
	case BITMAP_INVALID:
		break;
	}
	// Besides giving a known initial state, this initializes alpha channel to
	// all opaque in new 32bpp bitmaps.
	clear();
}

u8* SBitmap::pixel_loc(u32 x, u32 y) const {
	switch (btype) {
	case BITMAP_DEFAULT:
		return &bits[(y * w + x) * sizeof(u32)];
	case BITMAP_PALETTE:
	case BITMAP_GRAYSCALE:
		return &bits[y * w + x];
	case BITMAP_SUBBITMAP:
		return sbd->parent->pixel_loc(x + sbd->co.X1(sbd->parent), y + sbd->co.Y1(sbd->parent));
	case BITMAP_16BITS:
		return &bits[(y * w + x) * 2];
	case BITMAP_24BITS:
		return &bits[(y * w + x) * 3];
	case BITMAP_DISPLAY_OWNED:
	case BITMAP_INVALID:
	case BITMAP_MONO:
		break;
	}
	return nullptr;
}

u8* SBitmap::pixel_loc(const SPoint& p) const {
	return pixel_loc(p.X(this), p.Y(this));
}

SBitmap::SBitmap(u32 width, u32 height, u8* data) {
	w = width;
	h = height;
	btype = BITMAP_DISPLAY_OWNED;
	pal = nullptr;
	bits = data;
	sbd = nullptr;
	put_pixel_fn = SBitmap::put_pixel_32bpp;
	get_pixel_fn = SBitmap::get_pixel_32bpp;
}

SBitmap::~SBitmap() {
	if (nullptr != bits && btype != BITMAP_DISPLAY_OWNED)
		delete [] bits;
	bits = nullptr;
	if (nullptr != pal)
		delete pal;
	pal = nullptr;
	if (nullptr != sbd)
		delete sbd;
	sbd = nullptr;
}

void SBitmap::put_pixel(u32 x, u32 y, RGBColor c) {
	if (x >= w)
		return;
	if (y >= h)
		return;
	put_pixel_fn(this, x, y, c);
}

void SBitmap::put_pixel(int x, int y, RGBColor c) {
	if (x < 0 || y < 0)
		return;
	if ((u32)x >= w)
		return;
	if ((u32)y >= h)
		return;
	put_pixel_fn(this, u32(x), u32(y), c);
}

RGBColor SBitmap::get_pixel(u32 x, u32 y) const {
	if (x >= w)
		return X_NEON_PINK;
	if (y >= h)
		return X_NEON_PINK;
	return get_pixel_fn(this, x, y);
}

RGBColor SBitmap::get_pixel(int x, int y) const {
	if (x < 0 || y < 0)
		return X_NEON_PINK;
	if ((u32)x >= w)
		return X_NEON_PINK;
	if ((u32)y >= h)
		return X_NEON_PINK;
	return get_pixel_fn(this, u32(x), u32(y));
}

void SBitmap::put_pixel(const SPoint& p, RGBColor c) {
	put_pixel(p.X(this), p.Y(this), c);
}

void SBitmap::put_pixel_alpha(int x, int y, RGBColor c) {
	if (x < 0 || y < 0 || x >= w || y >= h)
		return;

	switch (btype) {
	case BITMAP_SUBBITMAP:
		sbd->parent->put_pixel_alpha(x + sbd->co.X1(sbd->parent), y + sbd->co.Y1(sbd->parent), c);
		break;

	case BITMAP_DEFAULT:
		{
		u32 *buf = (u32 *)bits;
		u32 offs = y * w + x;
		buf[offs] = c;
		}
		break;

	default:
		put_pixel(x, y, c);
		break;
	}
}

void SBitmap::put_pixel_alpha(const SPoint& p, RGBColor c) {
	put_pixel_alpha(p.X(this), p.Y(this), c);
}

RGBColor SBitmap::get_pixel(const SPoint& p) const {
	return get_pixel(p.X(this), p.Y(this));
}

RGBColor SBitmap::get_pixel_rgb(int x, int y) const {
	RGBColor ret = get_pixel(x, y);
	ret &= 0xffffff;
	return(ret);
}

RGBColor SBitmap::get_pixel_rgb(const SPoint& p) const {
	return get_pixel_rgb(p.X(this), p.Y(this));
}

void SBitmap::put_pixel_32bpp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u32 *buf = (u32 *)sb->bits;
	u32 offs = y * sb->w + x;
	// Don't change the alpha channel.
	char alp = *(((char *)(buf + offs)) + 3);
	c = ADD_ALPHA(c, alp);
	buf[offs] = c;
}

void SBitmap::put_pixel_32bpp_alpha(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u32 *buf = (u32 *)sb->bits;
	u32 offs = y * sb->w + x;
	buf[offs] = c;
}

void SBitmap::put_pixel_8bpp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u32 offs = y * sb->w + x;
	sb->bits[offs] = (u8)sb->pal->index_from_rgb(c);
}

void SBitmap::put_pixel_grey(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u32 offs = y * sb->w + x;
	u32 gray = RGBColorGrayscaleLevel(c);
	sb->bits[offs] = (u8)gray;
}

void SBitmap::put_pixel_1bpp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u32 offs = (y * sb->w) + x;
	u8* bits = sb->bits;
	if (c == 0xFFFFFF)
		bits[offs >> 3] |= (1 << (offs & 7));
	else if (c == 0x000000)
		bits[offs >> 3] &= (~(1 << (offs & 7)));
	else {
		u32 grey = RGBColorGrayscaleLevel((RGBColor)c);
		if (grey < 0x80)
			bits[offs >> 3] &= (~(1 << (offs & 7)));
		else
			bits[offs >> 3] |= (1 << (offs & 7));
	}
}

void SBitmap::put_pixel_24bpp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u8 *buf = (u8 *)sb->bits;
	u32 offs = (y * sb->w + x) * 3;
	buf[offs++] = RGB_RED(c);
	buf[offs++] = RGB_GREEN(c);
	buf[offs] = RGB_BLUE(c);
}

void SBitmap::put_pixel_16bpp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	u16 *buf = (u16 *)sb->bits;
	u32 offs = y * sb->w + x;
	buf[offs] = RGB565FromRGBColor(c);
}

void SBitmap::put_pixel_subbmp(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	sb->sbd->parent->put_pixel(sb->sbd->co.X1(sb->sbd->parent) + x, sb->sbd->co.Y1(sb->sbd->parent) + y, c);
}

void SBitmap::put_pixel_default(SBitmap* sb, u32 x, u32 y, RGBColor c) {
	// no-op
}

RGBColor SBitmap::get_pixel_32bpp(const SBitmap* sb, u32 x, u32 y) {
	u32* data = (u32*)sb->bits;
	u32 offs = y * sb->w + x;
	return data[offs];
}

RGBColor SBitmap::get_pixel_8bpp(const SBitmap* sb, u32 x, u32 y) {
	u32 offs = y * sb->w + x;
	u8 idx = sb->bits[offs];
	return sb->pal->clrs[idx];
}

RGBColor SBitmap::get_pixel_grey(const SBitmap* sb, u32 x, u32 y) {
	u32 offs = y * sb->w + x;
	u8 intsy = sb->bits[offs];
	return RGB_NO_CHECK(intsy, intsy, intsy);
}

RGBColor SBitmap::get_pixel_1bpp(const SBitmap* sb, u32 x, u32 y) {
	i32 off = (y * sb->w) + x;
	i32 val;

	off = (y * sb->w) + x;
	val = sb->bits[off >> 3];
	val &= (1 << (off & 7));

	if (val)
		return(C_WHITE);
	return(C_BLACK);
}

RGBColor SBitmap::get_pixel_24bpp(const SBitmap* sb, u32 x, u32 y) {
	u8 *buf = (u8 *)sb->bits;
	u32 offs = (y * sb->w + x) * 3;
	buf += offs;
	return RGB_NO_CHECK(buf[0], buf[1], buf[2]);
}

RGBColor SBitmap::get_pixel_16bpp(const SBitmap* sb, u32 x, u32 y) {
	RGB565 *buf = (RGB565 *)sb->bits;
	u32 offs = y * sb->w + x;
	return RGBColorFromRGB565(buf[offs]);
}

RGBColor SBitmap::get_pixel_subbmp(const SBitmap* sb, u32 x, u32 y) {
	return sb->sbd->parent->get_pixel(sb->sbd->co.X1(sb->sbd->parent) + x, sb->sbd->co.Y1(sb->sbd->parent) + y);
}

RGBColor SBitmap::get_pixel_default(const SBitmap* sb, u32 x, u32 y) {
	return X_NEON_PINK;
}

void SBitmap::rect(int x1, int y1, int x2, int y2, RGBColor c) {
	int x, y;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	VALID_RECT(this, x1, y1, x2, y2);

	for (x = x1; x <= x2; ++x) {
		put_pixel(x, y1, c);
		put_pixel(x, y2, c);
	}

	for (y = y1 + 1; y < y2; ++y) {
		put_pixel(x1, y, c);
		put_pixel(x2, y, c);
	}
}

void SBitmap::rect(const SPoint& p1, const SPoint& p2, RGBColor c) {
	rect(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::rect(const SCoord& co, RGBColor c) {
	rect(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), c);
}

void SBitmap::rect_fill(int x1, int y1, int x2, int y2, RGBColor c) {
	int x, y;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	VALID_RECT(this, x1, y1, x2, y2);

	for (y = y1; y <= y2; ++y)
		for (x = x1; x <= x2; ++x)
			put_pixel(x, y, c);
}

void SBitmap::rect_fill(const SPoint& p1, const SPoint& p2, RGBColor c) {
	rect_fill(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::rect_fill(const SCoord& co, RGBColor c) {
	rect_fill(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), c);
}

void SBitmap::rect_fill_pattern(int x1, int y1, int x2, int y2, PatternCallback callback, void* args) {
	int x;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	VALID_RECT(this, x1, y1, x2, y2);

	while (y1 <= y2) {
		x = x1;
		while (x <= x2) {
			put_pixel(x, y1, callback(x, y1, args));
			x++;
		}
		++y1;
	}
}

void SBitmap::rect_fill_pattern(const SPoint& p1, const SPoint& p2, PatternCallback callback, void* args) {
	rect_fill_pattern(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), callback, args);
}

void SBitmap::rect_fill_pattern(const SCoord& co, PatternCallback callback, void* args) {
	rect_fill_pattern(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), callback, args);
}

// rect() and rect_fill(), except they set the alpha channel in 32bpp SBitmaps.
void SBitmap::rect_alpha(int x1, int y1, int x2, int y2, RGBColor c) {
	int x, y;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	VALID_RECT(this, x1, y1, x2, y2);

	for (x = x1; x <= x2; ++x) {
		put_pixel_alpha(x, y1, c);
		put_pixel_alpha(x, y2, c);
	}

	for (y = y1 + 1; y < y2; ++y) {
		put_pixel_alpha(x1, y, c);
		put_pixel_alpha(x2, y, c);
	}
}

void SBitmap::rect_alpha(const SPoint& p1, const SPoint& p2, RGBColor c) {
	rect_alpha(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::rect_alpha(const SCoord& co, RGBColor c) {
	rect_alpha(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), c);
}

// rect() and rect_fill(), except they set the alpha channel in 32bpp SBitmaps.
void SBitmap::rect_fill_alpha(int x1, int y1, int x2, int y2, RGBColor c) {
	int x, y;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	VALID_RECT(this, x1, y1, x2, y2);

	for (y = y1; y <= y2; ++y) {
		for (x = x1; x <= x2; ++x) {
			put_pixel_alpha(x, y, c);
		}
	}
}

void SBitmap::rect_fill_alpha(const SPoint& p1, const SPoint& p2, RGBColor c) {
	rect_fill_alpha(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::rect_fill_alpha(const SCoord& co, RGBColor c) {
	rect_fill_alpha(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), c);
}

void SBitmap::clear(void) {
	clear(C_BLACK);
}

void SBitmap::clear(RGBColor c) {
	rect_fill(0, 0, w - 1, h - 1, c);
	fill_alpha_channel(ALPHA_OPAQUE);
}

void SBitmap::clear_alpha(void) {
	clear_alpha(ADD_ALPHA(C_BLACK, ALPHA_TRANSPARENT));
}

void SBitmap::clear_alpha(RGBColor c) {
	rect_fill_alpha(0, 0, w - 1, h - 1, c);
}

void SBitmap::clear_pattern(PatternCallback callback, void* args) {
	rect_fill_pattern(0, 0, width() - 1, height() - 1, callback, args);
}

u32 SBitmap::get_alpha(int x, int y) {
	if (btype == BITMAP_SUBBITMAP)
		return sbd->parent->get_alpha(x + sbd->co.X1(sbd->parent), y + sbd->co.Y1(sbd->parent));
	if (btype != BITMAP_DEFAULT)
		return 0;
	return RGB_ALPHA(get_pixel(x, y));
}

u32 SBitmap::get_alpha(const SPoint& p) {
	return get_alpha(p.X(this), p.Y(this));
}

void SBitmap::set_alpha(int x, int y, u32 alp) {
	if (btype == BITMAP_SUBBITMAP)
		sbd->parent->set_alpha(x + sbd->co.X1(sbd->parent), y + sbd->co.Y1(sbd->parent), alp);
	if (btype != BITMAP_DEFAULT)
		return;
	RGBColor c = get_pixel(x, y);
	c &= 0xffffff;
	put_pixel_alpha(x, y, ADD_ALPHA(c, alp));
}

void SBitmap::set_alpha(const SPoint& p, u32 alp) {
	set_alpha(p.X(this), p.Y(this), alp);
}

void SBitmap::copy_alpha_rect(SBitmap* bmp_from, const SCoord& region_from, const SPoint& pt) {
	u32 wf = region_from.width(), hf = region_from.height();
	u32 x, y;

	for (y = 0; y < hf; ++y) {
		int yo = y + pt.Y(this), ys = y + region_from.Y1(bmp_from);
		if (yo >= h || ys >= bmp_from->h)
			break;
		for (x = 0; x < wf; ++x) {
			int xo = x + pt.X(this), xs = x + region_from.X1(bmp_from);
			if (xo >= w || xs >= bmp_from->w)
				break;
			u32 a = bmp_from->get_alpha(xs, ys);
			set_alpha(xo, yo, a);
		}
	}
}

void SBitmap::negative_alpha(void) {
	u32 x, y;
	if (btype == BITMAP_SUBBITMAP)
		sbd->parent->negative_alpha();
	if (btype != BITMAP_DEFAULT)
		return;
	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			u32 a = get_alpha(x, y);
			a = 255 - a;
			set_alpha(x, y, a);
		}
	}
}

u32 SBitmap::get_channel(int x, int y, u32 channel_flags) {
	RGBColor c = get_pixel(x, y);
	return (c & channel_flags);
}

void SBitmap::set_channel(int x, int y, u32 channel_flags, u32 val) {
	RGBColor c = get_pixel(x, y);
	val &= channel_flags;
	c &= (~channel_flags);
	c |= val;
	if (channel_flags & CHANNEL_ALPHA) {
		put_pixel_alpha(x, y, c);
		return;
	}
	put_pixel(x, y, c);
}

void SBitmap::copy_channel_rect(SBitmap* bmp_from, const SCoord& region_from, const SPoint& pt, u32 channel_flags) {
	u32 wf = region_from.width(), hf = region_from.height();
	u32 x, y;

	for (y = 0; y < hf; ++y) {
		int yo = y + pt.Y(this), ys = y + region_from.Y1(bmp_from);
		if (yo >= h || ys >= bmp_from->h)
			break;
		for (x = 0; x < wf; ++x) {
			int xo = x + pt.X(this), xs = x + region_from.X1(bmp_from);
			if (xo >= w || xs >= bmp_from->w)
				break;
			u32 v = bmp_from->get_channel(xs, ys, channel_flags);
			set_channel(xo, yo, channel_flags, v);
		}
	}
}

void SBitmap::negative_channel(u32 channel_flags) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			RGBColor v = get_pixel(x, y);
			u32 r = 0, g = 0, b = 0, a = 0;
			if (channel_flags & CHANNEL_RED) {
				r = 255 - RGB_RED(v);
			}
			if (channel_flags & CHANNEL_GREEN) {
				g = 255 - RGB_GREEN(v);
			}
			if (channel_flags & CHANNEL_BLUE) {
				b = 255 - RGB_BLUE(v);
			}
			if (channel_flags & CHANNEL_ALPHA) {
				a = 255 - RGB_ALPHA(v);
			}
			set_channel(x, y, channel_flags, RGBA_NO_CHECK(r, g, b, a));
		}
	}
}

u32 SBitmap::get_red(int x, int y) {
	return get_channel(x, y, CHANNEL_RED);
}

void SBitmap::set_red(int x, int y, u32 val) {
	set_channel(x, y, CHANNEL_RED, val);
}

void SBitmap::copy_red_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest) {
	copy_channel_rect(bitmap_from, region_from, pt_dest, CHANNEL_RED);
}

void SBitmap::negative_red(void) {
	negative_channel(CHANNEL_RED);
}

u32 SBitmap::get_green(int x, int y) {
	return RGB_GREEN(get_channel(x, y, CHANNEL_GREEN));
}

void SBitmap::set_green(int x, int y, u32 val) {
	set_channel(x, y, CHANNEL_GREEN, MAKE_RGBA(0, val, 0, 0));
}

void SBitmap::copy_green_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest) {
	copy_channel_rect(bitmap_from, region_from, pt_dest, CHANNEL_GREEN);
}

void SBitmap::negative_green(void) {
	negative_channel(CHANNEL_GREEN);
}

u32 SBitmap::get_blue(int x, int y) {
	return RGB_BLUE(get_channel(x, y, CHANNEL_BLUE));
}

void SBitmap::set_blue(int x, int y, u32 val) {
	set_channel(x, y, CHANNEL_BLUE, MAKE_RGBA(0, 0, val, 0));
}

void SBitmap::copy_blue_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest) {
	copy_channel_rect(bitmap_from, region_from, pt_dest, CHANNEL_BLUE);
}

void SBitmap::negative_blue(void) {
	negative_channel(CHANNEL_BLUE);
}

u32 SBitmap::get_channel(const SPoint& pt, u32 channel_flags) {
	return get_channel(pt.X(this), pt.Y(this), channel_flags);
}

void SBitmap::set_channel(const SPoint& pt, u32 channel_flags, u32 val) {
	set_channel(pt.X(this), pt.Y(this), channel_flags, val);
}

u32 SBitmap::get_red(const SPoint& p) {
	return get_red(p.X(this), p.Y(this));
}

void SBitmap::set_red(const SPoint& p, u32 val) {
	set_red(p.X(this), p.Y(this), val);
}

u32 SBitmap::get_green(const SPoint& p) {
	return get_green(p.X(this), p.Y(this));
}

void SBitmap::set_green(const SPoint& p, u32 val) {
	set_green(p.X(this), p.Y(this), val);
}

u32 SBitmap::get_blue(const SPoint& p) {
	return get_blue(p.X(this), p.Y(this));
}

void SBitmap::set_blue(const SPoint& p, u32 val) {
	set_blue(p.X(this), p.Y(this), val);
}

SBitmap* SBitmap::subbitmap(int x1, int y1, int x2, int y2) {
	SCoord co(x1, y1, x2, y2);
	return subbitmap(co);
}

SBitmap* SBitmap::subbitmap(const SPoint& p1, const SPoint& p2) {
	SCoord co(p1, p2);
	return subbitmap(co);
}

SBitmap* SBitmap::subbitmap(const SCoord& co) {
	SBitmap* ret = new SBitmap;
	SCoord c_this(0, 0, w - 1, h - 1);
	ret->sbd = new SubBitmapData;
	ret->sbd->parent = this;
	// set the sbd SCoord to the intersection, to get the correct width & height corresponding to actual pixels.
	co.Isect(c_this, ret->sbd->co, this);
	ret->btype = BITMAP_SUBBITMAP;
	ret->put_pixel_fn = SBitmap::put_pixel_subbmp;
	ret->get_pixel_fn = SBitmap::get_pixel_subbmp;
	ret->w = ret->sbd->co.X2(this) - ret->sbd->co.X1(this) + 1;
	ret->h = ret->sbd->co.Y2(this) - ret->sbd->co.Y1(this) + 1;
	return ret;
}

SBitmap* SBitmap::copy(void) const {
	u32 sz;
	SBitmap* ret;

	if (btype == BITMAP_DISPLAY_OWNED)
		ret = new SBitmap(w, h);
	else
		ret = new SBitmap(w, h, btype);

	sz = 0;
	switch (ret->btype) {
	case BITMAP_DEFAULT:
		sz = w * h * sizeof(u32);
		break;
	case BITMAP_MONO:
		sz = ((w * h) >> 3) + 1;
		break;
	case BITMAP_PALETTE:
		sz = w * h;
		copy_palette(ret);
		break;
	case BITMAP_GRAYSCALE:
		sz = w * h;
		break;
	case BITMAP_SUBBITMAP:
		ret->sbd = new SubBitmapData;
		*ret->sbd = *sbd;
		break;
	case BITMAP_16BITS:
		sz = w * h * 2;
		break;
	case BITMAP_24BITS:
		sz = w * h * 3;
		break;
	case BITMAP_DISPLAY_OWNED:
	case BITMAP_INVALID:
		break;
	}

	if (sz)
		memcpy(ret->bits, bits, sz);

	return ret;
}

void SBitmap::copy_palette(SBitmap* dest) const {
	if (nullptr == dest || dest->btype != BITMAP_PALETTE)
		return;
	for (u32 e = 0; e < palette()->ncolors; ++e)
		dest->pal->clrs[e] = pal->clrs[e];
}

void SBitmap::reflect_xy(void) {
	for (u32 y = 0; y < h && y < w; ++y) {
		for (u32 x = 0; x < y && x < w; ++x) {
			RGBColor here = get_pixel(x, y);
			put_pixel(x, y, get_pixel(y, x));
			put_pixel(y, x, here);
		}
	}
}

SBitmap* SBitmap::transpose(void) {
	SBitmap* ret = new SBitmap(h, w, btype);
	if (btype == BITMAP_PALETTE)
		copy_palette(ret);
	for (u32 x = 0; x < w; ++x)
		for (u32 y = 0; y < h; ++y)
			ret->put_pixel(y, x, get_pixel(x, y));
	return (ret);
}

bool SBitmap::transpose_and_replace(void) {
	if (btype == BITMAP_SUBBITMAP)
		return false;
	SBitmap* nb = transpose();
	if (nullptr == nb)
		return false;
	swap(nb);
	delete nb;
	return true;
}

void SBitmap::flip_horiz(void) {
	u32 x1 = 0, x2 = w - 1;
	while (x1 < x2) {
		for (u32 y = 0; y < h; ++y) {
			RGBColor here = get_pixel(x1, y);
			put_pixel(x1, y, get_pixel(x2, y));
			put_pixel(x2, y, here);
		}
		--x2;
		++x1;
	}
}

void SBitmap::flip_vert(void) {
	u32 y1 = 0, y2 = h - 1;
	while (y1 < y2) {
		for (u32 x = 0; x < w; ++x) {
			RGBColor here = get_pixel(x, y1);
			put_pixel(x, y1, get_pixel(x, y2));
			put_pixel(x, y2, here);
		}
		--y2;
		++y1;
	}
}

void SBitmap::blit_u(u32 x1_src, u32 y1_src, u32 x2_src, u32 y2_src, SBitmap* bmp_dest, u32 x_dest, u32 y_dest) const {
	u32 x1_dest, y1_dest, x2_dest, y2_dest;
	u32 xd, yd;
	u32 e;

	// Determine the actual rects we'll use to blit. Handles the case where the source
	// rect runs off the source image, as well as the case where the destination rect
	// runs off the destination image.
	SORT2(x1_src, x2_src, u32);
	SORT2(y1_src, y2_src, u32);

	VALID_RECT(this, x1_src, y1_src, x2_src, y2_src);

	x1_dest = x_dest;
	y1_dest = y_dest;
	x2_dest = x_dest + (x2_src - x1_src);
	y2_dest = y_dest + (y2_src - y1_src);

	VALID_RECT(bmp_dest, x1_dest, y1_dest, x2_dest, y2_dest);

	// If the destination rect is smaller than the source rect (since the destination rect ran off the image),
	// adjust the source rect.
	if ((x2_src - x1_src) != (x2_dest - x1_dest)) {
		x1_src += (x1_dest - x_dest);
		x2_src = x1_src + (x2_dest - x1_dest);
	}
	if ((y2_src - y1_src) != (y2_dest - y1_dest)) {
		y1_src += (y1_dest - y_dest);
		y2_src = y1_src + (y2_dest - y1_dest);
	}

	// OK, now do the blit.
	yd = y1_dest;
	for (; y1_src <= y2_src; ++y1_src) {
		xd = x1_dest;
		for (e = x1_src; e <= x2_src; ++e, ++xd) {
			bmp_dest->put_pixel(xd, yd, get_pixel(e, y1_src));
		}
		++yd;
	}
}


void SBitmap::blit(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest, int x_dest, int y_dest) const {
	int x1_dest, y1_dest, x2_dest, y2_dest;
	int xd, yd;
	int e;

	// Determine the actual rects we'll use to blit. Handles the case where the source
	// rect runs off the source image, as well as the case where the destination rect
	// runs off the destination image.
	SORT2(x1_src, x2_src, u32);
	SORT2(y1_src, y2_src, u32);

	VALID_RECT(this, x1_src, y1_src, x2_src, y2_src);

	x1_dest = x_dest;
	y1_dest = y_dest;
	x2_dest = x_dest + (x2_src - x1_src);
	y2_dest = y_dest + (y2_src - y1_src);

	VALID_RECT(bmp_dest, x1_dest, y1_dest, x2_dest, y2_dest);

	// If the destination rect is smaller than the source rect (since the destination rect ran off the image),
	// adjust the source rect.
	if ((x2_src - x1_src) != (x2_dest - x1_dest)) {
		x1_src += (x1_dest - x_dest);
		x2_src = x1_src + (x2_dest - x1_dest);
	}
	if ((y2_src - y1_src) != (y2_dest - y1_dest)) {
		y1_src += (y1_dest - y_dest);
		y2_src = y1_src + (y2_dest - y1_dest);
	}

	// OK, now do the blit.
	yd = y1_dest;
	for (; y1_src <= y2_src; ++y1_src) {
		xd = x1_dest;
		for (e = x1_src; e <= x2_src; ++e, ++xd) {
			bmp_dest->put_pixel(xd, yd, get_pixel(e, y1_src));
		}
		++yd;
	}
}

void SBitmap::blit(const SCoord& src, SBitmap* bmp_dest, const SPoint& dest) const {
	blit(src.X1(this), src.Y1(this), src.X2(this), src.Y2(this), bmp_dest, dest.X(bmp_dest), dest.Y(bmp_dest));
}

void SBitmap::blit(SBitmap* bmp_dest, int x_dest, int y_dest) const {
	blit(0, 0, w - 1, h - 1, bmp_dest, x_dest, y_dest);
}

void SBitmap::blit(SBitmap* bmp_dest, const SPoint& dest) const {
	blit(0, 0, w - 1, h - 1, bmp_dest, dest.X(bmp_dest), dest.Y(bmp_dest));
}

void SBitmap::blit(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest) const {
	blit(x1_src, y1_src, x2_src, y2_src, bmp_dest, 0, 0);
}

void SBitmap::blit(const SCoord& src, SBitmap* bmp_dest) const {
	blit(src, bmp_dest, SPoint(0, 0));
}

void SBitmap::blit(SBitmap* bmp_dest) const {
	blit(bmp_dest, SPoint(0, 0));
}

void SBitmap::blit(SBitmap* bmp_dest, int x_src, int y_src, int x_dest, int y_dest, int blit_w, int blit_h) const {
	// Allegro-compatible prototype.
	blit(x_src, y_src, x_src + blit_w - 1, y_src + blit_h - 1, bmp_dest, x_dest, y_dest);
}

void SBitmap::blit_blend_u(u32 x1_src, u32 y1_src, u32 x2_src, u32 y2_src, SBitmap* bmp_dest, u32 x_dest, u32 y_dest) const {
	u32 x1_dest, y1_dest, x2_dest, y2_dest;
	u32 xd, yd;
	u32 e;

	// First determine the blit rects.
	SORT2(x1_src, x2_src, u32);
	SORT2(y1_src, y2_src, u32);
	VALID_RECT(this, x1_src, y1_src, x2_src, y2_src);

	x1_dest = x_dest;
	y1_dest = y_dest;
	x2_dest = x_dest + (x2_src - x1_src);
	y2_dest = y_dest + (y2_src - y1_src);

	VALID_RECT(bmp_dest, x1_dest, y1_dest, x2_dest, y2_dest);

	if ((x2_src - x1_src) != (x2_dest - x1_dest)) {
		x1_src += (x1_dest - x_dest);
		x2_src = x1_src + (x2_dest - x1_dest);
	}
	if ((y2_src - y1_src) != (y2_dest - y1_dest)) {
		y1_src += (y1_dest - y_dest);
		y2_src = y1_src + (y2_dest - y1_dest);
	}

	// do the blit with alpha-blend
	yd = y1_dest;
	for (; y1_src <= y2_src; ++y1_src) {
		xd = x1_dest;
		for (e = x1_src; e <= x2_src; ++e, ++xd) {
			RGBColor c_src, c_dest;
			u32 r, g, b, a;

			c_src = get_pixel(e, y1_src);
			a = RGB_ALPHA(c_src);

			// check for the easy cases -- most cases most of the time should be easy.
			if (a == 0xFF) {
				// opaque
				bmp_dest->put_pixel(xd, yd, c_src);
				continue;
			}
			if (a == 0x00) {
				// transparent
				continue;
			}

			// if we get here, we have to do the blending
			c_dest = bmp_dest->get_pixel(xd, yd);

			r = RGB_RED(c_src) * a + RGB_RED(c_dest) * (255 - a);
			g = RGB_GREEN(c_src) * a + RGB_GREEN(c_dest) * (255 - a);
			b = RGB_BLUE(c_src) * a + RGB_BLUE(c_dest) * (255 - a);

			r /= 255UL;
			g /= 255UL;
			b /= 255UL;

			bmp_dest->put_pixel(xd, yd, RGB_NO_CHECK(r, g, b));
			}
		++yd;
		}
}

void SBitmap::blit_blend(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest, int x_dest, int y_dest) const {
	int x1_dest, y1_dest, x2_dest, y2_dest;
	int xd, yd;
	int e;

	// First determine the blit rects.
	SORT2(x1_src, x2_src, u32);
	SORT2(y1_src, y2_src, u32);
	VALID_RECT(this, x1_src, y1_src, x2_src, y2_src);

	x1_dest = x_dest;
	y1_dest = y_dest;
	x2_dest = x_dest + (x2_src - x1_src);
	y2_dest = y_dest + (y2_src - y1_src);

	VALID_RECT(bmp_dest, x1_dest, y1_dest, x2_dest, y2_dest);

	if ((x2_src - x1_src) != (x2_dest - x1_dest)) {
		x1_src += (x1_dest - x_dest);
		x2_src = x1_src + (x2_dest - x1_dest);
	}
	if ((y2_src - y1_src) != (y2_dest - y1_dest)) {
		y1_src += (y1_dest - y_dest);
		y2_src = y1_src + (y2_dest - y1_dest);
	}

	// do the blit with alpha-blend
	yd = y1_dest;
	for (; y1_src <= y2_src; ++y1_src) {
		xd = x1_dest;
		for (e = x1_src; e <= x2_src; ++e, ++xd) {
			RGBColor c_src, c_dest;
			u32 r, g, b, a;

			c_src = get_pixel(e, y1_src);
			a = RGB_ALPHA(c_src);

			// check for the easy cases -- most cases most of the time should be easy.
			if (a == 0xFF) {
				// opaque
				bmp_dest->put_pixel(xd, yd, c_src);
				continue;
			}
			if (a == 0x00) {
				// transparent
				continue;
			}

			// if we get here, we have to do the blending
			c_dest = bmp_dest->get_pixel(xd, yd);

			r = RGB_RED(c_src) * a + RGB_RED(c_dest) * (255 - a);
			g = RGB_GREEN(c_src) * a + RGB_GREEN(c_dest) * (255 - a);
			b = RGB_BLUE(c_src) * a + RGB_BLUE(c_dest) * (255 - a);

			r /= 255UL;
			g /= 255UL;
			b /= 255UL;

			bmp_dest->put_pixel(xd, yd, RGB_NO_CHECK(r, g, b));
			}
		++yd;
		}
}

void SBitmap::blit_blend(const SCoord& src, SBitmap* bmp_dest, const SPoint& dest) const {
	blit_blend(src.X1(this), src.Y1(this), src.X2(this), src.Y2(this), bmp_dest, dest.X(bmp_dest), dest.Y(bmp_dest));
}

void SBitmap::blit_blend(SBitmap* bmp_dest, int x_dest, int y_dest) const {
	blit_blend(0, 0, w - 1, h - 1, bmp_dest, x_dest, y_dest);
}

void SBitmap::blit_blend(SBitmap* bmp_dest, const SPoint& dest) const {
	blit_blend(0, 0, w - 1, h - 1, bmp_dest, dest.X(bmp_dest), dest.Y(bmp_dest));
}

void SBitmap::blit_blend(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest) const {
	blit_blend(x1_src, y1_src, x2_src, y2_src, bmp_dest, 0, 0);
}

void SBitmap::blit_blend(const SCoord& src, SBitmap* bmp_dest) const {
	blit_blend(src, bmp_dest, SPoint(0, 0));
}

void SBitmap::blit_blend(SBitmap* bmp_dest) const {
	blit_blend(bmp_dest, SPoint(0, 0));
}

void SBitmap::hline(int x1, int x2, int y, RGBColor c) {
	for (int x = x1; x <= x2; ++x)
		put_pixel(x, y, c);
}

void SBitmap::vline(int x, int y1, int y2, RGBColor c) {
	for (int y = y1; y <= y2; ++y)
		put_pixel(x, y, c);
}

/* implementation of Bresenham's integer line drawing algorithm */
#define INCR 1
#define DECR -1
#define PREDX 1
#define PREDY 0
static int dx, dy, e, e_inc, e_noinc;

void SBitmap::line_helper(int x1, int y1, int x2, int y2, int pred, int incdec, RGBColor c, PatternCallback callback, void* args) {
	int i, start, end, var;

	if (pred == PREDX) {
		start = x1;
		end = x2;
		var = y1;
	} else {
		start = y1;
		end = y2;
		var = x1;
	}

	for (i = start; i <= end; i++) {
		if (pred == PREDY) {
			if (nullptr != callback)
				c = callback(var, i, args);
			put_pixel(var, i, c);
		} else {
			if (nullptr != callback)
				c = callback(i, var, args);
			put_pixel(i, var, c);
		}

		if (e < 0) {
			e += e_noinc;
		} else {
			var += incdec;
			e += e_inc;
		}
	}
}

void SBitmap::line(int x1, int y1, int x2, int y2, RGBColor c) {
	int incdec, i;

	put_pixel(x1, y1, c);
	if (x1 == x2 && y1 == y2)
		return;
	put_pixel(x2, y2, c);

	if (x1 > x2) {
		SWAP(x1, x2, int);
		SWAP(y1, y2, int);
	}

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx == 0) {
		if (y1 > y2) {
			SWAP(y1, y2, int);
		}

		for (i = y1; i <= y2; i++)
			put_pixel(x1, i, c);

		return;
	}

	if (dy == 0) {
		for (i = x1; i < x2; i++)
			put_pixel(i, y1, c);
		return;
	}

	/* 0 < m < 1 */
	if (dy < dx && dy > 0) {
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, INCR, c, nullptr, nullptr);
	}

	/* m = 1 */
	if (dy == dx && dy > 0) {
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, INCR, c, nullptr, nullptr);
	}

	/* 1 < m < infinity */
	if ( dy > dx && dy > 0 ) {
		e_noinc = 2 * dx;
		e = 2 * dx - dy;
		e_inc = 2 * (dx - dy);
		line_helper(x1, y1, x2, y2, PREDY, INCR, c, nullptr, nullptr);
	}

	/* 0 > m > -1 */
	if (-dy < dx && dy < 0) {
		dy = -dy;
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, DECR, c, nullptr, nullptr);
	}

	/* m = -1 */
	if (dy == -dx && dy < 0) {
		dy = -dy;
		e_noinc = (2 * dy);
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, DECR, c, nullptr, nullptr);
	}

	/* -1 > m > 0 */
	if (-dy > dx && dy < 0) {
		dx = -dx;
		e_noinc = -(2 * dx);
		e = 2 * dx - dy;
		e_inc = -2 * (dx - dy);
		line_helper(x2, y2, x1, y1, PREDY, DECR, c, nullptr, nullptr);
	}
}

void SBitmap::line(const SPoint& p1, const SPoint& p2, RGBColor c) {
	line(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::line_pattern(int x1, int y1, int x2, int y2, PatternCallback callback, void* args) {
	int incdec, i;
	if (is_null(callback))
		return;

	put_pixel(x1, y1, callback(x1, y1, args));
	if (x1 == x2 && y1 == y2)
		return;
	put_pixel(x2, y2, callback(x2, y2, args));

	if (x1 > x2) {
		SWAP(x1, x2, int);
		SWAP(y1, y2, int);
	}

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx == 0) {
		if (y1 > y2) {
			SWAP(y1, y2, int);
		}

		for (i = y1; i <= y2; i++)
			put_pixel(x1, i, callback(x1, i, args));

		return;
	}

	if (dy == 0) {
		for (i = x1; i < x2; i++)
			put_pixel(i, y1, callback(i, y1, args));
		return;
	}

	/* 0 < m < 1 */
	if (dy < dx && dy > 0) {
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, INCR, C_BLACK, callback, args);
	}

	/* m = 1 */
	if (dy == dx && dy > 0) {
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, INCR, C_BLACK, callback, args);
	}

	/* 1 < m < infinity */
	if ( dy > dx && dy > 0 ) {
		e_noinc = 2 * dx;
		e = 2 * dx - dy;
		e_inc = 2 * (dx - dy);
		line_helper(x1, y1, x2, y2, PREDY, INCR, C_BLACK, callback, args);
	}

	/* 0 > m > -1 */
	if (-dy < dx && dy < 0) {
		dy = -dy;
		e_noinc = 2 * dy;
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, DECR, C_BLACK, callback, args);
	}

	/* m = -1 */
	if (dy == -dx && dy < 0) {
		dy = -dy;
		e_noinc = (2 * dy);
		e = 2 * dy - dx;
		e_inc = 2 * (dy - dx);
		line_helper(x1, y1, x2, y2, PREDX, DECR, C_BLACK, callback, args);
	}

	/* -1 > m > 0 */
	if (-dy > dx && dy < 0) {
		dx = -dx;
		e_noinc = -(2 * dx);
		e = 2 * dx - dy;
		e_inc = -2 * (dx - dy);
		line_helper(x2, y2, x1, y1, PREDY, DECR, C_BLACK, callback, args);
	}
}

void SBitmap::line_pattern(const SPoint& p1, const SPoint& p2, PatternCallback callback, void* args) {
	line_pattern(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), callback, args);
}

void SBitmap::circle(int x0, int y0, u32 radius, RGBColor c) {
	// Bresenham algorithm
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	put_pixel(x0, y0 + (int)radius, c);
	put_pixel(x0, y0 - (int)radius, c);
	put_pixel(x0 + (int)radius, y0, c);
	put_pixel(x0 - (int)radius, y0, c);
 
	while (x < y) {
		if (f >= 0) {
			--y;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		put_pixel(x0 + x, y0 + y, c);
		put_pixel(x0 - x, y0 + y, c);
		put_pixel(x0 + x, y0 - y, c);
		put_pixel(x0 - x, y0 - y, c);
		put_pixel(x0 + y, y0 + x, c);
		put_pixel(x0 - y, y0 + x, c);
		put_pixel(x0 + y, y0 - x, c);
		put_pixel(x0 - y, y0 - x, c);
	}
}

void SBitmap::circle(const SPoint& p, u32 rad, RGBColor c) {
	circle(p.X(this), p.Y(this), rad, c);
}

static int __scanline1[1024];
static int __scanline2[1024];

/*** Draw a filled circle with center at the specified point and with the specified radius and color. ***/
void SBitmap::fillcircle(int x0, int y0, u32 radius, RGBColor c) {
	/* First, for each row, find the first and last pixel on the circumference using Bresenham's algorithm */
	int* p1, * p2;
	const int nrw = 2 * radius + 1;
	int e;
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	// allocations are slow so let's try to avoid them if possible
	if (nrw <= 1024) {
		p1 = __scanline1;
		p2 = __scanline2;
	} else {
		p1 = new int [nrw];
		p2 = new int [nrw];
	}

	for (e = nrw - 1; e >= 0; --e)
		p1[e] = 0x7FFFFFFFL;
	zeromem(p2, sizeof(int) * nrw);

	// on one long line to avoid tripping up syntax parsing in IDEs
#define plot(x, y)	do { const int rw = (y - y0 + radius); if (x < p1[rw])	p1[rw] = x; if (x > p2[rw]) p2[rw] = x; } while (0)

	// copy-pasta copy-pasta copy-pasta

	plot(x0, y0 + radius);
	plot(x0, y0 - radius);
	plot(x0 + radius, y0);
	plot(x0 - radius, y0);
 
	while(x < y) {
		if(f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    
		plot(x0 + x, y0 + y);
		plot(x0 - x, y0 + y);
		plot(x0 + x, y0 - y);
		plot(x0 - x, y0 - y);
		plot(x0 + y, y0 + x);
		plot(x0 - y, y0 + x);
		plot(x0 + y, y0 - x);
		plot(x0 - y, y0 - x);
	}
	
#undef plot	

	/* Now we can draw the circle as a succession of horizontal lines */
	for (e = nrw - 1; e >= 0; --e)
		hline(p1[e], p2[e], y0 - radius + e, c);

	if (p1 != __scanline1)
		delete [] p1;
	if (p2 != __scanline2)
		delete [] p2;
}

void SBitmap::fillcircle(const SPoint& p, u32 radius, RGBColor c) {
	fillcircle(p.X(this), p.Y(this), radius, c);
}

#define plot(x, y)	do { const int rw = (y - y0 + radius); \
						if (x < p1[rw]) p1[rw] = x; \
						if (x > p2[rw]) p2[rw] = x; \
						} while (0)

/*** Draw a filled circle using the specified callback function to get the fill pattern. ***/
/*** "args" is passed along as the third argument to the pattern callback. ***/
void SBitmap::fillcirclepattern(int x0, int y0, u32 radius, PatternCallback callback, void* args) {
	int* p1, * p2;
	const int nrw = 2 * radius + 1;
	int e;
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	if (nrw <= 1024) {
		p1 = __scanline1;
		p2 = __scanline2;
	} else {
		p1 = new int [nrw];
		p2 = new int [nrw];
	}

	for (e = nrw - 1; e >= 0; --e)
		p1[e] = (1 << 30);
	zeromem(p2, sizeof(int) * nrw);

	// copy-pasta copy-pasta copy-pasta

	plot(x0, y0 + radius);
	plot(x0, y0 - radius);
	plot(x0 + radius, y0);
	plot(x0 - radius, y0);
 
	while(x < y) 
		{
		if(f >= 0) 
			{
			y--;
			ddF_y += 2;
			f += ddF_y;
			}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;    
		plot(x0 + x, y0 + y);
		plot(x0 - x, y0 + y);
		plot(x0 + x, y0 - y);
		plot(x0 - x, y0 - y);
		plot(x0 + y, y0 + x);
		plot(x0 - y, y0 + x);
		plot(x0 + y, y0 - x);
		plot(x0 - y, y0 - x);
		}
	
#undef plot	

	for (e = nrw - 1; e >= 0; --e) {
		for (x = p1[e]; x <= p2[e]; ++x)
			put_pixel(x, y0 - (int)radius + e, callback(x, y0 - radius + e, args));
	}

	if (p1 != __scanline1)
		delete [] p1;
	if (p2 != __scanline2)
		delete [] p2;
}

void SBitmap::fillcirclepattern(const SPoint& p, u32 radius, PatternCallback callback, void* args) {
	fillcirclepattern(p.X(this), p.Y(this), radius, callback, args);
}

/*** Reading and rasterizing vector graphics in SVG format. ***/

SBitmap* SBitmap::load_svg(const std::string& fname, float scale) {
	return load_svg(fname.c_str(), scale);
}

SBitmap* SBitmap::load_svg(const char* fname, float size) {
	struct NSVGimage* image;
	static NSVGrasterizer* rast = NULL;
	SBitmap* bmp;

	if (is_null(rast)) {
		rast = nsvgCreateRasterizer();
		NOT_NULL_OR_RETURN(rast, NULL);
 	}

	image = nsvgParseFromFile(fname, "px", size);
	bmp = new SBitmap(image->width, image->height);

	nsvgRasterize(rast, image, 0, 0, 1, bmp->bits, image->width, image->height, bmp->w * 4);

	return(bmp);
}

SBitmap* SBitmap::render_svg_core(void* ptr, u32 width, u32 height) {
	NSVGimage* image = (NSVGimage*)ptr;
	static struct NSVGrasterizer* rast = nullptr;
	if (is_null(rast)) {
		rast = nsvgCreateRasterizer();
	}

	float scale = 1.0;
	u32 nw, nh;
	if (0 == width && 0 == height) {
		nw = image->width;
		nh = image->height;
	} else if (0 == width) {
		scale = (float)height / (float)image->height;
		nw = (u32)floor(image->width * scale + 0.5);
		nh = (u32)floor(image->height * scale + 0.5);
	} else if (0 == height) {
		scale = (float)width / (float)image->width;
		nw = (u32)floor(image->width * scale + 0.5);
		nh = (u32)floor(image->height * scale + 0.5);
	} else {
		// Both width and height are non-zero: ignore them.
		nw = image->width;
		nh = image->height;		
	}

	SBitmap* bmp = new SBitmap(nw, nh);
	nsvgRasterize(rast, image, 0, 0, scale, bmp->bits, nw, nh, bmp->width() * 4);
	nsvgDelete(image);
	return bmp;
}

SBitmap* SBitmap::render_svg(const char* fname, u32 width, u32 height) {
	struct NSVGimage* image;
	image = nsvgParseFromFile(fname, "px", 96);
	return render_svg_core(image, width, height);
}

SBitmap* SBitmap::render_svg_from_mem(char* mem, u32 width, u32 height) {
	struct NSVGimage* image;
	image = nsvgParse(mem, "px", 96);
	return render_svg_core(image, width, height);
}

SBitmap* SBitmap::render_svg(const std::string& fname, u32 width, u32 height) {
	return render_svg(fname.c_str(), width, height);
}

/*** Load an image of any supported format. ***/

#define FORMAT_UNK	(-1)
#define FORMAT_BMP	0
#define FORMAT_PNG	1
#define FORMAT_TGA	2
#define FORMAT_GIF	3
#define FORMAT_PCX	4
#define FORMAT_RAW	5
#define FORMAT_JPG	6
#define FORMAT_RFI	7

SBitmap* SBitmap::load_bmp(const std::string& fname) {
	return load_bmp(fname.c_str());
}

SBitmap* SBitmap::load_bmp(const char* fname) {
	int x, y;
	int comp;
	unsigned char* pixel_data;
	SBitmap* bmp_ret;

	if (has_extension(fname, "svg")) {
		bmp_ret = load_svg(fname, 96.);
		return(bmp_ret);
	}

	if (has_extension(fname, "pcx")) {
		bmp_ret = load_pcx(fname);
		return(bmp_ret);
	}

	if (has_extension(fname, "raw")) {
		bmp_ret = load_raw(fname);
		return(bmp_ret);
	}

	if (has_extension(fname, "rfi")) {
		bmp_ret = from_ramfile(fname);
		return(bmp_ret);
	}

	pixel_data = stbi_load(fname, &x, &y, &comp, 4);
	if (is_null(pixel_data))
		return(nullptr);
	if (comp < 3) {
		stbi_image_free(pixel_data);
		return(nullptr);
	}

	bmp_ret = new SBitmap(x, y);
	if (bmp_ret != nullptr) {
		memcpy(bmp_ret->bits, pixel_data, x * y * 4);
	}
	stbi_image_free(pixel_data);
	
	return(bmp_ret);
}

SBitmap* SBitmap::load_bmp(RamFile* rf) {
	int x, y;
	int comp;
	unsigned char* pixel_data;
	SBitmap* bmp_ret;
	
	pixel_data = stbi_load_from_memory((unsigned char*) rf->buffer(), (int) rf->length(), &x, &y, &comp, 4);
	if (is_null(pixel_data))
		return(nullptr);
	if (comp < 3) {
		stbi_image_free(pixel_data);
		return(nullptr);
	}

	bmp_ret = new SBitmap(x, y);
	if (bmp_ret != nullptr) {
		memcpy(bmp_ret->bits, pixel_data, x * y * 4);
	}
	stbi_image_free(pixel_data);

	return bmp_ret;
}

bool file_exists(const char* fname) {
	struct stat info;
	if (stat(fname, &info) != 0)
		return false;
	return true;
}

Font::Font(const char* fname) {
	u32 flen;
	FILE *f;

	font = new ttfont;
	f = fopen(fname, "rb");
	if (is_null(f))
		return;
	flen = filelen(f);
	builtin = false;
	
	font->data = new unsigned char [flen];
	if (is_null(font->data)) {
		fclose(f);
		return;
	}

	fread(font->data, 1, flen, f);
	fclose(f);

	if (stbtt_InitFont(&font->info, font->data, stbtt_GetFontOffsetForIndex(font->data, 0)) == 0) {
		codehappy_cerr << "*** Error loading font " << fname << "\n";
	}

	return;
}

Font::Font(const std::string& fname) {
	u32 flen;
	FILE *f;

	font = new ttfont;
	f = fopen(fname.c_str(), "rb");
	if (is_null(f))
		return;
	flen = filelen(f);
	builtin = false;
	
	font->data = new unsigned char [flen];
	if (is_null(font->data)) {
		fclose(f);
		return;
	}

	fread(font->data, 1, flen, f);
	fclose(f);

	if (stbtt_InitFont(&font->info, font->data, stbtt_GetFontOffsetForIndex(font->data, 0)) == 0) {
		codehappy_cerr << "*** Error loading font " << fname << "\n";
	}

	return;
}

Font::Font(ttfont* builtin_font) {
	built_in(builtin_font);
}

Font::Font(const ttfont& builtin_font) {
	built_in((ttfont*)&builtin_font);
}

SBitmap* Font::codepoint_bmp(int codepoint, int size) {
	u8* bits, *px;
	int w, h;
	int e, f;
	SBitmap *bmp;
	
	bits = stbtt_GetCodepointBitmap(&font->info,
						0,
						stbtt_ScaleForPixelHeight(&font->info, size),
						codepoint,
						&w,
						&h,
						0,
						0);

	bmp = new SBitmap(w, h, BITMAP_GRAYSCALE);
	px = bits;
	for (f = 0; f < h; ++f) {
		for (e = 0; e < w; ++e) {
			bmp->put_pixel(e, f, RGB_NO_CHECK(*px, *px, *px));
			++px;
		}
	}
	free(bits);

	return(bmp);
}

SBitmap* Font::codepoint_sprite(int codepoint, int size, RGBColor c) {
	SBitmap* sprite;
	u8* bits, *px;
	int w, h;
	int e, f;
	
	bits = stbtt_GetCodepointBitmap(&font->info,
						0,
						stbtt_ScaleForPixelHeight(&font->info, size),
						codepoint,
						&w,
						&h,
						0,
						0);

	sprite = new SBitmap(w, h);
	px = bits;
	for (f = 0; f < h; ++f) {
		for (e = 0; e < w; ++e) {
			sprite->put_pixel(e, f, ADD_ALPHA(c, *px));
			++px;
		}
	}
	free(bits);

	return(sprite);
}

SBitmap* Font::render_cstr(const char* str, int size, bool single_character_blend, u16** char_index_to_x_pos) {
	/*** translate to Unicode string and render ***/
	ustring ustr;
	SBitmap* bmpret;
	
	ustr = cstr2ustr(str);
	NOT_NULL_OR_RETURN(ustr, NULL);

	bmpret = this->render_ustr(ustr, size, single_character_blend, char_index_to_x_pos);
	delete [] ustr;

	return(bmpret);
}

struct FontPairHash {
    size_t operator()(std::pair<ttfont *, u32> p) const noexcept {
        return (size_t)p.first ^ (size_t)p.second;
    }
};

static std::unordered_map<std::pair<ttfont *, u32>, u32, FontPairHash> __font_width_cache;
u32 Font::width60(u32 pt_size) {
	const char msg60[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed";
	static ustring ustr = nullptr;
	u32 ret;
	std::pair<ttfont *, u32> pr = std::make_pair(this->font, pt_size);
	if (builtin && __font_width_cache.find(pr) != __font_width_cache.end()) {
		return __font_width_cache[pr];
	}

	if (is_null(ustr))
		ustr = cstr2ustr(msg60);

	SBitmap* render = render_ustr(ustr, pt_size, false, nullptr);
	ret = render->width();
	delete render;

	if (builtin)
		__font_width_cache[pr] = ret;
	return ret;
}

bool pixel_ok(const SBitmap* bmp, int x, int y) {
	if (nullptr == bmp)
		return false;
	if (x < 0 || x >= bmp->width())
		return false;
	if (y < 0 || y >= bmp->height())
		return false;
	return true;
}

/*** Helper function: Trim a text bitmap produced by bmp_ustr(). Note that this frees the passed-in bitmap. ***/
static SBitmap* trim_text_bmp(SBitmap* bmp, bool left_space_ok)
{
	int x1, x2, y1, y2, xs, ys;
	SBitmap* bmpret;
	for (y1 = 0; y1 < bmp->height(); ++y1)
		for (x1 = 0; x1 < bmp->width(); ++x1)
			{
			if (bmp->get_pixel(x1, y1) != C_BLACK)
				{
				goto Pass2;
				}
			}
Pass2:
	for (xs = 0; xs < bmp->width(); ++xs)
		for (ys = 0; ys < bmp->height(); ++ys)
			{
			if (bmp->get_pixel(xs, ys) != C_BLACK)
				{
				x1 = min_int(x1, xs);
				y1 = min_int(y1, ys);
				goto Pass3;
				}
			}
Pass3:
	for (y2 = bmp->height() - 1; y2 >= 0; --y2)
		for (x2 = bmp->width() - 1; x2 >= 0; --x2)
			{
			if (bmp->get_pixel(x2, y2) != C_BLACK)
				{
				goto Pass4;
				}
			}
Pass4:
	for (xs = bmp->width() - 1; xs >= 0; --xs)
		for (ys = bmp->height() - 1; ys >= 0; --ys)
			{
			if (bmp->get_pixel(xs, ys) != C_BLACK)
				{
				x2 = max_int(x2, xs);
				y2 = max_int(y2, ys);
				goto LDone;
				}
			}
LDone:
	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	x1 = CLAMP(x1, 0, bmp->width() - 1);
	x2 = CLAMP(x2, 0, bmp->width() - 1);
	y1 = CLAMP(y1, 0, bmp->height() - 1);
	y2 = CLAMP(y2, 0, bmp->height() - 1);
	if (left_space_ok)
		x1 = 0;
	bmpret = new SBitmap(x2 - x1 + 1, y2 - y1 + 1, bmp->type());
	bmp->blit(x1, y1, x2, y2, bmpret, 0, 0);
	delete bmp;
	return (bmpret);
}

SBitmap* Font::render_ustr(ustring str, int size, bool single_character_blend, u16** char_index_to_x_pos) {
	float scale = stbtt_ScaleForPixelHeight(&font->info, size);
	int ascent;
	int baseline;
	int advance;
	int lsb;
	int x;
	float xpos;
	const uch* w;
	SBitmap *outbmp;
	SBitmap* cbmp;
	int cx, cy;
	u32 len;
	int ix;
	const int TAB_SPACES = 4;

	if (not_null(char_index_to_x_pos)) {
		len = ustrlen(str);
		*char_index_to_x_pos = NEW_ARRAY(u16, len + 1);
	}

	stbtt_GetFontVMetrics(&font->info, &ascent, 0, 0);
	baseline = (int) (ascent * scale);

	cx = 128;
	cy = 128;

	x = 20;	/* left-padding */
	w = str;
	while (*w) {
		if (*w == '\t')
			stbtt_GetCodepointHMetrics(&font->info, ' ', &advance, &lsb);
		else
			stbtt_GetCodepointHMetrics(&font->info, *w, &advance, &lsb);
		if ((int)floor(advance * scale + 0.5) > cx)
			{
			cx = (int)floor(advance * scale + 0.5);
			cx += (cx >> 1);
			}
		if (*w == '\t')
			x += (int)floor(advance * scale + 0.5) * TAB_SPACES;
		else
			x += (int)floor(advance * scale + 0.5);
		++w;
	}

	// probably bigger than we need, but that's O.K. 
	outbmp = new SBitmap(x, size * 3, BITMAP_GRAYSCALE);
	outbmp->clear();

	if (outbmp->height() > cy / 2)
		cy = outbmp->height() * 2;

	xpos = 2.;
	w = str;
	ix = 0;
	while (*w) {
		int x0, y0, x1, y1;
		float x_shift = xpos - (float) floor(xpos);
		int e, n = 1;

		if (*w == '\t') {
			n = 4;
			stbtt_GetCodepointHMetrics(&font->info, ' ', &advance, &lsb);
			stbtt_GetCodepointBitmapBoxSubpixel(&font->info, ' ', scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
		} else {
			n = 1;
			stbtt_GetCodepointHMetrics(&font->info, *w, &advance, &lsb);
			stbtt_GetCodepointBitmapBoxSubpixel(&font->info, *w, scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
		}

		if (not_null(char_index_to_x_pos)) {
			(*char_index_to_x_pos)[ix] = xpos + x0;
			++ix;
		}

		for (e = 0; e < n; ++e) {
		if (!single_character_blend) {
			// Faster but slightly-lower quality render
			if (pixel_ok(outbmp, ((int)xpos + x0), baseline + y0 + 4) &&
				pixel_ok(outbmp, ((int)xpos + x1), (baseline + y1 + 4))) {
				stbtt_MakeCodepointBitmapSubpixel(&font->info,
						outbmp->pixel_loc(xpos + x0, baseline + y0 + 4),
						x1 - x0,
						y1 - y0,
						outbmp->width(),
						scale,
						scale,
						x_shift,
						0,
						*w);
			}
		} else {
			// Use codepoint_bmp() to generate the single-character SBitmap, then blend into the image.
			int xx, yy;
			if (*w == '\t')
				cbmp = this->codepoint_bmp(' ', size);
			else
				cbmp = this->codepoint_bmp(*w, size);
			if (not_null(cbmp)) {
				for (yy = 0; yy < cbmp->height(); ++yy)
					for (xx = 0; xx < cbmp->width(); ++xx) {
						RGBColor cc, cb;
						int xd, yd;
						u32 g1, g2;

						xd = (int)xpos + x0 + xx;
						yd = baseline + y0 + 4 + yy;

						cc = cbmp->get_pixel(xx, yy);
						if (cc == C_BLACK)
							continue;
						if (cc == C_WHITE) {
							outbmp->put_pixel(xd, yd, C_WHITE);
							continue;
						}
						cb = outbmp->get_pixel(xd, yd);
						g1 = RGB_RED(cb);
						g2 = RGB_RED(cc);
						g1 += g2;
						g1 >>= 1;
						outbmp->put_pixel(xd, yd, MAKE_RGB(g1, g1, g1));
					}
				delete cbmp;
			}
		}

		xpos += (advance * scale);
		}	// end for-loop
		++w;

		if (*w && *(w - 1) != '\t')
			xpos += scale * stbtt_GetCodepointKernAdvance(&font->info, *(w - 1), *w);
	}

	if (not_null(char_index_to_x_pos))
		(*char_index_to_x_pos)[ix] = xpos;

	return(trim_text_bmp(outbmp, true));
}

void Font::blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, int x, int y, PatternCallback callback, void* args) {
	int x0, x1, y0, y1;
	int xx, yy;
	int e, f;
	int sx, sy;

	x0 = x;
	y0 = y;
	x1 = x + ttf_bmp->width() - 1;
	y1 = y + ttf_bmp->height() - 1;

	if ((x > 0 && x >= dest_bmp->width()) || (y > 0 && y >= dest_bmp->height()))
		return;

	VALID_RECT(dest_bmp, x0, y0, x1, y1);

	xx = x1 - x0;
	yy = y1 - y0;

	sx = (x0 - x);
	sy = (y0 - y);

	for (f = 0; f <= yy; ++f) {
		for (e = 0; e <= xx; ++e) {
			RGBColor c = ttf_bmp->get_pixel(sx + e, sy + f);
			RGBColor cd;
			RGBColor text_color = callback(e, f, args);
			u32 gray = (c & 0xff);
			u32 r, g, b;
			if (c == C_BLACK)
				continue;
			if (c == C_WHITE) {
				dest_bmp->put_pixel(x0 + e, y0 + f, text_color);
				continue;
			}
			// OK, easy cases are out of the way: we need to do some blending
			cd = dest_bmp->get_pixel(x0 + e, y0 + f);

			r = (RGB_RED(text_color) * gray) + (RGB_RED(cd) * (255 - gray));
			g = (RGB_GREEN(text_color) * gray) + (RGB_GREEN(cd) * (255 - gray));
			b = (RGB_BLUE(text_color) * gray) + (RGB_BLUE(cd) * (255 - gray));
			r /= 255;
			g /= 255;
			b /= 255;

			dest_bmp->put_pixel(x0 + e, y0 + f, RGB_NO_CHECK(r, g, b));			
		}
	}
}

void Font::blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, int x, int y, RGBColor text_color) {
	int x0, x1, y0, y1;
	int xx, yy;
	int e, f;
	int sx, sy;

	x0 = x;
	y0 = y;
	x1 = x + ttf_bmp->width() - 1;
	y1 = y + ttf_bmp->height() - 1;

	if ((x > 0 && x >= dest_bmp->width()) || (y > 0 && y >= dest_bmp->height()))
		return;

	VALID_RECT(dest_bmp, x0, y0, x1, y1);

	xx = x1 - x0;
	yy = y1 - y0;

	sx = (x0 - x);
	sy = (y0 - y);

	for (f = 0; f <= yy; ++f) {
		for (e = 0; e <= xx; ++e) {
			RGBColor c = ttf_bmp->get_pixel(sx + e, sy + f);
			RGBColor cd;
			u32 gray = (c & 0xff);
			u32 r, g, b;
			if (c == C_BLACK)
				continue;
			if (c == C_WHITE) {
				dest_bmp->put_pixel(x0 + e, y0 + f, text_color);
				continue;
			}
			// OK, easy cases are out of the way: we need to do some blending
			cd = dest_bmp->get_pixel(x0 + e, y0 + f);

			r = (RGB_RED(text_color) * gray) + (RGB_RED(cd) * (255 - gray));
			g = (RGB_GREEN(text_color) * gray) + (RGB_GREEN(cd) * (255 - gray));
			b = (RGB_BLUE(text_color) * gray) + (RGB_BLUE(cd) * (255 - gray));
			r /= 255;
			g /= 255;
			b /= 255;

			dest_bmp->put_pixel(x0 + e, y0 + f, RGB_NO_CHECK(r, g, b));			
		}
	}
}

void Font::blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, const SCoord& region, RGBColor text_color, u32 center_align_flags) {
	int x = region.X1(dest_bmp), y = region.Y1(dest_bmp);

	if (center_align_flags & CENTERED_HORIZ) {
		x = region.center_X(dest_bmp);
		x -= ttf_bmp->width() / 2;
	}
	if (center_align_flags & CENTERED_VERT) {
		y = region.center_Y(dest_bmp);
		y -= ttf_bmp->height() / 2;
	}
	if (!iszero(center_align_flags & ALIGN_TOP)) {
		y = region.Y1(dest_bmp) + 2;
	}
	if (!iszero(center_align_flags & ALIGN_BOTTOM)) {
		y = region.Y2(dest_bmp) - (2 + ttf_bmp->height());
	}
	if (!iszero(center_align_flags & ALIGN_LEFT)) {
		x = region.X1(dest_bmp) + 2;
	}
	if (!iszero(center_align_flags & ALIGN_RIGHT)) {
		x = region.X2(dest_bmp) - (2 + ttf_bmp->width());
	}

	blit(ttf_bmp, dest_bmp, x, y, text_color);
}

void Font::built_in(ttfont* builtin_font) {
	builtin = true;
	font = builtin_font;
}

Font::~Font() {
	if (!builtin && not_null(font))
		free(font);
}

static void copy_and_translate_bmp(SBitmap* src_bmp, SBitmap* dest_bmp) {
	int y;
	int x;
	RGBColor c;

	assert(not_null(src_bmp) && not_null(dest_bmp));
	assert(src_bmp->width() == dest_bmp->width());
	assert(src_bmp->height() == dest_bmp->height());

	for (y = 0; y < src_bmp->height(); ++y)
		for (x = 0; x < src_bmp->width(); ++x) {
			RGBColor c;
			c = src_bmp->get_pixel(x, y);
			dest_bmp->put_pixel_alpha(x, y, c);
		}
}

/*** Write the SBitmap to disk file in raw RGB format (no alpha channel) ***/
int SBitmap::save_raw(SBitmap* bmp, const char* szFile) {
	FILE* f;
	u32 x, y;
	RGBColor c;

	f = fopen(szFile, "wb");
	NOT_NULL_OR_RETURN(f, 1);
	fputc(bmp->width() & 0xff, f);
	fputc(bmp->width() >> 8, f);
	fputc(bmp->height() & 0xff, f);
	fputc(bmp->height() >> 8, f);

	for (y = 0; y < bmp->h; ++y)
		for (x = 0; x < bmp->w; ++x) {
			c = bmp->get_pixel(x, y);
			fputc(RGB_RED(c), f);
			fputc(RGB_GREEN(c), f);
			fputc(RGB_BLUE(c), f);
		}

	fclose(f);

	return(0);
}

/*** Load an SBitmap from file in raw RGB format (no alpha channel) ***/
SBitmap* SBitmap::load_raw(const char* szFile) {
	FILE* f;
	u32 x, y;
	u32 r, g, b;
	RGBColor c;
	SBitmap* bmpret;

	// TODO: use ramfiles for I/O here, then the "raw" image could even be compressed.

	f = fopen(szFile, "rb");
	NOT_NULL_OR_RETURN(f, NULL);

	x = fgetc(f);
	x |= (fgetc(f) << 8);
	y = fgetc(f);
	y |= (fgetc(f) << 8);

	bmpret = new SBitmap(x, y, BITMAP_DEFAULT);
	for (y = 0; y < bmpret->h; ++y)
		for (x = 0; x < bmpret->w; ++x) {
			r = fgetc(f);
			g = fgetc(f);
			b = fgetc(f);
			c = RGB_NO_CHECK(r, g, b);
			bmpret->put_pixel(x, y, c);
		}

	return(bmpret);
}

/* Save an image to any supported format. */
u32 SBitmap::save_bmp(const char* fname) {
	SBitmap* bmp_use;
	u32 fmt = FORMAT_UNK;
	int bpp = 4;
	int ret;

	if (has_extension(fname, "bmp"))
		fmt = FORMAT_BMP;
	else if (has_extension(fname, "png"))
		fmt = FORMAT_PNG;
	else if (has_extension(fname, "tga"))
		fmt = FORMAT_TGA;
	else if (has_extension(fname, "gif"))
		fmt = FORMAT_GIF;
	else if (has_extension(fname, "pcx"))
		fmt = FORMAT_PCX;
	else if (has_extension(fname, "jpg") || has_extension(fname, "jpeg"))
		fmt = FORMAT_JPG;
	else if (has_extension(fname, "rfi"))
		fmt = FORMAT_RFI;
	else
		fmt = FORMAT_RAW;

	ret = 0;
	bmp_use = this;

	switch (btype) {
	default:
		return(1);

	case BITMAP_DEFAULT:
	case BITMAP_SUBBITMAP:
	case BITMAP_DISPLAY_OWNED:
		break;

	case BITMAP_GRAYSCALE:
		bpp = 1;
		break;

	case BITMAP_16BITS:
	case BITMAP_24BITS:
		bmp_use = new SBitmap(w, h, BITMAP_DEFAULT);
		copy_and_translate_bmp(this, bmp_use);
		break;

	case BITMAP_MONO:
	case BITMAP_PALETTE:
		if (FORMAT_GIF != fmt && FORMAT_PCX != fmt) {
			bmp_use = new SBitmap(w, h, BITMAP_DEFAULT);
			copy_and_translate_bmp(this, bmp_use);
		}
		break;
	}

	switch (fmt) {
	default:
		// unknown file type?
LErr:		if (bmp_use != this)
			delete bmp_use;
		return(1);
		
	case FORMAT_TGA:
		if (!stbi_write_tga(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits))
			goto LErr;
		break;

	case FORMAT_PNG:
		if (!stbi_write_png(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits, bpp * bmp_use->w))
			goto LErr;
		break;

	case FORMAT_BMP:
		if (!stbi_write_bmp(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits))
			goto LErr;
		break;

	case FORMAT_GIF:
		ret = save_gif(bmp_use, fname);
		break;

	case FORMAT_PCX:
		ret = save_pcx(bmp_use, fname);
		break;

	case FORMAT_RAW:
		ret = save_raw(bmp_use, fname);
		break;

	case FORMAT_JPG:
		ret = !tje_encode_to_file_at_quality(fname, 3, bmp_use->w, bmp_use->h, bpp, (const unsigned char*)bmp_use->bits);
		break;

	case FORMAT_RFI:
		ret = to_ramfile(fname);
		break;
	}

	if (bmp_use != this)
		delete bmp_use;
	return(ret);
}

u32 SBitmap::save_bmp(const std::string& fname) {
	return save_bmp(fname.c_str());
}

void SBitmap::fill_alpha_channel(int val) {
	if (bits) {
		u8 *ww = bits;
		int e, f;

		// only makes sense for 32bpp bitmaps
		switch (btype)
			{
		case BITMAP_MONO:
		case BITMAP_GRAYSCALE:
		case BITMAP_PALETTE:
		case BITMAP_16BITS:
		case BITMAP_24BITS:
		case BITMAP_INVALID:
			return;
		default:
			break;
			}

		ww += 3;
		for (f = 0; f < h; ++f)
			for (e = 0; e < w; ++e) {
				*ww = (u8)(val & 0xff);
				ww += 4;
			}
	}
}

void SBitmap::negative(void) {
	u32 x, y;
	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			u32 r, g, b, a;
			RGBColor c;
			c = get_pixel(x, y);
			r = RGB_RED(c);
			g = RGB_GREEN(c);
			b = RGB_BLUE(c);
			a = RGB_ALPHA(c);
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			put_pixel(x, y, MAKE_RGBA(r, g, b, a));
		}
}

void SBitmap::set_transparent_color(RGBColor c) {
	u32 x, y;
	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			RGBColor p = get_pixel(x, y);
			p &= 0xffffff;
			if (p == c) {
				p = ADD_ALPHA(p, ALPHA_TRANSPARENT);
				put_pixel(x, y, p);
			}
		}
}

SBitmap* SBitmap::resize(u32 new_width, u32 new_height) const {
	SBitmap* bmpret;
	int bpp;
	double ratio;
	SBitmap* bmpuse;

	bmpuse = const_cast<SBitmap*>(this);
	switch (btype)
		{
	case BITMAP_DEFAULT:
	case BITMAP_DISPLAY_OWNED:
		bpp = 4;
		break;
		
	case BITMAP_GRAYSCALE:
		bpp = 1;
		break;

	case BITMAP_MONO:
	case BITMAP_PALETTE:
	case BITMAP_SUBBITMAP:
	case BITMAP_16BITS:
	case BITMAP_24BITS:
	default:
		// support these by creating a temporary bitmap
		bmpuse = new SBitmap(w, h, BITMAP_DEFAULT);
		NOT_NULL_OR_RETURN(bmpuse, nullptr);
		this->blit(0UL, 0UL, w, h, bmpuse, 0UL, 0UL);
		bpp = 4;
		}

	if (new_width == 0) {
		/* set the width according to aspect ratio */
		if (unlikely(new_height == 0))
			return(nullptr);
		ratio = (double)(w) / (double)(h);
		ratio *= (double)new_height;
		new_width = ROUND_FLOAT_TO_INT(ratio);
	} else if (new_height == 0) {
		/* set the height acccording to aspect ratio */
		ratio = (double)(h)/ (double)(w);
		ratio *= (double)new_width;
		new_height = ROUND_FLOAT_TO_INT(ratio);
	}

	bmpret = new SBitmap(new_width, new_height, bmpuse->btype);
	if (unlikely(is_null(bmpret)))
		return(nullptr);

	if (!stbir_resize_uint8((unsigned char*)bmpuse->bits, bmpuse->w, bmpuse->h, 0, (unsigned char*)bmpret->bits, bmpret->w, bmpret->h, 0, bpp)) {
		delete bmpret;
		if (bmpuse != this)
			delete bmpuse;
		return nullptr;
	}

	if (bmpuse != this)
		delete bmpuse;

	return(bmpret);
}

SBitmap* SBitmap::scale_rational(u32 num, u32 den) const {
	u32 nw, nh;

	nw = ((w * num) / den);
	nh = ((h * num) / den);

	return (resize(nw, nh));
}

bool SBitmap::scale_rational_and_replace(u32 num, u32 den) {
	if (BITMAP_SUBBITMAP ==  btype)
		return false;
	SBitmap* nb = scale_rational(num, den);
	if (is_null(nb))
		return false;
	swap(nb);
	delete nb;
	return true;
}

bool SBitmap::resize_and_replace(u32 new_width, u32 new_height) {
	if (btype == BITMAP_SUBBITMAP)
		return false;
	SBitmap* nb = resize(new_width, new_height);
	if (is_null(nb))
		return false;
	swap(nb);
	delete nb;
	return true;
}

/*** Some default pattern callbacks that you can use for fills. ***/
/*** The void* argument, in these cases, is a struct "fillsettings", containing the desired background and foreground colors for the
	pattern fill. ***/
RGBColor diamond_pattern(int x, int y, void* fill_settings) {
	// A simple diamond pattern.
	// ..*..
	// .***.
	// *****
	// .***.
	// ..*..
	// TODO: generalize using the size field in fillsettings?
	const fillsettings* fs = (fillsettings*)fill_settings;
	x %= 5;
	y %= 5;
	switch (y) {
	case 0:
	case 4:
		return ((x == 2) ? fs->foreground : fs->background);

	case 1:
	case 3:
		return ((x == 0 || x == 4) ? fs->background : fs->foreground);

	case 2:
		return fs->foreground;
	}

	return fs->background;
}

/*** Draw a checkerboard pattern, where the squares are of length fill_settings->size. ***/
RGBColor checkerboard_pattern(int x, int y, void* fill_settings) {
	fillsettings* fs = (fillsettings*)fill_settings;

	if (iszero(fs->size))
		fs->size = 5UL;

	x /= fs->size;
	y /= fs->size;

	x = (x & 1) + (y & 1);
	if (iszero(x & 1))
		return fs->background;

	return fs->foreground;
}

struct DotSettings {
	u32 r1;
	u32 r2;
	RGBColor c;
};

/*** Dotted line pattern, used by line_dotted() below. ***/
RGBColor dotted_line_pattern(int x, int y, void* dot_settings) {
	DotSettings* ds = (DotSettings*)dot_settings;
	static u32 c = 0;
	if ((c % ds->r2) < ds->r1) {
		++c;
		return ds->c;
	}
	++c;
	return (RGBColor)(-1);
}

void SBitmap::hline_dotted(int x1, int x2, int y, RGBColor c, u32 r1, u32 r2) {
	u32 cc = 0;
	SORT2(x1, x2, int);
	if (r2 < r1)
		r2 = r1;
	if (0 == r1)
		r1 = r2 = 1;
	while (x1 <= x2) {
		if ((cc % r2) < r1)
			put_pixel(x1, y, c);
		x1++;
		cc++;
	}
}

void SBitmap::vline_dotted(int x, int y1, int y2, RGBColor c, u32 r1, u32 r2) {
	u32 cc = 0;
	SORT2(y1, y2, int);
	if (r2 < r1)
		r2 = r1;
	if (0 == r1)
		r1 = r2 = 1;
	while (y1 <= y2) {
		if ((cc % r2) < r1)
			put_pixel(x, y1, c);
		++y1;
		++cc;
	}
}

void SBitmap::rect_dotted(int x1, int y1, int x2, int y2, RGBColor c, u32 r1, u32 r2) {
	int x, y;
	u32 cc = 0;
	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	if (r2 < r1)
		r2 = r1;
	if (0 == r1)
		r1 = r2 = 1;
	x = x1;
	y = y1;
	while (x <= x2) {
		if ((cc % r2) < r1)
			put_pixel(x, y, c);
		++x;
		++cc;
	}
	x = x2;
	++y;
	while (y <= y2) {
		if ((cc % r2) < r1)
			put_pixel(x, y, c);
		++y;
		++cc;
	}
	y = y2;
	--x;
	while (x >= x1) {
		if ((cc % r2) < r1)
			put_pixel(x, y, c);
		--x;
		++cc;
	}
	x = x1;
	--y;
	while (y >= y1) {
		if ((cc % r2) < r1)
			put_pixel(x, y, c);
		--y;
		++cc;
	}
}

void SBitmap::line_dotted(int x1, int y1, int x2, int y2, RGBColor c, u32 r1, u32 r2) {
	DotSettings ds;
	if (r2 < r1)
		r2 = r1;
	if (0 == r1)
		r1 = r2 = 1;
	ds.r1 = r1;
	ds.r2 = r2;
	ds.c = c;
	line_pattern(x1, y1, x2, y2, dotted_line_pattern, (void *)&ds);
}

#define	NO_FILL		0
#define	TO_VISIT	1
#define	TO_FILL		2

// TODO: this floodfill method saves stack, but is pretty slow
static bool __visit_stopclr(SBitmap* bmp, unsigned char* bits, RGBColor stop_clr)
{
	const u32 xs = bmp->width();
	const u32 ys = bmp->height();
	const int dd[4][2] = { { 0, -1 }, { -1, 0 }, { 1, 0 }, {0, 1 } };
	int x, y;
	bool ret;

	ret = false;
	for (y = 0; y < ys; ++y)
		for (x = 0; x < xs; ++x) {
			if (bits[ARRAY_2D_INDEX(x, y, xs)] == TO_VISIT) {
				bits[ARRAY_2D_INDEX(x, y, xs)] = TO_FILL;
				for (int e = 0; e < 4; ++e) {
					int xx, yy;
					xx = x + dd[e][0];
					yy = y + dd[e][1];
					if (xx < 0 || xx >= xs)
						continue;
					if (yy < 0 || yy >= ys)
						continue;
					if (bits[ARRAY_2D_INDEX(xx, yy, xs)] != NO_FILL)
						continue;
					if (bmp->get_pixel(xx, yy) == stop_clr)
						continue;
					bits[ARRAY_2D_INDEX(xx, yy, xs)] = TO_VISIT;
				}
				ret = true;
			}
		}

	return (ret);
}

// save a buffer for flood-filling
static unsigned char* __ffbits = NULL;

void SBitmap::floodfill_stopclr(int x, int y, RGBColor fill_clr, RGBColor stop_clr) {
	unsigned char* bits;
	int xx, yy;

	if (is_null(__ffbits) && x * y <= 1024 * 1024)
		__ffbits = NEW_ARRAY_2D(unsigned char, 1024, 1024);

	if (not_null(__ffbits) && x * y <= 1024 * 1024)
		bits = __ffbits;
	else
		bits = NEW_ARRAY_2D(unsigned char, width(), height());
	zeromem(bits, width() * height() * sizeof(unsigned char));

	bits[ARRAY_2D_INDEX(x, y, width())] = TO_VISIT;

	while (__visit_stopclr(this, bits, stop_clr))
		no_op;

	for (yy = 0; yy < height(); ++yy)
		for (xx = 0; xx < width(); ++xx) {
			if (bits[ARRAY_2D_INDEX(xx, yy, width())] == TO_FILL) {
				put_pixel(xx, yy, fill_clr);
			}
		}

	if (bits != __ffbits)
		delete [] bits;
}

static bool __visit_contclr(SBitmap* bmp, unsigned char* bits, RGBColor cont_clr)
{
	const u32 xs = bmp->width();
	const u32 ys = bmp->height();
	const int dd[4][2] = { { 0, -1 }, { -1, 0 }, { 1, 0 }, {0, 1 } };
	int x, y;
	bool ret;

	ret = false;
	for (y = 0; y < ys; ++y)
		for (x = 0; x < xs; ++x) {
			if (bits[ARRAY_2D_INDEX(x, y, xs)] == TO_VISIT) {
				bits[ARRAY_2D_INDEX(x, y, xs)] = TO_FILL;
				for (int e = 0; e < 4; ++e) {
					int xx, yy;
					xx = x + dd[e][0];
					yy = y + dd[e][1];
					if (xx < 0 || xx >= xs)
						continue;
					if (yy < 0 || yy >= ys)
						continue;
					if (bits[ARRAY_2D_INDEX(xx, yy, xs)] != NO_FILL)
						continue;
					if (bmp->get_pixel(xx, yy) != cont_clr)
						continue;
					bits[ARRAY_2D_INDEX(xx, yy, xs)] = TO_VISIT;
				}
				ret = true;
			}
		}

	return (ret);
}

void SBitmap::floodfill_contclr(int x, int y, RGBColor fill_clr, RGBColor cont_clr) {
	unsigned char* bits;
	int xx, yy;

	if (is_null(__ffbits) && x * y <= 1024 * 1024)
		__ffbits = NEW_ARRAY_2D(unsigned char, 1024, 1024);

	if (not_null(__ffbits) && x * y <= 1024 * 1024)
		bits = __ffbits;
	else
		bits = NEW_ARRAY_2D(unsigned char, w, h);
	zeromem(bits, width() * height() * sizeof(unsigned char));

	bits[ARRAY_2D_INDEX(x, y, width())] = TO_VISIT;

	while (__visit_contclr(this, bits, cont_clr))
		no_op;

	for (yy = 0; yy < height(); ++yy)
		for (xx = 0; xx < width(); ++xx) {
			if (bits[ARRAY_2D_INDEX(xx, yy, width())] == TO_FILL) {
				put_pixel(xx, yy, fill_clr);
			}
		}

	if (bits != __ffbits)
		delete [] bits;
}

/*** As floodfillbmp_contclr, but uses the color at position (x, y) as the continue-color ***/
void SBitmap::floodfill(int x, int y, RGBColor fill_clr) {
	RGBColor cont_clr;

	if (!pixel_ok(this, x, y))
		return;

	cont_clr = get_pixel(x, y);
	floodfill_contclr(x, y, fill_clr, cont_clr);
}


void SBitmap::patternfill(int x, int y, PatternCallback pattern_callback, void* args) {
	unsigned char* bits;
	int xx, yy;
	RGBColor cont_clr;

	cont_clr = get_pixel(x, y);

	if (is_null(__ffbits) && x * y <= 1024 * 1024)
		__ffbits = NEW_ARRAY_2D(unsigned char, 1024, 1024);

	if (not_null(__ffbits) && x * y <= 1024 * 1024)
		bits = __ffbits;
	else
		bits = NEW_ARRAY_2D(unsigned char, w, h);
	zeromem(bits, width() * height() * sizeof(unsigned char));

	bits[ARRAY_2D_INDEX(x, y, width())] = TO_VISIT;

	while (__visit_contclr(this, bits, cont_clr))
		no_op;

	for (yy = 0; yy < height(); ++yy)
		for (xx = 0; xx < width(); ++xx) {
			if (bits[ARRAY_2D_INDEX(xx, yy, width())] == TO_FILL) {
				put_pixel(xx, yy, pattern_callback(xx, yy, args));
			}
		}

	if (bits != __ffbits)
		delete [] bits;
}

void SBitmap::floodfill_stopclr(const SPoint& p, RGBColor fill_clr, RGBColor stop_clr) {
	floodfill_stopclr(p.X(this), p.Y(this), fill_clr, stop_clr);
}

void SBitmap::floodfill_contclr(const SPoint& p, RGBColor fill_clr, RGBColor cont_clr) {
	floodfill_contclr(p.X(this), p.Y(this), fill_clr, cont_clr);
}

void SBitmap::floodfill(const SPoint& p, RGBColor fill_clr) {
	floodfill(p.X(this), p.Y(this), fill_clr);
}

void SBitmap::patternfill(const SPoint& p, PatternCallback pattern_callback, void* args) {
	patternfill(p.X(this), p.Y(this), pattern_callback, args);
}

/*** Draw a polygon on the bitmap. Will close the polygon if not closed. ***/
void SBitmap::polygon(u32 npoints, int* xpoints, int* ypoints, RGBColor c) {
	u32 e;

	if (0 == npoints)
		return;

	put_pixel(xpoints[0], ypoints[0], c);

	for (e = 1; e < npoints; ++e)
		line(xpoints[e - 1], ypoints[e - 1], xpoints[e], ypoints[e], c);

	line(xpoints[e - 1], ypoints[e - 1], xpoints[0], ypoints[0], c);
}

void SBitmap::polygon(u32 npoints, SPoint* points, RGBColor c) {
	int xx[256], yy[256];
	int* px, * py;
	if (npoints > 256) {
		px = new int [npoints];
		py = new int [npoints];
	} else {
		px = xx;
		py = yy;
	}
	for (u32 e = 0; e < npoints; ++e) {
		px[e] = points[e].X(this);
		py[e] = points[e].Y(this);
	}
	polygon(npoints, px, py, c);
	if (px != xx) {
		delete [] px;
		delete [] py;
	}
}

/*** Fade out effect -- 0 leaves the bitmap unaffected, 255 will fade it to black. ***/
void SBitmap::fadeout(u32 fade) {
	int y, x;

	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			i32 r, g, b, a;
			RGBColor c;

			c = get_pixel(x, y);
			
			r = RGB_RED(c) - fade;
			g = RGB_GREEN(c) - fade;
			b = RGB_BLUE(c) - fade;
			a = RGB_ALPHA(c);

			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			put_pixel(x, y, MAKE_RGBA(r, g, b, a));
		 }
}

/*** Draw an ellipse on the bitmap, by Bresenham's algorithm. ***/
void SBitmap::ellipse(int x_center, int y_center, int width_e, int height_e, RGBColor c) {
	int a2 = width_e * width_e;
	int b2 = height_e * height_e;
	int fa2 = 4 * a2, fb2 = 4 * b2;
	int x, y, sigma;

	/* first half */
	for (x = 0, y = height_e, sigma = 2 * b2 + a2 * (1 - 2 * height_e); b2 * x <= a2 * y; x++) {
		put_pixel(x_center + x, y_center + y, c);
		put_pixel(x_center - x, y_center + y, c);
		put_pixel(x_center + x, y_center - y, c);
		put_pixel(x_center - x, y_center - y, c);
		if (sigma >= 0) {
			sigma += fa2 * (1 - y);
			y--;
		}
		sigma += b2 * ((4 * x) + 6);
    	}

	/* second half */
	for (x = width_e, y = 0, sigma = 2 * a2 + b2 *(1 - 2 * width_e); a2 * y <= b2 * x; y++) {
		put_pixel(x_center + x, y_center + y, c);
		put_pixel(x_center - x, y_center + y, c);
		put_pixel(x_center + x, y_center - y, c);
		put_pixel(x_center - x, y_center - y, c);
		if (sigma >= 0) {
			sigma += fb2 * (1 - x);
			x--;
        	}
        	sigma += a2 * ((4 * y) + 6);
    	}
}

#define	ELLIPSE_SL(x, y)	do { \
	if ((x) < p1[(y) - (y_center - height_e)]) p1[(y) - (y_center - height_e)] = (x); \
	if ((x) > p2[(y) - (y_center - height_e)]) p2[(y) - (y_center - height_e)] = (x); \
		} while (0)

/*** Draw a filled ellipse on the bitmap, by Bresenham's ***/
void SBitmap::fillellipse(int x_center, int y_center, int width_e, int height_e, RGBColor c) {
	int a2 = width_e * width_e;
	int b2 = height_e * height_e;
	int fa2 = 4 * a2, fb2 = 4 * b2;
	int x, y, sigma;
	int* p1, * p2; 
	int e;

	/* Prepare scanline arrays */
	if (height_e + height_e + 1 < 1024) {
		p1 = __scanline1;
		p2 = __scanline2;
	} else {
		p1 = NEW_ARRAY(int, height_e + height_e + 1);
		p2 = NEW_ARRAY(int, height_e + height_e + 1);
	}

	for (e = 0; e <= height_e + height_e; ++e) {
		p1[e] = 0x7FFFFFFFL;
		p2[e] = -1;
	}

	for (x = 0, y = height_e, sigma = 2 * b2 + a2 * (1 - 2 * height_e); b2 * x <= a2 * y; x++) {
		ELLIPSE_SL(x_center + x, y_center + y);
		ELLIPSE_SL(x_center - x, y_center + y);
		ELLIPSE_SL(x_center + x, y_center - y);
		ELLIPSE_SL(x_center - x, y_center - y);
		if (sigma >= 0) {
			sigma += fa2 * (1 - y);
			y--;
        	}
		sigma += b2 * ((4 * x) + 6);
    	}

	for (x = width_e, y = 0, sigma = 2 * a2 + b2 *(1 - 2 * width_e); a2 * y <= b2 * x; y++) {
    		ELLIPSE_SL(x_center + x, y_center + y);
		ELLIPSE_SL(x_center - x, y_center + y);
		ELLIPSE_SL(x_center + x, y_center - y);
		ELLIPSE_SL(x_center - x, y_center - y);
		if (sigma >= 0) {
			sigma += fb2 * (1 - x);
			x--;
        	}
		sigma += a2 * ((4 * y) + 6);
    	}

	for (e = 0; e <= height_e + height_e; ++e)
		hline(p1[e], p2[e], y_center - height_e + e, c);

	if (p1 != __scanline1) {
		delete [] p1;
		delete [] p2;
	}
}

/*** Draw a pattern-filled ellipse on the bitmap. ***/
void SBitmap::fillellipse_pattern(int x_center, int y_center, int width_e, int height_e, PatternCallback pattern_callback, void* args) {
	int a2 = width_e * width_e;
	int b2 = height_e * height_e;
	int fa2 = 4 * a2, fb2 = 4 * b2;
	int x, y, sigma;
	int* p1, * p2; 
	int e;

	/* Prepare scanline arrays */
	if (height_e + height_e + 1 < 1024) {
		p1 = __scanline1;
		p2 = __scanline2;
	} else {
		p1 = NEW_ARRAY(int, height_e + height_e + 1);
		p2 = NEW_ARRAY(int, height_e + height_e + 1);
	}

	for (e = 0; e <= height_e + height_e; ++e) {
		p1[e] = 0x7FFFFFFFL;
		p2[e] = -1;
	}

	for (x = 0, y = height_e, sigma = 2 * b2 + a2 * (1 - 2 * height_e); b2 * x <= a2 * y; x++) {
		ELLIPSE_SL(x_center + x, y_center + y);
		ELLIPSE_SL(x_center - x, y_center + y);
		ELLIPSE_SL(x_center + x, y_center - y);
		ELLIPSE_SL(x_center - x, y_center - y);
		if (sigma >= 0) {
			sigma += fa2 * (1 - y);
			y--;
        	}
		sigma += b2 * ((4 * x) + 6);
    	}

	for (x = width_e, y = 0, sigma = 2 * a2 + b2 *(1 - 2 * width_e); a2 * y <= b2 * x; y++) {
    		ELLIPSE_SL(x_center + x, y_center + y);
		ELLIPSE_SL(x_center - x, y_center + y);
		ELLIPSE_SL(x_center + x, y_center - y);
		ELLIPSE_SL(x_center - x, y_center - y);
		if (sigma >= 0) {
			sigma += fb2 * (1 - x);
			x--;
        	}
		sigma += a2 * ((4 * y) + 6);
    	}

	for (e = 0; e <= height_e + height_e; ++e) {
		if (!pixel_ok(this, x, y_center - height_e + e))
			continue;
		put_pixel(x, y_center - height_e + e, pattern_callback(x, y_center - height_e + e, args));
	}

	if (p1 != __scanline1) {
		delete [] p1;
		delete [] p2;
	}
}

/*** Performs a Gaussian blur (diameter 6) of the bitmap and returns the allocated result ***/
SBitmap* SBitmap::gaussianblur(void) {
	static const double gaussian_matrix[7][7] =
		{
			{0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067 },
			{0.00002292, 0.00078634, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292 },
			{0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117 },
			{0.00038771, 0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373, 0.00038771 },
			{0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117 },
			{0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292 },
			{0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067 },
		};
	SBitmap* bmpret = new SBitmap(w, h, BITMAP_DEFAULT);
	NOT_NULL_OR_RETURN(bmpret, NULL);
	int x, y, dx, dy;

	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			double r, g, b;
			int rr, gg, bb;
			r = g = b = 0.;
			for (dy = -3; dy <= 3; ++dy) {
				for (dx = -3; dx <= 3; ++dx) {
					RGBColor c;
					int px, py;
					px = x + dx;
					py = y + dy;
					px = CLAMP(px, 0, w - 1);
					py = CLAMP(py, 0, h - 1);
					c = get_pixel(px, py);
					r += (double)(RGB_RED(c)) * gaussian_matrix[dy + 3][dx + 3];
					g += (double)(RGB_GREEN(c)) * gaussian_matrix[dy + 3][dx + 3];
					b += (double)(RGB_BLUE(c)) * gaussian_matrix[dy + 3][dx + 3];
				}
			}
			rr = ROUND_FLOAT_TO_INT(r);
			gg = ROUND_FLOAT_TO_INT(g);
			bb = ROUND_FLOAT_TO_INT(b);
			rr = COMPONENT_RANGE(rr);
			gg = COMPONENT_RANGE(gg);
			bb = COMPONENT_RANGE(bb);
			put_pixel(x, y, RGB_NO_CHECK(rr, gg, bb));
		}
	}

	return(bmpret); 
}

/*** Basic 6 x 8 bitfont, from IndicatorIndicator/KrisKwant. ***/
#include "bitfont.i"

void SBitmap::putch_bitfont(int ch, int x, int y, int s, RGBColor c, int bg) {
#if 0
	int i;
	int e, f, g, j;
	const char others[] = {'.', '-', ',', '&', '!', '+', '@', '(', ')', '%', '\'', '/', '$', '*', '\"', '?', '`', '#', -1};

	i = -1;

	if (ch >= 'a' && ch <= 'z')
		ch -= ' ';

	if ((ch >= 'A' && ch <= 'Z')) {
		i = 10 + ch - 'A';
	} else if (ch >= '0' && ch <= '9') {
		i = ch - '0';
	} else {
		for (e=0; others[e] > 0; ++e)
			if (others[e] == ch)
				{
				i = 36 + e;
				break;
				}
	}

	if (i < 0)
		return;

	for (e=0; e<8; ++e) {
		for (f=0; f<6; ++f) {
			for (g=0; g<s; ++g) {
				for (j=0; j<s; ++j) {
					if (numberfont[i][e][f] == 1)
						put_pixel(x + f*s + j, y + e*s + g, c);
					else if (bg > -1)
						put_pixel(x + f*s + j, y + e*s + g, (RGBColor)bg);
				}
			}
		}
	}
#else
	static std::unordered_map<u32, u32> glyph_idx;
	static std::unordered_map<u32, bool> descenders;
	if (0 == glyph_idx.size()) {
		u32 e, idx = 0;
		const u32 others[] = {'.', '-', ',', '&', '!', '+', '@', '(', ')',
					'%', '\'', '/', '$', '*', '\"', '?', '`', '#', 
					'[', ']', '^', '{', '}', '\\', '_', '=', '|',
					'~', ':', ';', '<', '>', 0x01, 0x02, 0x03, 0x04,
					0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
					0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
					0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
					0x1d, 0x1e, 0x1f, 0 };
		for (e = '0'; e <= '9'; ++e)
			glyph_idx[e] = idx++;
		for (e = 'A'; e <= 'Z'; ++e)
			glyph_idx[e] = idx++;
		for (e = 'a'; e <= 'z'; ++e)
			glyph_idx[e] = idx++;
		for (e = 0; others[e]; ++e)
			glyph_idx[others[e]] = idx++;
		descenders['g'] = true;
		descenders['j'] = true;
		descenders['p'] = true;
		descenders['q'] = true;
		descenders['y'] = true;
	}

	if (glyph_idx.find((u32)ch) == glyph_idx.end())
		return;
	u32 idx = glyph_idx[(u32)ch];
	bool des = descenders[(u32)ch];
	if (des)
		y += 3 * s;

	int e, f, g, j;
	for (e=0; e<8; ++e) {
		for (f=0; f<6; ++f) {
			for (g=0; g<s; ++g) {
				for (j=0; j<s; ++j) {
					if (numberfont[idx][e][f])
						put_pixel(x + f*s + j, y + e*s + g, c);
					else if (bg > -1)
						put_pixel(x + f*s + j, y + e*s + g, (RGBColor)bg);
				}
			}
		}
	}
#endif
}

void SBitmap::putstr_bitfont(const char* str, int x, int y, int size, RGBColor c) {
	while (*str) {
		putch_bitfont(*str, x, y, size, c, -1);
		x += 9 * size;
		++str;
	}
}

void SBitmap::putstr_bitfont_bg(const char* str, int x, int y, int size, RGBColor text_c, RGBColor bg_c) {
	while (*str) {
		putch_bitfont(*str, x, y, size, text_c, (int)bg_c);
		x += 9 * size;
		++str;
	}
}

/*** Put a pixel with color c at (x, y), blending into the background color. An intensity of 0 means the background is preserved; an
	intensity of 255 means the background is replaced. ***/
void SBitmap::blendpixel(int x, int y, RGBColor c, u32 intensity) {
	if (!pixel_ok(this, x, y))
		return;
	RGBColor bc = get_pixel(x, y);
	u32 rr, gg, bb;

	intensity = CLAMP(intensity, 0, 255);

	rr = RGB_RED(c) * intensity + RGB_RED(bc) * (255 - intensity);
	gg = RGB_GREEN(c) * intensity + RGB_GREEN(bc) * (255 - intensity);
	bb = RGB_BLUE(c) * intensity + RGB_BLUE(bc) * (255 - intensity);
	rr /= 255;
	gg /= 255;
	bb /= 255;

	put_pixel(x, y, RGB_NO_CHECK(rr, gg, bb));
}

/*** Draw an ellipse on the bitmap defined by the bounding rectangle. Use ellipse() for an ellipse defined by its
	center and radii. ***/
void SBitmap::ellipserect(int x0, int y0, int x1, int y1, RGBColor c) {
	SORT2(x0, x1, int);
	SORT2(y0, y1, int);
	int a = x1 - x0;
	int b = y1 - y0;
	int b1 = b & 1;
	double dx = 4 * (1.0 - a) * b * b;
	double dy = 4 * (b1 + 1.0) * a  * a;
	double err = dx + dy + b1 * a * a;
	double e2;

	y0 += (b + 1) / 2;
	y1 = y0 - b1;
	a = 8*a*a;
	b1 = 8*b*b;

#define	DO_PIXEL(x, y)		put_pixel((x), (y), c)
	do {                                     
		DO_PIXEL(x1, y0);                                      /*   I. Quadrant */
		DO_PIXEL(x0, y0);                                      /*  II. Quadrant */
		DO_PIXEL(x0, y1);                                      /* III. Quadrant */
		DO_PIXEL(x1, y1);                                      /*  IV. Quadrant */
	  	e2 = 2*err;
	  	if (e2 <= dy) { y0++; y1--; err += dy += a; }                 /* y step */
	  	if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; }  /* x step */
	} while (x0 <= x1);

   while (y0-y1 <= b) {                /* too early stop of flat ellipses a=1 */
		DO_PIXEL(x0-1, y0);                         /* -> finish tip of ellipse */
		DO_PIXEL(x1+1, y0++);
		DO_PIXEL(x0-1, y1);
		DO_PIXEL(x1+1, y1--);
   }
#undef	DO_PIXEL
}

/*** Draw an anti-aliased line from (x0, y0) to (x1, y1) on the bitmap. ***/
void SBitmap::aaline(int x0, int y0, int x1, int y1, RGBColor c) {
	int sx = (x0 < x1 ? 1 : -1);
	int sy = (y0 < y1 ? 1 : -1);
	int x2;
	int dx = std::abs(x1-x0), dy = std::abs(y1-y0), err = dx*dx+dy*dy;
	int e2 = err == 0 ? 1 : 0xffff7fl/sqrt(err);     /* multiplication factor */

	dx *= e2;
	dy *= e2;
	err = dx-dy;                       /* error value e_xy */
	forever {                                                 /* pixel loop */
		blendpixel(x0,y0, c, 255 - (std::abs(err-dx+dy)>>16));
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {                                            /* x step */
			if (x0 == x1)
		 		break;
			if (e2+dy < 0xff0000l)
				blendpixel(x0,y0+sy, c, 255 - ((e2+dy)>>16));
			err -= dy; x0 += sx; 
		}
		if (2*e2 <= dy) {                                             /* y step */
			if (y0 == y1)
				break;
			if (dx-e2 < 0xff0000l)
				blendpixel(x2+sx,y0, c, 255 - ((dx-e2)>>16));
        		err += dx; y0 += sy; 
    		}
   	}
}


/*** Draw an anti-aliased circle with center at (xm, ym) and radius r ***/
void SBitmap::aacircle(int xm, int ym, int r, RGBColor c) {
	int x = -r, y = 0;           /* II. quadrant from bottom left to top right */
	int i, x2, e2, err = 2-2*r;                             /* error of 1.step */
	r = 1-err;
	do {
		i = 255*std::abs(err-2*(x+y)-2)/r;               /* get blend value of pixel */
		blendpixel(xm-x, ym+y, c, 255 - i);                             /*   I. Quadrant */
		blendpixel(xm-y, ym-x, c, 255 - i);                             /*  II. Quadrant */
		blendpixel(xm+x, ym-y, c, 255 - i);                             /* III. Quadrant */
		blendpixel(xm+y, ym+x, c, 255 - i);                             /*  IV. Quadrant */
		e2 = err; x2 = x;                                    /* remember values */
		if (err+y > 0) {                                              /* x step */
			i = 255*(err-2*x-1)/r;                              /* outward pixel */
			if (i < 256) {
				blendpixel(xm-x, ym+y+1, c, 255 - i);
				blendpixel(xm-y-1, ym-x, c, 255 - i);
				blendpixel(xm+x, ym-y-1, c, 255 - i);
				blendpixel(xm+y+1, ym+x, c, 255 - i);
			}
			err += ++x*2+1;
      		}
		if (e2+x2 <= 0) {                                             /* y step */
			i = 255*(2*y+3-e2)/r;                                /* inward pixel */
			if (i < 256) {
				blendpixel(xm-x2-1, ym+y, c, 255 - i);
				blendpixel(xm-y, ym-x2-1, c, 255 - i);
				blendpixel(xm+x2+1, ym-y, c, 255 - i);
				blendpixel(xm+y, ym+x2+1, c, 255 - i);
			}
			err += ++y*2+1;
		}
	} while (x < 0);
}

/*** Draw an anti-aliased line of a specified width wd from (x0, y0) to (x1, y1). ***/
void SBitmap::aathickline(int x0, int y0, int x1, int y1, float wd, RGBColor c) {
	/* plot an anti-aliased line of width wd */
	int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1; 
	int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1; 
	int err = dx-dy, e2, x2, y2;                           /* error value e_xy */
	float ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);
                                                       
	for (wd = (wd+1)/2; ; ) {                                    /* pixel loop */
		blendpixel(x0, y0, c, 255 - ROUND_FLOAT_TO_INT(std::max(0.,255.*(abs(err-dx+dy)/ed-wd+1))));
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {                                            /* x step */
			for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				blendpixel(x0, y2 += sy, c, 255 - ROUND_FLOAT_TO_INT(std::max(0.,255.*(abs(e2)/ed-wd+1))));
			if (x0 == x1) break;
			e2 = err; err -= dy; x0 += sx; 
		} 
		if (2*e2 <= dy) {                                             /* y step */
			for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				blendpixel(x2 += sx, y0, c, 255 - ROUND_FLOAT_TO_INT(std::max(0.,255.*(abs(e2)/ed-wd+1))));
			if (y0 == y1) break;
			err += dx; y0 += sy; 
		}
	}
}

void SBitmap::aaline(const SPoint& p1, const SPoint& p2, RGBColor c) {
	aaline(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), c);
}

void SBitmap::aacircle(const SPoint& p, int r, RGBColor c) {
	aacircle(p.X(this), p.Y(this), r, c);
}

void SBitmap::aathickline(const SPoint& p1, const SPoint& p2, float wd, RGBColor c) {
	aathickline(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), wd, c);
}

SPalette* SBitmap::palette(void) const {
	if (!is_null(pal))
		return pal;
	if (sbd != nullptr)
		return sbd->parent->palette();
	return nullptr;
}

void SBitmap::set_palette(SPalette* spal) {
	if (!is_null(pal))
		delete pal;
	pal = new SPalette(spal->ncolors);
	for (int i = 0; i < spal->ncolors; ++i)
		pal->clrs[i] = spal->clrs[i];
}

void SBitmap::put_pixel_palette(int x, int y, u32 idx) {
	if (nullptr == pal)
		return;
	if (!pixel_ok(this, x, y))
		return;
	if (idx >= palette()->ncolors)
		return;
	u32 offs = y * w + x;
	bits[offs] = (u8)idx;
}

int SBitmap::get_pixel_palette(int x, int y) const {
	if (nullptr == pal || !pixel_ok(this, x, y))
		return (-1);
	u32 offs = y * w + x;
	return (int)bits[offs];
}

/*** Swap the contents of two bitmaps. Will fix sbd parents if swapped. ***/
void SBitmap::swap(SBitmap* bmp) {
	u8* bits_this = bits;
	u32 w_this = w;
	u32 h_this = h;
	BitmapType btype_this = btype;
	SPalette* pal_this = pal;
	SubBitmapData* sbd_this = sbd;
	PutPixelFn put_pixel_fn_this = put_pixel_fn;
	GetPixelFn get_pixel_fn_this = get_pixel_fn;
	SBitmap* parent_this = nullptr,* parent_bmp = nullptr;
	if (!is_null(sbd_this))
		parent_this = sbd_this->parent;
	if (!is_null(bmp->sbd))
		parent_bmp = bmp->sbd->parent;

	bits = bmp->bits;
	bmp->bits = bits_this;
	w = bmp->w;
	bmp->w = w_this;
	h = bmp->h;
	bmp->h = h_this;
	btype = bmp->btype;
	bmp->btype = btype_this;
	pal = bmp->pal;
	bmp->pal = pal_this;
	sbd = bmp->sbd;
	bmp->sbd = sbd_this;
	put_pixel_fn = bmp->put_pixel_fn;
	bmp->put_pixel_fn = put_pixel_fn_this;
	get_pixel_fn = bmp->get_pixel_fn;
	bmp->get_pixel_fn = get_pixel_fn_this;

	if (parent_this == bmp)
		bmp->sbd->parent = this;
	if (parent_bmp == this)
		sbd->parent = bmp;
}

/*** Change the temperature of the bitmap. temp is added to RGB components, so should be in the range [-255, 255].  ***/
void SBitmap::temperature(int temp) {
	RGBColor c;
	i32 r, g, b;
	int y, x;

	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			c = get_pixel(x, y);
			r = RGB_RED(c) + temp;
			g = RGB_GREEN(c);
			b = RGB_BLUE(c) - temp;
			r = COMPONENT_RANGE(r);
			b = COMPONENT_RANGE(b);
			put_pixel(x, y, RGB_NO_CHECK(r, g, b));
		}
	
}

/*** Adjust the tint of the bitmap. tint is added to RGB components, so should be in the range [-255, 255].  ***/
void SBitmap::tint(int tint) {
	RGBColor c;
	i32 r, g, b;
	int y, x;

	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			c = get_pixel(x, y);
			r = RGB_RED(c);
			g = RGB_GREEN(c) + tint;
			b = RGB_BLUE(c);
			g = COMPONENT_RANGE(g);
			put_pixel(x, y, RGB_NO_CHECK(r, g, b));
		}
}

/* Helper function: is the passed-in point inside the polygon specified by the array of points? */
bool point_in_polygon(const SPoint& pt, SPoint* poly, u32 npoints, SBitmap* bmp) {
	bool x0;
	int xings;
	double p, tx, ty, y;
	u32 cp = 0;

	xings = 0;
	tx = pt.X(bmp);
	ty = pt.Y(bmp);
	y = poly[npoints - 1].Y(bmp);

	p = poly[cp].Y(bmp);
	if ((y >= ty) != (p >= ty)) {
		if ((x0 = (poly[npoints - 1].X(bmp) >= tx)) ==
			(poly[0].X(bmp) >= tx)) {
  			if (x0) {
				xings++;
			}
		} else {
			xings += (poly[npoints - 1].X(bmp) - (y - ty) *
				(poly[0].X(bmp) - poly[npoints - 1].X(bmp)) /
				(p - y)) >= tx;
		}
	}

	for (y = poly[cp].Y(bmp); cp < npoints; ++cp) {
		if (y >= ty) {
			++cp;
			while (cp < npoints && poly[cp].Y(bmp) >= ty)
				++cp;
			if (cp >= npoints)
				break;
			if ((x0 = (poly[cp - 1].X(bmp) >= tx)) == (poly[cp].X(bmp) >= tx)) {
				if (x0) {
					xings++;
				}
			} else {
				xings += (poly[cp - 1].X(bmp) - (poly[cp - 1].Y(bmp) - ty) *
					(poly[cp].X(bmp) - poly[cp - 1].X(bmp)) /
					(poly[cp].Y(bmp) - poly[cp - 1].Y(bmp))) >= tx;
			}
		} else {
			++cp;
			while (cp < npoints && poly[cp].Y(bmp) < ty)
				cp++;
			if (cp >= npoints)
				break;
			if ((x0 = (poly[cp - 1].X(bmp) >= tx)) == (poly[cp].X(bmp) >= tx)) {
				if (x0) {
					xings++;
				}
			} else {
				xings += (poly[cp - 1].X(bmp) - (poly[cp - 1].Y(bmp) - ty) *
					(poly[cp].X(bmp) - poly[cp - 1].X(bmp)) /
					(poly[cp].Y(bmp) - poly[cp - 1].Y(bmp))) >= tx;
			}
		}
	}

	return xings & 0x01;
}

void SBitmap::random_pixel(SPoint* p) const {
	NOT_NULL_OR_RETURN_VOID(p);
	p->xt = POINT_PIXEL;
	p->yt = POINT_PIXEL;
	p->x = RandU32Range(0, w - 1);
	p->y = RandU32Range(0, h - 1);
}

void SBitmap::random_rect(SCoord* co) const {
	NOT_NULL_OR_RETURN_VOID(co);
	SPoint p1, p2;
	random_pixel(&p1);
	random_pixel(&p2);
	*co = SCoord(p1, p2);
}

static void __fillpolyhelper(SBitmap* bmp, u32 npoints, SPoint* points, RGBColor c, PatternCallback pattern_callback, void* args, int x0, int x1, int y0, int y1) {
	i32 x, y;
	for (y = y0; y <= y1; ++y) {
		for (x = x0; x <= x1; ++x) {
			SPoint pt(x, y);
			if (!pixel_ok(bmp, x, y))
				continue;
			if (point_in_polygon(pt, points, (int)npoints)) {
				if (not_null(pattern_callback))
					bmp->put_pixel(x, y, pattern_callback(x, y, args));
				else
					bmp->put_pixel(x, y, c);
			}
		}
	}
}

static void __fillpolyhelper(SBitmap* bmp, u32 npoints, int* xpoints, int* ypoints, RGBColor c, PatternCallback pattern_callback, void* args) {
	SPoint poly_pts[256];
	SPoint *pts;
	u32 e;
	i32 x0, y0, x1, y1;

	if (0 == npoints)
		return;

	if (not_null(pattern_callback))
		bmp->put_pixel(xpoints[0], ypoints[0], pattern_callback(xpoints[0], ypoints[0], args));
	else
		bmp->put_pixel(xpoints[0], ypoints[0], c);

	if (npoints < 256)
		pts = poly_pts;
	else
		pts = new SPoint[npoints + 1];

	// set up points array and find the bounding rectangle
	x0 = x1 = xpoints[0];
	y0 = y1 = ypoints[0];
	for (e = 0; e < npoints; ++e) {
		pts[e].x = xpoints[e];
		pts[e].y = ypoints[e];
		pts[e].xt = POINT_PIXEL;
		pts[e].yt = POINT_PIXEL;
		if (xpoints[e] < x0)
			x0 = xpoints[e];
		if (xpoints[e] > x1)
			x1 = xpoints[e];
		if (ypoints[e] < y0)
			y0 = ypoints[e];
		if (ypoints[e] > y1)
			y1 = ypoints[e];
	}
	__fillpolyhelper(bmp, npoints, pts, c, pattern_callback, args, x0, x1, y0, y1);

	if (poly_pts != pts)
		delete [] pts;
}

/*** Draw a filled polygon on the bitmap. ***/
void SBitmap::fillpolygon(u32 npoints, int* xpoints, int* ypoints, RGBColor c) {
	__fillpolyhelper(this, npoints, xpoints, ypoints, c, nullptr, nullptr);
}

/*** Draw a pattern-filled polygon on the bitmap. ***/
void SBitmap::fillpolygon_pattern(u32 npoints, int* xpoints, int* ypoints, PatternCallback pattern_callback, void* args) {
	__fillpolyhelper(this, npoints, xpoints, ypoints, C_BLACK, pattern_callback, args);
}

void SBitmap::fillpolygon(u32 npoints, SPoint* points, RGBColor c) {
	int x0 = 999999999, x1 = -999999999, y0 = 999999999, y1 = -999999999;
	for (u32 e = 0; e < npoints; ++e) {
		x0 = std::min(x0, points[e].X(this));
		x1 = std::max(x1, points[e].X(this));
		y0 = std::min(y0, points[e].Y(this));
		y1 = std::max(y1, points[e].Y(this));
	}
	if (npoints > 0)
		__fillpolyhelper(this, npoints, points, c, nullptr, nullptr, x0, x1, y0, y1);
}

void SBitmap::fillpolygon_pattern(u32 npoints, SPoint* points, PatternCallback pattern_callback, void* args) {
	int x0 = 999999999, x1 = -999999999, y0 = 999999999, y1 = -999999999;
	for (u32 e = 0; e < npoints; ++e) {
		x0 = std::min(x0, points[e].X(this));
		x1 = std::max(x1, points[e].X(this));
		y0 = std::min(y0, points[e].Y(this));
		y1 = std::max(y1, points[e].Y(this));
	}
	if (npoints > 0)
		__fillpolyhelper(this, npoints, points, C_BLACK, pattern_callback, args, x0, x1, y0, y1);
}

void SBitmap::ellipse(const SPoint& p_center, int width_e, int height_e, RGBColor c) {
	ellipse(p_center.X(this), p_center.Y(this), width_e, height_e, c);
}

void SBitmap::ellipserect(const SPoint& p1, const SPoint& p2, RGBColor c) {
	SCoord co(p1, p2);
	ellipserect(co, c);
}

void SBitmap::ellipserect(const SCoord& co, RGBColor c) {
	ellipserect(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), c);
}

void SBitmap::rect_bitmap(SCoord* co) const {
	NOT_NULL_OR_RETURN_VOID(co);
	co->p1 = SPoint(0, 0);
	co->p2 = SPoint(w - 1, h - 1);
}

SBitmap* SBitmap::top_level(void) {
	if (is_null(sbd))
		return this;
	return sbd->parent->top_level();
}

SBitmap* SBitmap::top_level(const SPoint& p, SPoint* p_out) {
	if (is_null(sbd)) {
		if (!is_null(p_out)) {
			*p_out = SPoint(p.X(this), p.Y(this));
		}
		return this;
	}
	SBitmap* parent = sbd->parent;
	SPoint p_up(p.X(this) + sbd->co.X1(parent), p.Y(this) + sbd->co.Y1(parent));
	return parent->top_level(p_up, p_out);
}

SBitmap* SBitmap::top_level(const SCoord& co, SCoord* co_out) {
	SPoint p1, p2;
	SBitmap* ret = top_level(co.p1, &p1);
	top_level(co.p2, &p2);
	if (!is_null(co_out)) {
		*co_out = SCoord(p1, p2);
	}
	return ret;
}

SBitmap* SBitmap::subbmp_left_half(void) {
	SPoint p1(0, 0), p2(50, POINT_PERCENT, 100, POINT_PERCENT);
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_right_half(void) {
	SPoint p1(50, POINT_PERCENT, 0, POINT_PIXEL), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
	p1.x = p1.X(this) + 1;
	p1.xt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_top_half(void) {
	SPoint p1(0, 0), p2(100, POINT_PERCENT, 50, POINT_PERCENT);
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_bottom_half(void) {
	SPoint p1(0, POINT_PIXEL, 50, POINT_PERCENT), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
	p1.y = p1.Y(this) + 1;
	p1.yt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_ul_quarter(void) {
	SPoint p1(0, 0), p2(50, POINT_PERCENT, 50, POINT_PERCENT);
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_ur_quarter(void) {
	SPoint p1(50, POINT_PERCENT, 0, POINT_PIXEL), p2(100, POINT_PERCENT, 50, POINT_PERCENT);
	p1.x = p1.X(this) + 1;
	p1.xt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_ll_quarter(void) {
	SPoint p1(0, POINT_PIXEL, 50, POINT_PERCENT), p2(50, POINT_PERCENT, 100, POINT_PERCENT);
	p1.y = p1.Y(this) + 1;
	p1.yt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_lr_quarter(void) {
	SPoint p1(50, POINT_PERCENT, 50, POINT_PERCENT), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
	p1.x = p1.X(this) + 1;
	p1.xt = POINT_PIXEL;
	p1.y = p1.Y(this) + 1;
	p1.yt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_top_third(void) {
	SPoint p1(0, POINT_PIXEL, 0, POINT_PIXEL), p2(100, POINT_PERCENT, 333, POINT_MILLES);
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_middlev_third(void) {
	SPoint p1(0, POINT_PIXEL, 333, POINT_MILLES), p2(100, POINT_PERCENT, 666, POINT_MILLES);
	p1.y = p1.Y(this) + 1;
	p1.yt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_bottom_third(void) {
	SPoint p1(0, POINT_PIXEL, 666, POINT_MILLES), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
	p1.y = p1.Y(this) + 1;
	p1.yt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_left_third(void) {
	SPoint p1(0, 0), p2(333, POINT_MILLES, 100, POINT_PERCENT);
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_middleh_third(void) {
	SPoint p1(333, POINT_MILLES, 0, POINT_PIXEL), p2(666, POINT_MILLES, 100, POINT_PERCENT);
	p1.x = p1.X(this) + 1;
	p1.xt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_right_third(void) {
	SPoint p1(666, POINT_MILLES, 0, POINT_PIXEL), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
	p1.x = p1.X(this) + 1;
	p1.xt = POINT_PIXEL;
	return subbitmap(p1, p2);
}

SBitmap* SBitmap::subbmp_complement(SBitmap* bmp) {
	if (is_null(bmp) || is_null(bmp->sbd))
		return (nullptr);
	SCoord co;
	SBitmap* parent = bmp->top_level(bmp->sbd->co, &co);
	// Horizontal strip?
	if (co.X1(parent) == 0 && co.X2(parent) == parent->w - 1) {
		// At top?
		if (co.Y1(parent) == 0) {
			SPoint p1(0, co.Y2(parent) + 1), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
			return parent->subbitmap(p1, p2);
		}
		// At bottom?
		if (co.Y2(parent) == parent->h - 1) {
			SPoint p1(0, 0), p2(100, POINT_PERCENT, co.Y1(parent) - 1, POINT_PIXEL);
			return parent->subbitmap(p1, p2);
		}
	}
	// Vertical strip?
	if (co.Y1(parent) == 0 && co.Y2(parent) == parent->h - 1) {
		// At right?
		if (co.X2(parent) == parent->w - 1) {
			SPoint p1(0, 0), p2(co.X1(parent) - 1, POINT_PIXEL, 100, POINT_PERCENT);
			return parent->subbitmap(p1, p2);
		}
		// At left?
		if (co.X1(parent) == 0) {
			SPoint p1(co.X2(parent) + 1, POINT_PIXEL, 0, POINT_PIXEL), p2(100, POINT_PERCENT, 100, POINT_PERCENT);
			return parent->subbitmap(p1, p2);
		}
	}
	// The complement won't be rectangular, return nullptr.
	return nullptr;
}

static void __quad_bezier_segment(SBitmap* bmp, int x0, int y0, int x1, int y1, int x2, int y2, RGBColor c) {
                                  		/* plot a limited quadratic Bezier segment */
  int sx = x2-x1, sy = y2-y1;
  long xx = x0-x1, yy = y0-y1, xy;              /* relative values for checks */
  double dx, dy, err, cur = xx*sy-yy*sx;                         /* curvature */

  assert(xx*sx <= 0 && yy*sy <= 0);       /* sign of gradient must not change */

  if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy)      /* begin with longer part */
  	{
    x2 = x0;
	x0 = sx+x1;
	y2 = y0;
	y0 = sy+y1;
	cur = -cur;       /* swap P0 P2 */
  	}
  if (cur != 0)
  	{                                         /* no straight line */
    xx += sx; xx *= sx = x0 < x2 ? 1 : -1;                /* x step direction */
    yy += sy; yy *= sy = y0 < y2 ? 1 : -1;                /* y step direction */
    xy = 2*xx*yy; xx *= xx; yy *= yy;               /* differences 2nd degree */
    if (cur*sx*sy < 0)
		{                                /* negated curvature? */
      	xx = -xx;
		yy = -yy;
		xy = -xy;
		cur = -cur;
    	}
    dx = 4.0*sy*cur*(x1-x0)+xx-xy;                  /* differences 1st degree */
    dy = 4.0*sx*cur*(y0-y1)+yy-xy;
    xx += xx;
	yy += yy;
	err = dx+dy+xy;                     /* error 1st step */
    do
		{
      	bmp->put_pixel(x0, y0, c);                                          /* plot curve */
      	if (x0 == x2 && y0 == y2)
			return;       /* last pixel -> curve finished */
      	y1 = 2*err < dx;                       /* save value for test of y step */
      	if (2*err > dy)
			{
			x0 += sx;
			dx -= xy;
			err += dy += yy;
			}      /* x step */
     	if (y1)
			{
			y0 += sy;
			dy -= xy;
			err += dx += xx;
			}      /* y step */
    	} while (dy < 0 && dx > 0);        /* gradient negates -> algorithm fails */
  	}
  bmp->line(x0, y0, x2, y2, c);                       /* plot remaining part to end */
}

/*** Plot a quadratic Bezier curve through the specified points. ***/
void SBitmap::quadbezier(int x0, int y0, int x1, int y1, int x2, int y2, RGBColor c) {
                                          /* plot any quadratic Bezier curve */
   int x = x0-x1, y = y0-y1;
   double t = x0-2*x1+x2, r;

   if ((long)x*(x2-x1) > 0)
   	{                        /* horizontal cut at P4? */
      if ((long)y*(y2-y1) > 0)                     /* vertical cut at P6 too? */
         if (fabs((y0-2*y1+y2)/t*x) > abs(y)) {               /* which first? */
            x0 = x2; x2 = x+x1; y0 = y2; y2 = y+y1;            /* swap points */
         }                            /* now horizontal cut at P4 comes first */
      t = (x0-x1)/t;
      r = (1-t)*((1-t)*y0+2.0*t*y1)+t*t*y2;                       /* By(t=P4) */
      t = (x0*x2-x1*x1)*t/(x0-x1);                       /* gradient dP4/dx=0 */
      x = floor(t+0.5); y = floor(r+0.5);            
      r = (y1-y0)*(t-x0)/(x1-x0)+y0;                  /* intersect P3 | P0 P1 */
      __quad_bezier_segment(this, x0, y0, x, floor(r+0.5), x, y, c);
      r = (y1-y2)*(t-x2)/(x1-x2)+y2;                  /* intersect P4 | P1 P2 */
      x0 = x1 = x; y0 = y; y1 = floor(r+0.5);             /* P0 = P4, P1 = P8 */
   	}                                                 
   if ((long)(y0-y1)*(y2-y1) > 0)
   	{                    /* vertical cut at P6? */
      t = y0-2*y1+y2; t = (y0-y1)/t;                 
      r = (1-t)*((1-t)*x0+2.0*t*x1)+t*t*x2;                       /* Bx(t=P6) */
      t = (y0*y2-y1*y1)*t/(y0-y1);                       /* gradient dP6/dy=0 */
      x = floor(r+0.5); y = floor(t+0.5);            
      r = (x1-x0)*(t-y0)/(y1-y0)+x0;                  /* intersect P6 | P0 P1 */
      __quad_bezier_segment(this, x0, y0, floor(r+0.5), y, x, y, c);
      r = (x1-x2)*(t-y2)/(y1-y2)+x2;                  /* intersect P7 | P1 P2 */
      x0 = x; x1 = floor(r+0.5); y0 = y1 = y;             /* P0 = P6, P1 = P7 */
   	}
   __quad_bezier_segment(this, x0, y0, x1, y1, x2, y2, c);                  /* remaining part */
}

static void __quad_bezier_segment_rational(SBitmap* bmp, int x0, int y0, int x1, int y1, int x2, int y2, float w, RGBColor c) {
                   /* plot a limited rational Bezier segment, squared weight */
  int sx = x2-x1, sy = y2-y1;                   /* relative values for checks */
  double dx = x0-x2, dy = y0-y2, xx = x0-x1, yy = y0-y1;
  double xy = xx*sy+yy*sx, cur = xx*sy-yy*sx, err;               /* curvature */

  assert(xx*sx <= 0.0 && yy*sy <= 0.0);   /* sign of gradient must not change */

  if (cur != 0.0 && w > 0.0) {                            /* no straight line */
    if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy) {    /* begin with longer part */
      x2 = x0; x0 -= dx; y2 = y0; y0 -= dy; cur = -cur;         /* swap P0 P2 */
    }
    xx = 2.0*(4.0*w*sx*xx+dx*dx);                   /* differences 2nd degree */
    yy = 2.0*(4.0*w*sy*yy+dy*dy);
    sx = x0 < x2 ? 1 : -1;                                /* x step direction */
    sy = y0 < y2 ? 1 : -1;                                /* y step direction */
    xy = -2.0*sx*sy*(2.0*w*xy+dx*dy);

    if (cur*sx*sy < 0.0) {                              /* negated curvature? */
      xx = -xx; yy = -yy; xy = -xy; cur = -cur;
    }
    dx = 4.0*w*(x1-x0)*sy*cur+xx/2.0+xy;            /* differences 1st degree */
    dy = 4.0*w*(y0-y1)*sx*cur+yy/2.0+xy;

    if (w < 0.5 && (dy > xy || dx < xy)) {   /* flat ellipse, algorithm fails */
       cur = (w+1.0)/2.0; w = sqrt(w); xy = 1.0/(w+1.0);
       sx = floor((x0+2.0*w*x1+x2)*xy/2.0+0.5);    /* subdivide curve in half */
       sy = floor((y0+2.0*w*y1+y2)*xy/2.0+0.5);
       dx = floor((w*x1+x0)*xy+0.5); dy = floor((y1*w+y0)*xy+0.5);
       __quad_bezier_segment_rational(bmp, x0,y0, dx,dy, sx,sy, cur, c);/* plot separately */
       dx = floor((w*x1+x2)*xy+0.5); dy = floor((y1*w+y2)*xy+0.5);
       __quad_bezier_segment_rational(bmp, sx,sy, dx,dy, x2,y2, cur, c);
       return;
    }
    err = dx+dy-xy;                                           /* error 1.step */
    do {
      bmp->put_pixel(x0, y0, c);                                          /* plot curve */
      if (x0 == x2 && y0 == y2) return;       /* last pixel -> curve finished */
      x1 = 2*err > dy; y1 = 2*(err+yy) < -dy;/* save value for test of x step */
      if (2*err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }/* y step */
      if (2*err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }/* x step */
    } while (dy <= xy && dx >= xy);    /* gradient negates -> algorithm fails */
  }
  bmp->line(x0, y0, x2, y2, c);                     /* plot remaining needle to end */
}

static void __quad_bezier_rational(SBitmap* bmp, int x0, int y0, int x1, int y1, int x2, int y2, float w, RGBColor c) {
                                 /* plot any quadratic rational Bezier curve */
   int x = x0-2*x1+x2, y = y0-2*y1+y2;
   double xx = x0-x1, yy = y0-y1, ww, t, q;

   assert(w >= 0.0);

   if (xx*(x2-x1) > 0) {                             /* horizontal cut at P4? */
      if (yy*(y2-y1) > 0)                          /* vertical cut at P6 too? */
         if (fabs(xx*y) > fabs(yy*x)) {                       /* which first? */
            x0 = x2; x2 = xx+x1; y0 = y2; y2 = yy+y1;          /* swap points */
         }                            /* now horizontal cut at P4 comes first */
      if (x0 == x2 || w == 1.0) t = (x0-x1)/(double)x;
      else {                                 /* non-rational or rational case */
         q = sqrt(4.0*w*w*(x0-x1)*(x2-x1)+(x2-x0)*(long)(x2-x0));
         if (x1 < x0) q = -q;
         t = (2.0*w*(x0-x1)-x0+x2+q)/(2.0*(1.0-w)*(x2-x0));        /* t at P4 */
      }
      q = 1.0/(2.0*t*(1.0-t)*(w-1.0)+1.0);                 /* sub-divide at t */
      xx = (t*t*(x0-2.0*w*x1+x2)+2.0*t*(w*x1-x0)+x0)*q;               /* = P4 */
      yy = (t*t*(y0-2.0*w*y1+y2)+2.0*t*(w*y1-y0)+y0)*q;
      ww = t*(w-1.0)+1.0; ww *= ww*q;                    /* squared weight P3 */
      w = ((1.0-t)*(w-1.0)+1.0)*sqrt(q);                         /* weight P8 */
      x = floor(xx+0.5); y = floor(yy+0.5);                             /* P4 */
      yy = (xx-x0)*(y1-y0)/(x1-x0)+y0;                /* intersect P3 | P0 P1 */
      __quad_bezier_segment_rational(bmp, x0,y0, x,floor(yy+0.5), x,y, ww, c);
      yy = (xx-x2)*(y1-y2)/(x1-x2)+y2;                /* intersect P4 | P1 P2 */
      y1 = floor(yy+0.5); x0 = x1 = x; y0 = y;            /* P0 = P4, P1 = P8 */
   }
   if ((y0-y1)*(long)(y2-y1) > 0) {                    /* vertical cut at P6? */
      if (y0 == y2 || w == 1.0) t = (y0-y1)/(y0-2.0*y1+y2);
      else {                                 /* non-rational or rational case */
         q = sqrt(4.0*w*w*(y0-y1)*(y2-y1)+(y2-y0)*(long)(y2-y0));
         if (y1 < y0) q = -q;
         t = (2.0*w*(y0-y1)-y0+y2+q)/(2.0*(1.0-w)*(y2-y0));        /* t at P6 */
      }
      q = 1.0/(2.0*t*(1.0-t)*(w-1.0)+1.0);                 /* sub-divide at t */
      xx = (t*t*(x0-2.0*w*x1+x2)+2.0*t*(w*x1-x0)+x0)*q;               /* = P6 */
      yy = (t*t*(y0-2.0*w*y1+y2)+2.0*t*(w*y1-y0)+y0)*q;
      ww = t*(w-1.0)+1.0; ww *= ww*q;                    /* squared weight P5 */
      w = ((1.0-t)*(w-1.0)+1.0)*sqrt(q);                         /* weight P7 */
      x = floor(xx+0.5); y = floor(yy+0.5);                             /* P6 */
      xx = (x1-x0)*(yy-y0)/(y1-y0)+x0;                /* intersect P6 | P0 P1 */
      __quad_bezier_segment_rational(bmp, x0,y0, floor(xx+0.5),y, x,y, ww, c);
      xx = (x1-x2)*(yy-y2)/(y1-y2)+x2;                /* intersect P7 | P1 P2 */
      x1 = floor(xx+0.5); x0 = x; y0 = y1 = y;            /* P0 = P6, P1 = P7 */
   }
   __quad_bezier_segment_rational(bmp, x0,y0, x1,y1, x2,y2, w*w, c);          /* remaining */
}

/*** Plot an ellipse bounded by the specified rectangle, and rotated by integer angle zd ***/
void SBitmap::rotatedellipserect(int x0, int y0, int x1, int y1, long zd, RGBColor c) {
                  /* rectangle enclosing the ellipse, integer rotation angle */
   int xd = x1-x0, yd = y1-y0;
   float w = xd*(long)yd;
   if (zd == 0) {
   		ellipserect(x0, y0, x1, y1,c);          /* looks nicer */
		return;
   }
   if (w != 0.0)
   	w = (w-zd)/(w+w);                    /* squared weight of P1 */
   assert(w <= 1.0 && w >= 0.0);                /* limit angle to |zd|<=xd*yd */
   xd = floor(xd*w+0.5); yd = floor(yd*w+0.5);           /* snap xe,ye to int */
   __quad_bezier_segment_rational(this, x0, y0+yd, x0, y0, x0+xd, y0, 1.0-w, c);
   __quad_bezier_segment_rational(this, x0, y0+yd, x0, y1, x1-xd, y1, w, c);
   __quad_bezier_segment_rational(this, x1, y1-yd, x1, y1, x1-xd, y1, 1.0-w, c);
   __quad_bezier_segment_rational(this, x1, y1-yd, x1, y0, x0+xd, y0, w, c);
}

/*** Plot an ellipse with center at (x, y), radii a and b, rotated by "angle" radians. ***/
void SBitmap::rotatedellipse(int x, int y, int a, int b, float angle, RGBColor c) {
                                   /* plot ellipse rotated by angle (radian) */
   float xd = (long)a*a, yd = (long)b*b;
   float s = sin(angle), zd = (xd-yd)*s;                  /* ellipse rotation */
   xd = sqrt(xd-zd*s), yd = sqrt(yd+zd*s);           /* surrounding rectangle */
   a = xd+0.5; b = yd+0.5; zd = zd*a*b/(xd*yd);           /* scale to integer */
   rotatedellipserect(x-a, y-b, x+a, y+b, (long)(4*zd*cos(angle)), c);
}

static void __cubic_bezier_segment(SBitmap* bmp, int x0, int y0, float x1, float y1,
	float x2, float y2, int x3, int y3, RGBColor c) {
                                        /* plot limited cubic Bezier segment */
   int f, fx, fy, leg = 1;
   int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;        /* step direction */
   float xc = -fabs(x0+x1-x2-x3), xa = xc-4*sx*(x1-x2), xb = sx*(x0-x1-x2+x3);
   float yc = -fabs(y0+y1-y2-y3), ya = yc-4*sy*(y1-y2), yb = sy*(y0-y1-y2+y3);
   double ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01;
                                                 /* check for curve restrains */
   /* slope P0-P1 == P2-P3    and  (P0-P3 == P1-P2      or   no slope change) */
   assert((x1-x0)*(x2-x3) < EP && ((x3-x0)*(x1-x2) < EP || xb*xb < xa*xc+EP));
   assert((y1-y0)*(y2-y3) < EP && ((y3-y0)*(y1-y2) < EP || yb*yb < ya*yc+EP));

   if (xa == 0 && ya == 0) {                              /* quadratic Bezier */
      sx = floor((3*x1-x0+1)/2); sy = floor((3*y1-y0+1)/2);   /* new midpoint */
      __quad_bezier_segment(bmp,x0,y0, sx,sy, x3,y3, c);
	  return;
   }
   x1 = (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+1;                    /* line lengths */
   x2 = (x2-x3)*(x2-x3)+(y2-y3)*(y2-y3)+1;
   do {                                                /* loop over both ends */
      ab = xa*yb-xb*ya; ac = xa*yc-xc*ya; bc = xb*yc-xc*yb;
      ex = ab*(ab+ac-3*bc)+ac*ac;       /* P0 part of self-intersection loop? */
      f = ex > 0 ? 1 : sqrt(1+1024/x1);               /* calculate resolution */
      ab *= f; ac *= f; bc *= f; ex *= f*f;            /* increase resolution */
      xy = 9*(ab+ac+bc)/8; cb = 8*(xa-ya);  /* init differences of 1st degree */
      dx = 27*(8*ab*(yb*yb-ya*yc)+ex*(ya+2*yb+yc))/64-ya*ya*(xy-ya);
      dy = 27*(8*ab*(xb*xb-xa*xc)-ex*(xa+2*xb+xc))/64-xa*xa*(xy+xa);
                                            /* init differences of 2nd degree */
      xx = 3*(3*ab*(3*yb*yb-ya*ya-2*ya*yc)-ya*(3*ac*(ya+yb)+ya*cb))/4;
      yy = 3*(3*ab*(3*xb*xb-xa*xa-2*xa*xc)-xa*(3*ac*(xa+xb)+xa*cb))/4;
      xy = xa*ya*(6*ab+6*ac-3*bc+cb); ac = ya*ya; cb = xa*xa;
      xy = 3*(xy+9*f*(cb*yb*yc-xb*xc*ac)-18*xb*yb*ab)/8;

      if (ex < 0) {         /* negate values if inside self-intersection loop */
         dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; cb = -cb;
      }                                     /* init differences of 3rd degree */
      ab = 6*ya*ac; ac = -6*xa*ac; bc = 6*ya*cb; cb = -6*xa*cb;
      dx += xy; ex = dx+dy; dy += xy;                    /* error of 1st step */

      for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3; ) {
         bmp->put_pixel(x0, y0, c);                                       /* plot curve */
         do {                                  /* move sub-steps of one pixel */
            if (dx > *pxy || dy < *pxy) goto exit;       /* confusing values */
            y1 = 2*ex-dy;                    /* save value for test of y step */
            if (2*ex >= dx) {                                   /* x sub-step */
               fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab;
            }
            if (y1 <= 0) {                                      /* y sub-step */
               fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += cb;
            }
         } while (fx > 0 && fy > 0);                       /* pixel complete? */
         if (2*fx <= f) { x0 += sx; fx += f; }                      /* x step */
         if (2*fy <= f) { y0 += sy; fy += f; }                      /* y step */
         if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;  /* pixel ahead valid */
      }
exit: xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;             /* swap legs */
      yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
   } while (leg--);                                          /* try other end */
   bmp->line(x0, y0, x3, y3, c);       /* remaining part in case of cusp or crunode */
}

/*** Plot a cubic Bezier curve through the specified points. ***/
void SBitmap::cubicbezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, RGBColor c) {
                                              /* plot any cubic Bezier curve */
   int n = 0, i = 0;
   long xc = x0+x1-x2-x3, xa = xc-4*(x1-x2);
   long xb = x0-x1-x2+x3, xd = xb+4*(x1+x2);
   long yc = y0+y1-y2-y3, ya = yc-4*(y1-y2);
   long yb = y0-y1-y2+y3, yd = yb+4*(y1+y2);
   float fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
   double t1 = xb*xb-xa*xc, t2, t[5];
                                 /* sub-divide curve at gradient sign changes */
   if (xa == 0) {                                               /* horizontal */
      if (abs(xc) < 2*abs(xb)) t[n++] = xc/(2.0*xb);            /* one change */
   } else if (t1 > 0.0) {                                      /* two changes */
      t2 = sqrt(t1);
      t1 = (xb-t2)/xa; if (fabs(t1) < 1.0) t[n++] = t1;
      t1 = (xb+t2)/xa; if (fabs(t1) < 1.0) t[n++] = t1;
   }
   t1 = yb*yb-ya*yc;
   if (ya == 0) {                                                 /* vertical */
      if (abs(yc) < 2*abs(yb)) t[n++] = yc/(2.0*yb);            /* one change */
   } else if (t1 > 0.0) {                                      /* two changes */
      t2 = sqrt(t1);
      t1 = (yb-t2)/ya; if (fabs(t1) < 1.0) t[n++] = t1;
      t1 = (yb+t2)/ya; if (fabs(t1) < 1.0) t[n++] = t1;
   }
   for (i = 1; i < n; i++)                         /* bubble sort of 4 points */
      if ((t1 = t[i-1]) > t[i]) { t[i-1] = t[i]; t[i] = t1; i = 0; }

   t1 = -1.0; t[n] = 1.0;                                /* begin / end point */
   for (i = 0; i <= n; i++) {                 /* plot each segment separately */
      t2 = t[i];                                /* sub-divide at t[i-1], t[i] */
      fx1 = (t1*(t1*xb-2*xc)-t2*(t1*(t1*xa-2*xb)+xc)+xd)/8-fx0;
      fy1 = (t1*(t1*yb-2*yc)-t2*(t1*(t1*ya-2*yb)+yc)+yd)/8-fy0;
      fx2 = (t2*(t2*xb-2*xc)-t1*(t2*(t2*xa-2*xb)+xc)+xd)/8-fx0;
      fy2 = (t2*(t2*yb-2*yc)-t1*(t2*(t2*ya-2*yb)+yc)+yd)/8-fy0;
      fx0 -= fx3 = (t2*(t2*(3*xb-t2*xa)-3*xc)+xd)/8;
      fy0 -= fy3 = (t2*(t2*(3*yb-t2*ya)-3*yc)+yd)/8;
      x3 = floor(fx3+0.5); y3 = floor(fy3+0.5);        /* scale bounds to int */
      if (fx0 != 0.0) { fx1 *= fx0 = (x0-x3)/fx0; fx2 *= fx0; }
      if (fy0 != 0.0) { fy1 *= fy0 = (y0-y3)/fy0; fy2 *= fy0; }
      if (x0 != x3 || y0 != y3)                            /* segment t1 - t2 */
         __cubic_bezier_segment(this, x0,y0, x0+fx1, y0+fy1, x0+fx2, y0+fy2, x3, y3, c);
      x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
   }
}

void SBitmap::quadbezier(SPoint* pts, RGBColor c) {
	quadbezier(pts[0], pts[1], pts[2], c);
}

void SBitmap::cubicbezier(SPoint* pts, RGBColor c) {
	cubicbezier(pts[0], pts[1], pts[2], pts[3], c);
}

void SBitmap::quadbezier(const SPoint& p1, const SPoint& p2, const SPoint& p3, RGBColor c) {
	quadbezier(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), p3.X(this), p3.Y(this), c);
}

void SBitmap::cubicbezier(const SPoint& p1, const SPoint& p2, const SPoint& p3, const SPoint& p4, RGBColor c) {
	cubicbezier(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), p3.X(this), p3.Y(this), p4.X(this), p4.Y(this), c);
}

// Keep a cache of font sizes for built-in fonts.
static std::unordered_map<std::pair<ttfont *, u32>, u32, FontPairHash> __font_size_cache;

u32 Font::font_size_for_height(u32 height) {
	// Check the map for built-in fonts.
	std::pair<ttfont *, u32> pr = std::make_pair(this->font, height);
	if (builtin && __font_size_cache.find(pr) != __font_size_cache.end()) {
		return __font_size_cache[pr];
	}

	SBitmap* test;
	u32 ret;

	test = render_cstr("test stringABCQWXjpqy!/&,.", 200, false, nullptr);
	ret = (200 * height + (test->height() / 2)) / test->height();
	delete test;

	// If this is a built-in, cache the result.
	if (builtin)
		__font_size_cache[pr] = ret;

	return ret;
}

void SBitmap::fillrect_gradient(int x1, int y1, int x2, int y2, RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right) {
	int x, y;
	int i;
	RGBColor top, bottom;

	SORT2(x1, x2, int);
	SORT2(y1, y2, int);

	VALID_RECT(this, x1, y1, x2, y2);
	/* use libdivide in the loops here for speed */
	int div_x = x2 - x1, div_y = y2 - 1;
	/* Avoid zero divisors; the loops below handle them correctly, but libdivide will kvetch if we try
		to create a libdivide::divider(0). */
	if (0 == div_x)
		++div_x;
	if (0 == div_y)
		++div_y;
	libdivide::divider<int> magic_x(div_x), magic_y(div_y); 

	for (x = x1; x <= x2; ++x) {
		if (x1 == x2) {
			top = upper_left;
			bottom = lower_left;
		} else {
			i = 1000 - ((x - x1) * 1000) / magic_x;
			interpolate_rgbcolor(upper_left, upper_right, &top, i);
			interpolate_rgbcolor(lower_left, lower_right, &bottom, i);
		}
			
		for (y = y1; y <= y2; ++y) {
			RGBColor c;			
			if (y1 == y2) {
				c = top;
			} else {
				i = 1000 - ((y - y1) * 1000) / magic_y;
				interpolate_rgbcolor(top, bottom, &c, i);
			}
			put_pixel(x, y, c);
		}
	}
}

void SBitmap::fillrect_gradient(const SCoord& region, RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right) {
	fillrect_gradient(region.X1(this), region.Y1(this), region.X2(this), region.Y2(this), upper_left, upper_right, lower_left, lower_right);
}

void SBitmap::fillbmp_gradient(RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right) {
	SCoord co;
	rect_bitmap(&co);
	fillrect_gradient(co, upper_left, upper_right, lower_left, lower_right);
}

void perturb_color_randomly_even(RGBColor *c) {
	double bright = RGB_RED(*c) + RGB_GREEN(*c) + RGB_BLUE(*c);
	u32 ran;
	double ranf;
	double bright2;
	double ratio;
	double rf, gf, bf;
	u32 r, g, b;

	if (bright <= 0.) {
		perturb_color_randomly(c);
		return;
	}

	ran = RandU32();
	ranf = ((double)(ran & 0xFFFF) / 32767.5) - 1.0;	/* [-1, 1] */
	
	bright /= 3.0;
	bright2 = bright + ranf * 8.0;	
	bright2 = CLAMP(bright2, 0., 255.);
	ratio = bright2 / bright;
	
	rf = RGB_RED(*c) * ratio;
	gf = RGB_GREEN(*c) * ratio;
	bf = RGB_BLUE(*c) * ratio;
	r = COMPONENT_RANGE(ROUND_FLOAT_TO_INT(rf));
	g = COMPONENT_RANGE(ROUND_FLOAT_TO_INT(gf));
	b = COMPONENT_RANGE(ROUND_FLOAT_TO_INT(bf));

	*c = RGB_NO_CHECK(r, g, b);
}

/*** Given a line from (x1, y1) to (x2, y2), return the x location on the line at the specified
	y value. Returns LINE_NO_VALUE if there is no point on the line at the specified y value,
	and returns LINE_ALL_VALUES if all x values are valid at y (i.e. the slope of the line is zero.) ***/
double line_at_y(double x1, double y1, double x2, double y2, double y) {
	double p;
	if (y1 == y2)
		return(LINE_ALL_VALUES);
	p = (y - y1) / (y2 - y1);
	if (p < 0. || p > 1.)
		return(LINE_NO_VALUE);
	return x1 + ((x2 - x1) * p);
}

/*** Given a line from (x1, y1) to (x2, y2), return the y location on the line at the specified
	x value. Returns LINE_NO_VALUE if there is no point on the line at the specified x value,
	and returns LINE_ALL_VALUES if all y values are valid at y (i.e. the line is vertical.) ***/
double line_at_x(double x1, double y1, double x2, double y2, double x) {
	// simple change of variables
	return line_at_y(y1, x1, y2, x2, x);
}

/*** Normalize a vector to unit length. ***/
void normalize_vector(double* x, double* y) {
	double mag;
	mag = sqrt((*x) * (*x) + (*y) * (*y));
	if (mag == 0.)
		return;
	(*x) /= mag;
	(*y) /= mag;
}

/*** internal helper function to draw a line with the specified thickness ***/
static void internal_thicklinebmp(SBitmap* bmp, double x1, double y1, double x2, double y2,
									double thickness, RGBColor clr, bool antialiasing, PatternCallback pattern, void* args) {
	NOT_NULL_OR_RETURN_VOID(bmp);
	if (thickness <= 0.)
		return;
	// first order of business: find the four corners of the rectangle we'll be drawing.
	double xp[4], yp[4];
	double sx, sy;	// unit slope of the line
	int xx, yy;
	sx = (x2 - x1);
	sy = (y2 - y1);
	if (sx == 0. && sy == 0.) {
		// a special case -- the line is a single point
		xx = ROUND_FLOAT_TO_INT(x1);
		yy = ROUND_FLOAT_TO_INT(y1);
		if (is_null(pattern))
			bmp->put_pixel(xx, yy, clr);
		else
			bmp->put_pixel(xx, yy, pattern(xx, yy, args));
		return;
	}
	normalize_vector(&sx, &sy);
	xp[0] = x1 - sx * thickness;
	yp[0] = y1 + sy * thickness;
	xp[1] = x2 - sx * thickness;
	yp[1] = y2 + sy * thickness;
	xp[2] = x2 + sx * thickness;
	yp[2] = y2 - sy * thickness;
	xp[3] = x1 + sx * thickness;
	yp[3] = y1 - sy * thickness;

	// Four corners are given. Now, for each possible y value, determine the min and max x values to draw the line.
	int min_y, max_y;

	min_y = min_int4((int)floor(yp[0]),
					 (int)floor(yp[1]),
 					 (int)floor(yp[2]),
 					 (int)floor(yp[3]));
	max_y = max_int4((int)floor(yp[0] + 0.5),
					 (int)floor(yp[1] + 0.5),
 					 (int)floor(yp[2] + 0.5),
 					 (int)floor(yp[3] + 0.5));

	// TODO: better anti-aliasing -- perhaps another sweep, over x values?
	for (yy = min_y; yy <= max_y; ++yy) {
		double x_min, x_max, x_res;
		int e;
		int ix_min, ix_max;
		x_min = 9999999.;
		x_max = -9999999.;
		for (e = 0; e < 4; ++e) {
			x_res = line_at_y(xp[e], yp[e], xp[(e + 1) & 3], yp[(e + 1) & 3], (double)yy);
			if (x_res == LINE_NO_VALUE || x_res == LINE_ALL_VALUES)
				continue;
			x_min = min_double(x_min, x_res);
			x_max = max_double(x_max, x_res);
		}
		if (x_min > x_max)
			continue;
		ix_min = (int)floor(x_min); //ROUND_FLOAT_TO_INT(x_min);
		ix_max = ROUND_FLOAT_TO_INT(x_max);
		if (!antialiasing) {
			// the easier case: just draw a horizontal line
			if (is_null(pattern)) {
				bmp->hline(ix_min, ix_max, yy, clr);
			} else {
				while (ix_min <= ix_max) {
					bmp->put_pixel(ix_min, yy, pattern(ix_min, yy, args));
					++ix_min;
				}
			}
		} else {
			double intensity;
			RGBColor rgb;
			
			if (ix_min == ix_max) {
				// special case: only a single pixel, so it's effectively an edge on both sides
				intensity = x_max - x_min;
				if (is_null(pattern))
					rgb = clr;
				else
					rgb = pattern(ix_min, yy, args);
				interpolate_rgbcolor(rgb, bmp->get_pixel(ix_min, yy), &rgb, ROUND_FLOAT_TO_INT(1000. * intensity));
				bmp->put_pixel(ix_min, yy, rgb);
			} else {
				// handle the edge pixels separately
				intensity = 1.0 - (x_min - floor(x_min));
				if (is_null(pattern))
					rgb = clr;
				else
					rgb = pattern(ix_min, yy, args);
				interpolate_rgbcolor(rgb, bmp->get_pixel(ix_min, yy), &rgb, ROUND_FLOAT_TO_INT(1000. * intensity));
				bmp->put_pixel(ix_min, yy, rgb);

				intensity = x_max - floor(x_max);
				if (is_null(pattern))
					rgb = clr;
				else
					rgb = pattern(ix_max, yy, args);
				interpolate_rgbcolor(rgb, bmp->get_pixel(ix_max, yy), &rgb, ROUND_FLOAT_TO_INT(1000. * intensity));
				bmp->put_pixel(ix_max, yy, rgb);

				// and draw a horizontal line.
				rgb = clr;
				++ix_min;
				while (ix_min < ix_max) {
					if (not_null(pattern))
						rgb = pattern(ix_min, yy, args);
					bmp->put_pixel(ix_min, yy, rgb);
					++ix_min;
				}
			}
		}
	}
}

/*** Draw a line with the specified thickness from (x1, y1) to (x2, y2) with the specified color. If antialiasing is true,
	the edges will be anti-aliased. Fractional pixels and fractional thickness are all right with this function, and with anti-aliasing,
	will look pretty good. Written for pretty output, not so much speed. ***/
void SBitmap::drawthickline(double x1, double y1, double x2, double y2, double thickness, RGBColor rgb, bool antialiasing) {
	internal_thicklinebmp(this, x1, y1, x2, y2, thickness, rgb, antialiasing, NULL, NULL);
}

void SBitmap::drawthickline(int x1, int y1, int x2, int y2, double thickness, RGBColor rgb, bool antialiasing) {
	drawthickline((double)x1, (double)y1, (double)x2, (double)y2, thickness, rgb, antialiasing);
}

void SBitmap::drawthickline(const SPoint& p1, const SPoint& p2, double th, RGBColor rgb, bool aa) {
	drawthickline(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), th, rgb, aa);
}

void SBitmap::drawthickline(const SCoord& co, double th, RGBColor rgb, bool aa) {
	drawthickline(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), th, rgb, aa);
}

/*** As above, but calls the specified PatternCallback when drawing the pixels. ***/
void SBitmap::drawthickline_pattern(double x1, double y1, double x2, double y2, double thickness, PatternCallback pattern, void* args, bool antialiasing) {
	internal_thicklinebmp(this, x1, y1, x2, y2, thickness, C_BLACK, antialiasing, pattern, args);
}

void SBitmap::drawthickline_pattern(int x1, int y1, int x2, int y2, double thickness, PatternCallback pattern, void* args, bool antialiasing) {
	drawthickline_pattern((double) x1, (double) y1, (double)x2, (double)y2, thickness, pattern, args, antialiasing);
}

void SBitmap::drawthickline_pattern(const SPoint& p1, const SPoint& p2, double th, PatternCallback pat, void* args, bool aa) {
	drawthickline_pattern(p1.X(this), p1.Y(this), p2.X(this), p2.Y(this), th, pat, args, aa);
}

void SBitmap::drawthickline_pattern(const SCoord& co, double th, PatternCallback pat, void* args, bool aa) {
	drawthickline_pattern(co.X1(this), co.Y1(this), co.X2(this), co.Y2(this), th, pat, args, aa);
}

/*** Some image filters. ***/
#define	RGBCOUNT(c)		(RGB_RED(c) + RGB_GREEN(c) + RGB_BLUE(c))

/*** A line art filter ***/
SBitmap* SBitmap::line_art_filter(u32 tol) {
	SBitmap* ret = new SBitmap(w, h, BITMAP_DEFAULT);
	NOT_NULL_OR_RETURN(ret, NULL);
	int e, f;
	RGBColor c1, c2;

	/* first pass */
	for (f = 0; f < h; ++f) {
		c1 = get_pixel(0, f);
		for (e = 1; e < w; ++e) {
			c2 = get_pixel(e, f);
			if (abs((int)(RGBCOUNT(c1) - RGBCOUNT(c2))) >= tol)
				ret->put_pixel(e, f, C_BLACK);
			else
				ret->put_pixel(e, f, C_WHITE);
			c1 = c2;
		}
	}

	/* second pass */
	for (e = 0; e < w; ++e) {
		c1 = get_pixel(e, 0);
		for (f = 1; f < h; ++f) {
			c2 = get_pixel(e, f);
			if (abs((int)RGBCOUNT(c1) - (int)RGBCOUNT(c2)) >= tol)
				ret->put_pixel(e, f, C_BLACK);
			c1 = c2;
		}
	}

	return ret;
}

/*** Brighten or darken the bitmap by adjustment (negative values darken, positive brighten). Respects the alpha channel. ***/
void SBitmap::brighten_darken(int adjustment) {
	int e, f;
	RGBColor c;
	int r, g, b, a;
	for (f = 0; f < h; ++f)
		for (e = 0; e < w; ++e) {
			c = get_pixel(e, f);
			r = RGB_RED(c) + adjustment;
			g = RGB_GREEN(c) + adjustment;
			b = RGB_BLUE(c) + adjustment;
			a = RGB_ALPHA(c);
			r = CLAMP(r, 0, 255);
			g = CLAMP(g, 0, 255);
			b = CLAMP(b, 0, 255);
			put_pixel(e, f, RGBA_NO_CHECK(r, g, b, a));
		}
}

SBitmap* SBitmap::heighten_edges(void) {
	/* Makes the edges in the picture more pronounced */
	int e,f;
	SBitmap* ret;

	ret = new SBitmap(w, h, BITMAP_DEFAULT);
	NOT_NULL_OR_RETURN(ret, ret);
	
	for (f = 0; f < h; ++f) {
		for (e = 0; e + 1 < w; ++e) {	// should this be e += 2?
			int c1, c2, r1, r2, g1, g2, b1, b2;
			c1 = get_pixel(e, f);
			c2 = get_pixel(e+1, f);
			r1 = RGB_RED(c1);
			r2 = RGB_RED(c2);
			g1 = RGB_GREEN(c1);
			g2 = RGB_GREEN(c2);
			b1 = RGB_BLUE(c1);
			b2 = RGB_BLUE(c2);
			if (r1 > r2) {
			   int d;
			   d = (r1 - r2) >> 3;
			   r1 += d;
			   r2 -= d;
			   if (r1 > 255)		r1 = 255;
			   if (r2 < 0)			r2 = 0;
			} else {
			   int d;
			   d = (r2 - r1) >> 3;
			   r2 += d;
			   r1 -= d;
			   if (r2 > 255)		r2 = 255;
			   if (r1 < 0)			r1 = 0;
			}
			if (g1 > g2) {
			   int d;
			   d = (g1 - g2) >> 3;
			   g1 += d;
			   g2 -= d;
			   if (g1 > 255)		g1 = 255;
			   if (g2 < 0)			g2 = 0;
			} else {
			   int d;
			   d = (g2 - g1) >> 3;
			   g2 += d;
			   g1 -= d;
			   if (g2 > 255)		g2 = 255;
			   if (g1 < 0)			g1 = 0;
			}
			if (b1 > b2) {
			   int d;
			   d = (b1 - b2) >> 3;
			   b1 += d;
			   b2 -= d;
			   if (b1 > 255)		b1 = 255;
			   if (b2 < 0)			b2 = 0;
			} else {
			   int d;
			   d = (b2 - b1) >> 3;
			   b2 += d;
			   b1 -= d;
			   if (b2 > 255)		b2 = 255;
			   if (b1 < 0)			b1 = 0;
			}
			ret->put_pixel(e, f, RGB(r1, g1, b1));
			ret->put_pixel(e+1, f, RGB(r2, g2, b2));
		}
	}
	// TODO: this is just overwriting what's already in ret... maybe we should
	// try interpolating in the second pass values?
	// And should we increment e/f by 1 or by 2 in their respective loops?
#if 0
	for (f=0; f+1<pic->h; ++f) {
		for (e=0; e<pic->w; ++e) {
			int c1, c2, r1, r2, g1, g2, b1, b2;
			c1 = fast_getpixelbmp(pic, e, f);
			c2 = fast_getpixelbmp(pic, e, f+1);
			r1 = RGB_RED(c1);
			r2 = RGB_RED(c2);
			g1 = RGB_GREEN(c1);
			g2 = RGB_GREEN(c2);
			b1 = RGB_BLUE(c1);
			b2 = RGB_BLUE(c2);
			if (r1 > r2) {
			   int d;
			   d = (r1 - r2) >> 3;
			   r1 += d;
			   r2 -= d;
			   if (r1 > 255)		r1 = 255;
			   if (r2 < 0)			r2 = 0;
			} else {
			   int d;
			   d = (r2 - r1) >> 3;
			   r2 += d;
			   r1 -= d;
			   if (r2 > 255)		r2 = 255;
			   if (r1 < 0)			r1 = 0;
			}
			if (g1 > g2) {
			   int d;
			   d = (g1 - g2) >> 3;
			   g1 += d;
			   g2 -= d;
			   if (g1 > 255)		g1 = 255;
			   if (g2 < 0)			g2 = 0;
			} else {
			   int d;
			   d = (g2 - g1) >> 3;
			   g2 += d;
			   g1 -= d;
			   if (g2 > 255)		g2 = 255;
			   if (g1 < 0)			g1 = 0;
			}
			if (b1 > b2) {
			   int d;
			   d = (b1 - b2) >> 3;
			   b1 += d;
			   b2 -= d;
			   if (b1 > 255)		b1 = 255;
			   if (b2 < 0)			b2 = 0;
			} else {
			   int d;
			   d = (b2 - b1) >> 3;
			   b2 += d;
			   b1 -= d;
			   if (b2 > 255)		b2 = 255;
			   if (b1 < 0)			b1 = 0;
			}
			fast_putpixelbmp(ret, e, f, RGB(r1, g1, b1));
			fast_putpixelbmp(ret, e, f+1, RGB(r2, g2, b2));	
		}
	}
#endif

	return ret;
}

void SBitmap::regular_polygon(int x_center, int y_center, u32 nsides, double radius, double angle_rotate_rad, RGBColor rgb) {
	std::vector<SPoint> pts;
	if (nsides < 3)
		return;
	if (radius == 0.) {
		put_pixel(x_center, y_center, rgb);
		return;
	}
	pts.reserve(16);

	double angle = angle_rotate_rad;
	const double step = (2.0 * M_PI) / (double)nsides;
	const double xc = double(x_center), yc = double(y_center);
	for (u32 e = 0; e < nsides; ++e) {
		double x = cos(angle) * radius + xc, y = yc - sin(angle) * radius;
		angle += step;
		SPoint pt(ROUND_FLOAT_TO_INT(x), ROUND_FLOAT_TO_INT(y));
		pts.push_back(pt);
	}
	for (u32 e = 0; e < nsides; ++e) {
		if (e + 1 == nsides)
			line(pts[e], pts[0], rgb);
		else
			line(pts[e], pts[e + 1], rgb);
	}
}

void SBitmap::regular_polygon(const SPoint& center, u32 nsides, double radius, double angle_rotate_rad, RGBColor rgb) {
	regular_polygon(center.X(this), center.Y(this), nsides, radius, angle_rotate_rad, rgb);
}

void SBitmap::render_text(const char* str, const SCoord& rect, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	SBitmap* render;
	int x = rect.X1(this), y = rect.Y1(this);
	u32 sz;
	u32 slen = strlen(str);
	const u32 BITFONT_CHAR_W = 9;
	const u32 BITFONT_CHAR_H = 6;

	if (is_null(f)) {
		/* Render using the bitfont. */
		u32 sx, sy;
		sz = desired_height >> 3;
		sx = (BITFONT_CHAR_W * slen * sz);
		sy = (BITFONT_CHAR_H * slen * sz);
		render = new SBitmap(sx, sy, BITMAP_GRAYSCALE);
		render->clear(C_BLACK);
		render->putstr_bitfont(str, 0, 0, sz, C_WHITE);
	} else {
		sz = f->font_size_for_height(desired_height);
		render = f->render_cstr(str, sz, false, nullptr);
	}

	if (!iszero(center_align_flags & CENTERED_BOTH)) {
		if (center_align_flags & CENTERED_HORIZ) {
			x = rect.center_X(this);
			x -= render->width() / 2;
		}
		if (center_align_flags & CENTERED_VERT) {
			y = rect.center_Y(this);
			y -= render->height() / 2;
		}
	}
	if (!iszero(center_align_flags & ALIGN_TOP)) {
		y = rect.Y1(this) + 2;
	}
	if (!iszero(center_align_flags & ALIGN_BOTTOM)) {
		y = rect.Y2(this) - (2 + render->height());
	}
	if (!iszero(center_align_flags & ALIGN_LEFT)) {
		x = rect.X1(this) + 2;
	}
	if (!iszero(center_align_flags & ALIGN_RIGHT)) {
		x = rect.X2(this) - (2 + render->width());
	}

	Font::blit(render, this, x, y, c);

	delete render;
}

void SBitmap::render_text(const char* str, const SCoord& rect, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	Font ft(f);
	render_text(str, rect, &ft, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const std::string& str, const SCoord& rect, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	render_text(str.c_str(), rect, f, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const std::string& str, const SCoord& rect, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	Font ft(f);
	render_text(str.c_str(), rect, &ft, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const char* str, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	SCoord co;
	rect_bitmap(&co);
	render_text(str, co, f, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const char* str, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	SCoord co;
	rect_bitmap(&co);
	Font ft(f);
	render_text(str, co, &ft, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const std::string& str, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	SCoord co;
	rect_bitmap(&co);
	render_text(str.c_str(), co, f, c, desired_height, center_align_flags);
}

void SBitmap::render_text(const std::string& str, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags) {
	SCoord co;
	rect_bitmap(&co);
	Font ft(f);
	render_text(str.c_str(), co, &ft, c, desired_height, center_align_flags);
}

void SBitmap::out_channel_rf(RamFile* rf, u32 idx) {
	for (u32 y = 0; y < h; ++y) {
		for (u32 x = 0; x < w; ++x) {
			int b;
			switch (idx) {
			case 0: b = RGB_RED(get_pixel(x, y));	break;
			case 1: b = RGB_GREEN(get_pixel(x, y));	break;
			case 2: b = RGB_BLUE(get_pixel(x, y));	break;
			case 3: b = RGB_ALPHA(get_pixel(x, y));	break;
			}
			rf->putc(b);
		}
	}
}

void SBitmap::in_channel_rf(SBitmap* bmp, RamFile* rf, u32 idx) {
	for (u32 y = 0; y < bmp->h; ++y) {
		for (u32 x = 0; x < bmp->w; ++x) {
			int b = rf->getc();
			switch (idx) {
			case 0: bmp->set_red(x, y, b);	break;
			case 1: bmp->set_green(x, y, b);	break;
			case 2: bmp->set_blue(x, y, b);	break;
			case 3: bmp->set_alpha(x, y, b);	break;
			}
		}
	}
}

bool SBitmap::all_opaque(void) {
	for (u32 y = 0; y < h; ++y) {
		for (u32 x = 0; x < w; ++x) {
			u32 a = get_alpha(x, y);
			if (a != ALPHA_OPAQUE)
				return false;
		}
	}
	return true;
}

SBitmap* SBitmap::from_ramfile(RamFile* rf) {
	SBitmap* ret = nullptr;
	u32 ww, hh;
	BitmapType bt;
	bool has_alpha;

	ww = rf->getu32();
	hh = rf->getu32();
	bt = (BitmapType)rf->getu32();
	if (BITMAP_DISPLAY_OWNED == bt)
		bt = BITMAP_DEFAULT;
	ret = new SBitmap(ww, hh, bt);
	NOT_NULL_OR_RETURN(ret, ret);
	ret->clear(); // Sets alpha channel to all opaque, which is what we want if !has_alpha
		
	switch (bt) {
	case BITMAP_DEFAULT:
	case BITMAP_DISPLAY_OWNED:
		has_alpha = rf->getbool();
		in_channel_rf(ret, rf, 0);
		in_channel_rf(ret, rf, 1);
		in_channel_rf(ret, rf, 2);
		if (has_alpha)
			in_channel_rf(ret, rf, 3);
		break;
	case BITMAP_MONO:
		rf->getmem((u8*) ret->bits, ((ret->w * ret->h) >> 3) + 1);
		break;
	case BITMAP_16BITS:
		rf->getmem((u8*) ret->bits, (ret->w * ret->h * 2));
		break;
	case BITMAP_24BITS:
		rf->getmem((u8*) ret->bits, (ret->w * ret->h * 3));
		break;
 	case BITMAP_PALETTE:
		assert(!is_null(ret->pal));
		ret->pal->from_ramfile(rf);
		/* fallthrough */
	case BITMAP_GRAYSCALE:
		/* possible fallthrough from above */
		rf->getmem((u8*) ret->bits, ret->w * ret->h);
		break;
	case BITMAP_SUBBITMAP:
	default:
		assert(false);
		delete ret;
		return nullptr;
	}

	return ret;
}

SBitmap* SBitmap::from_ramfile(const char* fname) {
	RamFile rf;
	if (rf.open(fname, RAMFILE_READONLY)) {
		return(nullptr);
	}
	SBitmap* ret = from_ramfile(&rf);
	rf.close();
	return ret;
}

SBitmap* SBitmap::from_ramfile(const std::string& fname) {
	return from_ramfile(fname.c_str());
}

u32 SBitmap::to_ramfile(const char* fname) {
	RamFile rf;
	if (rf.open(fname, RAMFILE_COMPRESS | RAMFILE_CREATE_IF_MISSING)) {
		return 1;
	}
	u32 ret = to_ramfile(&rf);
	rf.close();
	return ret;
}

u32 SBitmap::to_ramfile(RamFile* rf) {
	bool has_alpha;
	rf->putu32(w);
	rf->putu32(h);
	rf->putu32((u32)btype);
	switch (btype) {
	case BITMAP_DEFAULT:
	case BITMAP_DISPLAY_OWNED:
		has_alpha = !all_opaque();
		rf->putbool(has_alpha);
		/* one chroma channel at a time, for better compression */
		out_channel_rf(rf, 0);
		out_channel_rf(rf, 1);
		out_channel_rf(rf, 2);
		if (has_alpha)
			out_channel_rf(rf, 3);
		break;
	case BITMAP_MONO:
		rf->putmem((char*)bits, ((w * h) >> 3) + 1);
		break;
 	case BITMAP_PALETTE:
		pal->to_ramfile(rf);
		/* fallthrough */
	case BITMAP_GRAYSCALE:
		rf->putmem((char*)bits, w * h);
		break;
	case BITMAP_16BITS:
		rf->putmem((char*)bits, w * h * 2);
		break;
	case BITMAP_24BITS:
		rf->putmem((char*)bits, w * h * 3);
		break;
	case BITMAP_SUBBITMAP:
	default:
		assert(false);
		return 1;
	}

	return 0;
}

u32 SBitmap::to_ramfile(const std::string& fname) {
	return to_ramfile(fname.c_str());
}

RGBColor RandColor(void) {
	return (RGBColor)(RandU32() & 0xffffff);
}

void SBitmap::fill_static(bool gray) {
	u32 x, y;
	for (y = 0; y < h; ++y)
		for (x = 0; x < w; ++x) {
			if (gray) {
				int i = (int)RandU8();
				put_pixel(x, y, RGB_GRAY(i));
			} else {
				put_pixel(x, y, RandColor());
			}
		}
}

/* Convert a color to gray scale using the standard ITU-R 601-2 luma transform. */
RGBColor to_gray_luma_transform(RGBColor c) {
	u32 r, g, b;
	u32 i;
	r = RGB_RED(c);
	g = RGB_GREEN(c);
	b = RGB_BLUE(c);
	i = (r * 299) + (g * 587) + (b * 114);
	i /= 1000UL;
	return RGB_NO_CHECK(i, i, i);
}

SBitmap* SBitmap::aspect_ratio(u32 aw, u32 ah) {
	u32 nx, ny;
	if (0 == aw)
		aw++;
	if (0 == ah)
		ah++;
	if (width() > height()) {
		nx = width();
		ny = (nx * ah) / aw;
	} else {
		ny = height();
		nx = (ny * aw) / ah;
	}
	return resize(nx, ny);
}

bool SBitmap::aspect_ratio_replace(u32 aw, u32 ah) {
	if (BITMAP_SUBBITMAP ==  btype)
		return false;
	SBitmap* nb = aspect_ratio(aw, ah);
	if (is_null(nb))
		return false;
	swap(nb);
	delete nb;
	return true;
}

u32 SBitmap::intensity_by_index(u32 i) const {
	u32 ci = i / (width() * height()), pi = i % (width() * height());
	u32 x = (pi % width()), y = pi / width();
	RGBColor r = get_pixel(x, y);
	switch (ci) {
	case 0:
		return RGB_RED(r);
	case 1:
		return RGB_GREEN(r);
	case 2:
		return RGB_BLUE(r);
	}
	return 0;
}

/* let's save time on allocs/frees when computing earth movers' distance. */
static i32* ensure_emd_buckets(u32 n) {
	static i32 *emd_buckets = nullptr;
	static u32 emd_alloc = 0;
	if (0 == n) {
		// special case: if called with n == 0, free the emd_buckets.
		if (!is_null(emd_buckets))
			delete emd_buckets;
		emd_buckets = nullptr;
		emd_alloc = 0;
	} else if (emd_alloc < n) {
		if (!is_null(emd_buckets))
			delete emd_buckets;
		emd_buckets = new i32 [n + 1];
		emd_alloc = n;
	}
	return emd_buckets;
}

u64 SBitmap::distance_emd(const SBitmap* b2) const {
	i32* emd_buckets;
	u64 ret;
	i32 n;
	u32 i;
	if (is_null(b2)) {
		// just free the EMD buckets
		ensure_emd_buckets(0);
		return 0ULL;
	}
	if (width() != b2->width() || height() != b2->height()) {
LErr:	assert(false);
		return UINT64_MAX;
	}

	/* Calculate 1-D earth mover's distance over the whole image. */
	n = width() * height() * 3;
	emd_buckets = ensure_emd_buckets(n);
	if (is_null(emd_buckets))
		goto LErr;
	i = 0;
	emd_buckets[0] = 0;
	for (u32 e = 0; e < n; ++e) {
		i32 ity1 = (i32)intensity_by_index(e), ity2 = (i32)b2->intensity_by_index(e);
		emd_buckets[e + 1] = ity1 + emd_buckets[e] - ity2;
	}
	ret = (u64)emd_buckets[0];
	for (u32 e = 1; e <= n; ++e) {
		if (emd_buckets[e] < 0)
			ret += (u64)(-emd_buckets[e]);
		else
			ret += (u64)emd_buckets[e];
	}

	return ret;
}

void libcodehappy_free_caches() {
	__font_width_cache.clear();
	__font_size_cache.clear();
	if (!is_null(__ffbits)) {
		delete __ffbits;
		__ffbits = nullptr;
	}
	ensure_emd_buckets(0);
}

u64 SBitmap::hash() const {
	u64 ret;
	u64 seed = 0;

	assert(btype != BITMAP_SUBBITMAP);

	seed = (u64(width()) << 18) + u64(height() << 4) + btype;

	u32 sz = 0;
	switch (btype) {
	case BITMAP_DEFAULT:
	case BITMAP_DISPLAY_OWNED:
		sz = w * h * sizeof(u32);
		break;
	case BITMAP_MONO:
		sz = ((w * h) >> 3) + 1;
		break;
	case BITMAP_PALETTE:
	case BITMAP_GRAYSCALE:
		sz = w * h;
		break;
	case BITMAP_16BITS:
		sz = w * h * 2;
		break;
	case BITMAP_24BITS:
		sz = w * h * 3;
		break;
	case BITMAP_SUBBITMAP:
	case BITMAP_INVALID:
		assert(false);
		break;
	}

	ret = SpookyHash::Hash64((void *)bits, sz, seed);
	return ret;
}

#if 0
********************************************************************************

Example code showing put_pixel() alpha channel behavior:

/* create a 32 bits per pixel bitmap */
SBitmap* bmp = new SBitmap(w, h);
/* rectangular region representing the entire bitmap */
SCoord co = bmp;
/* this clears the bitmap to black, but also fills the alpha channel with ALPHA_OPAQUE. */
/* (note that clear() behavior is a special case and always sets the alpha channel, regardless
    of the put_pixel alpha setting.) */
bmp->clear();
/* this next line doesn't change the alpha channel; it's still fully opaque. */
bmp->rect_fill(co, C_BLACK);
/* change the default put_pixel() behavior */
bmp->putpixel_affects_alpha(true);
/* this line now zeros the alpha channel (ALPHA_TRANSPARENT) as well as the RGB content. */
bmp->rect_fill(co, C_BLACK);

********************************************************************************
#endif 

void SBitmap::putpixel_affects_alpha(bool does_affect) {
	if (put_pixel_fn != put_pixel_32bpp_alpha &&
		put_pixel_fn != put_pixel_32bpp) {
		return;
	}
	if (does_affect) {
		put_pixel_fn = put_pixel_32bpp_alpha;
	} else {
		put_pixel_fn = put_pixel_32bpp;
	}
}

void SBitmap::putpixel_affects_alpha_toggle() {
	if (put_pixel_fn == put_pixel_32bpp_alpha) {
		put_pixel_fn = put_pixel_32bpp;
	} else if (put_pixel_fn == put_pixel_32bpp) {
		put_pixel_fn = put_pixel_32bpp_alpha;
	}
}

bool SBitmap::putpixel_affects_alpha() const {
	return put_pixel_fn == put_pixel_32bpp_alpha;
}

SBitmap* SBitmap::center_crop(u32 size) const {
	if (w == h) {
		return resize(size, size);
	}

	SBitmap* blt = new SBitmap(std::min(w, h), std::min(w, h));
	NOT_NULL_OR_RETURN(blt, blt);

	if (w > h) {
		// Landscape
		blit(int(w - h) / 2, 0, (int(w + h) / 2) + 1, (int) h, blt);
	} else {
		// Portrait
		blit(0, int(h - w) / 2, (int) w, (int(h + w) / 2) + 1, blt);
	}

	if (size > 0) {
		blt->resize_and_replace(size, size);
	}
	return blt;
}

bool SBitmap::center_crop_and_replace(u32 size) {
	if (w == h) {
		if (0 == size)
			return true;
		return resize_and_replace(size, size);
        }
	if (btype == BITMAP_SUBBITMAP)
		return false;
	SBitmap* nb = center_crop(size);
	if (nullptr == nb)
		return false;
	swap(nb);
	delete nb;
	return true;
}

SBitmap* SBitmap::rotate_clockwise_90() const {
	SBitmap* ret = new SBitmap(height(), width());
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			RGBColor c = get_pixel(x, y);
			ret->put_pixel((int)ret->width() - 1 - y, x, c);
		}
	}
	return ret;
}

SBitmap* SBitmap::rotate_counterclockwise_90() const {
	SBitmap* ret = new SBitmap(height(), width());
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			RGBColor c = get_pixel(x, y);
			ret->put_pixel(y, (int)ret->height() - 1 - x, c);
		}
	}
	return ret;
}

void SBitmap::stretch_blit(SBitmap* bmp_dest, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h) const {
	assert(not_null(bmp_dest));
	SBitmap* stretch = new SBitmap(src_w, src_h);
	assert(not_null(stretch));
	blit(src_x, src_y, src_x + src_w - 1, src_y + src_h - 1, stretch, 0, 0);
	assert(stretch->resize_and_replace(dest_w, dest_h));
	stretch->blit(bmp_dest, dest_x, dest_y);
	delete stretch;
}

void SBitmap::noise_rgb(int x, int y, int mag) {
	int r, g, b, c;
	c = get_pixel(x, y);
	r = RGB_RED(c);
	g = RGB_GREEN(c);
	b = RGB_BLUE(c);
	r += randbetween(-mag, mag);
	g += randbetween(-mag, mag);
	b += randbetween(-mag, mag);
	r = CLAMP(r, 0, 255);
	g = CLAMP(g, 0, 255);
	b = CLAMP(b, 0, 255);
	put_pixel(x, y, RGB_NO_CHECK(r, g, b));
}

void SBitmap::noise_rgb(int mag) {
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			noise_rgb(x, y, mag);
		}
	}
}

RGB565 RGB565FromRGBColor(RGBColor c) {
	uint r, g, b, v;
	r = RGB_RED(c) + 4;
	g = RGB_GREEN(c) + 2;
	b = RGB_BLUE(c) + 4;
	r >>= 3;
	g >>= 2;
	b >>= 3;
	if (r > 31) r = 31;
	if (g > 63) g = 63;
	if (b > 31) b = 31;
	v = (r << 11) + (g << 5) + b;
	return RGB565(v);
}

RGBColor RGBColorFromRGB565(RGB565 c) {
	return RGB_NO_CHECK(RGB565_red(c), RGB565_green(c), RGB565_blue(c));
}

uint RGB565_red(RGB565 c) {
	c &= 0xF800;
	return (uint(c) >> 11) << 3;
}

uint RGB565_green(RGB565 c) {
	c &= 0x07E0;
	return (uint(c) >> 5) << 2;
}

uint RGB565_blue(RGB565 c) {
	c &= 0x001F;
	return uint(c) << 3;
}

bool is_img_extension(const char* fname) {
	const char* extf[] = { ".bmp", ".png", ".tga", ".gif", ".pcx", ".raw", ".jpg", ".rfi", ".svg" };
	for (auto ex : extf)
		if (__stristr(fname, ex) != nullptr)
			return true;
	return false;
}

uint rgb_intensity(RGBColor c) {
	uint r, g, b;
	r = RGB_RED(c);
	g = RGB_GREEN(c);
	b = RGB_BLUE(c);
	r = (r + g + b);
	r /= 3;
	return r;
}

RGBColor complementary_color(RGBColor c) {
	int r, g, b, a;
	r = RGB_RED(c);
	g = RGB_GREEN(c);
	b = RGB_BLUE(c);
	a = RGB_ALPHA(c);
	r = 255 - r;
	g = 255 - g;
	b = 255 - b;
	return RGBA_NO_CHECK(r, g, b, a);
}

ttfont* ttf_from_font(const Font* font_in) {
	NOT_NULL_OR_RETURN(font_in, nullptr);
	return font_in -> font;
}

/*** end drawing.cpp ***/
