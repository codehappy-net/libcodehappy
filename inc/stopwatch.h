/***

	stopwatch.h

	Timing for libcodehappy.

	Copyright (c) 2022 Chris Street.

***/
#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

enum StopwatchUnits {
	// Time units. The stopwatch returns integeral values, so choose the unit
	// with the desired granularity.
	UNIT_MINUTE,
	UNIT_SECOND,
	UNIT_MILLISECOND,
	UNIT_MICROSECOND,
	UNIT_NANOSECOND,

	// Units below are specific to certain library build targets.
#ifdef CODEHAPPY_NATIVE
	UNIT_CPU_COUNTER,	// clicks of the hardware counter
#endif
#ifdef CODEHAPPY_SDL
	UNIT_UI_REFRESH_RATE,	// UI frames
	UNIT_UI_FRAMES = UNIT_UI_REFRESH_RATE,
#endif
};

class Stopwatch {
public:
	// Default constructor, starts measuring time elapsed since construction.
	Stopwatch();

	// Constructor that starts the stopwatch with a specified amount of time.
	Stopwatch(u64 lim, StopwatchUnits units);

	// Start measuring elapsed time (again).
	void start();

	// Start measuring elapsed time (again), and also set a period to keep track of.
	void start(u64 limit, StopwatchUnits units);

	// Return the elapsed time since the last start(), in the specified units. (The stopwatch can 
	// be stopped multiple times before being reset by another start() or restart() call.)
	u64 stop(StopwatchUnits units) const;

	// Restart the period.
	void restart();

	// Has the specified maximum period elapsed?
	bool elapsed() const;

	// Busy wait until the maximum period has elapsed.
	void busy_wait() const;

	// Sleep until the maximum period has elapsed.
	void sleep() const;

	// Is this stopwatch tracking a period?
	bool has_period() const { return start_limit > 0ULL; }

private:
	bool short_period_helper(u64 left) const;

#ifdef CODEHAPPY_NATIVE
	u64 cpu_counter;
#endif
#ifdef CODEHAPPY_SDL
	u32 ui_frame;
#endif
	u64 start_limit;
	StopwatchUnits start_units;
	std::chrono::high_resolution_clock::time_point tp;
};

#endif  // _STOPWATCH_H_
/* end stopwatch.h */