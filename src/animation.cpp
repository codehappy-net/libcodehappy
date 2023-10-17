/***

	animation.cpp

	Sprite animation and other animation effects.

	Copyright (c) 2014-2022 C. M. Street

***/


/*** Some global state -- OK since there will only be one animation handling thread in the app ***/

/*** the current count of active animations ***/
static i32	__count_animations = 0;
/*** the next available animation handle ***/
static animation_handle __next_available = 0UL;
/*** the animation ticker ***/
static volatile u64 __animation_ticker = 0ULL;
/*** is the animation update thread running? ***/
static bool __animation_init = false;
/*** mutex for controlling access to animations ***/
static mtx_t* __animation_mutex = NULL;

/*** The animation struct. Defined here because I want this to be opaque to consumers of the library -- 
	they should refer to the animations as handles only. ***/
typedef struct __animation
	{
	bool				valid;		/* is this animation valid? */
	SBitmap*			owner;		/* the owning bitmap for this animation */
	bool				cycle;		/* is the animation cycle active? */
	bool				shown;		/* is the animation drawn on the owner bitmap? */
	bool				moving;		/* is the animation moving at x_rate/y_rate? */
	i32					x_pos;		/* the animation's current x-position on the owning bitmap, in 1/256ths of a pixel */
	i32					y_pos;		/* the animation's current y-position on the owning bitmap, in 1/256ths of a pixel */
	i32					z_val;		/* the "depth" of this animation */
	u32					max_x;		/* maximum x-size of this animation */
	u32					max_y;		/* maximum y-size of this animation */
	u32					nframes;	/* the number of frames in this animation */
	u32					iframe;		/* the index of the current frame for this animation */
	SBitmap**			frames;		/* the actual animation frames */
	u32					loops;		/* the number of loops the animation will run */
	u32					rloops;		/* the number of remaining loops */
	time_unit			delay;		/* the current number of hundredths of a second between frames */
	u64					tnframe;	/* the ticker on which the animation should advance to the next frame */
	bool				havepath;	/* does the animation have a path? */
	i32					dest_x;		/* if the animation is in path movement, this is its destination */
	i32					dest_y;		/* if the animation is in path movement, this is its destination */
	move_rate			x_rate;		/* the animation's x-movement rate, in 1/256ths of a pixel per 1/100th of a second */
	move_rate			y_rate;		/* the animation's y-movement rate, in 1/256ths of a pixel per 1/100th of a second */
	u32					arrive;		/* the ticker on which we expect to arrive at our destination */
	bool				dos;		/* destroy-on-stop flag */
	u32					min_alpha;	/* minimum alpha value for a pixel before we declare a collision */
	AnimationCallback	loopend;	/* when we stop looping, call this function */
	AnimationCallback	pathend;	/* when we stop pathing, call this function */
	AnimationCallback	frm;		/* call this function when we reach a certain frame index */
	u32					frc;		/* frame index corresponding to the callback */
	AnimationCallback	oob;		/* when we are animating but out of bounds, call this function */
	CollisionCallback	collide;	/* when we collide with another animation, call this function */
	AnimationCallback	draw;		/* a callback for custom-drawing of the animation */
	void*				data;		/* data that the user has specified to associate with this animation */
	} animation;

static animation __animations[ANIMATION_HANDLE_MAX_VALID + 1];

static animation* __animation_sort[ANIMATION_HANDLE_MAX_VALID + 1];

static u32 __animation_handler(u32 ival, void* arg)
{
	u32 e;

	++__animation_ticker;
	
	for (e = ANIMATION_HANDLE_MIN_VALID; e <= ANIMATION_HANDLE_MAX_VALID; ++e)
		{
		if (!__animations[e].valid)
			continue;

		if (__animations[e].cycle && __animation_ticker >= __animations[e].tnframe)
			{
			mtx_lock(__animation_mutex);
			++__animations[e].iframe;
			if (__animations[e].iframe >= __animations[e].nframes)
				{
				__animations[e].iframe = 0;
				if (__animations[e].rloops > 0)
					--__animations[e].rloops;
				if (0 == __animations[e].rloops)
					{
					__animations[e].cycle = false; 
					}
				}
			__animations[e].tnframe += __animations[e].delay;
			mtx_unlock(__animation_mutex);
			}

		if (__animations[e].moving)
			{
			if (__animations[e].havepath)
				{
				// TODO: if adding x_rate/y_rate takes us beyond the destination point, put us exactly there.
				}
			__animations[e].x_pos += __animations[e].x_rate;
			__animations[e].y_pos += __animations[e].y_rate;
			}

		if (__animations[e].oob)
			{
			// if any pixel of the current animation is off the bounds of the owner's bitmap, call the out-of-bounds callback.
			if (__animations[e].x_pos < 0 || __animations[e].y_pos < 0 ||
				(__animations[e].x_pos + __animations[e].max_x > PIXEL_TO_POSITION(__animations[e].owner->w)) || 
				(__animations[e].y_pos + __animations[e].max_y > PIXEL_TO_POSITION(__animations[e].owner->h)))
				{
				__animations[e].oob(__animations[e].owner, (animation_handle)e);
				}
			}

		if (not_null(__animations[e].collide))
			{
			// TODO: collision algorithm below
			/***
				Collision detection algorithm:
				We really should put the y_top and y_bottom of each sprite in an array and sort; then the only times we
				need to worry about collisions is when there is a y_top that comes between a y_top and its corresponding y_bottom --
				we can determine that by counting -- sort the tuples (y, top/bottom, which animation) by y value ***/
			}

		// TODO: handle other callbacks
		}

	return(ival);
}

static animation_handle __get_next_handle(void)
{
	u32 i;

	if (__count_animations > ANIMATION_HANDLE_MAX_VALID)
		return(ANIMATION_INVALID);
	
	if (__next_available <= ANIMATION_HANDLE_MAX_VALID)
		return(__next_available++);

	for (i = 0; i <= ANIMATION_HANDLE_MAX_VALID; ++i)
		if (!__animations[i].valid)
			return((animation_handle) i);
	
	return (ANIMATION_INVALID);
}

/*** Starts the animation update thread. Call this before any other animation functions. ***/
void animation_init(void)
{
	u32 i;
	
	if (__animation_init)
		return;

	/*** first, mark all animation handles as invalid. ***/
	for (i = ANIMATION_HANDLE_MIN_VALID; i <= ANIMATION_HANDLE_MAX_VALID; ++i)
		__animations[i].valid = false;

	/*** create the animation mutex ***/
	__animation_mutex = new_mutex();

	/*** and kick off the animation handler thread ***/
	SDL_AddTimer(10, __animation_handler, NULL);

	__animation_init = true;
}

/*** 
     Registers a new animation with the sprites given in "frames".
	When the animation is started, every delay hundredths of 
	a second, the animation will transition to the next frame.
     bmp_owner is the owning bitmap (usually the main window's
       draw surface, but could be another window's draw surface or
       a bitmap you want to later blit to the draw surface, etc.)
     Registering the animation does not show or start it immediately;
	use animation_start() and friends for that.
     Returns an animation_handle that must be used to refer to
	this animation. There can be up to MAX_ANIMATIONS active
	registered animations at a time. Destroyed animation
	handles do not count toward that limit.
***/
animation_handle animation_register(SBitmap* bmp_owner, SBitmap** frames, u32 num_frames, time_unit delay)
{
	animation_handle a;
	int e;
	
	if (!__animation_init)
		return(ANIMATION_INVALID);

	a = __get_next_handle();
	if (ANIMATION_INVALID == a)
		return(a);

	__animations[a].owner = bmp_owner;

	/*** set values to their defaults ***/
	__animations[a].cycle = false;
	__animations[a].shown = false;
	__animations[a].moving = false;
	__animations[a].x_pos = 0;
	__animations[a].y_pos = 0;
	__animations[a].z_val = Z_VALUE_DEFAULT;
	__animations[a].max_x = 0UL;
	__animations[a].max_y = 0UL;
	__animations[a].iframe = 0;
	__animations[a].loops = ANIMATION_LOOP;
	__animations[a].rloops = ANIMATION_LOOP;
	__animations[a].havepath = false;
	__animations[a].dest_x = 0;
	__animations[a].dest_y = 0;
	__animations[a].x_rate = 0;
	__animations[a].y_rate = 0;
	__animations[a].arrive = 0;
	__animations[a].loopend = NULL;
	__animations[a].pathend = NULL;
	__animations[a].frm = NULL;
	__animations[a].frc = 0;
	__animations[a].oob = NULL;
	__animations[a].collide = NULL;
	__animations[a].dos = false;
	__animations[a].min_alpha = 1UL;
	__animations[a].draw = NULL;
	__animations[a].data = NULL;

	/*** set the frames ***/
	__animations[a].frames = frames;
	__animations[a].nframes = num_frames;
	__animations[a].delay = delay;

	/*** get the maximum x and y dimensions ***/
	for (e = 0; e < __animations[a].nframes; ++e)
		{
		if (PIXEL_TO_POSITION(__animations[a].frames[e]->w) > __animations[a].max_x)
			__animations[a].max_x = PIXEL_TO_POSITION(__animations[a].frames[e]->w);
		if (PIXEL_TO_POSITION(__animations[a].frames[e]->h) > __animations[a].max_y)
			__animations[a].max_y = PIXEL_TO_POSITION(__animations[a].frames[e]->h);
		}

	/*** OK, we're a valid animation. ***/
	mtx_lock(__animation_mutex);
	++__count_animations;
	__animations[a].valid = true;
	mtx_unlock(__animation_mutex);
	
	return(a);
}

#define	VALID_OR_RETURN(a)		if (!animation_handle_is_valid(a))	return
#define	VALID_OR_RETURN_NULL(a)	if (!animation_handle_is_valid(a))	return (NULL)

/*** Stops, hides, and destroys the specified animation. The handle is no longer valid after the call. ***/
void animation_destroy(animation_handle a)
{
	VALID_OR_RETURN(a);

	mtx_lock(__animation_mutex);
	__animations[a].valid = false;
	--__count_animations;
	mtx_unlock(__animation_mutex);
	__animations[a].shown = false;
	__animations[a].cycle = false;
	__animations[a].havepath = false;
	__animations[a].frames = NULL;
	__animations[a].nframes = 0;
	__animations[a].owner = NULL;
}

/*** Associate data with the specified animation. This way you can check the data in your callbacks. ***/
void animation_associate_data(animation_handle a, void* data)
{
	VALID_OR_RETURN(a);
	__animations[a].data = data;
}

/*** Retrieve the data associated with the animation handle, if any. ***/
void* animation_get_data(animation_handle a)
{
	VALID_OR_RETURN_NULL(a);
	return(__animations[a].data);
}

/*** Starts, or unpauses, the animation cycle and animation path movement. If "show" is true, also
	shows/unhides the animation. (Note that if the animation is already shown, show == false has
	no effect.) ***/
void animation_start(animation_handle a, bool show)
{
	VALID_OR_RETURN(a);
	__animations[a].tnframe = __animation_ticker + __animations[a].delay;
	__animations[a].cycle = true;
	__animations[a].moving = true;
	if (show)
		__animations[a].shown = true;
}

/*** Stops, or pauses, the animation cycle and animation path movement. The animation will still draw unless
	animation_hide() or animation_destroy_on_stop() has been called. If "hide" is true, also hides the
	animation. (Note that if the animation is already hidden, hide == false has no effect.) ***/
void animation_stop(animation_handle a, bool hide)
{
	VALID_OR_RETURN(a);
	__animations[a].cycle = false;
	__animations[a].moving = false;
	if (hide)
		__animations[a].shown = false;
}

/*** Start, or unpause, the animation cycle for the specified animation. The sprite will not start path movement
	or unhide itself. ***/
void animation_start_cycle(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].tnframe = __animation_ticker + __animations[a].delay;
	__animations[a].cycle = true;
}

/*** Stops, or pauses, the animation cycle. The sprite will still draw if shown and will move if it has a path
	in progress, but it will no longer cycle. ***/
void animation_stop_cycle(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].cycle = false;
}

/*** When the animation is stopped, destroy it. ***/
void animation_destroy_on_stop(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].dos = true;
}

/*** Starts or resumes the sprite animation's path movement. ***/
void animation_start_movement(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].moving = true;
}

/*** Stops or pauses the sprite animation's path movement. ***/
void animation_stop_movement(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].moving = false;
}

/*** Make the specified animation visible on the screen. ***/
void animation_show(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].shown = true;
}

/*** Hide the specified animation from sight on the screen. ***/
void animation_hide(animation_handle a)
{
	VALID_OR_RETURN(a);
	__animations[a].shown = false;
}

/*** Animation Z-sort (or zort, I guess) ***/
static int __animation_zort(const void* v1, const void* v2)
{
	animation* a1, * a2;
	
	a1 = (animation*)v1;
	a2 = (animation*)v2;

	return (a1->z_val - a2->z_val);
}

/*** Draws all animations belonging to the specified bitmap. Probably a good idea to call after you've drawn any
		background to bmp_owner and before you draw any UI/pop-up windows, etc. ***/
void animation_draw_all(SBitmap* bmp_owner)
{
	u32 e;
	i32 rx, ry;
	u32 acount;
	static bool in_draw = false;

	if (in_draw)
		{
		assert(false);
		return;
		}

	acount = 0;

	mtx_lock(__animation_mutex);
	in_draw = true;

	// find all of the active animations on this bitmap
	for (e = ANIMATION_HANDLE_MIN_VALID; e <= ANIMATION_HANDLE_MAX_VALID; ++e)
		{		
		if (!__animations[e].valid)
			continue;
		if (!__animations[e].shown)
			continue;
		if (__animations[e].owner != bmp_owner)
			continue;

		__animation_sort[acount++] = &__animations[e];
		}

	if (0UL == acount)
		return;	// nothing more to do

	// sort by z-value
	OUR_QSORT(__animation_sort, acount, sizeof(animation *), __animation_zort);

	// draw them onto the owner bmp with alpha-blending
	for (e = 0; e < acount; ++e)
		{
		SBitmap* frame;

		rx = POSITION_TO_PIXEL(__animation_sort[e]->x_pos);
		ry = POSITION_TO_PIXEL(__animation_sort[e]->y_pos);

		frame = __animation_sort[e]->frames[__animation_sort[e]->iframe];

		blitbmp_alphablend(frame, 0, 0, frame->w - 1, frame->h - 1, bmp_owner, rx, ry);
		}

	in_draw = false;
	mtx_unlock(__animation_mutex);
}

/*** Set the number of times the animation cycle will loop before stopping. If loops is ANIMATION_LOOP (this
	is the default value), the animation will loop indefinitely. ***/
void animation_loop(animation_handle a, u32 loop_setting)
{
	VALID_OR_RETURN(a);
	__animations[a].loops = loop_setting;
}

/*** Get the number of animation loops remaining. ***/
u32 animation_loops_remain(animation_handle a)
{
	if(!animation_handle_is_valid(a))
		return(0UL);
	return (__animations[a].rloops);
}

/*** Set the number of animation loops remaining. ***/
void animation_set_loops_remain(animation_handle a, u32 remain_loops)
{
	VALID_OR_RETURN(a);
	__animations[a].rloops = remain_loops;
}

/*** Sets a straight-line path to a point. The sprite will arrive there in "tim" time units. When it arrives,
	it is set to frame #0 and the animation cycle is paused. A callback will be called if present. */
void animation_move_to(animation_handle a, i32 x, i32 y, time_unit tim)
{
	i32 xd, yd;

	if (tim < 1)
		tim = 1UL;

	__animations[a].dest_x = PIXEL_TO_POSITION(x);
	__animations[a].dest_y = PIXEL_TO_POSITION(y);

	xd = __animations[a].dest_x - __animations[a].x_pos;
	yd = __animations[a].dest_y - __animations[a].y_pos;

	__animations[a].x_rate = xd / (i32)tim;
	__animations[a].y_rate = yd / (i32)tim;

	__animations[a].havepath = true;
}

/*** Sets the z-value of the sprite. Sprites are drawn in increasing z-order, so if you want a sprite to
	be drawn before another (to simulate depth, etc.) give it a low z-value. By default, a sprite
	is given a z-value of Z_VALUE_DEFAULT (0) on creation. ***/
void animation_set_z(animation_handle a, i32 z)
{
	VALID_OR_RETURN(a);
	__animations[a].z_val = z;
}

/*** Immediately sets the specified frame in the animation. The animation cycle, if it is on, will continue
	from this frame. Returns false on error (frame index out of bounds). ***/
bool animation_set_frame(animation_handle a, u32 frame_no)
{
	if (!animation_handle_is_valid(a))
		return(false);
	if (frame_no >= __animations[a].nframes)
		return(false);

	__animations[a].iframe = frame_no;

	return(true);
}

/*** Set the position of the specified animation on the owner bitmap. ***/
void animation_set_position(animation_handle a, i32 x, i32 y)
{
	VALID_OR_RETURN(a);

	// TODO: if we have a path, re-set the move rate here.

	__animations[a].x_pos = PIXEL_TO_POSITION(x);
	__animations[a].y_pos = PIXEL_TO_POSITION(y);
}

/*** Get the position on the owning bitmap of the animation. If the animation is not currently
	shown, POS_INVALID will be set to xpos and ypos. ***/
void animation_get_position(animation_handle a, i32* xpos, i32* ypos)
{
	VALID_OR_RETURN(a);
	if (!__animations[a].shown)
		{
		if (not_null(xpos))
			*xpos = POS_INVALID;
		if (not_null(ypos))
			*ypos = POS_INVALID;
		return;
		}

	if (not_null(xpos))
		*xpos = POSITION_TO_PIXEL(__animations[a].x_pos);
	if (not_null(ypos))
		*ypos = POSITION_TO_PIXEL(__animations[a].y_pos);
}

/*** Sets a constant movement rate. The units are 1/256th pixels per hundredth-second. ***/
void animation_set_movement_rate(animation_handle a, move_rate x_rate, move_rate y_rate)
{
	VALID_OR_RETURN(a);

	__animations[a].x_rate = x_rate;
	__animations[a].y_rate = y_rate;
}

/*** Gets the animation's current movement rate. The units are 1/256th pixels per hundredth-second. ***/
void animation_get_movement_rate(animation_handle a, move_rate* x_rate, move_rate* y_rate)
{
	VALID_OR_RETURN(a);

	if (not_null(x_rate))
		*x_rate = __animations[a].x_rate;
	if (not_null(y_rate))
		*y_rate = __animations[a].y_rate;
}

/*** Set a callback to be called when the animation loop ends. It must take an SBitmap* and an animation_handle argument. ***/
void animation_register_loop_end_callback(animation_handle a, AnimationCallback callback)
{
	VALID_OR_RETURN(a);

	__animations[a].loopend = callback;
}

/*** Set a callback to be called when a moving sprite arrives at its destination.. ***/
void animation_register_path_end_callback(animation_handle a, AnimationCallback callback)
{
	VALID_OR_RETURN(a);

	__animations[a].pathend = callback;
}

/*** Set a callback to be called when the animation cycle reaches frame #X. ***/
void animation_register_frame_callback(animation_handle a, AnimationCallback callback, u32 X)
{
	VALID_OR_RETURN(a);

	__animations[a].frm = callback;
	__animations[a].frc = X;
}

/*** Set a callback to be called when any portion of the animation (while active) is off the physical bounds of the owner bitmap. ***/
void animation_register_out_of_bounds_callback(animation_handle a, AnimationCallback callback)
{
	VALID_OR_RETURN(a);

	__animations[a].oob = callback;
}

/*** Set a callback to be called when there is a collision between this animated sprite and any other. The CollisionCallback
	takes the owner bitmap, this animation's handle, and the handle of the sprite with which it collides. ***/
void animation_register_collision_callback(animation_handle a, CollisionCallback callback)
{
	VALID_OR_RETURN(a);

	__animations[a].collide = callback;
}

/*** Returns true iff the specified animation handle is currently registered and not destroyed. Can be used to determine
	all active animations by testing from ANIMATION_HANDLE_MIN_VALID to ANIMATION_HANDLE_MAX_VALID. ***/
bool animation_handle_is_valid(animation_handle a)
{
	if (!__animation_init)
		return(false);
	if (a > ANIMATION_HANDLE_MAX_VALID)
		return(false);
	return (__animations[a].valid);
}

/*** end animation.cpp ***/