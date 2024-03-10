/***

	calendar.h

	Basic calendar and date functions.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef CALENDAR_H
#define CALENDAR_H

typedef int32_t
	Year;

typedef uint32_t
	Month;

typedef uint32_t
	Day;

typedef uint32_t
	Hour;

typedef uint32_t
	Minute;

typedef uint32_t
	Second;

enum Weekday {
	weekday_Monday = 0,
	weekday_Tuesday = 1,
	weekday_Wednesday = 2,
	weekday_Thursday = 3,
	weekday_Friday = 4,
	weekday_Saturday = 5,
	weekday_Sunday = 6,
};

enum Calendar {
	calendar_Auto = -1,
	calendar_Julian = 0,
	calendar_Gregorian,
	calendar_British,
};

/*** Holidays and days of note on the calendar. ***/
enum Holiday {
	HOLIDAY_UNKNOWN = 0,
	HOLIDAY_CHRISTMAS,
	HOLIDAY_EASTER,
	HOLIDAY_NEW_YEARS_DAY,
	HOLIDAY_ASH_WEDNESDAY,
	HOLIDAY_PENTECOST,
	HOLIDAY_HALLOWEEN,
	HOLIDAY_MARDI_GRAS,
	HOLIDAY_CHRISTMAS_EVE,
	HOLIDAY_THANKSGIVING_USA,
	HOLIDAY_THANKSGIVING_CANADA,
	HOLIDAY_HANUKKAH_STARTS,
	HOLIDAY_PASSOVER_BEGINS,
	HOLIDAY_PURIM,
	HOLIDAY_ROSH_HASHANAH,
	HOLIDAY_YOM_KIPPUR,
	HOLIDAY_KWANZAA_STARTS,
	HOLIDAY_BOXING_DAY,
	HOLIDAY_NEW_YEARS_EVE,
	HOLIDAY_MOTHERS_DAY_USA,
	HOLIDAY_FATHERS_DAY_USA,
	HOLIDAY_LABOR_DAY_USA,
	HOLIDAY_MARTIN_LUTHER_KING_DAY,
	HOLIDAY_GROUNDHOG_DAY,
	HOLIDAY_PRESIDENTS_DAY_USA,
	HOLIDAY_EARTH_DAY,
	HOLIDAY_JUNETEENTH,
	HOLIDAY_ARBOR_DAY_USA,
	HOLIDAY_CANADA_DAY,
	HOLIDAY_INDEPENDENCE_DAY_USA,
	HOLIDAY_INDIGENOUS_PEOPLES_DAY_USA,	// sometimes known as "Columbus Day"
	HOLIDAY_GUY_FAWKES_NIGHT,
	HOLIDAY_ASCENSION_THURSDAY,
	HOLIDAY_CANDLEMAS,
	HOLIDAY_GOOD_FRIDAY,
	HOLIDAY_ST_STEPHENS_DAY,
	HOLIDAY_APRIL_FOOLS_DAY,
	HOLIDAY_TALK_LIKE_A_PIRATE_DAY,
	HOLIDAY_PI_DAY,
	HOLIDAY_ST_PATRICKS_DAY,
	HOLIDAY_PALM_SUNDAY,
};

#define	AVG_DAYS_YEAR	365.25

struct Date {
	void out_ramfile(RamFile* rf) const;
	void in_ramfile(RamFile* rf);
	Year	  y;
	Month	  m;
	Day	  d;
	Calendar  c;
};

struct Time {
	void out_ramfile(RamFile* rf) const;
	void in_ramfile(RamFile* rf);
	Hour	h;
	Minute	m;
	Second	s;
	int32_t tz;
	int32_t daylight;
};

struct DateTime {
	void out_ramfile(RamFile* rf) const;
	void in_ramfile(RamFile* rf);
	Date d;
	Time t;
};

struct FullTime {
	void out_ramfile(RamFile* rf) const;
	void in_ramfile(RamFile* rf);
	Date d;
	Time t;
	uint32_t nanos;
};

/*** is the specified year (Gregorian calendar) a leap year? ***/
extern int isleapyear(Year y);

/*** is the specified year in the specified calendar a leap year? ***/
extern int isleapyear_cal(Year y, Calendar c);

/*** the number of days in a (Gregorian calendar) year ***/
extern uint32_t days_in_year(Year y);
extern uint32_t days_in_year_date(const Date* d);
extern u32 days_in_year(const DateTime* dt);

/*** the number of days in a year of the specified calendar ***/
extern uint32_t days_in_year_cal(Year y, Calendar c);

/*** the number of days in a specified (Gregorian calendar) month ***/
extern uint32_t days_in_month(Year y, Month m);
extern uint32_t days_in_month_date(const Date* d);

/*** the number of days in a month of the specified calendar ***/
extern uint32_t days_in_month_cal(Year y, Month m, Calendar c);

/*** Return Julian date. Use the calendar specified by the Date object. ***/
extern int32_t julian_date(const Date* d);
extern int32_t julian_date(const DateTime* dt);

/*** Return a Julian date from a yyyymmdd date. ***/
extern int32_t yyyymmdd_to_julian(const int yyyymmdd);

/*** Converts a yyyymmdd date to an mdy date. ***/
extern void yyyymmdd_to_mdy(i32 yyyymmdd, Month* mout, Day* dout, Year* yout);
extern int yyyymmdd_year(int yyyymmdd);
extern int yyyymmdd_month(int yyyymmdd);
extern int yyyymmdd_day(int yyyymmdd);

/*** Return a Julian date from an mdy date. ***/
extern int32_t mdy_to_julian(const Month m, const Day d, const Year y);

/*** Converts a mdy date to a yyyymmdd date. ***/
int32_t mdy_to_yyyymmdd(const Month m, const Day d, const Year y);

// what about the Eastern Orthodox calendar? They didn't assume the
// Gregorian calendar until ~1918 or so

/*** Return the date as an integer in YYYYMMDD form ***/
extern uint32_t date_to_yyyymmdd(const Date* d);
extern uint32_t datetime_to_yyyymmdd(const DateTime* dt);

/*** Fill a Date structure from a yyyymmdd date; return 1 on success. ***/
extern int yyyymmdd_to_date(const int yyyymmdd, Date* d);
extern int yyyymmdd_to_datetime(int yyyymmdd, DateTime* dt);

/*** Return the day of the week ***/
extern Weekday weekday(const Date* d);
extern Weekday weekday(const DateTime* dt);
extern Weekday weekday_from_mdy(Month m, Day d, Year y);
extern Weekday weekday_yyyymmdd(int yyyymmdd);
extern Weekday weekday_from_julian_date(int32_t jd);

/*** Return a string representing the specified day of the week ***/
extern const char* weekday_cstr(const Weekday wd);

/*** Is the date specified by the structure valid? ***/
extern int valid_date(const Date* d);
extern int valid_date(int yyyymmdd);

/*** Determine the difference between two dates, in days. ***/
/*** This is effectively d2 - d1, so negative # of days is possible. ***/
extern int days_difference(const Date* d1, const Date* d2);
extern int days_difference(const DateTime* d1, const DateTime* d2);
extern int days_difference_yyyymmdd(const int d1, const int d2);

/*** Determine the difference between two dates, in years. ***/
extern double years_difference(const Date* d1, const Date* d2);
extern double years_difference_yyyymmdd(const int d1, const int d2);

/*** Construct Time structures. ***/
extern void hhmmss_to_time(const int hhmmss, Time* t);
extern void hhmm_to_time(const int hhmm, Time* t);

/*** Output Time structures as hhmm or hhmmss times. ***/
extern uint32_t time_to_hhmmss(const Time* t);
extern uint32_t time_to_hhmm(const Time* t);

/*** Determine the difference between two times, in seconds. ***/
extern int64_t seconds_difference(const Time* t1, const Time* t2);
extern int64_t seconds_difference_hhmm(const int h1, const int h2);
extern int64_t seconds_difference_hhmmss(const int hms1, const int hms2);

/*** Determine the difference between two times, in minutes. ***/
extern int32_t full_minutes_difference(const Time* t1, const Time* t2);
extern double minutes_difference(const Time* t1, const Time* t2);

/*** Determine the difference between two times, in hours. ***/
extern int32_t full_hours_difference(const Time* t1, const Time* t2);
extern double hours_difference(const Time* t1, const Time* t2);

/*** Determine the difference between two date/times, in seconds. ***/
extern int64_t seconds_difference_dt(const DateTime* t1, const DateTime* t2);

/*** Determine the difference between two fulltimes, in seconds. ***/
extern int64_t full_seconds_difference_ft(const FullTime *f1, const FullTime *f2);

/*** Determine the difference between two date/times, in days. ***/
extern int full_days_difference(const DateTime* t1, const DateTime* t2);
extern double days_difference_dt(const DateTime* t1, const DateTime* t2);

/*** Return the effective calendar for the specified date. ***/
extern Calendar effective_calendar_for_date(const Date* d);

/*** Convert a DateTime to a Unix epoch timestamp ***/
extern int64_t unix_epoch_seconds(const DateTime* dt);

/*** Convert a FullTime to a Unix epoch timestamp (in s, ms, or us) ***/
extern int64_t unix_epoch_seconds_ft(const FullTime* ft);
extern int64_t unix_epoch_msec_ft(const FullTime* ft);
extern int64_t unix_epoch_usec_ft(const FullTime* ft);

/*** Return the Julian day of the year (Jan. 1 = 1, Feb. 1 = 32, etc.) ***/
extern uint32_t julian_day_of_year(const Date* d);
extern uint32_t julian_day_of_year(const DateTime* dt);

/*** Normalize a Time or DateTime object to use UTC timezone ***/
extern void normalize_tz_dt(DateTime* dt);
extern void normalize_tz(Time* t);

/*** Increment/decrement the DateTime by the specified number of seconds ***/
extern void change_datetime(DateTime* dt, const int nsec);

/*** Increment/decrement the Time by the specified number of seconds ***/
extern void change_time(Time* dt, const int nsec);

/*** Get the current local DateTime ***/
extern void get_current_datetime(DateTime* dt);

/*** Set the Date from the passed-in Julian date. Uses the calendar
	on the Date. ***/
extern void set_julian_date(Date *d, const int32_t jd);
extern void set_julian_date(DateTime *dt, const int32_t jd);

/***
	Formats the specified DateTime as a string in the given	
	format.

	The format string recognizes the following escape sequences
	(note that they are case-sensitive):

		%D	insert day (two digits, zero-padded if necessary)
		%d	insert day (as digits, not padded)
		%H	insert hour (two digits, 12 hour clock, zero-padded if necessary)
		%h	insert hour (as digits, 12 hour clock, not padded)
		%I	insert hour (two digits, 24 hour clock, zero-padded if necessary)
		%i	insert hour (as digits, 24 hour clock, not padded)
		%M	insert month (full word)
		%m	insert month (three letter abbreviation, e.g. "Feb")
		%N	insert month (two digits, zero-padded if necessary)
		%n	insert month (as digits, not padded)
		%O	insert day's ordinal (e.g. "rd" for the 3rd day)
		%P	insert meridian in caps ("A.M." or "P.M.")
		%p	insert meridian in lowercase ("a.m." or "p.m.")
		%Q	as %P but without periods ("AM" or "PM")
		%q	as %p but without periods ("am" or "pm")
		%S	insert seconds (two digits, zero-padded if necessary)
		%s	insert seconds (as digits, not padded)
		%T	insert minute (two digits, zero-padded if necessary)
		%t	insert minute (as digits, not padded)
		%W	insert weekday (full word)
		%w	insert weekday (three letter abbreviation, e.g. "Wed")
		%x	insert DST flag value
		%Y	insert year (all digits)
		%y	insert two-digit year (zero padded if necessary)
		%z	insert timezone offset

	Other characters are written as literals to the output string.

	The return value is allocated with malloc() and should
	be free()d.
***/
extern char *fmt_datetime(const DateTime* dt, const char* fmt);

/*** A variety of pre-defined datetime formats. ***/
/*** "Monday, May 25th, 2015 12:41 p.m." ***/
extern char *fmt_datetime_american_long(const DateTime* dt);
/*** "Monday, May 25th, 2015" ***/
extern char *fmt_date_american_long(const DateTime* dt);
/*** "May 25, 2015 12:41 p.m." fixed width ***/
extern char *fmt_datetime_american_short(const DateTime* dt);
/*** "Feb 25, 2015" fixed width ***/
extern char *fmt_date_american_short(const DateTime* dt);
/*** "Monday 25 Feb 2015 17:41" ***/
extern char *fmt_datetime_europe_long(const DateTime* dt);
/*** "Monday 25 Feb 2015" ***/
extern char *fmt_date_europe_long(const DateTime* dt);
/*** "25 Feb 2015" fixed width ***/
extern char *fmt_date_europe_short(const DateTime* dt);
/*** "Mon 25-May-2015 17:41:03" fixed width ***/
extern char *fmt_datetime_shell_long(const DateTime* dt);
/*** "25-May-2015 17:41" fixed width ***/
extern char *fmt_datetime_shell(const DateTime* dt);
/*** "25-May-2015" fixed width ***/
extern char *fmt_date_shell(const DateTime* dt);
/*** "25-May-15 17:41" fixed width ***/
extern char *fmt_datetime_dos(const DateTime* dt);
/*** "25-May-15" fixed width ***/
extern char *fmt_date_dos(const DateTime* dt);

/*** Versions of the above date formatting functions that take yyyymmdd format dates. */
extern char *fmt_date_american_long(int yyyymmdd);
extern char *fmt_date_american_short(int yyyymmdd);
extern char *fmt_date_europe_long(int yyyymmdd);
extern char *fmt_date_europe_short(int yyyymmdd);
extern char *fmt_date_shell(int yyyymmdd);
extern char *fmt_date_dos(int yyyymmdd);

/*** general date formatting ***/
extern char *fmt_date(const Date* d, const char* fmt);
extern char *fmt_date(int yyyymmdd, const char* fmt);

/*** Output the given string to ostream, and then free it (can be used to output fmt_datetime() return values, e.g.). ***/
extern void output_and_free(std::ostream& o, char* str);

/*** Places the nth weekday of the specified type in the specified month
	in the Date object. Returns 0 on success or -1 if the date does not
	exist. ***/
extern int nth_weekday_in_month(Date* d, int n, Weekday wd);

/*** Places the last weekday of the specified type in the specified month
	in the Date object. ***/
extern int last_weekday_in_month(Date* d, Weekday wd);

/*** Return the day of the year for the specified date: Jan. 1 is 1, Dec. 31 is 365 or 366 ***/
// Note: these functions are faster than julian_day_of_year() -- deprecate the latter?
extern int day_of_year(const Date* date);
extern int day_of_year(const DateTime* date);
extern int day_of_year_yyyymmdd(const int date);
extern int day_of_year_mdy(const Month m, const Day d, const Year y);

/*** Find the next and previous weekday (Monday through Friday) ***/
extern void next_weekday(const Date* in_date, Date* out_date);
extern int next_weekday_yyyymmdd(const int yyyymmdd);
extern void next_weekday_mdy(const Month m, const Day d, const Year y,
	Month* mout, Day *dout, Year *yout);
extern void prev_weekday(const Date* in_date, Date* out_date);
extern int prev_weekday_yyyymmdd(const int yyyymmdd);
extern void prev_weekday_mdy(const Month m, const Day d, const Year y,
	Month* mout, Day *dout, Year *yout);

/*** Fill the Date object from the given Julian date ***/
extern void julian_to_date(u32 jdate, Date* date_output);
extern void julian_to_date(u32 jdate, DateTime* date_output);

/*** Add days to the specified Date object ***/
extern void add_days(Date* date, i32 num_days);
extern void add_days(DateTime* date, i32 num_days);

/*** Returns the Date for the specified Holiday in the given Year. ***/
extern void holiday(Holiday holiday, Year y, Date* date_out);
extern void holiday(Holiday holiday, Year y, DateTime* date_out);

#endif // CALENDAR_H
