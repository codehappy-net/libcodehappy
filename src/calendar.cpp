/***

	calendar.cpp

	Basic calendar, date, and time functions. Now part of libcodehappy!

	Copyright (c) 2014-2022 Chris Street

***/
#include "libcodehappy.h"
#include <time.h>

int isleapyear(Year y) {
	if (unlikely(y < 0))
		y = (-y) - 1;
	if ((y & 3) != 0)
		return(0);
	if ((y % 25) == 0)
		return ((y & 15) == 0);
	return(1);
}

uint32_t days_in_year(Year y) {
	return 365UL + isleapyear(y);
}

uint32_t days_in_month(Year y, Month m) {
	const uint32_t __dim[] =
		{ 0UL, 31UL, 28UL, 31UL, 30UL, 31UL, 30UL, 31UL, 31UL,
		 30UL, 31UL, 30UL, 31UL };
	if (2UL == m)
		return(28UL + isleapyear(y));
	if (unlikely(m < 1 || m > 12))
		return(0);
	return __dim[m];
}

static int32_t __julian_date_base(const Date* d) {
	return ymd_to_jdn(d->y, d->m, d->d, -1, 1);
}

static int32_t __julian_date_protestant(const Date* d) {
	return ymd_to_jdn(d->y, d->m, d->d, -1, 0);
}

static int32_t __julian_date_gregorian(const Date* d) {
	return ymd_to_jdn(d->y, d->m, d->d, 0, 0);
}

static int32_t __julian_date_julian(const Date* d) {
	return ymd_to_jdn(d->y, d->m, d->d, 1, 0);
}

int32_t julian_date(const Date* d) {
	switch (effective_calendar_for_date(d))
		{
	case calendar_Julian:
		return(__julian_date_julian(d));
	case calendar_British:
		return(__julian_date_protestant(d));
	case calendar_Gregorian:
	case calendar_Auto:
		break;
		}
	return(__julian_date_base(d));
}

Calendar effective_calendar_for_date(const Date* d) {
	int dd;

	switch (d->c)
		{
	case calendar_Auto:
		dd = date_to_yyyymmdd(d);
		if (dd <= 15821004)
			return(calendar_Julian);
		return(calendar_Gregorian);

	default:
		return(d->c);
		}

	return(calendar_Auto);	// hum
}

int isleapyear_cal(Year y, Calendar c) {
	int BC = 0;

	if (unlikely(y < 0)) {
		y = (-y) - 1;
		BC = 1;
	}
	switch (c) {
	case calendar_Julian:
		if (y < 1582 || BC)
			return((y & 3) == 0);
		break;
	case calendar_British:
		if (y <= 1752 || BC)
			return((y & 3) == 0);
		break;
	case calendar_Gregorian:
	case calendar_Auto:
		break;
	}
	return(isleapyear(y));
}

uint32_t days_in_year_cal(Year y, Calendar c) {
	// Handle 1582, 1752 as special cases
	switch (y) {
	case 1582:
		if (c != calendar_British)
			return 355UL;
		break;
	case 1752:
		if (c == calendar_British)
			return 355UL;
		break;
	}
	return(365 + isleapyear_cal(y, c));
}

uint32_t days_in_month_cal(Year y, Month m, Calendar c) {
	// Handle December 1582, September 1752 as special cases
	switch (y) {
	case 1582:
		if (c != calendar_British && m == 12)
			return 21UL;
		break;
	case 1752:
		if (c == calendar_British && m == 9)
			return 19UL;
		break;
	}
	if (2 == m)
		return(28UL + isleapyear_cal(y, c));
	return days_in_month(y, m);
}

uint32_t date_to_yyyymmdd(const Date* d) {
	return (d->y * 10000UL + d->m * 100UL + d->d);
}

Weekday weekday(const Date* d) {
	return (Weekday)(julian_date(d) % 7);
}

Weekday weekday_from_julian_date(int32_t jd) {
	return (Weekday)(jd % 7);
}

const char* weekday_cstr(const Weekday wd) {
	static const char* wds[] = {
		"Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday"
	};
	if (wd > 6)
		return wds[0];

	return wds[(unsigned int)(wd)];
}

Weekday weekday_from_mdy(Month m, Day d, Year y) {
	Date dd;
	dd.m = m;
	dd.d = d;
	dd.y = y;
	dd.c = calendar_Auto;
	return weekday(&dd);
}

Weekday weekday_yyyymmdd(int yyyymmdd) {
	Date d;
	yyyymmdd_to_date(yyyymmdd, &d);
	return weekday(&d);
}

int yyyymmdd_to_date(const int yyyymmdd, Date* d) {
	d->y = (Year)(yyyymmdd / 10000);
	d->m = (Month)((yyyymmdd / 100) % 100);
	d->d = (Day)(yyyymmdd % 100);
	d->c = calendar_Auto;
	return valid_date(d);
}

int yyyymmdd_year(int yyyymmdd) {
	return yyyymmdd / 10000;
}

int yyyymmdd_month(int yyyymmdd) {
	return (yyyymmdd / 100) % 100;
}

int yyyymmdd_day(int yyyymmdd) {
	return yyyymmdd % 100;
}

int32_t yyyymmdd_to_julian(const int yyyymmdd) {
	Date d;
	yyyymmdd_to_date(yyyymmdd, &d);
	return julian_date(&d);
}

int32_t mdy_to_julian(const Month m, const Day d, const Year y) {
	Date dd;
	dd.m = m;
	dd.d = d;
	dd.y = y;
	dd.c = calendar_Auto;
	return julian_date(&dd);
}

int32_t mdy_to_yyyymmdd(const Month m, const Day d, const Year y) {
	return (i32)m * 100 + (i32)d + (i32)y * 10000;	
}

int valid_date(const Date* d) {
	if (d->m == 0 || d->m > 12)
		return(0);
	if (d->d == 0 || d->d > days_in_month_date(d))
		return(0);
	return(1);
}

int valid_date(int yyyymmdd) {
	Date d;
	yyyymmdd_to_date(yyyymmdd, &d);
	return valid_date(&d);
}


uint32_t days_in_year_date(const Date* d) {
	return days_in_year_cal(d->y, d->c);
}

uint32_t days_in_month_date(const Date* d) {
	return days_in_month_cal(d->y, d->m, d->c);
}

int days_difference(const Date* d1, const Date* d2) {
	return julian_date(d2) - julian_date(d1);
}

int days_difference_yyyymmdd(const int d1, const int d2) {
	Date date1;
	Date date2;
	yyyymmdd_to_date(d1, &date1);
	yyyymmdd_to_date(d2, &date2);
	return days_difference(&date1, &date2);
}

double years_difference(const Date* d1, const Date* d2) {
	return (double)(days_difference(d1, d2)) / AVG_DAYS_YEAR;
}

double years_difference_yyyymmdd(const int d1, const int d2) {
	return (double)(days_difference_yyyymmdd(d1, d2)) / AVG_DAYS_YEAR;
}

void hhmmss_to_time(const int hhmmss, Time* t) {
	t->h = hhmmss / 10000;
	t->m = (hhmmss / 100) % 100;
	t->s = hhmmss % 100;
	t->tz = 0;
	t->daylight = 0;
}

void hhmm_to_time(const int hhmm, Time* t) {
	t->h = hhmm / 100;
	t->m = hhmm % 100;
	t->s = 0;
	t->tz = 0;
	t->daylight = 0;
}

static int64_t __normalized_seconds_difference(const Time* t1, const Time* t2) {
	/*** normalized: timezone is UTC ***/
	return
		(int)(t2->s) - (int)(t1->s) + 
		((int)(t2->m) - (int)(t1->m)) * 60LL +
		((int)(t2->h) - (int)(t1->h)) * 3600LL;
}

int64_t seconds_difference(const Time* t1, const Time* t2) {
	Time tn1, tn2;

	tn1 = *t1;
	tn2 = *t2;
	normalize_tz(&tn1);
	normalize_tz(&tn2);

	return __normalized_seconds_difference(&tn1, &tn2);
}

int64_t seconds_difference_hhmmss(const int hms1, const int hms2) {
	Time t1;
	Time t2;
	hhmmss_to_time(hms1, &t1);
	hhmmss_to_time(hms2, &t2);
	return seconds_difference(&t1, &t2);
}

int64_t seconds_difference_hhmm(const int hm1, const int hm2) {
	Time t1;
	Time t2;
	hhmm_to_time(hm1, &t1);
	hhmm_to_time(hm2, &t2);
	return seconds_difference(&t1, &t2);
}

uint32_t time_to_hhmmss(const Time* t) {
	return t->h * 10000UL + t->m * 100UL + t->s;
}

uint32_t time_to_hhmm(const Time* t) {
	return t->h * 100UL + t->m;
}

int32_t full_minutes_difference(const Time* t1, const Time* t2) {
	int32_t m;
	if (time_to_hhmmss(t2) < time_to_hhmmss(t1))
		return -full_minutes_difference(t2, t1);
	m = ((int)(t2->h) - (int)(t1->h)) * 60 +
		((int)(t2->m) - (int)(t1->m));
	if ((int)(t2->s) < (int)(t1->s))
		--m;
	return(m);
}

double minutes_difference(const Time* t1, const Time* t2) {
	return (double)(seconds_difference(t1, t2)) / 60.0;
}

double hours_difference(const Time* t1, const Time* t2) {
	return (double)(seconds_difference(t1, t2)) / 3600.0;
}

static int32_t __normalized_full_hours_difference(const Time* t1, const Time* t2) {
	/*** normalized: timezone is UTC and t1 is <= t2 ***/
	int32_t h;

	h = ((int)(t2->h) - (int)(t1->h));
	if ((int)(t2->m) < (int)(t1->m))
		--h;
	else if ((int)(t2->m) == (int)(t1->m) && 
		(int)(t2->s) < (int)(t1->s))
		--h;
	return(h);
}

int32_t full_hours_difference(const Time* t1, const Time* t2) {
	int32_t h;
	Time tn1, tn2;

	tn1 = *t1;
	tn2 = *t2;
	normalize_tz(&tn1);
	normalize_tz(&tn2);

	if (time_to_hhmmss(&tn2) < time_to_hhmmss(&tn1))
		return -__normalized_full_hours_difference(&tn2, &tn1);

	return __normalized_full_hours_difference(&tn1, &tn2);
}

static int64_t __normalized_seconds_difference_dt(const DateTime* t1, const DateTime* t2) {
	/*** normalized: timezone is UTC and t1 is <= t2 ***/
	int64_t ret = 0LL;

	ret = (int64_t)(days_difference(&t1->d, &t2->d)) * 86400LL;
	ret += seconds_difference(&t1->t, &t2->t);

	return(ret);
}

int64_t seconds_difference_dt(const DateTime* t1, const DateTime* t2) {
	DateTime tn1, tn2;
	int d1, d2;

	tn1 = *t1;
	tn2 = *t2;
	normalize_tz_dt(&tn1);
	normalize_tz_dt(&tn2);

	d1 = date_to_yyyymmdd(&tn1.d);
	d2 = date_to_yyyymmdd(&tn2.d);
	if (d1 < d2)
		return -__normalized_seconds_difference_dt(&tn2, &tn1);	
	else if (d1 == d2 && time_to_hhmmss(&tn1.t) < time_to_hhmmss(&tn2.t))
		return -__normalized_seconds_difference_dt(&tn2, &tn1);	

	return __normalized_seconds_difference_dt(&tn1, &tn2);
}

static int __normalized_full_days_difference(const DateTime* t1, const DateTime* t2) {
	/*** normalized: timezone is UTC and t1 is <= t2 ***/
	int d;
	int d1, d2;
	
	d = days_difference(&t1->d, &t2->d);
	d1 = time_to_hhmmss(&t1->t);
	d2 = time_to_hhmmss(&t2->t);
	if (d1 < d2)
		--d;
	return(d);
}

int full_days_difference(const DateTime* t1, const DateTime* t2) {
	DateTime dt1, dt2;
	int d1, d2;

	dt1 = *t1;
	dt2 = *t2;
	normalize_tz_dt(&dt1);
	normalize_tz_dt(&dt2);

	d1 = date_to_yyyymmdd(&dt1.d);
	d2 = date_to_yyyymmdd(&dt2.d);
	if (d1 < d2)
		return -__normalized_full_days_difference(&dt2, &dt1);

	return __normalized_full_days_difference(&dt1, &dt2);
}

double days_difference_dt(const DateTime* t1, const DateTime* t2) {
	return ((double)seconds_difference_dt(t1, t2)) / 86400.0;
}

int64_t unix_epoch_seconds(const DateTime* dt) {
	const DateTime dt_unix_epoch = { { 1970, 01, 01, calendar_Auto },
					 { 00, 00, 00, 00, 00 } };
	return seconds_difference_dt(&dt_unix_epoch, dt);
}

extern uint32_t julian_day_of_year(const Date* d) {
	Date dd = { 0, 1, 0, calendar_Auto };
	dd.y = d->y;
	dd.c = d->c;
	return days_difference(&dd, d);
}

int64_t full_seconds_difference_ft(const FullTime *f1, const FullTime *f2) {
	// TODO: TBI
	assert(false);
	return(0LL);
}

int64_t unix_epoch_seconds_ft(const FullTime* ft) {
	// TODO: TBI
	assert(false);
	return(0LL);
}

int64_t unix_epoch_msec_ft(const FullTime* ft) {
	// TODO: TBI
	assert(false);
	return(0LL);
}

int64_t unix_epoch_usec_ft(const FullTime* ft) {
	// TODO: TBI
	assert(false);
	return(0LL);
}

void normalize_tz(Time* t) {
	// TODO: TBI
//	assert(false);
}

void normalize_tz_dt(DateTime* t) {
	// TODO: TBI
//	assert(false);
}

void get_current_datetime(DateTime* dt) {
	time_t t;
	struct tm *lt;

	t = time(NULL);
	lt = localtime(&t);

	dt->d.y = 1900 + lt->tm_year;
	dt->d.m = 1 + lt->tm_mon;
	dt->d.d = lt->tm_mday;
	dt->d.c = calendar_Auto;
	dt->t.h = lt->tm_hour;
	dt->t.m = lt->tm_min;
	dt->t.s = lt->tm_sec;
	dt->t.daylight = lt->tm_isdst;
	dt->t.tz = 0; //timezone;
}

void set_julian_date(Date *d, const int32_t jd) {
	switch (d->c)
		{
	default:
		jdn_to_ymd(jd, (int*)&d->y, (int*)&d->m, (int*)&d->d, -1, 1);
		break;
	case calendar_Julian:
		jdn_to_ymd(jd, (int*)&d->y, (int*)&d->m, (int*)&d->d, 1, 0);
		break;
	case calendar_Gregorian:
		jdn_to_ymd(jd, (int*)&d->y, (int*)&d->m, (int*)&d->d, 0, 0);
		break;
	case calendar_British:
		jdn_to_ymd(jd, (int*)&d->y, (int*)&d->m, (int*)&d->d, -1, 0);
		break;
		}
}

void change_datetime(DateTime* dt, const int nsec) {
	int adj;
	int jd;

	dt->t.s += nsec;
	if (dt->t.s < 0)
		adj = -(dt->t.s / 60) + 1;
	else
		adj = -(dt->t.s / 60);
	dt->t.s += (60 * adj);
	dt->t.m -= adj;

	if (dt->t.m < 0)
		adj = -(dt->t.m / 60) + 1;
	else
		adj = -(dt->t.m / 60);
	dt->t.m += (60 * adj);
	dt->t.h -= adj;

	if (dt->t.h < 0)
		adj = -(dt->t.h / 24) + 1;
	else
		adj = -(dt->t.h / 24);
	dt->t.h += (24 * adj);

	if (adj != 0) {
		jd = julian_date(&dt->d);
		jd -= adj;
		set_julian_date(&dt->d, jd);
	}
}

void change_time(Time* t, const int nsec) {
	int adj;

	t->s += nsec;
	if (t->s < 0)
		adj = -(t->s / 60) + 1;
	else
		adj = -(t->s / 60);
	t->s += (60 * adj);
	t->m -= adj;

	if (t->m < 0)
		adj = -(t->m / 60) + 1;
	else
		adj = -(t->m / 60);
	t->m += (60 * adj);
	t->h -= adj;

	if (t->h < 0)
		adj = -(t->h / 24) + 1;
	else
		adj = -(t->h / 24);
	t->h += (24 * adj);
}

static char *fmt_field(int field, int pad, char *w, char *wend) {
	char add[8];

	if (pad)
		sprintf(add, "%02d", field);
	else
		sprintf(add, "%d", field);

	if (w + strlen(add) >= wend)
		return(wend);

	strcpy(w, add);
	w += strlen(add);
	*w = '\000';
	return(w);
}

static char *fmt_meridian(int hour, int caps, int periods, char *w, char *wend) {
	char add[8];

	if (hour < 12)
		OUR_STRCPY(add, (periods ? "a.m." : "am"));
	else
		OUR_STRCPY(add, (periods ? "p.m." : "pm"));
	if (caps)
		__strupr(add);

	if (w + strlen(add) >= wend)
		return(wend);

	strcpy(w, add);
	w += strlen(add);
	*w = '\000';
	return(w);
}

/*** nice flexible date and time formatting for my library ***/
char *fmt_datetime(const DateTime* dt, const char* fmt) {
	char* ret, *retm, *w;
	int l, wd;
	const char* mnames[] = {
		"", "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December",
		};
	const char* wdnames[] = {
		"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday",
		};

	l = strlen(fmt) * 4;
	if (l < 16)
		l = 16;
	ret = NEW_ARRAY(char, l);
	retm = ret + l;
	w = ret;
	*w = '\000';

	forever
		{
		if (*fmt == '%')
			{
			int field = -99;
			++fmt;

			switch (*fmt)
				{
			default:
				if (w + 1 >= retm)
					goto LDone;
				*w = *fmt;
				++w;
				*w = '\000';
				break;
			case 0:
				goto LDone;
			case 'D':
			case 'd':
				field = dt->d.d;
				break;
			case 'H':
			case 'h':
				field = dt->t.h % 12;
				if (field == 0)
					field = 12;
				break;
			case 'I':
			case 'i':
				field = dt->t.h;
				break;

			case 'M':
				l = strlen(mnames[dt->d.m]);
				if (w + l >= retm)
					goto LDone;
				strcpy(w, mnames[dt->d.m]);
				w += l;
				*w = '\000';
				break;

			case 'm':
				if (w + 3 >= retm)
					goto LDone;
				strncpy(w, mnames[dt->d.m], 3);
				w += 3;
				*w = '\000';
				break;

			case 'N':
			case 'n':
				field = dt->d.m;
				break;

			case 'O':
				if (w + 2 >= retm)
					goto LDone;
				strncpy(w, ordinal_text_suffix(dt->d.d), 2);
				w += 2;
				*w = '\000';
				break;

			case 'P':
			case 'p':
			case 'Q':
			case 'q':
				w = fmt_meridian(dt->t.h, isupper(*fmt), (isupper(*fmt) == 'P'), w, retm);
				if (w >= retm)
					goto LDone;
				break;

			case 'S':
			case 's':
				field = dt->t.s;
				break;

			case 'x':
				field = dt->t.daylight;
				break;

			case 'T':
			case 't':
				field = dt->t.m;
				break;

			case 'W':
				wd = (int)weekday(&dt->d);
				l = strlen(wdnames[wd]);
				if (w + l >= retm)
					goto LDone;
				strcpy(w, wdnames[wd]);
				w += l;
				*w = '\000';
				break;

			case 'w':
				wd = (int)weekday(&dt->d);
				if (w + 3 >= retm)
					goto LDone;
				strncpy(w, wdnames[wd], 3);
				w += 3;
				*w = '\000';
				break;

			case 'Y':
				field = dt->d.y;
				break;

			case 'y':
				field = dt->d.y % 100;
				break;

			case 'z':
				field = dt->t.tz;
				break;
				}

			if (field > -99)
				{
				w = fmt_field(field, isupper(*fmt) || ((*fmt) == 'y'), w, retm);
				if (w >= retm)
					break;
				}
			}
		else
			{
			if (w + 1 >= retm)
				break;
			*w = *fmt;
			++w;
			*w = '\000';
			}
		++fmt;
		}

LDone:
	return(ret);
}

char *fmt_datetime_american_long(const DateTime* dt) {
	return fmt_datetime(dt, "%W, %M %d%O, %Y %H:%T %p");
}

char *fmt_date_american_long(const DateTime* dt) {
	return fmt_datetime(dt, "%W, %M %d%O, %Y");
}

char *fmt_datetime_american_short(const DateTime* dt) {
	return fmt_datetime(dt, "%m %d, %Y %H:%T %p");
}

char *fmt_date_american_short(const DateTime* dt) {
	return fmt_datetime(dt, "%m %d, %Y");
}

char *fmt_datetime_europe_long(const DateTime* dt) {
	return fmt_datetime(dt, "%W %d %m %Y %I:%T");
}

char *fmt_date_europe_long(const DateTime* dt) {
	return fmt_datetime(dt, "%W %d %m %Y");
}

char *fmt_date_europe_short(const DateTime* dt) {
	return fmt_datetime(dt, "%d %m %Y");
}

char *fmt_datetime_shell_long(const DateTime* dt) {
	return fmt_datetime(dt, "%w %d-%m-%Y %I:%T:%S");
}

char *fmt_datetime_shell(const DateTime* dt) {
	return fmt_datetime(dt, "%d-%m-%Y %I:%T");
}

char *fmt_date_shell(const DateTime* dt) {
	return fmt_datetime(dt, "%d-%m-%Y");
}

char *fmt_datetime_dos(const DateTime* dt) {
	return fmt_datetime(dt, "%d-%m-%y %I:%T");
}

char *fmt_date_dos(const DateTime* dt) {
	return fmt_datetime(dt, "%d-%m-%y");
}

char *fmt_date_american_long(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_american_long(&dt);
}

char *fmt_date_american_short(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_american_short(&dt);
}

char *fmt_date_europe_long(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_europe_long(&dt);
}

char *fmt_date_europe_short(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_europe_short(&dt);
}

char *fmt_date_shell(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_shell(&dt);
}

char *fmt_date_dos(int yyyymmdd) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_date_dos(&dt);
}

char *fmt_date(const Date* d, const char* fmt) {
	DateTime dt;
	dt.d = *d;
	return fmt_datetime(&dt, fmt);
}

char *fmt_date(int yyyymmdd, const char* fmt) {
	DateTime dt;
	yyyymmdd_to_date(yyyymmdd, &dt.d);
	return fmt_datetime(&dt, fmt);
}

void output_and_free(std::ostream& o, char* str) {
	o << str;
	delete str;
}

int nth_weekday_in_month(Date* d, int n, Weekday wd) {
	Date dd;
	int dim;
	int jd;
	int wd1, wdi;
	int f;

	if (n < 1 || n > 5)
		return(-1);

	dd.m = d->m;
	dd.d = 0;
	dd.y = d->y;
	dd.c = d->c;
	dim = days_in_month_date(d);
	jd = julian_date(&dd);
	wd1 = (int)(weekday_from_julian_date(jd));
	wdi = (int)(wd);

	if (wd1 < wdi)
		f = (wdi - wd1);	// this is the first wd of the month
	else
		f = (wdi - wd1) + 7;	// this is the first wd of the month

	// now find the return value
	f += ((n - 1) * 7);
	if (f <= dim)
		d->d = f;

	return((f <= dim) ? 0 : -1);
}

int last_weekday_in_month(Date* d, Weekday wd) {
	Date dd;
	dd = *d;
	if (nth_weekday_in_month(&dd, 5, wd) == 0) {
		*d = dd;
		return(0);
	}
	return nth_weekday_in_month(d, 4, wd);
}

int day_of_year_mdy(const Month m, const Day d, const Year y) {
	const int day_by_month[] =
		{
		1,		// Jan 1
		32,		// Feb 1
		60, 	// Mar 1 (non-leap year)
		91, 	// Apr 1
		121,	// May 1
		152,	// Jun 1
		182,	// Jul 1
		213,	// Aug 1
		244,	// Sep 1
		274,	// Oct 1
		305,	// Nov 1
		335		// Dec 1
		};
	int ret;

	check_or_die(is_between32((u32)m, 1, 12));

	ret = day_by_month[(u32)m - 1] + (u32)d - 1;
	if (isleapyear(y) && (u32)m > 2)
		++ret;
	return(ret);
}

int day_of_year_yyyymmdd(const int date) {
	Month m;
	Day d;
	Year y;

	m = (date / 100) % 100;
	d = date % 100;
	y = date / 10000;

	return day_of_year_mdy(m, d, y);
}

int day_of_year(const Date* date) {
	// TODO: use the Calendar object!
	return (day_of_year_mdy(date->m, date->d, date->y));
}

void julian_to_date(u32 jdate, Date* date_output) {
	set_julian_date(date_output, jdate);
}

void add_days(Date* date, i32 num_days) {
	int jd = julian_date(date);
	jd += num_days;
	julian_to_date(jd, date);
}

void next_weekday(const Date* in_date, Date* out_date) {
	Weekday dow = weekday(in_date);
	*out_date = *in_date;
	switch (dow)
		{
	default:
		break;
	case weekday_Friday:
		add_days(out_date, 3);
		return;
	case weekday_Saturday:
		add_days(out_date, 2);
		return;
		}

	add_days(out_date, 1);
}

int next_weekday_yyyymmdd(const int yyyymmdd) {
	Date in, out;
	yyyymmdd_to_date(yyyymmdd, &in);
	next_weekday(&in, &out);
	return date_to_yyyymmdd(&out);
}

void yyyymmdd_to_mdy(i32 yyyymmdd, Month* mout, Day* dout, Year* yout) {
	*mout = (Month)((yyyymmdd / 100) % 100);
	*dout = (Day)(yyyymmdd % 100);
	*yout = (Year)(yyyymmdd / 10000);
}

void next_weekday_mdy(const Month m, const Day d, const Year y,
	Month* mout, Day *dout, Year *yout) {
	i32 yyyymmdd = mdy_to_yyyymmdd(m, d, y);
	yyyymmdd = next_weekday_yyyymmdd(yyyymmdd);
	yyyymmdd_to_mdy(yyyymmdd, mout, dout, yout);
}

void prev_weekday(const Date* in_date, Date* out_date) {
	Weekday dow = weekday(in_date);
	*out_date = *in_date;
	switch (dow)
		{
	default:
		break;
	case weekday_Sunday:
		add_days(out_date, -2);
		return;
	case weekday_Monday:
		add_days(out_date, -3);
		return;
		}

	add_days(out_date, -1);
}

int prev_weekday_yyyymmdd(const int yyyymmdd) {
	Date in, out;
	yyyymmdd_to_date(yyyymmdd, &in);
	prev_weekday(&in, &out);
	return date_to_yyyymmdd(&out);
}

void prev_weekday_mdy(const Month m, const Day d, const Year y,
	Month* mout, Day *dout, Year *yout) {
	i32 yyyymmdd = mdy_to_yyyymmdd(m, d, y);
	yyyymmdd = prev_weekday_yyyymmdd(yyyymmdd);
	yyyymmdd_to_mdy(yyyymmdd, mout, dout, yout);
}

void holiday(Holiday holiday, Year y, Date* date_out) {
	date_out->y = y;

#define	MD(_m, _d)	date_out->m = (Month)_m; date_out->d = (Day)_d; break
	switch (holiday)
		{
	/*** the fixed-date holidays ***/
	case HOLIDAY_CHRISTMAS:
		MD(12, 25);
	case HOLIDAY_NEW_YEARS_DAY:
		MD(1, 1);
	case HOLIDAY_HALLOWEEN:
		MD(10, 31);
	case HOLIDAY_CHRISTMAS_EVE:
		MD(12, 24);
	case HOLIDAY_BOXING_DAY:
	case HOLIDAY_KWANZAA_STARTS:
	case HOLIDAY_ST_STEPHENS_DAY:
		MD(12, 26);
	case HOLIDAY_NEW_YEARS_EVE:
		MD(12, 31);
	case HOLIDAY_GROUNDHOG_DAY:
		MD(2, 2);
	case HOLIDAY_CANADA_DAY:
		MD(7, 1);
	case HOLIDAY_INDEPENDENCE_DAY_USA:
		MD(7, 4);
	case HOLIDAY_GUY_FAWKES_NIGHT:
		MD(11, 5);
	case HOLIDAY_EARTH_DAY:
		MD(4, 22);
	case HOLIDAY_JUNETEENTH:
		MD(6, 19);
	case HOLIDAY_APRIL_FOOLS_DAY:
		MD(4, 1);
	case HOLIDAY_TALK_LIKE_A_PIRATE_DAY:
		MD(9, 19);
	case HOLIDAY_PI_DAY:
		MD(3, 14);
	case HOLIDAY_ST_PATRICKS_DAY:
		MD(3, 17);
#undef	MD
	/*** and the floating-date holidays ***/
	case HOLIDAY_EASTER:
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		break;
	case HOLIDAY_ASH_WEDNESDAY:
		// 46 days before Easter Sunday
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		add_days(date_out, -46);
		break;
	case HOLIDAY_PENTECOST:
		// 49 days after Easter Sunday
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		add_days(date_out, 49);
		break;
	case HOLIDAY_MARDI_GRAS:
		// 47 days before Easter Sunday
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		add_days(date_out, -47);
		break;
	case HOLIDAY_GOOD_FRIDAY:
		// 3 days before Easter Sunday
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		add_days(date_out, -3);
		break;
	case HOLIDAY_PALM_SUNDAY:
		// One week before Easter Sunday
		easter((int) y, (int *)&date_out->m, (int *)&date_out->d);
		add_days(date_out, -7);
		break;
	case HOLIDAY_THANKSGIVING_USA:
		// Fourth Thursday in November
		date_out->m = 11;
		nth_weekday_in_month(date_out, 4, weekday_Thursday);
		break;
	case HOLIDAY_MARTIN_LUTHER_KING_DAY:
		// third Monday in January
		date_out->m = 1;
		nth_weekday_in_month(date_out, 3, weekday_Monday);
		break;
	case HOLIDAY_MOTHERS_DAY_USA:
		// second Sunday in May 
		date_out->m = 5;
		nth_weekday_in_month(date_out, 2, weekday_Sunday);
		break;
	case HOLIDAY_FATHERS_DAY_USA:
		// third Sunday in June
		date_out->m = 6;
		nth_weekday_in_month(date_out, 3, weekday_Sunday);
		break;
	case HOLIDAY_PRESIDENTS_DAY_USA:
		// third Monday in February
		date_out->m = 2;
		nth_weekday_in_month(date_out, 3, weekday_Monday);
		break;
	case HOLIDAY_THANKSGIVING_CANADA:
	case HOLIDAY_HANUKKAH_STARTS:
	case HOLIDAY_PASSOVER_BEGINS:
		// 15th Nisan
	case HOLIDAY_PURIM:
	case HOLIDAY_ROSH_HASHANAH:
	case HOLIDAY_YOM_KIPPUR:
	case HOLIDAY_LABOR_DAY_USA:
	case HOLIDAY_ARBOR_DAY_USA:
	case HOLIDAY_INDIGENOUS_PEOPLES_DAY_USA:
	case HOLIDAY_ASCENSION_THURSDAY:
	case HOLIDAY_CANDLEMAS:
		// TODO: implement
		assert(false);
		break;
	default:
		break;
		}
}

void Date::out_ramfile(RamFile* rf) const {
	rf->put32((i32)y);
	rf->putu32((u32)m);
	rf->putu32((u32)d);
	rf->putu32((u32)c);
}

void Date::in_ramfile(RamFile* rf) {
	y = (Year)rf->get32();
	m = (Month)rf->getu32();
	d = (Day)rf->getu32();
	c = (Calendar)rf->getu32();
}

void Time::out_ramfile(RamFile* rf) const {
	rf->putu32((u32)h);
	rf->putu32((u32)m);
	rf->putu32((u32)s);
	rf->put32(tz);
	rf->put32(daylight);
}

void Time::in_ramfile(RamFile* rf) {
	h = (Hour)rf->getu32();
	m = (Minute)rf->getu32();
	s = (Second)rf->getu32();
	tz = rf->get32();
	daylight = rf->get32();
}

void DateTime::out_ramfile(RamFile* rf) const {
	d.out_ramfile(rf);
	t.out_ramfile(rf);
}

void DateTime::in_ramfile(RamFile* rf) {
	d.in_ramfile(rf);
	t.in_ramfile(rf);
}

void FullTime::out_ramfile(RamFile* rf) const {
	d.out_ramfile(rf);
	t.out_ramfile(rf);
	rf->putu32(nanos);
}

void FullTime::in_ramfile(RamFile* rf) {
	d.in_ramfile(rf);
	t.in_ramfile(rf);
	nanos = rf->getu32();
}

/* end calendar.cpp */
