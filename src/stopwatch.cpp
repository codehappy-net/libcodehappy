/***

	stopwatch.cpp

	Timing for libcodehappy.

	Copyright (c) 2022 Chris Street.

***/
#include <thread>

Stopwatch::Stopwatch() {
	start();
}

Stopwatch::Stopwatch(u64 lim, StopwatchUnits units) {
	start(lim, units);
}

void Stopwatch::start() {
	start_limit = 0ULL;
	start_units = UNIT_MILLISECOND;
	restart();
}

void Stopwatch::start(u64 limit, StopwatchUnits units) {
	start_limit = limit;
	start_units = units;
	restart();
}

u64 Stopwatch::stop(StopwatchUnits units) const {
	switch (units) {
	case UNIT_MINUTE:
		{
		std::chrono::minutes elapsed = std::chrono::duration_cast<std::chrono::minutes>(
				std::chrono::high_resolution_clock::now() - tp);
		return elapsed.count();
		}
	case UNIT_SECOND:
		{
		std::chrono::seconds elapsed = std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::high_resolution_clock::now() - tp);
		return elapsed.count();
		}
	case UNIT_MILLISECOND:
		{
		std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - tp);
		return elapsed.count();
		}
	case UNIT_MICROSECOND:
		{
		std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::high_resolution_clock::now() - tp);
		return elapsed.count();
		}
	case UNIT_NANOSECOND:
		{
		std::chrono::nanoseconds elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::high_resolution_clock::now() - tp);
		return elapsed.count();
		}

#ifdef CODEHAPPY_NATIVE
	case UNIT_CPU_COUNTER:
		return hardware_counter() - cpu_counter;
#endif

#ifdef CODEHAPPY_SDL
	case UNIT_UI_REFRESH_RATE:
		return u64(codehappy_frame() - ui_frame);
#endif
	}
	return 0ULL;
}

bool Stopwatch::elapsed() const {
	if (iszero(start_limit))
		return true;
	return stop(start_units) >= start_limit;
}

void Stopwatch::busy_wait() const {
	while (!elapsed())
		;
}

bool Stopwatch::short_period_helper(u64 left) const {
#ifdef CODEHAPPY_NATIVE
	if (start_units == UNIT_CPU_COUNTER && left < 1000000000ULL) {
		busy_wait();
		return true;
	}
#endif
#ifdef CODEHAPPY_SDL
	if (start_units == UNIT_UI_REFRESH_RATE && left <= 1) {
		busy_wait();
		return true;
	}
#endif
	if (start_units == UNIT_MILLISECOND && left < 10UL) {
		busy_wait();
		return true;
	}
	if (start_units == UNIT_MICROSECOND && left < 10000UL) {
		busy_wait();
		return true;
	}
	if (start_units == UNIT_NANOSECOND && left < 10000000UL) {
		busy_wait();
		return true;
	}
	return false;
}

void Stopwatch::sleep() const {
	/* For very short periods, just busy wait. */
	if (short_period_helper(start_limit))
		return;
	forever {
		u64 el = stop(start_units);
		if (el >= start_limit)
			return;
		u64 remain = start_limit - el;
		if (short_period_helper(remain))
			return;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Stopwatch::restart() {
	tp = std::chrono::high_resolution_clock::now();
#ifdef CODEHAPPY_SDL
	ui_frame = codehappy_frame();
#endif
#ifdef CODEHAPPY_NATIVE
	cpu_counter = hardware_counter();
#endif
}

/*** end stopwatch.cpp ***/