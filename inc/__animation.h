/***

	__animation.h

	Sprite animation for my library.

	C. M. Street

***/
#ifndef ANIMATION_H
#define	ANIMATION_H

/*** animation_handle: uniquely identifies a registered animation ***/
typedef	u32
	animation_handle;

/*** time_unit: the time unit used in animation delays, etc. Each time
	unit is 1/100th of a second. ***/
typedef	u32
	time_unit;

/*** move_rate: the unit in which animations move. Each move_rate means
	1/256 of a pixel per 1/100th of a second. ***/
typedef	i32
	move_rate;

/*** animation callback function ptr types ***/
typedef void (*AnimationCallback)(SBitmap *, animation_handle);
typedef void (*CollisionCallback)(SBitmap *, animation_handle, animation_handle);

// TODO: CollisionCallback should also take a void* provided when the callback was registered -- this should be saved on animation

#define	ANIMATION_LOOP				(0xffffffff)
#define	Z_VALUE_DEFAULT				(0)
#define	POS_INVALID					(0x8fffffff)
#define	ANIMATION_HANDLE_MIN_VALID	(0)
#define	ANIMATION_HANDLE_MAX_VALID	(1023)
#define	MAX_ANIMATIONS				(ANIMATION_HANDLE_MAX_VALID - ANIMATION_HANDLE_MIN_VALID + 1)
#define	ANIMATION_INVALID			((animation_handle) 0xffffffff)

#define	PIXEL_TO_POSITION(p)		((p) << 8)
#define	POSITION_TO_PIXEL(p)		((p) >> 8)

// TODO: spline/Bezier curve paths fit to multiple points.

// TODO: there are other kinds of animations besides sprites: marquee text, color cycles/animations/fade in/out, plasmas, sfx, etc.

/*** Starts the internal animation handler thread. Must call
	before registering any animations or using any other
	animation functions. ***/
extern void animation_init(void);

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
extern animation_handle animation_register(SBitmap* bmp_owner, SBitmap** frames, u32 num_frames, time_unit delay);

/*** Stops, hides, and destroys the specified animation. The handle is no longer valid after the call. ***/
extern void animation_destroy(animation_handle a);

/*** Starts, or unpauses, the animation cycle and animation path movement. If "show" is true, also
	shows/unhides the animation. (Note that if the animation is already shown, show == false has
	no effect.) ***/
extern void animation_start(animation_handle a, bool show);

/*** Stops, or pauses, the animation cycle and animation path movement. The animation will still draw unless
	animation_hide() or animation_destroy_on_stop() has been called. If "hide" is true, also hides the
	animation. (Note that if the animation is already hidden, hide == false has no effect.) ***/
extern void animation_stop(animation_handle a, bool hide);

/*** Start, or unpause, the animation cycle for the specified animation. The sprite will not start path movement
	or unhide itself. ***/
extern void animation_start_cycle(animation_handle a);

/*** Stops, or pauses, the animation cycle. The sprite will still draw if shown and will move if it has a path
	in progress, but it will no longer cycle. ***/
extern void animation_stop_cycle(animation_handle a);

/*** When the animation is stopped, destroy it. ***/
extern void animation_destroy_on_stop(animation_handle a);

/*** Starts or resumes the sprite animation's path movement. ***/
extern void animation_start_movement(animation_handle a);

/*** Stops or pauses the sprite animation's path movement. ***/
extern void animation_stop_movement(animation_handle a);

/*** Make the specified animation visible on the screen. ***/
extern void animation_show(animation_handle a);

/*** Hide the specified animation from sight on the screen. ***/
extern void animation_hide(animation_handle a);

/*** Draws all animations belonging to the specified bitmap. Probably a good idea to call after you've drawn any
		background to bmp_owner and before you draw any UI/pop-up windows, etc. ***/
extern void animation_draw_all(SBitmap* bmp_owner);

/*** Set the number of times the animation cycle will loop before stopping. If loops is ANIMATION_LOOP (this
	is the default value), the animation will loop indefinitely. ***/
extern void animation_loop(animation_handle a, u32 loop_setting);

/*** Get the number of animation loops remaining. ***/
extern u32 animation_loops_remain(animation_handle a);

/*** Set the number of animation loops remaining. ***/
extern void animation_set_loops_remain(animation_handle a, u32 remain_loops);

/*** Sets a straight-line path to a point. The sprite will arrive there in "tim" time units. When it arrives,
	it is set to frame #0 and the animation cycle is paused. */
extern void animation_move_to(animation_handle a, i32 x, i32 y, time_unit tim);

/*** Sets the z-value of the sprite. Sprites are drawn in increasing z-order, so if you want a sprite to
	be drawn before another (to simulate depth, etc.) give it a low z-value. By default, a sprite
	is given a z-value of Z_VALUE_DEFAULT (0) on creation. ***/
extern void animation_set_z(animation_handle a, i32 z);

/*** Immediately sets the specified frame in the animation. The animation cycle, if it is on, will continue
	from this frame. Returns false on error (frame index out of bounds). ***/
extern bool animation_set_frame(animation_handle a, u32 frame_no);

/*** Set the position of the specified animation on the owner bitmap. ***/
extern void animation_set_position(animation_handle a, i32 x, i32 y);

/*** Get the position on the owning bitmap of the animation. If the animation is not currently
	shown, POS_INVALID will be set to xpos and ypos. ***/
extern void animation_get_position(animation_handle a, i32* xpos, i32* ypos);

/*** Sets a constant movement rate. The units are 1/256th pixels per hundredth-second. ***/
extern void animation_set_movement_rate(animation_handle a, move_rate x_rate, move_rate y_rate);

/*** Gets the animation's current movement rate. The units are 1/256th pixels per hundredth-second. ***/
extern void animation_get_movement_rate(animation_handle a, move_rate* x_rate, move_rate* y_rate);

/*** Set a callback to be called when the animation loop ends. It must take an SBitmap* and an animation_handle argument. ***/
extern void animation_register_loop_end_callback(animation_handle a, AnimationCallback callback);

/*** Set a callback to be called when a moving sprite arrives at its destination.. ***/
extern void animation_register_path_end_callback(animation_handle a, AnimationCallback callback);

/*** Set a callback to be called when the animation cycle reaches frame #X. 
	Only one can be registered at a time for a given animation, but you can always register the next desired frame in
	the callback function itself. ***/
extern void animation_register_frame_callback(animation_handle a, AnimationCallback callback, u32 X);

/*** Set a callback to be called when any portion of the animation (while active) is off the physical bounds of the owner bitmap. ***/
extern void animation_register_out_of_bounds_callback(animation_handle a, AnimationCallback callback);

/*** Set a callback to be called when there is a collision between this animated sprite and any other. The CollisionCallback
	takes the owner bitmap, this animation's handle, and the handle of the sprite with which it collides. ***/
extern void animation_register_collision_callback(animation_handle a, CollisionCallback callback);

/*** Returns true iff the specified animation handle is currently registered and not destroyed. Can be used to determine
	all active animations by testing from ANIMATION_HANDLE_MIN_VALID to ANIMATION_HANDLE_MAX_VALID. ***/
extern bool animation_handle_is_valid(animation_handle a);

/*** Registers and immediately starts a special animation: the fade out (fade-to-black). Will complete in tim time units. ***/
extern void fade_out(SBitmap* bmp_owner, time_unit tim);

/***
	Draws all the animations that belong to the specified bmp. Typically should be called from your screen-
	redraw function. This function does not force a UI dirty -- it is left to the caller to do that. It also does
	not lock the redraw mutex -- the assumption is that the caller is responsible for that also.
***/
extern void animation_draw_all(SBitmap* bmp);

/*** Associate data with the specified animation. This way you can check the data in your callbacks. ***/
extern void animation_associate_data(animation_handle a, void* data);

/*** Retrieve the data associated with the animation handle, if any. ***/
extern void* animation_get_data(animation_handle a);

// going to have to cache the bmp somewhere to do fade_in/fade_to, etc.
#if 0
/*** Registers and immediately starts a special animation: the fade in (fade-from-black). Will complete in tim time units. ***/
extern void fade_in(SBitmap* bmp_owner, time_unit tim);

extern void fade_to(SBitmap* bmp_owner, SBitmap* bmp_fade_to, time_unit tim);
#endif

#endif  // ANIMATION_H 
/* end __animation.h */
