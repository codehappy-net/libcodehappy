/***

	drawing.h

	Useful drawing declarations for libcodehappy.

	Copyright (c) Chris Street, 2022

***/
#ifndef	__DRAWING_H
#define __DRAWING_H

/* 24 bit RGB or 32 bit RGBA color */
typedef u32 RGBColor;
/* 16 bit (5-6-5) RGB color */
typedef u16 RGB565;
class RamFile;

/*** Pattern callback for drawing functions. ***/
typedef RGBColor (*PatternCallback)(int, int, void *);

/*** Bitmap and drawing types ***/
struct SPalette {
	SPalette();
	SPalette(u32 nclrs);
	~SPalette();

	/* Get the closest match to "c" from this palette. */
	u32 index_from_rgb(RGBColor c);

	/* Read from or write to ramfile. */
	void to_ramfile(RamFile* rf) const;
	void from_ramfile(RamFile* rf);

	u32 ncolors;
	RGBColor* clrs;
};

enum PointType {
	POINT_PIXEL = 0,	/* The point is expressed in pixels, with the default coordinates, (0, 0) at upper left */
	POINT_PERCENT,		/* Expressed as a percentage. */
	POINT_CENTER,		/* Pixels, but the coordinate system has (0, 0) at the center of the bitmap/SCoord */
	POINT_MILLES,		/* Like a percentage, but parts per thousand instead of parts per hundred. */
	POINT_PIXEL_REV,	/* Pixels, but counting from right to left or from bottom to top. */
	POINT_PERCENT_REV,	/* Percentage, but counting from right to left or from bottom to top. */
	POINT_MILLES_REV,	/* Milles, but counting from right to left or from bottom to top. */
};

/* Forward declarations. */
class SBitmap;
class Font;
struct ttfont;
class SCoord;

/* Point and coordinate objects ***/
struct SPoint {
public:
	SPoint();
	SPoint(int xx, int yy);
	SPoint(int xx, PointType xxt, int yy, PointType yyt);

	int X(const SBitmap* bmp = nullptr) const;
	int Y(const SBitmap* bmp = nullptr) const;
	// Allow POINT_PERCENTs to be used on SCoords, as well as SBitmaps.
	int X(const SCoord& co, const SBitmap* bmp = nullptr) const;
	int Y(const SCoord& co, const SBitmap* bmp = nullptr) const;

	void Flatten(const SBitmap* bmp = nullptr);
	void Flatten(const SCoord& co, const SBitmap* bmp = nullptr);

	// Add or subtract SPoints together.
	SPoint operator+(const SPoint& rhs) const;
	SPoint operator-(const SPoint& rhs) const;
	SPoint operator+=(const SPoint& rhs);
	SPoint operator-=(const SPoint& rhs);

	// Serialization.
	void to_ramfile(RamFile* rf) const;
	void from_ramfile(RamFile* rf);

	int x, y;
	PointType xt, yt;

private:
	// ensure xt/yt match for both this and p.
	void MatchTo(SPoint& p, SBitmap* bmp = nullptr);
};

struct SCoord {
public:
	SCoord();
	SCoord(const SPoint& p1_, const SPoint& p2_);
	SCoord(int x1, int y1, int x2, int y2);
	SCoord(int x1, PointType x1t, int y1, PointType y1t, int x2, PointType x2t, int y2, PointType y2t);

	int X1(const SBitmap* bmp = nullptr) const;
	int X2(const SBitmap* bmp = nullptr) const;
	int Y1(const SBitmap* bmp = nullptr) const;
	int Y2(const SBitmap* bmp = nullptr) const;
	int center_X(const SBitmap* bmp = nullptr) const;
	int center_Y(const SBitmap* bmp = nullptr) const;
	int dx(const SBitmap* bmp = nullptr) const;
	int dy(const SBitmap* bmp = nullptr) const;
	int width(const SBitmap* bmp = nullptr) const  { return dx(bmp) + 1; }
	int height(const SBitmap* bmp = nullptr) const { return dy(bmp) + 1; }
	bool Contains(int x, int y, SBitmap* bmp = nullptr) const;
	bool Contains(const SPoint& p, SBitmap* bmp = nullptr) const;
	bool Isect(const SCoord& c2, SCoord& result, SBitmap* bmp = nullptr) const;
	void Flatten(const SBitmap* bmp);
	void min_height(u32 h, const SBitmap* bmp = nullptr);
	void min_width(u32 w, const SBitmap* bmp = nullptr);

	// Adjust an SCoord by adding or subtracting an SPoint.
	SCoord operator+(const SPoint& rhs) const;
	SCoord operator-(const SPoint& rhs) const;
	SCoord operator+=(const SPoint& rhs);
	SCoord operator-=(const SPoint& rhs);

	// Set an SCoord to the rect_bitmap() by "assigning" an SBitmap to it.
	SCoord operator=(const SBitmap* rhs);

	// Serialization.
	void to_ramfile(RamFile* rf) const;
	void from_ramfile(RamFile* rf);

	SPoint p1, p2;
};

extern SCoord operator-(const SPoint& lhs, const SCoord& rhs);
extern SCoord operator+(const SPoint& lhs, const SCoord& rhs);

struct SubBitmapData {
	SBitmap* parent;
	SCoord	co;
};

enum BitmapType {
	BITMAP_DEFAULT = 0,			/* 32 bit bitmap compatible with SDL format. */
	BITMAP_MONO,				/* 1-bpp bitmap: 0 = black, 1 = white */
	BITMAP_PALETTE,				/* Palettized bitmap, with a palette of up to 256 colors */
	BITMAP_GRAYSCALE,			/* 8-bpp bitmap, with 256 levels of gray. */
	BITMAP_GREYSCALE = BITMAP_GRAYSCALE,	/* Alternate name. */
	BITMAP_16BITS,				/* 16-bpp (5-6-5) bitmap */
	BITMAP_24BITS,				/* 24-bpp bitmap */
	BITMAP_DISPLAY_OWNED,			/* Special type: 32bpp, doesn't own its bits array. */
	BITMAP_SUBBITMAP,			/* Special type: this bitmap is actually a sub-bitmap of its parent. */
	BITMAP_INVALID
};

/* putpixel/getpixel callback types */
class SBitmap;
typedef void (*PutPixelFn)(SBitmap*, u32, u32, RGBColor);
typedef RGBColor (*GetPixelFn)(const SBitmap*, u32, u32);

/*** Alpha definitions ***/
#define	ALPHA_OPAQUE		0xFF
#define	ALPHA_HALF		0x80
#define	ALPHA_TRANSPARENT	0x00

/*** Centering and alignment ***/
#define CENTERED_HORIZ		1
#define CENTERED_VERT		2
#define CENTERED_BOTH		(CENTERED_HORIZ | CENTERED_VERT)
#define ALIGN_LEFT		4
#define ALIGN_TOP		8
#define ALIGN_BOTTOM		16
#define ALIGN_RIGHT		32
#define ALIGN_UPPER_LEFT	(ALIGN_LEFT | ALIGN_TOP)
#define ALIGN_UPPER_RIGHT	(ALIGN_RIGHT | ALIGN_TOP)
#define ALIGN_LOWER_LEFT	(ALIGN_BOTTOM | ALIGN_LEFT)
#define ALIGN_LOWER_RIGHT	(ALIGN_BOTTOM | ALIGN_RIGHT)
#define	SIDE_TOP		64
#define SIDE_BOTTOM		128
#define SIDE_LEFT		256
#define SIDE_RIGHT		512

/*** Channel flags. ***/
#define	CHANNEL_RED		0xff
#define CHANNEL_GREEN		0xff00
#define CHANNEL_BLUE		0xff0000
#define CHANNEL_ALPHA		0xff000000
#define CHANNEL_FIRST		CHANNEL_RED
#define CHANNEL_SECOND		CHANNEL_GREEN
#define CHANNEL_THIRD		CHANNEL_BLUE
#define CHANNEL_FOURTH		CHANNEL_ALPHA

/* "S" is for "Simple", but the functionality is anything but! */
class SBitmap {
public:
	// Creates an empty bitmap.
	SBitmap();
	// Creates a bitmap of specified size and 32bpp.
	SBitmap(u32 width, u32 height);
	// Creates a bitmap of specified size and type.
	SBitmap(u32 width, u32 height, BitmapType typ);
	// Creates a bitmap whose pixel data is not owned by this object.
	SBitmap(u32 width, u32 height, u8* data);
	~SBitmap();

	/* Accessors. */
	u32 height(void) const { return h; }
	u32 width(void) const { return w; }
	BitmapType type(void) const { return btype; }
	SPalette* palette(void) const;
	void set_palette(SPalette* spal);

	/* Pixel primitives. */
	void put_pixel(u32 x, u32 y, RGBColor c);
	void put_pixel(const SPoint& p, RGBColor c);
	RGBColor get_pixel(u32 x, u32 y) const;
	RGBColor get_pixel(const SPoint& p) const;
	/* Signed int versions. */
	void put_pixel(int x, int y, RGBColor c);
	RGBColor get_pixel(int x, int y) const;
	void put_pixel_palette(int x, int y, u32 idx);
	int get_pixel_palette(int x, int y) const;
	/* Get pixel with the alpha mask (if present) removed. */
	RGBColor get_pixel_rgb(int x, int y) const;
	RGBColor get_pixel_rgb(const SPoint& p) const;
	/* Put pixel, including the alpha channel. (put_pixel() in 32bpp preserves any existing
		alpha channel.) Distinct from put_pixel() only in 32bpp bitmaps. */
	void put_pixel_alpha(int x, int y, RGBColor c);
	void put_pixel_alpha(const SPoint& p, RGBColor c);

	/* Rectangles. */
	void rect(int x1, int y1, int x2, int y2, RGBColor c);
	void rect_fill(int x1, int y1, int x2, int y2, RGBColor c);
	void rect_fill_pattern(int x1, int y1, int x2, int y2, PatternCallback callback, void* args);
	void rect(const SPoint& p1, const SPoint& p2, RGBColor c);
	void rect(const SCoord& co, RGBColor c);
	void rect_fill(const SPoint& p1, const SPoint& p2, RGBColor c);
	void rect_fill(const SCoord& co, RGBColor c);
	void rect_fill_pattern(const SPoint& p1, const SPoint& p2, PatternCallback callback, void* args);
	void rect_fill_pattern(const SCoord& co, PatternCallback callback, void* args);
	void rect_alpha(int x1, int y1, int x2, int y2, RGBColor c);
	void rect_alpha(const SPoint& p1, const SPoint& p2, RGBColor c);
	void rect_alpha(const SCoord& co, RGBColor c);
	void rect_fill_alpha(int x1, int y1, int x2, int y2, RGBColor c);
	void rect_fill_alpha(const SPoint& p1, const SPoint& p2, RGBColor c);
	void rect_fill_alpha(const SCoord& co, RGBColor c);
	void clear(void);
	void clear(RGBColor c);
	void clear_pattern(PatternCallback callback, void* args);
	void clear_alpha(void);
	void clear_alpha(RGBColor c);
	void fill_static(bool gray = false);

	/* Functions that manipulate the alpha channel only (these only function on 32bpp and subbitmaps of 32bpp, for other
		bitmap types they will be no-ops.) */
	u32 get_alpha(int x, int y);
	u32 get_alpha(const SPoint& p);
	void set_alpha(int x, int y, u32 alp);
	void set_alpha(const SPoint& p, u32 alp);
	void copy_alpha_rect(SBitmap* bmp_from, const SCoord& region_from, const SPoint& point_upper_left_dest);
	void negative_alpha(void);
	bool all_opaque(void);

	/* Functions that manipulate the red, blue, or green channels specifically. These are available to all 
		bitmap types. */
	/* (You can convert SBitmaps to other color spaces, like HSV. In this case, "red" is the first dimension
	   in the name, "green" is the second, and "blue" is the third channel.) */
	u32 get_channel(int x, int y, u32 channel_flags);
	u32 get_channel(const SPoint& pt, u32 channel_flags);
	void set_channel(int x, int y, u32 channel_flags, u32 val);
	void set_channel(const SPoint& pt, u32 channel_flags, u32 val);
	void copy_channel_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest, u32 channel_flags);
	void negative_channel(u32 channel_flags);
	u32 get_red(int x, int y);
	void set_red(int x, int y, u32 val);
	u32 get_red(const SPoint& p);
	void set_red(const SPoint& p, u32 val);
	void copy_red_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest);
	void negative_red(void);
	u32 get_green(int x, int y);
	void set_green(int x, int y, u32 val);
	u32 get_green(const SPoint& p);
	void set_green(const SPoint& p, u32 val);
	void copy_green_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest);
	void negative_green(void);
	u32 get_blue(int x, int y);
	void set_blue(int x, int y, u32 val);
	u32 get_blue(const SPoint& p);
	void set_blue(const SPoint& p, u32 val);
	void copy_blue_rect(SBitmap* bitmap_from, const SCoord& region_from, const SPoint& pt_dest);
	void negative_blue(void);

	/* Returns a sub-bitmap of the current bitmap. The sub-bitmap acts as a window on the parent bitmap, and
		has no allocated bits of its own; drawing operations on the sub-bitmap are clipped to within its borders. */
	SBitmap* subbitmap(int x1, int y1, int x2, int y2);
	SBitmap* subbitmap(const SPoint& p1, const SPoint& p2);
	SBitmap* subbitmap(const SCoord& co);
	SBitmap* subbmp_left_half(void);
	SBitmap* subbmp_right_half(void);
	SBitmap* subbmp_top_half(void);
	SBitmap* subbmp_bottom_half(void); 
	SBitmap* subbmp_ul_quarter(void);
	SBitmap* subbmp_ur_quarter(void);
	SBitmap* subbmp_ll_quarter(void);
	SBitmap* subbmp_lr_quarter(void);
	SBitmap* subbmp_top_third(void);
	SBitmap* subbmp_middlev_third(void);
	SBitmap* subbmp_bottom_third(void);
	SBitmap* subbmp_left_third(void);
	SBitmap* subbmp_middleh_third(void);
	SBitmap* subbmp_right_third(void);
	SBitmap* subbmp_complement(SBitmap* bmp); 	 	
	SubBitmapData* subbmpdata() const { return sbd; }

	/* Copies. */
	SBitmap* copy(void) const;
	void copy_palette(SBitmap* dest) const;
	void swap(SBitmap* bmp);

	/* Transposition, and reflections in place. */
	SBitmap* transpose(void);
	bool transpose_and_replace(void);
	void reflect_xy(void);
	void flip_horiz(void);
	void flip_vert(void);

	/*** Color effects and filters. ***/
	void fadeout(u32 fade);
	SBitmap* gaussianblur(void);
	void blendpixel(int x, int y, RGBColor c, u32 intensity);
	void temperature(int temp);
	void tint(int tint);
	SBitmap* line_art_filter(u32 tol);
	void brighten_darken(int adjustment);
	SBitmap* heighten_edges(void);

	/* Blitting. */
	void blit_u(u32 x1_src, u32 y1_src, u32 x2_src, u32 y2_src, SBitmap* bmp_dest, u32 x_dest, u32 y_dest) const;
	void blit(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest, int x_dest, int y_dest) const;
	void blit(const SCoord& src, SBitmap* bmp_dest, const SPoint& dest) const;
	void blit(SBitmap* bmp_dest, int x_dest, int y_dest) const;
	void blit(SBitmap* bmp_dest, const SPoint& dest) const;
	void blit(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest) const;
	void blit(const SCoord& src, SBitmap* bmp_dest) const;
	void blit(SBitmap* bmp_dest) const;
	void blit(SBitmap* bmp_dest, int x_src, int y_src, int x_dest, int y_dest, int blit_w, int blit_h) const;
	/* With alpha blend. */
	void blit_blend_u(u32 x1_src, u32 y1_src, u32 x2_src, u32 y2_src, SBitmap* bmp_dest, u32 x_dest, u32 y_dest) const;
	void blit_blend(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest, int x_dest, int y_dest) const;
	void blit_blend(const SCoord& src, SBitmap* bmp_dest, const SPoint& dest) const;
	void blit_blend(SBitmap* bmp_dest, int x_dest, int y_dest) const;
	void blit_blend(SBitmap* bmp_dest, const SPoint& dest) const;
	void blit_blend(int x1_src, int y1_src, int x2_src, int y2_src, SBitmap* bmp_dest) const;
	void blit_blend(const SCoord& src, SBitmap* bmp_dest) const;
	void blit_blend(SBitmap* bmp_dest) const;
	/* Stretch blits. This format is used by the Allegro library. */
	void stretch_blit(SBitmap* bmp_dest, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h) const;

	/* Center crop to a square of the specified dimensions. (size == 0 to preserve the smaller dimension.) */
	SBitmap* center_crop(u32 size) const;
	bool center_crop_and_replace(u32 size);

	/* Lines. The general purpose line functions can take ints as coordinates; this allows for plotting a 
		line from points off-bitmap. The plotting functions all check bounds. */
	void hline(int x1, int x2, int y, RGBColor c);
	void vline(int x, int y1, int y2, RGBColor c);
	void line(int x1, int y1, int x2, int y2, RGBColor c);
	void line(const SPoint& p1, const SPoint& p2, RGBColor c);
	void line_pattern(int x1, int y1, int x2, int y2, PatternCallback callback, void* args);
	void line_pattern(const SPoint& p1, const SPoint& p2, PatternCallback callback, void* args);

	/* Shapes. */
	void circle(int x, int y, u32 radius, RGBColor c);
	void circle(const SPoint& p, u32 rad, RGBColor c);
	void fillcircle(int x, int y, u32 radius, RGBColor c);
	void fillcircle(const SPoint& p, u32 rad, RGBColor c);
	void fillcirclepattern(int x, int y, u32 radius, PatternCallback callback, void* args);
	void fillcirclepattern(const SPoint& p, u32 radius, PatternCallback callback, void* args);
	void polygon(u32 npoints, int* xpoints, int* ypoints, RGBColor c);
	void polygon(u32 npoints, SPoint* points, RGBColor c);
	void fillpolygon(u32 npoints, int* xpoints, int* ypoints, RGBColor c);
	void fillpolygon(u32 npoints, SPoint* points, RGBColor c);
	void fillpolygon_pattern(u32 npoints, int* xpoints, int* ypoints, PatternCallback pattern_callback, void* args);
	void fillpolygon_pattern(u32 npoints, SPoint* points, PatternCallback pattern_callback, void* args);
	void ellipse(int x_center, int y_center, int width_e, int height_e, RGBColor c);
	void ellipse(const SPoint& p_center, int width_e, int height_e, RGBColor c);
	void ellipserect(int x0, int y0, int x1, int y1, RGBColor c);
	void ellipserect(const SPoint& p1, const SPoint& p2, RGBColor c);
	void ellipserect(const SCoord& co, RGBColor c);
	void fillellipse(int x_center, int y_center, int width_e, int height_e, RGBColor c);
	void fillellipse_pattern(int x_center, int y_center, int width_e, int height_e, PatternCallback pattern_callback, void* args);
	void rotatedellipserect(int x0, int y0, int x1, int y1, long zd, RGBColor c);
	void rotatedellipse(int x, int y, int a, int b, float angle, RGBColor c);
	void regular_polygon(int x_center, int y_center, u32 nsides, double radius, double angle_rotate_rad, RGBColor rgb);
	void regular_polygon(const SPoint& center, u32 nsides, double radius, double angle_rotate_rad, RGBColor rgb);

	/*** Image file format support. Supports JPG, PNG, SVG, TGA, BMP, GIF, PCX, etc. ***/
	static SBitmap* load_bmp(const char* fname);
	static SBitmap* load_bmp(const std::string& fname);
	static SBitmap* load_bmp(RamFile* rf);
	static SBitmap* load_svg(const char* fname, float scale);
	static SBitmap* load_svg(const std::string& fname, float scale);
	/* Render the vector graphics (SVG format) to the desired width/height -- only one should be non-zero,
		the other will be set by aspect ratio. (Both width and height equal to 0 give the default size.)
		Returns the rendered SBitmap. */
	static SBitmap* render_svg(const char* fname, u32 width, u32 height);
	static SBitmap* render_svg(const std::string& fname, u32 width, u32 height);
	static SBitmap* render_svg_from_mem(char* membuf, u32 width, u32 height);
	u32 save_bmp(const char* fname);
	u32 save_bmp(const std::string& fname);
	/* these methods load or save to the .rfi RamFile format -- to load an image directly from a RamFile of a .jpg etc., you can use load_bmp() above. */
	static SBitmap* from_ramfile(RamFile* rf);
	static SBitmap* from_ramfile(const char* fname);
	static SBitmap* from_ramfile(const std::string& fname);
	u32 to_ramfile(RamFile* rf);
	u32 to_ramfile(const char* fname);
	u32 to_ramfile(const std::string& fname);

	/*** Address of a pixel. ***/
	u8* pixel_loc(u32 x, u32 y) const;
	u8* pixel_loc(const SPoint& p) const;

	/*** Fill the alpha channel of a 32bpp bitmap. ***/
	void fill_alpha_channel(int val);
	void alpha_opaque(void)		{ fill_alpha_channel(ALPHA_OPAQUE); }
	void alpha_transparent(void)	{ fill_alpha_channel(ALPHA_TRANSPARENT); }

	/*** Creates an RGB negative of the bitmap. ***/
	void negative(void);

	/*** For all pixels of the specified color, set the alpha channel to transparent. ***/
	void set_transparent_color(RGBColor c);

	/*** Get a random point or random rect entirely within the current bitmap. ***/
	void random_pixel(SPoint* p) const;
	void random_rect(SCoord* co) const;

	/*** Returns a new bitmap that is the input bitmap rotated 90 degrees clockwise (or counterclockwise). ***/
	SBitmap* rotate_clockwise_90() const;
	SBitmap* rotate_counterclockwise_90() const;

	/*** Fills an SCoord ref to cover the entire bitmap draw/window area. ***/
	void rect_bitmap(SCoord* co) const;

	/*** Get the top-level SBitmap and (x, y) coordinate for an SPoint or SCoord. ***/
	SBitmap* top_level(const SPoint& p, SPoint* p_out);
	SBitmap* top_level(const SCoord& co, SCoord* co_out);
	SBitmap* top_level(void);

	/*** Resizing bitmaps. If one of new_width/new_height are 0, it is set to maintain the aspect ratio of the original bitmap. ***/
	SBitmap* resize(u32 new_width, u32 new_height) const;
	SBitmap* scale_rational(u32 num, u32 den) const;
	bool resize_and_replace(u32 new_width, u32 new_height);
	bool scale_rational_and_replace(u32 num, u32 den);
	SBitmap* aspect_ratio(u32 aw, u32 ah);
	bool aspect_ratio_replace(u32 aw, u32 ah);

	/*** Dotted line functions. r1/r2 of pixels will be plotted. ***/
	void hline_dotted(int x1, int x2, int y, RGBColor c, u32 r1 = 3, u32 r2 = 5);
	void vline_dotted(int x, int y1, int y2, RGBColor c, u32 r1 = 3, u32 r2 = 5);
	void rect_dotted(int x1, int y1, int x2, int y2, RGBColor c, u32 r1 = 3, u32 r2 = 5);
	void line_dotted(int x1, int y1, int x2, int y2, RGBColor c, u32 r1 = 3, u32 r2 = 5);

	/*** Flood fills. ***/
 	void floodfill_stopclr(int x, int y, RGBColor fill_clr, RGBColor stop_clr);
	void floodfill_contclr(int x, int y, RGBColor fill_clr, RGBColor cont_clr);
	void floodfill(int x, int y, RGBColor fill_clr);
	void patternfill(int x, int y, PatternCallback pattern_callback, void* args);
 	void floodfill_stopclr(const SPoint& p, RGBColor fill_clr, RGBColor stop_clr);
	void floodfill_contclr(const SPoint& p, RGBColor fill_clr, RGBColor cont_clr);
	void floodfill(const SPoint& p, RGBColor fill_clr);
	void patternfill(const SPoint& p, PatternCallback pattern_callback, void* args);

	/*** Anti-aliased drawing primitives. ***/
	void aaline(int x0, int y0, int x1, int y1, RGBColor c);
	void aacircle(int xm, int ym, int r, RGBColor c);
	void aathickline(int x0, int y0, int x1, int y1, float wd, RGBColor c);
	void aaline(const SPoint& p1, const SPoint& p2, RGBColor c);
	void aacircle(const SPoint& p, int r, RGBColor c);
	void aathickline(const SPoint& p1, const SPoint& p2, float wd, RGBColor c);

	/*** Bitfont functions. ***/
	void putstr_bitfont(const char* str, int x, int y, int size, RGBColor c);
	void putstr_bitfont_bg(const char* str, int x, int y, int size, RGBColor text_c, RGBColor bg_c);
	void putch_bitfont(int ch, int x, int y, int s, RGBColor c, int bg);

	/*** Bezier curves. ***/
	void quadbezier(int x0, int y0, int x1, int y1, int x2, int y2, RGBColor c);
	void quadbezier(SPoint* pts, RGBColor c);
	void quadbezier(const SPoint& p1, const SPoint& p2, const SPoint& p3, RGBColor c);
	void cubicbezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, RGBColor c);
	void cubicbezier(SPoint* pts, RGBColor c);
	void cubicbezier(const SPoint& p1, const SPoint& p2, const SPoint& p3, const SPoint& p4, RGBColor c);

	/*** Color gradients. ***/
	void fillrect_gradient(int x1, int y1, int x2, int y2, RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right);
	void fillrect_gradient(const SCoord& region, RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right);
	void fillbmp_gradient(RGBColor upper_left, RGBColor upper_right, RGBColor lower_left, RGBColor lower_right);

	/*** Draw a line with the specified thickness from (x1, y1) to (x2, y2) with the specified color. If antialiasing is true,
		the edges will be anti-aliased. Fractional pixels and fractional thickness are all right with this function, and with anti-aliasing,
		will look pretty good. Written for pretty output, not so much speed. ***/
	void drawthickline(double x1, double y1, double x2, double y2, double thickness, RGBColor rgb, bool antialiasing);
	void drawthickline(int x1, int y1, int x2, int y2, double thickness, RGBColor rgb, bool antialiasing);
	void drawthickline(const SPoint& p1, const SPoint& p2, double thickness, RGBColor rgb, bool antialiasing);
	void drawthickline(const SCoord& co, double thickness, RGBColor rgb, bool antialiasing);
	void drawthickline_pattern(double x1, double y1, double x2, double y2, double thickness, PatternCallback pattern, void* args, bool antialiasing);
	void drawthickline_pattern(int x1, int y1, int x2, int y2, double thickness, PatternCallback pattern, void* args, bool antialiasing);
	void drawthickline_pattern(const SPoint& p1, const SPoint& p2, double thickness, PatternCallback pattern, void* args, bool antialiasing);
	void drawthickline_pattern(const SCoord& co, double thickness, PatternCallback pattern, void* args, bool antialiasing);

	/* Text. Can be rendered using the bitfont (if nullptr Font/ttfont is given), can be centered (horizontally or vertically
		or both), or aligned (left, right, top, bottom, or combinations.) */
	void render_text(const char* str, const SCoord& rect, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const char* str, const SCoord& rect, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const std::string& str, const SCoord& rect, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const std::string& str, const SCoord& rect, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const char* str, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const char* str, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const std::string& str, Font* f, RGBColor c, u32 desired_height, u32 center_align_flags);
	void render_text(const std::string& str, ttfont* f, RGBColor c, u32 desired_height, u32 center_align_flags);

	/* Distance between two bitmaps. The two bitmaps must be the same dimensions. */
	/* Uses 1-D earth mover's distance, full calculation. */
	u64 distance_emd(const SBitmap* b2) const;
	/* Intensity by index -- 0-2 RGB at (0,0), 3-5 RGB at (0,1), etc. */
	u32 intensity_by_index(u32 i) const;

	/* A hash function representing the content of the bitmap; use it to look for duplicates. */
	u64 hash() const;

	/* Make the default put_pixel function set the alpha channel by default (normally, 32bpp put_pixel doesn't
	   affect the alpha channel -- this is to allow easier editing of sprites, etc.) No-op on non-32bpp bitmaps. */
	void putpixel_affects_alpha(bool does_affect);
	void putpixel_affects_alpha_toggle();
	/* Does the default put_pixel function change the alpha channel? */
	bool putpixel_affects_alpha() const;

	/* add some noise of the specified magnitude to the RGB components of the color at (x, y), or over the entire bitmap. */
	void noise_rgb(int x, int y, int mag);
	void noise_rgb(int mag);

private:
	/* Built-in putpixel/getpixel functions. */
	static void put_pixel_32bpp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_24bpp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_16bpp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_8bpp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_grey(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_1bpp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_subbmp(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_default(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static void put_pixel_32bpp_alpha(SBitmap* sb, u32 x, u32 y, RGBColor c);
	static RGBColor get_pixel_32bpp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_24bpp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_16bpp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_8bpp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_grey(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_1bpp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_subbmp(const SBitmap* sb, u32 x, u32 y);
	static RGBColor get_pixel_default(const SBitmap* sb, u32 x, u32 y);

	/* Helper functions. */
	void line_helper(int x1, int y1, int x2, int y2, int pred, int incdec, RGBColor c, PatternCallback callback, void* args);
	static SBitmap* load_raw(const char* szFile);
	static int save_raw(SBitmap* bmp, const char* szFile);
	static SBitmap* render_svg_core(void* image, u32 width, u32 height);
	void out_channel_rf(RamFile* rf, u32 idx);
	static void in_channel_rf(SBitmap* bmp, RamFile* rf, u32 idx);

	/* SBitmap data. */
	u8*		bits;
	u32		w;
	u32		h;
	BitmapType	btype;
	SPalette*	pal;
	SubBitmapData*	sbd;
	PutPixelFn	put_pixel_fn;
	GetPixelFn	get_pixel_fn;
};

#ifndef HAVE_STBTT_FONTINFO
typedef struct stbtt_fontinfo
{
   void           * userdata;
   unsigned char  * data;              // pointer to .ttf file
   int              fontstart;         // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph
} stbtt_fontinfo;
#endif  // !HAVE_STBTT_FONTINFO

/*** Unicode TrueType font data. ***/
struct ttfont {
	stbtt_fontinfo	info;
	unsigned char*	data;
};

/*** Built-in fonts. ***/
#include "fonts.h"

/*** The Font class represents a TrueType or custom font. It can render strings to SBitmaps. 
	Handles both regular 8-bit char strings and Unicode 32 bit strings. ***/
class Font {
public:
	Font(const char* fname);
	Font(const std::string& fname);
	Font(ttfont* builtin_font);
	Font(const ttfont& builtin_font);
	~Font();

	/* Returns an allocated grayscale SBitmap representing a specific codepoint in this font. */
	SBitmap* codepoint_bmp(int codepoint, int size);

	/* Returns a sprite (bitmap with transparency) representing a codepoint in this font. */
	SBitmap* codepoint_sprite(int codepoint, int size, RGBColor c);

	/*** Text rendering: in the functions below, single_character_blend() does some extra smoothing.
		For large text captions, it may be superior quality; at smaller font points it looks more blurry. ***/	
	/* Renders a C string and returns the allocated SBitmap. */
	SBitmap* render_cstr(const char* str, int size, bool single_character_blend, u16** char_index_to_x_pos);

	/* Renders a UTF-32 string and returns the allocated SBitmap. */
	SBitmap* render_ustr(ustring str, int size, bool single_character_blend, u16** char_index_to_x_pos);

	/* Blits a bitmap created by render_ustr()/render_cstr() onto the destination bitmap. */
	static void blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, int x, int y, RGBColor text_color);
	static void blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, int x, int y, PatternCallback callback, void* args);

	/* As above, but render the text in a region with centering/alignment flags. */
	static void blit(SBitmap* ttf_bmp, SBitmap* dest_bmp, const SCoord& region, RGBColor text_color, u32 center_align_flags);

	/* Fills this font with one of the built-in fonts. */
	void built_in(ttfont* builtin_font);

	/* Returns the point size to use to get a specified height. */
	u32 font_size_for_height(u32 height);

	/* Returns the width of a 'typical' 60 character string in this font at the specified point size. */
	u32 width60(u32 pt_size);

private:
	ttfont* font;
	bool	builtin;
};

/*** Helper: verify that this is a valid pixel for this bitmap. ***/
extern bool pixel_ok(const SBitmap* bmp, int x, int y);

/*** Other exported drawing functions. ***/
extern void perturb_color_randomly_even(RGBColor *c);
extern double line_at_x(double x1, double y1, double x2, double y2, double x);
extern double line_at_y(double x1, double y1, double x2, double y2, double y);
extern void normalize_vector(double* x, double* y);

#define	LINE_ALL_VALUES	(9999999.)
#define LINE_NO_VALUE	(-9999999.)

/* Create RGB or RGBA colors. */
#define	MAKE_RGB(r, g, b)	((((b) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((r) & 0xFF))
#define MAKE_RGBA(r, g, b, a)	(((a) << 24) | MAKE_RGB(r, g, b))

/* add an alpha channel value to an RGB color */
#define	ADD_ALPHA(rgb, a)	((rgb & 0xffffff) | ((a) << 24))

/* as MAKE_RGB() above, but with no check on range */
#define	RGB_NO_CHECK(r, g, b)	(RGBColor)(((b) << 16) + ((g) << 8) + (r))
#define RGBA_NO_CHECK(r,g,b,a)	(RGBColor)(((a) << 24) + ((b) << 16) + ((g) << 8) + (r))

/* get the individual components of an RGB color */
#define	RGB_BLUE(x)		(((x) >> 16) & 0xff)
#define	RGB_GREEN(x)		(((x) >> 8) & 0xff)
#define	RGB_RED(x)		((x) & 0xff)

/* allow this too */
#define	RGB_ALPHA(x)	(((x) >> 24) & 0xff)

/* swap the byte order of the color components, if necessary */
#define	RGB_TO_BGR(x)	((((x) >> 16) & 0xff) + ((x) & 0x00ff00) + (((x) << 16) & 0xff0000))

#define COMPONENT_RANGE(x)	(((x) > 255) ? 255 : (((x) < 0) ? 0 : (x)))

typedef u32 RGBColor;
typedef u32 RGBAColor;

/* create an RGB 32-bit color from 1/100ths intensity components */
#ifdef __GNUC__
/* do this in floating point... yes, I care about the errors in the simple integer approximation (component * 255 / 100) */
/* in GCC we can do this as a constexpr by using the built-in floor function. */
#define	SCALE100(val)		((int)__builtin_floor(((double)(val) * 255.0) / 100.0 + 0.5))
#define	RGB100(r, g, b)		RGB_NO_CHECK(SCALE100(r), SCALE100(g), SCALE100(b))
#else
/* even without built-in floor() functions, I can still name this tune in (a bunch of) integer arithmetic -- this
	is exactly the same integer approximation as the SCALE100() defined above, but written without any
	floating point; hooray for wizardry */
#define	SCALE100(v)		((((v) * 255) / 100) + ((v) & 1) - \
							zo1(ODD_BETWEEN(v, 11, 19) || ODD_BETWEEN(v, 31, 39) || \
								ODD_BETWEEN(v, 51, 59) || ODD_BETWEEN(v, 71, 79) || ODD_BETWEEN(v, 91, 99)) + \
							zo1(EVEN_BETWEEN(v, 10, 18) || EVEN_BETWEEN(v, 30, 38) || EVEN_BETWEEN(v, 50, 58) || \
								EVEN_BETWEEN(v, 70, 78) || EVEN_BETWEEN(v, 90, 98)))

/* as ridiculous as that is, with constant inputs, it is constant folded into zero code! */
#define	RGB100(r, g, b)		RGB_NO_CHECK(SCALE100(r), SCALE100(g), SCALE100(b))
#endif

/*** Some helper functions. ***/
/* extract the grayscale level of the given color */
extern u32 RGBColorGrayscaleLevel(RGBColor rgb);

/* Add a ridiculous number of color defines. */
#include "colors.i"

// use after sorting the x/y coordinates to determine the part of the rectangle that's within the bitmap
// Will return if the rectangle is completely outside the bmp boundaries.
#define	VALID_RECT(bmp, x1, y1, x2, y2)	\
	if (x1 < 0)					\
		{						\
		if (x2 < 0)				\
			return;				\
		x1 = 0;					\
		}						\
	if (y1 < 0)					\
		{						\
		if (y2 < 0)				\
			return;				\
		y1 = 0;					\
		}						\
	if (x2 >= bmp->width())		\
		{						\
		if (x1 >= bmp->width())	\
			return;				\
		x2 = bmp->width() - 1; 		\
		}						\
	if (y2 >= bmp->height())		\
		{						\
		if (y1 >= bmp->height())	\
			return;				\
		y2 = bmp->height() - 1;		\
		}

/*** Pattern fill settings. ***/
struct fillsettings {
	RGBColor	background;	/* the background color */
	RGBColor	foreground;	/* the foreground/pattern color */
	u32			size;		/* size in pixels of fill type */
};

/*** Some built-in pattern callbacks (to use with fills, etc.) follow. ***/
/*** The void* argument, in these cases, is a struct "fillsettings", containing the desired background and foreground colors for the
	pattern fill. That structure should be passed to patternfillbmp() etc. as the void* args argument. The fill structure may also contain
	other arguments, such as the size (for checkerboard squares, etc.) ***/
extern RGBColor diamond_pattern(int x, int y, void* fill_settings);
extern RGBColor checkerboard_pattern(int x, int y, void* fill_settings);

extern void zeromem(void* ptr, u32 num_bytes);
extern bool file_exists(const char* fname);
extern RGBColor RandColor(void);
extern bool is_img_extension(const char* fname);
/* Helper function: is the passed-in point inside the polygon specified by the array of points? */
extern bool point_in_polygon(const SPoint& p, SPoint* poly, u32 npoints, SBitmap* bmp = nullptr);
/* Convert a color to gray scale using the standard ITU-R 601-2 luma transform. */
extern RGBColor to_gray_luma_transform(RGBColor c);
/* Free static memory allocated by the library for performance/caching reasons. */
extern void libcodehappy_free_caches();

extern RGB565 RGB565FromRGBColor(RGBColor c);
extern RGBColor RGBColorFromRGB565(RGB565 c);
extern uint RGB565_red(RGB565 c);
extern uint RGB565_green(RGB565 c);
extern uint RGB565_blue(RGB565 c);
extern uint rgb_intensity(RGBColor c);
extern RGBColor complementary_color(RGBColor c);

#endif  // __DRAWING_H
