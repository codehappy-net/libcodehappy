/***

	ref.h

	References to rectangular areas. Also points and sqrects (arbitrary arrays of rectangular references). Use to define
	regions on the screen, on a grid, etc.

	Copyright (c) 2022 C. M. Street.

***/
#ifndef POINT_RECT_SQRECT
#define POINT_RECT_SQRECT

struct point {
	i32 x;
	i32 y;
};

struct rect {
	point first;
	point last;
};

struct sqrect {
	darray(rect)	regions;
	bool			valid;
};

/*** point API ***/
extern point point_from_xy(i32 x, i32 y);
extern i32 point_x(const point* p);
extern i32 point_y(const point* p);
extern bool points_equal(const point* p1, const point* p2);

/*** rect operations ***/

/* returns an empty region; useful for initialization */
extern rect rect_empty(void);

/* returns true iff the rect represents an empty region */
extern bool rect_is_empty(const rect* prect);

/* returns true iff the rect represents a single point */
extern bool rect_is_point(const rect* r);

/* return the bounds of the rect -- these only make sense if !rect_empty(r) */
extern i32 rect_xmin(const rect* r);
extern i32 rect_xmax(const rect* r);
extern i32 rect_ymin(const rect* r);
extern i32 rect_ymax(const rect* r);
extern void rect_get_bounds(const rect* r, i32* x1, i32* x2, i32* y1, i32* y2);

/* return the dimensions of the rect */
extern u32 rect_xsize(const rect* r);
extern u32 rect_ysize(const rect* r);
extern u64 rect_area(const rect* r);

/* set the bounds of the rect */
extern void rect_set(rect* r, i32 x1, i32 y1, i32 x2, i32 y2);
extern void rect_point(rect* r, point* pt);
extern void rect_points(rect* r, point* p1, point* p2);

/* are the two rects equal? */
extern bool rects_equal(const rect* r1, const rect* r2);

/* return the result of intersecting two rects -- will be an empty rect if there is no intersection */
extern void rect_isect(const rect* r1, const rect* r2, rect* rect_result);

/* this checks that rect_contained is fully inside rect_containing */
extern bool rect_contains_rect(const rect* rect_containing, const rect* rect_contained);

/* is the sqrect entirely within r? */
extern bool rect_contains_sqrect(const rect* r, const sqrect* sr);

/* this returns true iff r1 and r2 have a non-NULL intersection */
extern bool rects_overlap(const rect* r1, const rect* r2);

/* more contains operations */
extern bool rect_contains_x(const rect* prect, i32 x);
extern bool rect_contains_y(const rect* prect, i32 y);
extern bool rect_contains_point(const rect* prect, const point* pt);

/* if the two regions are the same shape (i.e. the same size) */
extern bool rects_same_shape(const rect* r1, const rect* r2);

/* if the two rects are unionable (i.e. they can be directly concatenated into a single rect) */
extern bool rects_unionable(const rect* r1, const rect* r2);

/* if the input rects are unionable, put the union in rout and return true, else return false */
extern bool rects_union(const rect* rin1, const rect* rin2, rect* rout);

/* Return the region obtained by removing r2 from r1. */
extern void rects_subtract(const rect* r1, const rect* r2, sqrect* srout);

/* return an empty sqrect */
extern void sqrect_empty(sqrect* srout);

/* initialize a malloced sqrect */
extern void sqrect_init(sqrect* sr);

/* return a sqrect containing a single rect */
extern void sqrect_from_rect(const rect* r, sqrect* srout);

/* return the number of rectangular regions in a sqrect */
extern u32 sqrect_nregions(const sqrect* sr);

/* add a new rect to a sqrect */
extern void sqrect_addrect(sqrect* sr, const rect* r);

/* does the passed in sqrect represent an empty region? */
extern bool sqrect_is_empty(const sqrect* sr);

/* put the bounding rect for the sqrect in rout */
extern void sqrect_boundingrect(const sqrect* sr, rect* rout);

/* does the rect intersect with the sqrect? */
extern bool sqrect_rect_has_isect(const sqrect* sr, const rect* r);

/* return the intersection of a sqrect and a rect in srout (empty if none) */
extern void sqrect_rect_isect(const sqrect* sr, const rect* r, sqrect* srout);

/* add a sqrect to a sqrect -- does not empty sr1_inout first */
extern void sqrect_add(sqrect* sr1_inout, const sqrect* sr2);

/* return the union of two sqrects */
extern void sqrect_union(const sqrect* sr1, const sqrect* sr2, sqrect* srout);

/* return the intersection of two sqrects */
extern void sqrect_isect(const sqrect* sr1, const sqrect* sr2, sqrect* srout);

/* Make a copy of the input sqrect */
extern void sqrect_copy(const sqrect* sr_in, sqrect* sr_out);

/* Return a compacted version of the input sqrect -- no point will be present in more than one region. All unionable rects will be unioned,
	and overlapping rects will be split or removed. */
extern void sqrect_compact(const sqrect* sr_in, sqrect* sr_out);

/* Return an array of n allocated sqrects. Better to use this than NEW_ARRAY() since it initializes the sqrects for you. */
extern sqrect* sqrect_new_array(u32 n);

/* Return a single allocated and properly initialized sqrect. */
#define		sqrect_new()	sqrect_new_array(1UL)

/* Return the minimum x value in the sqrect. */
extern i32 sqrect_minx(const sqrect* sr);

/* Return the maximum x value in the sqrect. */
extern i32 sqrect_maxx(const sqrect* sr);

/* Return the minimum y value in the sqrect. */
extern i32 sqrect_miny(const sqrect* sr);

/* Return the maximum y value in the sqrect. */
extern i32 sqrect_maxy(const sqrect* sr);

/* Return the total area of the sqrect -- calculated as sum of the child rects. If assume_compact is true, does not do compaction first. */
extern u64 sqrect_area(const sqrect* sr, bool assume_compact);

/* Subtract a rect from a sqrect and return the remaining region. */
extern void sqrect_subtract_rect(const sqrect* sr, const rect* r, sqrect* srout);

/* Subtract sqrect sr2 from sqrect sr1 and return the remaining region. */
extern void sqrect_subtract(const sqrect* sr1, const sqrect* sr2, sqrect* srout);

/* Does the sqrect contain every point of the passed-in rect? Handles the case where the rect crosses over multiple regions of the sqrect. */
extern bool sqrect_contains_rect(const sqrect* sr, const rect* r);

/* Does sqrect sr_containing contain every point of sr_contained? */
extern bool sqrect_contains(const sqrect* sr_containing, const sqrect* sr_contained);

/* Do the two sqrects represent the exact same area? */
extern bool sqrects_equal(const sqrect* sr1, const sqrect* sr2);

/* Free memory owned by the sqrect. */
extern void sqrect_free(sqrect* sr);

#endif  // POINT_RECT_SQRECT
/* end ref.h */