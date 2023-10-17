/***

	quadtree.h

	Represent an image in a quad tree: each non-leaf has up to four children, representing four quadrants.
	Can be used for computer vision, lossy compression or image manipulation.

	Copyright (c) 2022, C. M. Street.

***/
#ifndef __QUADTREES__
#define __QUADTREES__

struct quadnode {
	u32	min_val;
	u32	max_val;
	// TODO: avg_val?
	quadnode* children[4];
};

struct quadtree {
	/*** dimensions of image ***/
	u32 w;
	u32 h;
	colorspace channel_type;
	/*** one tree for each channel ***/
	quadnode channel[3];
};

/*** quadtree callback type, used when walking the tree. The function will be called for each quadtree node within
	tolerance. The arguments are, in order: a rect representing the region of the bitmap the node represents, a pointer to the
	quadnode, the channel index of the node, and a void pointer that is the arguments passed to walk_quadtree(). ***/
typedef	void (*QtCallback)(rect*, quadnode*, u32, void *);

/*** Construct a quadtree representation of the passed-in bitmap. Use the specified color space. ***/
extern quadtree* quadtree_from_bmp(SBitmap* bmp, colorspace space);

/*** Render a bitmap from the given quadtree, using the specified tolerance -- if (max_val - min_val <= tolerance), the node
	is treated as a leaf and the value ((max_val + min_val) / 2) is assigned to it. ***/
extern SBitmap* bmp_from_quadtree(quadtree* qt, u32 tolerance);
/*** As above, but you can use a different tolerance for each channel. Useful since the human eye is less sensitive to certain
	channels, so you may use larger tolerance values for those channels while preserving more (apparent) image data. 
	For RGB, tol1 is used for red, tol2 is for green, tol3 is for blue. For HSV, tol1 is hue, tol2 is saturation, etc. ***/
extern SBitmap* bmp_from_quadtree_tolerance_by_channel(quadtree* qt, u32 tol1, u32 tol2, u32 tol3);

/*** Serialize the quadtree with the given tolerances. Returns a scratchpad containing the quadtree representation.
	Quadtrees offer a simple means of lossy image compression (Use 0 for tolerance if you do not want any image data discarded.) ***/
extern Scratchpad* serialize_quadtree(const quadtree* qt, u32 tol1, u32 tol2, u32 tol3);

/*** Free the passed in quadtree. ***/
extern void free_quadtree(quadtree* qt);

/*** Walk the quadtree and call the specified function for each node within the specified tolerances. args will be passed to the callback. ***/
extern void walk_quadtree(quadtree* qt, u32 tol1, u32 tol2, u32 tol3, QtCallback callback_fn, void* args);

/*** Return a bitmap that shows the regions (nodes within tolerance) in the quadtree for the specified channel. ***/
extern SBitmap* render_quadtree_regions(quadtree* qt, u32 tolerance, u32 channel_index);

#endif  // __QUADTREES__
/*** end of quadtree.h ***/