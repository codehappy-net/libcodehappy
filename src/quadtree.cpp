/***

	quadtree.cpp

	Represent an image in a quad tree: each non-leaf has up to four children, representing four quadrants.
	Can be used for computer vision, lossy compression or image manipulation.

	Copyright (c) 2022 C. M. Street.

***/

/*** Helper function: get the value of the given channel at the given pixel. ***/
static u32 __channel_value(SBitmap* bmp, u32 channel_index, u32 x, u32 y) {
	RGBColor clr;
	clr = bmp->get_pixel(x, y);
	switch (channel_index)
		{
	case 0:		return RGB_RED(clr);
	case 1:		return RGB_GREEN(clr);
	case 2:		return RGB_BLUE(clr);
		}
	assert(false);
	return 0;
}

/*** Helper function: given a region and a channel index, return the min and max values over that region. ***/
static void __find_minmax_region(SBitmap* bmp, rect* r, u32 channel_index, u32* min_out, u32* max_out) {
	NOT_NULL_OR_RETURN_VOID(bmp);
	NOT_NULL_OR_RETURN_VOID(r);
	NOT_NULL_OR_RETURN_VOID(min_out);
	NOT_NULL_OR_RETURN_VOID(max_out);
	u32 e, f;

	*min_out = 999;
	*max_out = 0;
	for (f = r->first.y; f <= r->last.y; ++f)
		for (e = r->first.x; e <= r->last.x; ++e) {
			u32 v = __channel_value(bmp, channel_index, e, f);
			if (v < *min_out)
				*min_out = v;
			if (v > *max_out)
				*max_out = v;
		}
}

/*** Helper function: given a region rin, return in rout the specified quadrant. ***/
static void __rect_quadrant(rect* rin, rect* rout, u32 quadrant_index)
{
	/* Quadrants: 0 = upper left, 1 = upper right, 2 = lower left, 3 = lower right */
	if (quadrant_index < 2) {
		/* these are the upper quadrants */
		rout->first.y = rin->first.y;
		rout->last.y = rin->first.y + (rin->last.y - rin->first.y) / 2;
	} else {
		/* lower quadrants */
		rout->first.y = rin->first.y + (rin->last.y - rin->first.y) / 2 + 1;
		rout->last.y = rin->last.y;
	}
	if (is_even(quadrant_index)) {
		/* these are the left quadrants */
		rout->first.x = rin->first.x;
		rout->last.x = rin->first.x + (rin->last.x - rin->first.x) / 2;
	} else {
		/* the right quadrants */
		rout->first.x = rin->first.x + (rin->last.x - rin->first.x) / 2 + 1;
		rout->last.x = rin->last.x;
	}
}

/*** Helper function: construct the quadtree on and below this node. ***/
static void __construct_quadtree_node(quadnode* node, SBitmap* bmp, u32 channel_index, rect* r) {
	u32 i;
	__find_minmax_region(bmp, r, channel_index, &node->min_val, &node->max_val);
	for (i = 0; i < 4; ++i)
		node->children[i] = NULL;
	/* if this region is all one value, we're done */
	if (node->min_val == node->max_val)
		return;
	/* else divide the region into four quadrants and construct the quadtree nodes beneath this one */
	for (i = 0; i < 4; ++i) {
		rect rq;
		__rect_quadrant(r, &rq, i);
		if (rect_is_empty(&rq))
			continue;
		node->children[i] = NEW(quadnode);
		NOT_NULL_OR_RETURN_VOID(node->children[i]);
		__construct_quadtree_node(node->children[i], bmp, channel_index, &rq);
	}
}

/*** Construct a quadtree representation of the passed-in bitmap. Use the specified color space. ***/
quadtree* quadtree_from_bmp(SBitmap* bmp, colorspace space) {
	NOT_NULL_OR_RETURN(bmp, NULL);
	quadtree* ret;
	SBitmap* bmpuse;

	ret = NEW(quadtree);
	NOT_NULL_OR_RETURN(ret, ret);
	if (space == colorspace_rgb) {
		bmpuse = bmp;
	} else {
		bmpuse = bmp->copy();
		NOT_NULL_OR_RETURN(bmpuse, NULL);
		bitmap_to_colorspace(bmpuse, space);
	}

	ret->w = bmpuse->width();
	ret->h = bmpuse->height();
	ret->channel_type = space;

	rect r;
	rect_set(&r, 0, 0, bmp->width() - 1, bmp->height() - 1);
	__construct_quadtree_node(&ret->channel[0], bmpuse, 0, &r);
	__construct_quadtree_node(&ret->channel[1], bmpuse, 1, &r);
	__construct_quadtree_node(&ret->channel[2], bmpuse, 2, &r);

	if (bmpuse != bmp)
		delete bmpuse;

	return ret;
}

/*** Render a bitmap from the given quadtree, using the specified tolerance -- if (max_val - min_val <= tolerance), the node
	is treated as a leaf and the value ((max_val + min_val) / 2) is assigned to it. ***/
// TODO: Store the average intensity of a node and use that instead of the average of the min and max intensities?
SBitmap* bmp_from_quadtree(quadtree* qt, u32 tolerance) {
	return bmp_from_quadtree_tolerance_by_channel(qt, tolerance, tolerance, tolerance);
}

/*** Helper function: returns whether the given quadnode is a leaf node ***/
static bool __leaf_quadnode(const quadnode* node) {
	u32 i;
	for (i = 0; i < 4; ++i)
		if (not_null(node->children[i]))
			break;
	return (4 == i);
}

/*** Helper function: render the given quadtree node using the specified channel and tolerance. ***/
static void __render_quadtree_node(quadnode* node, SBitmap* bmp, u32 channel_index, u32 tolerance, rect* r) {
	NOT_NULL_OR_RETURN_VOID(node);
	NOT_NULL_OR_RETURN_VOID(bmp);
	NOT_NULL_OR_RETURN_VOID(r);

	if ((node->max_val - node->min_val <= tolerance) || __leaf_quadnode(node)) {
		/* OK, we can render this node */
		int y, x;
		u32 val;

		val = (node->max_val + node->min_val) / 2;

		for (y = r->first.y; y <= r->last.y; ++y)
			for (x = r->first.x; x <= r->last.x; ++x) {
				RGBColor c;
				c = bmp->get_pixel(x, y);
				switch (channel_index) {
				case 0:
					bmp->put_pixel(x, y, RGB_NO_CHECK(val, RGB_GREEN(c), RGB_BLUE(c)));
					break;
				case 1:
					bmp->put_pixel(x, y, RGB_NO_CHECK(RGB_RED(c), val, RGB_BLUE(c)));
					break;
				case 2:
					bmp->put_pixel(x, y, RGB_NO_CHECK(RGB_RED(c), RGB_GREEN(c), val));
					break;
				}
			}
	} else {
		/* This node is not within tolerance; render the child nodes */
		u32 i;
		for (i = 0; i < 4; ++i) {
			rect rq;
			__rect_quadrant(r, &rq, i);
			if (rect_is_empty(&rq))
				continue;
			__render_quadtree_node(node->children[i], bmp, channel_index, tolerance, &rq);
		}
	}
}

/*** As above, but you can use a different tolerance for each channel. Useful since the human eye is less sensitive to certain
	channels, so you may use larger tolerance values for those channels while preserving more (apparent) image data. 
	For RGB, tol1 is used for red, tol2 is for green, tol3 is for blue. For HSV, tol1 is hue, tol2 is saturation, etc. ***/
SBitmap* bmp_from_quadtree_tolerance_by_channel(quadtree* qt, u32 tol1, u32 tol2, u32 tol3) {
	NOT_NULL_OR_RETURN(qt, NULL);
	SBitmap* ret, *render;
	render = new SBitmap(qt->w, qt->h, BITMAP_DEFAULT);
	NOT_NULL_OR_RETURN(render, NULL);
	render->clear();

	rect r;
	rect_set(&r, 0, 0, qt->w - 1, qt->h - 1);
	__render_quadtree_node(&qt->channel[0], render, 0, tol1, &r);
	__render_quadtree_node(&qt->channel[1], render, 1, tol2, &r);
	__render_quadtree_node(&qt->channel[2], render, 2, tol3, &r);

	if (qt->channel_type != colorspace_rgb) {
		ret = render->copy();
		NOT_NULL_OR_RETURN(ret, NULL);
		bitmap_from_colorspace(ret, qt->channel_type);
	} else {
		ret = render;
	}

	ret->fill_alpha_channel(ALPHA_OPAQUE);

	if (ret != render)
		delete render;

	return ret;
}

/*** Helper function: Serialize the specified quadnode. ***/
static void __serialize_quadnode(Scratchpad* sp, const quadnode* node, u32 tol, rect* r) {
	NOT_NULL_OR_RETURN_VOID(node);
	// TODO: could increase efficiency of the encoding a lot by writing the 0/1 as a single bit.
	if ((node->max_val - node->min_val <= tol) || __leaf_quadnode(node)) {
		sp->addc(1);
		sp->addc((node->max_val + node->min_val) / 2);
	} else {
		sp->addc(0);
		for (u32 i = 0; i < 4; ++i) {
			rect rq;
			__rect_quadrant(r, &rq, i);
			if (rect_is_empty(&rq))
				continue;
			__serialize_quadnode(sp, node->children[i], tol, &rq);
		}
	}
}

/*** Serialize the quadtree with the given tolerances. Returns a scratchpad containing the quadtree representation.
	Quadtrees offer a simple means of lossy image compression (Use 0 for tolerance if you do not want any image data discarded.) ***/
Scratchpad* serialize_quadtree(const quadtree* qt, u32 tol1, u32 tol2, u32 tol3) {
	Scratchpad* sp = new Scratchpad;
	NOT_NULL_OR_RETURN(sp, NULL);

	sp->addc(qt->w & 0xff);
	sp->addc(qt->w >> 8);
	sp->addc(qt->h & 0xff);
	sp->addc(qt->h >> 8);
	sp->addc((char)qt->channel_type);

	rect r;
	rect_set(&r, 0, 0, qt->w - 1, qt->h - 1);
	__serialize_quadnode(sp, &qt->channel[0], tol1, &r);
	__serialize_quadnode(sp, &qt->channel[1], tol2, &r);
	__serialize_quadnode(sp, &qt->channel[2], tol3, &r);

	return sp;
}

/*** Helper function: Free a quadnode. ***/
static void __free_quadnode(quadnode* node) {
	int e;
	NOT_NULL_OR_RETURN_VOID(node);
	for (e = 0; e < 4; ++e) {
		__free_quadnode(node->children[e]);
		delete node->children[e];
	}
}

/*** Free the passed in quadtree. ***/
void free_quadtree(quadtree* qt) {
	NOT_NULL_OR_RETURN_VOID(qt);
	__free_quadnode(&qt->channel[0]);
	__free_quadnode(&qt->channel[1]);
	__free_quadnode(&qt->channel[2]);
	delete qt;
}

/*** Helper function: if the node is within tolerance, call the callback function, else, walk the child nodes. ***/
static void walk_quadnode(quadnode* node, u32 channel_index, u32 tol, rect* r, QtCallback callback_fn, void* args) {
	NOT_NULL_OR_RETURN_VOID(node);
	if ((node->max_val - node->min_val <= tol) || __leaf_quadnode(node)) {
		callback_fn(r, node, channel_index, args);
	} else {
		int e;
		for (e = 0; e < 4; ++e) {
			rect rq;
			__rect_quadrant(r, &rq, e);
			if (rect_is_empty(&rq))
				continue;
			walk_quadnode(node->children[e], channel_index, tol, &rq, callback_fn, args);
		}
	}
}

/*** Walk the quadtree and call the specified function for each node within the specified tolerances. args will be passed to the callback. ***/
void walk_quadtree(quadtree* qt, u32 tol1, u32 tol2, u32 tol3, QtCallback callback_fn, void* args) {
	NOT_NULL_OR_RETURN_VOID(qt);
	rect r;
	rect_set(&r, 0, 0, qt->w - 1, qt->h - 1);
	walk_quadnode(&qt->channel[0], 0, tol1, &r, callback_fn, args);
	walk_quadnode(&qt->channel[1], 1, tol2, &r, callback_fn, args);
	walk_quadnode(&qt->channel[2], 2, tol3, &r, callback_fn, args);
}

struct borderargs {
	SBitmap*	bmp;
	u32		channel_index;
};

/*** The callback used to render quadtree regions ***/
void __region_callback(rect* r, quadnode* node, u32 channel_index, void* args) {
	borderargs* ba = (borderargs*)args;
	if (channel_index != ba->channel_index)
		return;
	ba->bmp->rect(r->first.x, r->first.y, r->last.x, r->last.y, C_BLACK);
}

/*** Return a bitmap that shows the regions (nodes within tolerance) in the quadtree for the specified channel. ***/
SBitmap* render_quadtree_regions(quadtree* qt, u32 tolerance, u32 channel_index) {
	borderargs ba;
	ba.bmp = new SBitmap(qt->w, qt->h, BITMAP_DEFAULT);
	NOT_NULL_OR_RETURN(ba.bmp, ba.bmp);

	ba.bmp->clear(C_WHITE);
	ba.channel_index = channel_index;
	walk_quadtree(qt, tolerance, tolerance, tolerance, __region_callback, (void *)&ba);
	ba.bmp->fill_alpha_channel(ALPHA_OPAQUE);

	return ba.bmp;
}

/*** end quadtree.cpp ***/