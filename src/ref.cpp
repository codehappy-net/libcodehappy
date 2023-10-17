/***

	ref.cpp

	References to rectangular areas. Also points and sqrects (arbitrary arrays of rectangular references). Use to define
	regions on the screen, on a grid, etc.

	Copyright (c) 2022 C. M. Street

***/

// TODO: consider the restrict keyword for some of these functions? Do we support the case where output pointer equals one of the inputs?

/*** point API ***/
point point_from_xy(i32 x, i32 y) {
	point pret;
	pret.x = x;
	pret.y = y;
	return(pret);
}

i32 point_x(const point* p) {
	NOT_NULL_OR_RETURN(p, 0);
	return p->x;
}

i32 point_y(const point* p) {
	NOT_NULL_OR_RETURN(p, 0);
	return p->y;
}

bool points_equal(const point* p1, const point* p2) {
	NOT_NULL_OR_RETURN(p1, is_null(p2));
	NOT_NULL_OR_RETURN(p2, false);
	return ((p1->y == p2->y) && (p1->x == p2->x));
}

/*** rect operations ***/

/* returns an empty region; useful for initialization */
rect rect_empty(void) {
	rect rret;
	rret.first.x = 1;
	rret.first.y = 0;
	rret.last.x = 0;
	rret.last.y = 0;
	return rret;
}

/* returns true iff the rect represents an empty region */
bool rect_is_empty(const rect* prect) {
	NOT_NULL_OR_RETURN(prect, true);
	return (prect->first.x > prect->last.x) || (prect->first.y > prect->last.y);
}

/* returns true iff the rect represents a single point */
bool rect_is_point(const rect* r) {
	NOT_NULL_OR_RETURN(r, true);
	return points_equal(&r->first, &r->last);
}

/* return the bounds of the rect -- these really only make sense if !rect_is_empty(r), though we try to do the right thing anyway */
i32 rect_xmin(const rect* r) {
	if (rect_is_empty(r))
		return(1);		/* since a NULL rect is treated logically like the return value of rect_empty() */
	return r->first.x;
}

i32 rect_xmax(const rect* r) {
	if (rect_is_empty(r))
		return(0);
	return r->last.x;
}

i32 rect_ymin(const rect* r) {
	if (rect_is_empty(r))
		return(0);
	return r->first.y;
}

i32 rect_ymax(const rect* r) {
	if (rect_is_empty(r))
		return(0);
	return r->last.y;
}

void rect_get_bounds(const rect* r, i32* x1, i32* x2, i32* y1, i32* y2) {
	if (rect_is_empty(r))
		return;
	if (not_null(x1))
		*x1 = rect_xmin(r);
	if (not_null(x2))
		*x2 = rect_xmax(r);
	if (not_null(y1))
		*y1 = rect_ymin(r);
	if (not_null(y2))
		*y2 = rect_ymax(r);
}

/* return the dimensions of the rect */
u32 rect_xsize(const rect* r) {
	if (rect_is_empty(r))
		return(0);
	return (u32)((r->last.x - r->first.x) + 1);
}

u32 rect_ysize(const rect* r) {
	if (rect_is_empty(r))
		return(0);
	return (u32)((r->last.y - r->first.y) + 1);
}

u64 rect_area(const rect* r) {
	return ((u64)rect_xsize(r)) * ((u64)rect_ysize(r));
}

/* set the bounds of the rect */
void rect_set(rect* r, i32 x1, i32 y1, i32 x2, i32 y2) {
	NOT_NULL_OR_RETURN_VOID(r);
	SORT2(x1, x2, i32);
	SORT2(y1, y2, i32);
	r->first = point_from_xy(x1, y1);
	r->last = point_from_xy(x2, y2);
}

void rect_point(rect* r, point* pt) {
	NOT_NULL_OR_RETURN_VOID(r);
	if (unlikely(is_null(pt)))
		{
		*r = rect_empty();
		return;
		}
	r->first = *pt;
	r->last = *pt;
}

void rect_points(rect* r, point* p1, point* p2) {
	i32 x1, x2, y1, y2;
	NOT_NULL_OR_RETURN_VOID(r);
	x1 = point_x(p1);
	x2 = point_x(p2);
	y1 = point_y(p1);
	y2 = point_y(p2);
	rect_set(r, x1, y1, x2, y2);
}

/* are the two rects equal? */
bool rects_equal(const rect* r1, const rect* r2) {
	if (rect_is_empty(r1))
		return rect_is_empty(r2);
	return points_equal(&r1->first, &r2->first) && points_equal(&r1->last, &r2->last);
}

/* return the result of intersecting two rects -- will be an empty rect if there is no intersection */
void rect_isect(const rect* r1, const rect* r2, rect* rect_result) {
	i32 x1, x2, y1, y2;
	
	NOT_NULL_OR_RETURN_VOID(rect_result);
	if (rect_is_empty(r1) || rect_is_empty(r2))
		{
LEmptyIsect:
		*rect_result = rect_empty();
		return;
		}

	x1 = max_int(r1->first.x, r2->first.x);
	x2 = min_int(r1->last.x, r2->last.x);
	if (x1 > x2)
		goto LEmptyIsect;
	y1 = max_int(r1->first.y, r2->first.y);
	y2 = min_int(r1->last.y, r2->last.y);
	if (y1 > y2)
		goto LEmptyIsect;

	rect_set(rect_result, x1, y1, x2, y2);
}

/* this checks that rect_contained is fully inside rect_containing */
bool rect_contains_rect(const rect* rect_containing, const rect* rect_contained) {
	rect r;
	rect_isect(rect_containing, rect_contained, &r);
	return rects_equal(&r, rect_contained);
}

/* is the sqrect entirely within r? */
bool rect_contains_sqrect(const rect* r, const sqrect* sr) {
	NOT_NULL_OR_RETURN(sr, true);
	NOT_NULL_OR_RETURN(r, false);
	int i;
	for (i = 0; i < darray_size(sr->regions); ++i)
		{
		if (!rect_contains_rect(r, &darray_item(sr->regions, i)))
			return(false);
		}
	return(true);
}

/* this returns true iff r1 and r2 have a non-NULL intersection */
bool rects_overlap(const rect* r1, const rect* r2) {
	rect r;
	rect_isect(r1, r2, &r);
	return !rect_is_empty(&r);
}

/* more contains operations */
bool rect_contains_x(const rect* prect, i32 x) {
	if (rect_is_empty(prect))
		return(false);

	return (prect->first.x <= x && prect->last.x >= x);
}

bool rect_contains_y(const rect* prect, i32 y) {
	if (rect_is_empty(prect))
		return(false);

	return (prect->first.y <= y && prect->last.y >= y);
}

bool rect_contains_point(const rect* prect, const point* pt) {
	NOT_NULL_OR_RETURN(pt, true);
	return rect_contains_x(prect, pt->x) && rect_contains_y(prect, pt->y);
}

/* if the two regions are the same shape (i.e. the same size) */
bool rects_same_shape(const rect* r1, const rect* r2) {
	if (rect_is_empty(r1))
		return rect_is_empty(r2);
	if (rect_is_empty(r2))
		return(false);
	return (rect_xsize(r1) == rect_xsize(r2)) && (rect_ysize(r1) == rect_ysize(r2));
}

/* if the two rects are unionable (i.e. they can be directly concatenated into a single rect) */
bool rects_unionable(const rect* r1, const rect* r2) {
	const rect* rr1,* rr2;
	if (rect_is_empty(r1) || rect_is_empty(r2))
		return(true);
	if (r1->first.x > r2->first.x)
		{
		rr1 = r2;
		rr2 = r1;
		}
	else
		{
		rr1 = r1;
		rr2 = r2;
		}
	if (rr1->last.x < rr2->first.x)
		return(false);
	if (r1->first.y > r2->first.y)
		{
		rr1 = r2;
		rr2 = r1;
		}
	else
		{
		rr1 = r1;
		rr2 = r2;
		}
	if (rr1->last.y < rr2->first.y)
		return(false);
	return(true);
}

/* if the input rects are unionable, put the union in rout and return true, else return false */
bool rects_union(const rect* rin1, const rect* rin2, rect* rout) {
	NOT_NULL_OR_RETURN(rout, false);
	if (rect_is_empty(rin1))
		{
		if (is_null(rin2))
			*rout = rect_empty();
		else
			*rout = *rin2;
		return true;
		}
	if (rect_is_empty(rin2))
		{
		*rout = *rin1;
		return true;
		}
	if (!rects_unionable(rin1, rin2))
		return false;

	i32 x1, x2, y1, y2;
	x1 = min_int(rect_xmin(rin1), rect_xmin(rin2));
	x2 = max_int(rect_xmax(rin1), rect_xmax(rin2));
	y1 = min_int(rect_ymin(rin1), rect_ymin(rin2));
	y2 = max_int(rect_ymax(rin1), rect_ymax(rin2));

	rect_set(rout, x1, y1, x2, y2);

	return true;
}

/* Return the region obtained by removing r2 from r1. */
void rects_subtract(const rect* r1, const rect* r2, sqrect* srout) {
	NOT_NULL_OR_RETURN_VOID(srout);
	sqrect_empty(srout);

	/* some easy cases */
	if (rect_is_empty(r2))
		{
LNoSubtract:
		sqrect_addrect(srout, r1);	/* handles the case when r1 is empty properly */
		return;
		}
	if (rect_contains_rect(r2, r1))
		return;						/* srout is the empty region */
	rect isect;
	rect_isect(r1, r2, &isect);
	if (rect_is_empty(&isect))
		goto LNoSubtract;			/* no intersection, return the entire region r1 */

	/* Full case: there are up to four regions remaining after the subtraction, the portion above, left, right, and below the intersection */
	rect r;
	// the region above the intersection //
	r.first.x = r1->first.x;
	r.last.x = r1->last.x;
	r.first.y = r1->first.y;
	r.last.y = isect.first.y - 1;
	sqrect_addrect(srout, &r);
	// the region below the intersection //
	r.first.y = isect.last.y + 1;
	r.last.y = r1->last.y;
	sqrect_addrect(srout, &r);
	// the region to the left of the intersection //
	r.first.y = max_int(r1->first.y, isect.first.y);	// do it this way so none of the four regions overlap.
	r.last.y = min_int(r1->last.y, isect.last.y);
	r.first.x = r1->first.x;
	r.last.x = isect.first.x - 1;
	sqrect_addrect(srout, &r);
	// the region to the right of the intersection //
	r.first.x = isect.last.x + 1;
	r.last.x = r1->last.x;
	sqrect_addrect(srout, &r);
}

/* initialize a malloced sqrect */
void sqrect_init(sqrect* sr) {
	NOT_NULL_OR_RETURN_VOID(sr);
	darray_init(sr->regions);
	sr->valid = true;
}

/* return an empty sqrect */
void sqrect_empty(sqrect* sr) {
	NOT_NULL_OR_RETURN_VOID(sr);
	if (sr->valid)
		darray_free(sr->regions);
	darray_init(sr->regions);
	sr->valid = true;
}

/* return a sqrect containing a single rect */
void sqrect_from_rect(const rect* r, sqrect* sr) {
	sqrect_empty(sr);
	darray_push(sr->regions, *r);
}

/* return the number of rectangular regions in a sqrect */
u32 sqrect_nregions(const sqrect* sr) {
	NOT_NULL_OR_RETURN(sr, 0UL);
	return darray_size(sr->regions);
}

/* add a new rect to a sqrect */
void sqrect_addrect(sqrect* sr, const rect* r) {
	NOT_NULL_OR_RETURN_VOID(sr);
	if (rect_is_empty(r))
		return;	// pfft
	darray_push(sr->regions, *r);
}

/* does the passed in sqrect represent an empty region? */
bool sqrect_is_empty(const sqrect* sr) {
	NOT_NULL_OR_RETURN(sr, true);
	if (!sr->valid)
		return(true);
	u32 i;
	for (i = 0; i < sqrect_nregions(sr); ++i) {
		if (!rect_is_empty(&darray_item(sr->regions, i)))
			return(false);
	}
	return(true);
}

/* put the bounding rect for the sqrect in rout */
void sqrect_boundingrect(const sqrect* sr, rect* rout) {
	NOT_NULL_OR_RETURN_VOID(rout);
	if (sqrect_is_empty(sr)) {
		*rout = rect_empty();
		return;
	}
	*rout = darray_item(sr->regions, 0);
	u32 i;
	for (i = 1; i < sqrect_nregions(sr); ++i) {
		const rect* r = &darray_item(sr->regions, i);
		rout->first.x = min_int(rout->first.x, r->first.x);
		rout->first.y = min_int(rout->first.y, r->first.y);
		rout->last.x = max_int(rout->last.x, r->last.x);
		rout->last.y = max_int(rout->last.y, r->last.y);
	}
}

/* does the rect intersect with the sqrect? */
bool sqrect_rect_has_isect(const sqrect* sr, const rect* r) {
	/* some simple cases */
	rect brect, irect;
	if (rect_is_empty(r))
		return(true);
	if (sqrect_is_empty(sr))
		return(false);
	sqrect_boundingrect(sr, &brect);
	rect_isect(r, &brect, &irect);
	if (rect_is_empty(&irect))
		return(false);
	/* do the full check */
	u32 i;
	for (i = 0; i < sqrect_nregions(sr); ++i) {
		rect_isect(r, &darray_item(sr->regions, i), &irect);
		if (!rect_is_empty(&irect))
			return(true);
	}
	return(false);
}

/* return the intersection of a sqrect and a rect (empty if none) */
void sqrect_rect_isect(const sqrect* sr, const rect* r, sqrect* ret) {
 	rect brect, irect;
	NOT_NULL_OR_RETURN_VOID(ret);
	sqrect_empty(ret);
	if (rect_is_empty(r))
		return;
	if (sqrect_is_empty(sr))
		return ;
	sqrect_boundingrect(sr, &brect);
	rect_isect(r, &brect, &irect);
	if (rect_is_empty(&irect))
		return;
	u32 i;
	for (i = 0; i < sqrect_nregions(sr); ++i) {
		rect_isect(r, &darray_item(sr->regions, i), &irect);
		if (!rect_is_empty(&irect))
			darray_push(ret->regions, irect);
	}
	return;
}

/* add a sqrect to a sqrect -- does not empty sr1_inout first */
void sqrect_add(sqrect* sr1_inout, const sqrect* sr2) {
	NOT_NULL_OR_RETURN_VOID(sr1_inout);
	NOT_NULL_OR_RETURN_VOID(sr2);
	u32 i;
	for (i = 0; i < sqrect_nregions(sr2); ++i) {
		if (!rect_is_empty(&darray_item(sr2->regions, i)))
			darray_push(sr1_inout->regions, darray_item(sr2->regions, i));
	}
}

/* return the union of two sqrects */
void sqrect_union(const sqrect* sr1, const sqrect* sr2, sqrect* srout) {
	NOT_NULL_OR_RETURN_VOID(srout);
	sqrect_empty(srout);
	sqrect_add(srout, sr1);
	sqrect_add(srout, sr2);
}

/* return the intersection of two sqrects */
void sqrect_isect(const sqrect* sr1, const sqrect* sr2, sqrect* srout) {
	NOT_NULL_OR_RETURN_VOID(srout);
	sqrect isect;
	u32 i;
	sqrect_empty(srout);
	if (sqrect_is_empty(sr1) || sqrect_is_empty(sr2))
		return;
	for (i = 0; i < sqrect_nregions(sr1); ++i) {
		sqrect_rect_isect(sr2, &darray_item(sr1->regions, i), &isect);
		if (!sqrect_is_empty(&isect))
			sqrect_add(srout, &isect);
	}
}

/* Make a copy of the input sqrect */
void sqrect_copy(const sqrect* sr_in, sqrect* sr_out) {
	NOT_NULL_OR_RETURN_VOID(sr_out);
	sqrect_empty(sr_out);
	sqrect_add(sr_out, sr_in);
}

/* Helper function: part of sqrect compact. */
static void sqrect_union_all_unionable(const sqrect* sr_in, sqrect* sr_out) {
	if (unlikely(is_null(sr_in))) {
		sqrect_empty(sr_out);
		return;
	}

	// TODO: O(n^2), will probably be slow on sqrects with 1,000s of regions. By sorting the rects we can do better.
	sqrect compact;
	sqrect_empty(&compact);
	
	int nr = sqrect_nregions(sr_in);
	int i, j;
	rect u;
	for (i = 0; i < nr; ++i)
		for (j = i + 1; j < nr; ++i) {
			if (rects_union(&darray_item(sr_in->regions, i), &darray_item(sr_in->regions, j), &u)) {
				int e;
				// sqrect_addrect special cases empty rects, so if sr_in somehow has empty rects we won't add them to the compacted sqrect
				sqrect_addrect(&compact, &u);
				for (e = 0; e < nr; ++e)
					{
					if (e == i || e == j)
						continue;
					sqrect_addrect(&compact, &darray_item(sr_in->regions, e));
					}
				// tail recursion, not as evil as it looks
				sqrect_union_all_unionable(&compact, sr_out);
				return;
			}
		}

	// if we get here the sqrect is no longer unionable, so copy it.
	sqrect_copy(sr_in, sr_out);
}

/* Helper function: split or remove any overlapping rects so that any point will belong to at most one region of the sqrect. */
static void sqrect_remove_intersections(const sqrect* sr_in, sqrect* sr_out) {
	if (unlikely(is_null(sr_in))) {
		sqrect_empty(sr_out);
		return;
	}
	sqrect compact;
	sqrect_empty(&compact);

	u32 nr = sqrect_nregions(sr_in);
	u32 i, j;
	sqrect subtract;
	for (i = 0; i < nr; ++i)
		for (j = i + 1; j < nr; ++i) {
			rect r;
			rect_isect(&darray_item(sr_in->regions, i), &darray_item(sr_in->regions, j), &r);
			if (!rect_is_empty(&r)) {
				u32 e;
				rects_subtract(&darray_item(sr_in->regions, i), &darray_item(sr_in->regions, j), &subtract);
				sqrect_add(&compact, &subtract);
				for (e = 0; e < nr; ++e) {
					if (e == i || e == j)
						continue;
					sqrect_addrect(&compact, &darray_item(sr_in->regions, e));
				}
				sqrect_remove_intersections(&compact, sr_out);
				return;
			}
		}

	sqrect_copy(sr_in, sr_out);
}

/* Return a compacted version of the input sqrect -- no point will be present in more than one region. All unionable rects will be unioned,
	and overlapping rects will be split or removed. */
void sqrect_compact(const sqrect* sr_in, sqrect* sr_out) {
	/* First order of business: union all unionable rects. */
	sqrect u;
	sqrect_union_all_unionable(sr_in, &u);

	/* Next, find all overlapping rects and split or remove them as necessary. */
	sqrect_remove_intersections(&u, sr_out);

	/* Done. Let's pick up our toys. */
	sqrect_free(&u);
}

/* Return an array of n allocated sqrects. Better to use this than NEW_ARRAY() since it initializes the sqrects for you. */
sqrect* sqrect_new_array(u32 n) {
	sqrect* ret;
	u32 e;
	if (unlikely(iszero(n)))
		return(NULL);

	ret = NEW_ARRAY(sqrect, n);
	for (e = 0; e < n; ++e)
		sqrect_init(ret + e);

	return ret;
}

/* Return the minimum x value in the sqrect. */
i32 sqrect_minx(const sqrect* sr) {
	rect r;
	sqrect_boundingrect(sr, &r);
	return r.first.x;
}

/* Return the maximum x value in the sqrect. */
i32 sqrect_maxx(const sqrect* sr) {
	rect r;
	sqrect_boundingrect(sr, &r);
	return r.last.x;
}

/* Return the minimum y value in the sqrect. */
i32 sqrect_miny(const sqrect* sr) {
	rect r;
	sqrect_boundingrect(sr, &r);
	return r.first.y;
}

/* Return the maximum y value in the sqrect. */
i32 sqrect_maxy(const sqrect* sr) {
	rect r;
	sqrect_boundingrect(sr, &r);
	return r.last.y;
}

/* Return the total area of the sqrect -- calculated as sum of the child rects. If assume_compact is true, does not do compaction first. */
u64 sqrect_area(const sqrect* sr, bool assume_compact) {
	sqrect compact;
	const sqrect* sr_use;
	if (!assume_compact)
		sqrect_compact(sr, &compact);
	sr_use = (assume_compact ? sr : &compact);
	u32 i;
	u64 a = 0ULL;
	for (i = 0; i < sqrect_nregions(sr_use); ++i)
		a += rect_area(&darray_item(sr_use->regions, i));
	return(a);
}

/* Subtract a rect from a sqrect and return the remaining region. */
void sqrect_subtract_rect(const sqrect* sr, const rect* r, sqrect* srout) {
	sqrect subtract;
	NOT_NULL_OR_RETURN_VOID(srout);
	sqrect_empty(srout);
	u32 i;
	for (i = 0; i < sqrect_nregions(sr); ++i) {
		rects_subtract(&darray_item(sr->regions, i), r, &subtract);
		sqrect_add(srout, &subtract);
	}
}


/* Subtract sqrect sr2 from sqrect sr1 and return the remaining region. */
void sqrect_subtract(const sqrect* sr1, const sqrect* sr2, sqrect* srout) {
	sqrect si1, si2;
	u32 i;
	i32 v = 0;
	sqrect_copy(sr1, &si1);
	sqrect_empty(&si2);
	for (i = 0; i < sqrect_nregions(sr2); ++i) {
		// alternating to avoid unnecessary copies
		if (iszero(v))
			sqrect_subtract_rect(&si1, &darray_item(sr2->regions, i), &si2);
		else
			sqrect_subtract_rect(&si2, &darray_item(sr2->regions, i), &si1);
		alternate(v, 0, 1);
	}
	if (iszero(v))
		sqrect_copy(&si1, srout);
	else
		sqrect_copy(&si2, srout);
	// don't need to worry about freeing above before the sqrect_subtract_rect() operations -- that function frees srout for us
	sqrect_free(&si1);
	sqrect_free(&si2);
}

/* Does the sqrect contain every point of the passed-in rect? Handles the case where the rect crosses over multiple regions of the sqrect. */
bool sqrect_contains_rect(const sqrect* sr, const rect* r) {
	// check the vacuous case first, to shortcircuit
	if (rect_is_empty(r))
		return(true);

	// subtract from the rect, if we're left with no area we contain the rect	
	sqrect s1, s2;
	bool ret;
	sqrect_from_rect(r, &s1);
	sqrect_subtract(&s1, sr, &s2);
	ret = sqrect_is_empty(&s2);
	sqrect_free(&s1);
	sqrect_free(&s2);
	return ret;
}

/* Does sqrect sr_containing contain every point of sr_contained? */
bool sqrect_contains(const sqrect* sr_containing, const sqrect* sr_contained) {
	sqrect s1, s2;
	bool ret;
	sqrect_copy(sr_contained, &s1);
	sqrect_subtract(&s1, sr_containing, &s2);
	ret = sqrect_is_empty(&s2);
	sqrect_free(&s1);
	sqrect_free(&s2);
	return ret;
}

/* Do the two sqrects represent the exact same area? */
bool sqrects_equal(const sqrect* sr1, const sqrect* sr2) {
	return sqrect_contains(sr1, sr2) && sqrect_contains(sr2, sr1);
}

/* Free memory owned by the sqrect. */
void sqrect_free(sqrect* sr) {
	// shh, it's just an alias for sqrect_empty()
	sqrect_empty(sr);
}

/*** end ref.cpp ***/