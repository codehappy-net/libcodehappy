/***

	misc.cpp

	Miscellaneous useful functions from various projects.

	Copyright (c) 2014-2022 C. M. Street

***/

#include "libcodehappy.h"
#include <stdarg.h>
#ifdef CODEHAPPY_LINUX
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#endif

/*** some shared strings ***/
const char __oom_msg[] = "Out of memory.\n";
const char __fatal_error_msg[] = "Fatal error!\n";
#if	CODEHAPPY_SAFE
const char __unsafe_call_msg[] = "Unsafe call to %s\n";
#endif
const char __assert_failed_msg[] = "Assertion failed!";
const char __impl_error_msg[] = "Implementation error\n";

// TODO: a fair number of fns in the library with names of the form str* --
// by the standard these are reserved for possible future stdlib use

/*** stricmp() for platforms that don't have it ***/
int __stricmp(const char* s1, const char* s2) {
	forever {
		int i1, i2;

		i1 = tolower(*s1);
		i2 = tolower(*s2);
		if (i1 != i2)
			return (i1 - i2);
		if (!(*s1))
			break;
		++s1;
		++s2;
	}

	return(0);
}

/*** strnicmp() for platforms that don't have it ***/
int __strnicmp(const char* s1, const char* s2, unsigned int n) {
	const char* s1e = s1 + n;
	while (s1 < s1e) {
		int i1, i2;

		i1 = tolower(*s1);
		i2 = tolower(*s2);
		if (i1 != i2)
			return (i1 - i2);
		if (!(*s1))
			break;

		++s1;
		++s2;
	}

	return(0);
}

/*** strlwr() for platforms that don't have it ***/
void __strlwr(char* s1) {
	while (*s1) {
		(*s1) = tolower(*s1);
		++s1;
	}
}

/*** strupr() for platforms that don't have it ***/
void __strupr(char* s1)
{
	while (*s1) {
		(*s1) = toupper(*s1);
		++s1;
	}
}

/*** strdup() for platforms that don't have it ***/
char *__strdup(const char* s)
{
	size_t len = malloc_len(s);
	char *p = (char*)OUR_MALLOC(len);
	return p ? ((char*)OUR_MEMCPY(p, s, len)) : NULL;
}

/*** strlen() implementation that may or may not be faster than stdlib version ***/
uint _strlen(const char* str)
{
	const char* p = str;
	uint* address = (uint *)(str);
#ifdef CODEHAPPY_64BIT
	u64 alignment = ((u64)(address + 3) & 0xfffffffffffffffcULL) - (u64)address;
#else
	u64 alignment = ((u64)(address + 3) & 0xfffffffc) - (u64)address;
#endif
	u32 v = 0;
	const u32* ap;
	u32 s;
	uint i;

	assert(not_null(str));

	if (alignment)
		{
		for (i = 0; i < alignment; ++i)
			{
			if (*p++ == 0)
				return (uint)(p - str) - 1;
			}
		}

	ap = (const u32*)(p);

	until (truth(v))
		{
		u32 u = *ap++;
		v = (u - 0x01010101) & ~u & 0x80808080;
		}

	s = (u32)((const char*)(ap) - p) + alignment - 3;

#if HAVE_BIG_ENDIAN				
	if (!(v & 0x80800000))
		return v & 0x8000 ? s + 1 : s + 2;

	return v & 0x80000000 ? s - 1 : s;
#endif  // HAVE_BIG_ENDIAN

#ifdef HAVE_LITTLE_ENDIAN
	if (!(v & 0x8080))
		return v & 0x00800000 ? s + 1 : s + 2;

	return v & 0x0080 ? s - 1 : s;
#endif  // HAVE_LITTLE_ENDIAN
}

/*** stristr for platforms that need it ***/
char *__stristr(const char* haystack, const char* needle) {
	u32 l = strlen(needle);
	while (*haystack) {
		if (!__strnicmp(haystack, needle, l))
			return (char*)haystack;
		haystack++;
	}
	return (NULL);
}

/*** Replace all occurences of "replace" in "buf" with "with" ***/
void strreplace(char *buf, const char *replace, const char *with) {
	char *w;
	int move;
	int lr;
	int lw;
	int e;

	lr = strlen(replace);
	lw = strlen(with);
	move = lr - lw;

	if (move < 0)
		return;
	
	w = strstr(buf, replace);
	while (w)
		{
		if (move > 0)
			memmove(w + lw, w + lr, strlen(w + lr) + 1); 
		for (e = 0; e < lw; ++e)
			w[e] = with[e];
		w = strstr(buf, replace);
		}
}

int strncmp_backwards(const char *w1, const char *w2, int n) {
	// like strncmp(), but acts as if the words are spelt backwards.
	char* bw1;
	char* bw2;
	int l;
	int e;
	int ret;

	bw1 = NEW_STR(strlen(w1));
	check_mem_or_die(bw1);
	bw2 = NEW_STR(strlen(w2));
	check_mem_or_die(bw2);
	
	l = strlen(w1);
	for (e = 0; e < l; ++e)
		bw1[e] = w1[l - 1 - e];
	bw1[e] = '\000';

	l = strlen(w2);
	for (e = 0; e < l; ++e)
		bw2[e] = w2[l - 1 - e];
	bw2[e] = '\000';

	ret = (strncmp(bw1, bw2, n));
	delete bw1;
	delete bw2;
	return(ret);
}

int strcmp_backwards(const char *w1, const char *w2) {
	// like strcmp(), but acts as if the words are spelt backwards.
	char *bw1;
	char *bw2;
	int l;
	int e;
	int ret;

	bw1 = NEW_STR(strlen(w1));
	bw2 = NEW_STR(strlen(w2));
	check_mem_or_die(bw1);
	check_mem_or_die(bw2);
	
	l = strlen(w1);
	for (e = l - 1; e >= 0; --e)
		bw1[e] = w1[l - 1 - e];
	bw1[e] = '\000';

	l = strlen(w2);
	for (e = l - 1; e >= 0; --e)
		bw2[e] = w2[l - 1 - e];
	bw2[e] = '\000';

	ret = strcmp(bw1, bw1);
	delete bw1;
	delete bw2;

	return (ret);
}

/* the value of a hexadecimal digit */
int valhexdigit(const char ch) {
	// TODO: lookup table would be faster
	int v;

	if (isdigit(ch))
		return ch - '0';
	if (islower(ch))
		v = ch - 'a';
	else
		v = ch - 'A';
	if (v < 0 || v > 6)
		return(-1);

	return (v + 10);
}

/*** best-fit linear regression ***/
void BestFitLinearSolution(double *xpoints, double *ypoints, int npoints, double *alpha, double *beta, double *rsquared) {
	// beta: the x-coefficient
	// alpha: the y-intercept
	// rsquared: measure of the fitness
	double x, y, xy, x2;
	double alpha_c, beta_c;
	int i;

	x = 0.0;
	y = 0.0;
	xy = 0.0;
	x2 = 0.0;

	for (i = npoints - 1; i >= 0; --i) {
		x += xpoints[i];
		x2 += (xpoints[i] * xpoints[i]);
		xy += (xpoints[i] * ypoints[i]);
		y += ypoints[i];
	}

	alpha_c = ((x2 * y) - (x * xy)) / ((x2 * npoints) - (x * x));
	beta_c = ((xy * npoints) - (x * y)) / ((x2 * npoints) - (x * x));
	if (alpha)
		*alpha = alpha_c;
	if (beta)
		*beta = beta_c;

	if (NULL != rsquared) {
		// calculate r-squared
		double mean = 0.0, mse, variance, error;
		int e;

		for (e = npoints - 1; e >= 0; --e)
			mean += ypoints[e];
		mean /= (double)(npoints);

		mse = 0.0;
		variance = 0.0;
		for (e = npoints - 1; e >= 0; --e) {
			error = ypoints[e] - (xpoints[e] * beta_c + alpha_c);
			mse += (error * error);
			variance += (ypoints[e] - mean) * (ypoints[e] - mean);
		}

		variance /= (double)(npoints - 1.0);
		mse /= (double)(npoints);
		*rsquared = 1.0 - mse / variance;
	}

	return;
}

/*** Pythagorean metric ***/
uint32_t distance_squared(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	return (uint32_t)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

uint64_t distance_squared_64(int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
	return (uint64_t)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

uint32_t distance_squared_3d(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2) {
	return (uint32_t)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

uint64_t distance_squared_3d_64(int64_t x1, int64_t y1, int64_t z1, int64_t x2, int64_t y2, int64_t z2) {
	return (uint64_t)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

/* given the degrees of freedom and the Student t score, estimates the total
	cumulative probability falling below that score. 
   check out http://stattrek.com/Tables/t.aspx	*/
double confidence(double t, int dof) {
	int e;

	double tconf[9][10] = {
		// dof = 7, confidence 0.50, 0.55, 0.60, etc.
		{0.0, 0.130, 0.263, 0.402, 0.549, 0.711, 0.896, 1.119, 1.415, 1.895},
		// dof = 8
		{0.0, 0.130, 0.262, 0.399, 0.546, 0.706, 0.889, 1.108, 1.397, 1.860},
		// dof = 9
		{0.0, 0.129, 0.261, 0.398, 0.543, 0.703, 0.883, 1.110, 1.383, 1.833},
		// dof = 10
		{0.0, 0.129, 0.260, 0.397, 0.542, 0.700, 0.879, 1.093, 1.372, 1.812},
		// dof = 11
		{0.0, 0.129, 0.260, 0.396, 0.540, 0.697, 0.876, 1.088, 1.363, 1.796},
		// dof = 12
		{0.0, 0.128, 0.259, 0.395, 0.539, 0.695, 0.873, 1.083, 1.356, 1.782},
		// dof = 13
		{0.0, 0.128, 0.259, 0.394, 0.538, 0.694, 0.870, 1.079, 1.350, 1.771},
		// dof = 14
		{0.0, 0.128, 0.258, 0.393, 0.537, 0.692, 0.868, 1.076, 1.345, 1.761},
		// dof = 15
		{0.0, 0.128, 0.258, 0.393, 0.536, 0.691, 0.866, 1.074, 1.341, 1.753},
		// dof = 16
		};

	dof -= 7;
	dof = CLAMP(dof, 0, 8);

	for (e=0; e<10; ++e) {
		if (t < tconf[dof][e])
			break;
	}

	return (0.5 + 0.05 * (double)(e-1));
}

/* number of significant figures in a string representing a number */
int CountSigFigs(char *v) {
	int cs;
	bool digseen;
	int cz;
	char *s;

	digseen = false;
	cs = 0;
	s = strchr(v, '.');
	while (*v) {
		if (*v >= '0' && *v <= '9') {
			if (digseen || *v != '0')	// leading zeroes don't count as significant figures
				cs++;
			if (*v != '0')
				digseen = true;
		}
		if (*v == ',')
			break;
		v++;
	}
	// trailing zeros after a decimal point don't count, either
	if (s != NULL && s < v) {
		v--;
		cz = 0;
		while (v > s) {
			if (*v == '0')
				cz++;
			else
				break;
			--v;
		}
		cs -= cz;
		if (cs < 1)
			cs = 1;
	}
	return(cs);
}

static bool __nameeq(const char* name, const char* cname, int l)
{
	int l2 = strlen(cname);
	int e;

	if (l < l2 - 1)
		return false;

	for (e = 0; e < l2; ++e) {
		if (tolower(name[e]) != cname[e])
			return(false);
		if (cname[e] == ' ')
			break;
	}
	if (e < l)
		return(name[e] == ' ');
	return(true);
}

/* hardly infallible, but can catch some cases checking titles won't */
bool FNameLooksFemale(const char *name)
{
	// most frequent women's names
	const char *wnames[] = {
		"mary ", "patricia ", "linda ", "barbara ", "elizabeth ", "jennifer ",
		"maria ", "susan ", "margaret ", "dorothy ", "lisa ", "nancy ", "karen ",
		"betty ", "helen ", "sandra ", "donna ", "carol ", "ruth ", "sharon ", 
		"michelle ", "laura ", "sarah ", "kimberly ", "jessica ", "shirley ",
		"cynthia ", "angela ", "melissa ", "brenda ", "amy ", "anna ", "rebecca ",
		"virginia ", "kathleen ", "pamela ", "martha ", "debra ", "amanda ", 
		"stephanie ", "carolyn ", "christine ", "marie ", "janet ", "catherine ",
		"frances ", "ann ", "diane ", "alice ", "julie ", "heather ", "teresa ",
		"doris ", "gloria ", "evelyn ", "cheryl ", "mildred ", "katherine ", "joan ",
		"judith ", "rose ", "janice ", "nicole ", "judy ", "christina ", "kathy ",
		"theresa ", "beverly ", "denise ", "tammy ", "irene ", "jane ", "lori ",
		"rachel ", "marilyn ", "andrea ", "kathryn ", "louise ", "sara ", "anne ",
		"jacqueline ", "wanda ", "bonnie ", "julia ", "ruby ", "lois ", "tina ",
		"phyllis ", "norma ", "paula ", "diana ", "annie ", "lillian ", "emily ",
		"peggy ", "crystal ", "gladys ", "rita ", "dawn ", "connie ", "florence ",
		"tracy ", "edna ", "tiffany ", "rosa ", "cindy ", "grace ", "wendy ", "victoria ",
		"edith ", "sherry ", "sylvia ", "josephine ", "thelma ", "shannon ", "sheila ",
		"ethel ", "ellen ", "elaine ", "marjorie ", "carrie ", "charlotte ",
		"monica ", "esther ", "pauline ", "emma ", "juanita ", "anita ", "rhonda ",
		"hazel ", "amber ", "eva ", "debbie ", "april ", "clara ", "lucille ", "joanne ",
		"eleanor ", "valerie ", "danielle ", "megan ", "alicia ", "suzanne ", "michele ",
		"gail ", "bertha ", "darlene ", "veronica ", "jill ", "erin ", "geraldine ",
		"cathy ", "lauren ", "joann ", "lorraine ", "lynn ", "sally ", "regina ", "erica ",
		"beatrice ", "dolores ", "bernice ", "yvonne ", "annette ", "samantha ", "marion ",
		"dana ", "stacy ", "renee ", "ida ", "vivian ", "roberta ", "holly ", "brittany ",
		"melanie ", "loretta ", "yolanda ", "jeanette ", "laurie ", "katie ", "kristen ",
		"vanessa ", "alma ", "sue ", "elsie ", "beth ", "jeanne ", "vicki ", "carla ",
		"tara ", "rosemary ", "eileen ", "terri ", "gertrude ", "lucy ", "tonya ", "ella ",
		"stacey ", "wilma ", "gina ", "kristin ", "jessie ", "natalie ", "agnes ", "vera ",
		"jenny ", "miriam ", "becky ", "bobbie ", "velma ", "dianne ", "priscilla ", "margie ",
		"nora ", "penny ", "kay ", "olga ", "brandy ", "leah ", "cassandra ", "nina ",
		"maxine ", "vickie ", "jo ", "jennie ", "gwendolyn ", "glenda ", "heidi ", "christin ",
		"claudia ", "marcia ", "viola ", "lydia ", "marlene ", "hannah ", 
		NULL
		};
	int n;

	n = 0;
	while (wnames[n]) {
		if (__nameeq(name, wnames[n], strlen(wnames[n])))
			return (true);
		n++;
	}
	return (false);	// may well actually be a woman's name, but we don't know it 
}

/* Does the C string str end with the C string end? */
/* starts_with() is a macro defined in __useful.h */
bool ends_with(const char* str, const char* end) {
	const char *w = TERMINATOR(str);
	u32 el = strlen(end);
	if ((w - el) > w)
		return(false);
	w -= el;
	if (w < str)
		return(false);
	return (strcmp(w, end) == 0);
}

/*** Zero the specified memory ***/
void zeromem(void* ptr, u32 size) {
	memset(ptr, 0, size);
}

/*** print a character n times ***/
void nputc(char ch, u32 count) {
	char buf[256];
	if (count > 4096)
		/* no */
		return;
	if (count < 256) {
		char* bufe = buf + count;
		char* w = buf;
		*bufe = 0;
		while (w < bufe) {
			*w = ch;
			++w;
		}
		printf("%s", buf);
	} else {
		while (count >= 255) {
			// this could be made iterative to avoid creating multiple 255-character strings, but
			// the dominant operation here is the console output. We don't even have to worry
			// about stack since we check for ridiculously high inputs above.
			nputc(ch, 255);
			count -= 255;
		}
		nputc(ch, count);
	}
}

/*** Returns the position of the first occurence of "str_search" in the string "str_in",
	at or after the (0-indexed) position "pos", or -1 if not found. ***/
i32 findinstr(i32 pos, const char* str_search, const char* str_in) {
	const char* w;
	if (pos >= strlen(str_in) || pos < 0)
		return(-1);
	w = strstr(str_in + pos, str_search);
	if (is_null(w))
		return(-1);
	return (w - str_in);
}

/*** Empty string predicate: returns true iff the string is NULL or zero-length. ***/
bool FEmpty(const char *str) {
	if (str == NULL)
		return(true);
	if (str[0] == '\000')
		return(true);

	return(false);
}

/*** Returns the index of the string that matches the argument string (case-insensitive), or -1 if not found. nstr needs to be
	the number of strings we're matching against. ***/
int striindex(const char *match, int nstr, ...) {
	va_list args;
	char* teststr;
	int i = 0;

	va_start(args, nstr);

	while (i < nstr) {
		teststr = va_arg(args, char*);
		if (!__stricmp(match, teststr)) {
			va_end(args);
			return(i);
		}
		++i;
	}

	va_end(args);

	return(-1);
}

/*** As above in striindex(), but case-sensitive. ***/
int strindex(const char *match, int nstr, ...) {
	va_list args;
	char* teststr;
	int i = 0;

	va_start(args, nstr);

	while (i < nstr) {
		teststr = va_arg(args, char*);
		if (!strcmp(match, teststr)) {
			va_end(args);
			return(i);
		}
		++i;
	}

	va_end(args);

	return(-1);
}

/*** Remove all instances of string "r" from string "str". ***/
void RemoveStr(char *str, char *r) {
	char *w;
	char *z1, *z2;
	uint l = strlen(r);

	w = strstr(str, r);
	while (w) {
		z1 = w;
		z2 = w + l;
		while (*z2) {
			*z1 = *z2;
			++z1;
			++z2;
		}
		*z1 = 0;
		w = strstr(str, r);
	}
}

/*** Is the given character text? (For a rather limited definition of "text".) ***/
int istext(int c) {
	return isalnum(c) || (c=='&' || c==39 || c==32 || c==',' || c=='.' || c=='?' || c=='!');
}

/*** Perform a perfect shuffle (i.e. every possible permutation is equally likely) of nel elements each of size elsize bytes. ***/
void perfect_shuffle(void* data, uint nel, size_t elsize) {
	char* buf = (char*)data;
	char cp[256];
	char *acp = NULL;
	char *ucp;
	u32 e, f;

	if (elsize > 256) {
		acp = NEW_ARRAY(char, elsize);
		ucp = acp;
	} else {
		ucp = cp;
	}

	for (e = nel - 1; e > 0; --e) {
		f = randint() % (e + 1);
		if (e != f) {
			OUR_MEMCPY(ucp, buf + (e * elsize), elsize);
			OUR_MEMCPY(buf + (e * elsize), buf + (f * elsize), elsize);
			OUR_MEMCPY(buf + (f * elsize), ucp, elsize);
		}
	}

	if (not_null(acp))
		delete acp;
}

/*** Ask user to make a selection in [min_valid, max_valid]. Prompt until the user gives the expected response. ***/
int menu_response(int min_valid, int max_valid, const char* prompt) {
	int ret;
	// TODO: cleanup scanf()
	UNSAFE_FUNCTION("menu_response");
	forever {
		printf("%s", prompt);
		scanf("%d", &ret);
		if (is_between_int(ret, min_valid, max_valid))
			break;
		printf("Invalid response. Please give a value from %d to %d.\n", min_valid, max_valid);
	}
	return(ret);
}

/*** Verify that the number of command-line arguments (besides program name/path) is at least min_arg. If not,
	print the specified error message and quit. ***/
void check_number_args(int argc, int min_arg, const char* errmsg) {
	if (argc <= min_arg) {
		printf("%s", errmsg);
		exit(1);
	}
}
/*** Yes or no response from user -- returns true iff yes ***/
bool yes_or_no(const char* prompt) {
	char res[256];
	// TODO: cleanup scanf()
	UNSAFE_FUNCTION("yes_or_no");
	forever {
		printf("%s",  prompt);
		scanf("%8s", res);
		if (tolower(res[0]) == 'y')
			return(true);
		if (tolower(res[0]) == 'n')
			return(false);
	}
	return(false);
}

/*** A hashing/fingerprinting function. Based on  IEEE POSIX P1003.2. ***/
u64 hash_FNV1a(const void* data, u32 data_size) {
	u64 hash = 14695981039346656037ULL;
	u64 vll;
	u32 v;
	char* w, *we;
	char vc;
	u16 vs;

	w = (char*)data;
	we = w + data_size;
	
	while (w + 7 < we) {
		vll = *((u64*)w);
		hash = hash ^ v;
		hash *= 1099511628211ULL;
		w += 8;
	}

	while (w + 3 < we) {
		v = *((u32*)w);
		hash = hash ^ v;
		hash *= 1099511628211ULL;
		w += 4;
	}
	
	switch (we - w) {
	case 0:
		break;
	case 1:
		vc = *w;
		hash = hash ^ vc;
		hash *= 1099511628211ULL;
		break;
	case 2:
		vs = *((u16*)w);
		hash = hash ^ vc;
		hash *= 1099511628211ULL;
		break;
	case 3:
		vs = *((u16*)w);
		hash = hash ^ vc;
		hash *= 1099511628211ULL;
		w += 2;
		vc = *w;
		hash = hash ^ vc;
		hash *= 1099511628211ULL;
		break;
	default:
		assert(false);
		exit(1);
	}

	return(hash);
}

static const int xorprimes[] =
	{
       39139,  39157,  39161,  39163,  39181,  39191,  39199,  39209,  39217,  39227, 
       39229,  39233,  39239,  39241,  39251,  39293,  39301,  39313,  39317,  39323, 
       39341,  39343,  39359,  39367,  39371,  39373,  39383,  39397,  39409,  39419, 
       39439,  39443,  39451,  39461,  39499,  39503,  39509,  39511,  39521,  39541, 
       39551,  39563,  39569,  39581,  39607,  39619,  39623,  39631,  39659,  39667, 
        8933,   8941,   8951,   8963,   8969,   8971,   8999,   9001,   9007,   9011, 
        9013,   9029,   9041,   9043,   9049,   9059,   9067,   9091,   9103,   9109, 
        9127,   9133,   9137,   9151,   9157,   9161,   9173,   9181,   9187,   9199, 
        9203,   9209,   9221,   9227,   9239,   9241,   9257,   9277,   9281,   9283, 
        9293,   9311,   9319,   9323,   9337,   9341,   9343,   9349,   9371,   9377, 
       24317,  24329,  24337,  24359,  24371,  24373,  24379,  24391,  24407,  24413, 
       24419,  24421,  24439,  24443,  24469,  24473,  24481,  24499,  24509,  24517, 
       24527,  24533,  24547,  24551,  24571,  24593,  24611,  24623,  24631,  24659, 
       53129,  53147,  53149,  53161,  53171,  53173,  53189,  53197,  53201,  53231, 
       53233,  53239,  53267,  53269,  53279,  53281,  53299,  53309,  53323,  53327, 
       53353,  53359,  53377,  53381,  53401,  53407,  53411,  53419,  53437,  53441, 
       53453,  53479,  53503,  53507,  53527,  53549,  53551,  53569,  53591,  53593, 
       53597,  53609,  53611,  53617,  53623,  53629,  53633,  53639,  53653,  53657, 
       42461,  42463,  42467,  42473,  42487,  42491,  42499,  42509,  42533,  42557, 
       42569,  42571,  42577,  42589,  42611,  42641,  42643,  42649,  42667,  42677, 
       42683,  42689,  42697,  42701,  42703,  42709,  42719,  42727,  42737,  42743, 
       42751,  42767,  42773,  42787,  42793,  42797,  42821,  42829,  42839,  42841, 
       40289,  40343,  40351,  40357,  40361,  40387,  40423,  40427,  40429,  40433, 
       40459,  40471,  40483,  40487,  40493,  40499,  40507,  40519,  40529,  40531, 
       40543,  40559,  40577,  40583,  40591,  40597,  40609,  40627,  40637,  40639, 
       40693,  40697,  40699,  40709,  40739,  40751,  40759,  40763,  40771,  40787, 
       17203,  17207,  17209,  17231,  17239,  17257,  17291,  17293,  17299,  17317, 
       17321,  17327,  17333,  17341,  17351,  17359,  17377,  17383,  17387,  17389, 
       17393,  17401,  17417,  17419,  17431,  17443,  17449,  17467,  17471,  17477, 
       17483,  17489,  17491,  17497,  17509,  17519,  17539,  17551,  17569,  17573, 
       17579,  17581,  17597,  17599,  17609,  17623,  17627,  17657,  17659,  17669 
};

/*** Reversable obfuscation for a (small, 9-bit) integer. ***/
int Obfuscate(int fid) {
	int oid;
	int x;
	int np;

	np = sizeof(xorprimes) / sizeof(int);

	// in the low word, lowest 7 bits are total garbage, the value is contained in the high 9 bits
	// this is then XORed with a random prime number from the array above
	// the high word contains the index into the array for the XOR prime used

	oid = (fid << 7) + (randint() & 127);
	x = randint() % np;
	oid ^= xorprimes[x];
	oid += (x << 16);

	return(oid);
}

int Unobfuscate(int ofid) {
	// inverse of the above operation.
	int x;

	x = (ofid >> 16);
	ofid &= 0xffff;
	ofid ^= xorprimes[x];
	ofid >>= 7;

	return(ofid) ;
}

int Obfuscate16(int id) {
	int oid;
	int x;
	int np;

	np = sizeof(xorprimes) / sizeof(int);

	x = randint() % np;
	oid = id;
	oid ^= xorprimes[x];
	oid += (x << 16);

	return(oid);
}

int Unobfuscate16(int oid) {
	int x;

	x = (oid >> 16);
	oid &= 0xffff;
	oid ^= xorprimes[x];

	return(oid) ;
}

/*** Compare two blocks of memory without short-circuiting. Useful if you want to compare
	a password or key (e.g.) but do not want to leak information about the comparison by
	taking a different amount of time depending on the number of leading bytes that match. ***/
bool secure_memequal(const void* array1, u32 array1_size, const void* array2, u32 array2_size) {
	const uchar* a1 = (const uchar*)array1;
	const uchar* a2 = (const uchar*)array2;
	const uchar* a1e = a1 + array1_size;
	u32 result = 0;

	if (array1_size != array2_size)
		return(false);
	
	while (a1 < a1e) {
		result |= ((*a1) ^ (*a2));
		++a1;
		++a2;
	}

	return(iszero(result));
}

/*** As above; a version of strcmp() that does not use any short-circuiting. ***/
bool secure_strcmp(const char* str1, const char* str2) {
	u32 l1, l2, l;
	bool ret;
	l1 = strlen(str1);
	l2 = strlen(str2);
	l = ((l1 < l2) ? l1 : l2);
	ret = secure_memequal(str1, l, str2, l);
	// TODO: may want to turn off optimization in this fn or give secure_memequal() a subtle side-effect (modifying l, then compare l to both l1 and l2 here)
	// to avoid moving the (l1 == l2) check before secure_memequal()
	return (l1 == l2) && ret;
}

static bool pattern_is_asterisks(const char* pat) {
	while (*pat) {
		if (*pat != '*')
			return(false);
		++pat;
	}

	return(true);
}

// TODO: ustring_matches_pattern
bool string_matches_pattern(const char* str, const char* pattern, bool is_case_sensitive) {
	const char* w1 = str;
	const char* w2 = pattern;
	bool ret;

	forever {
		if (iszero(*w1))
			return iszero(*w2) || pattern_is_asterisks(w2);
		if (iszero(*w2))
			return false;
		switch	(*w2) {
		case '?':
			break;
		case '*':
			ret = string_matches_pattern(w1, w2 + 1, is_case_sensitive);
			if (!ret) {
				do {
					++w1;
					ret = string_matches_pattern(w1, w2 + 1, is_case_sensitive);
				} while (!ret && nonzero(*w1));
			}
			return ret;
		default:
			if (is_case_sensitive && (*w1) != (*w2))
				return false;
			else if (!is_case_sensitive && tolower(*w1) != tolower(*w2))
				return false;
			break;
		}
		++w1;
		++w2;
	}
}

/*** Produce a beep of the specified frequency (in Hz) and duration (in milliseconds). ***/
void speaker_beep(u32 frequency, u32 duration_msec) {
#ifdef CODEHAPPY_WINDOWS
	/* just use the Windows API */
	Beep(frequency, duration_msec);
#else
#ifdef CODEHAPPY_NATIVE
	/* Linux version: this only appears to work in a (non-virtual) terminal. TODO: For
	   non-terminal applications, play the tone using WavRender + SDL? */
	const u32 SPEAKER_CLOCK_TICKS = 1193180;
	int file_desc = 1;
	if (ioctl(file_desc, KDMKTONE, 0)) {
		file_desc = open("/dev/tty0", O_RDONLY);
	}
	if (file_desc < 0) {
		file_desc = open("/dev/console", O_RDONLY);
	}
	if (file_desc < 0) {
		return;
	}
	ioctl(file_desc, KDMKTONE, (duration_msec << 16) | (SPEAKER_CLOCK_TICKS / frequency));
#endif  // TODO: WASM version?
#endif
}

// Set an ostream to max floating point precision.
void max_precision(std::ostream& o) {
	o.precision(std::numeric_limits<double>::max_digits10);
}

/* Replace the text "rw" in the string "p" with the text "with". */
void p_replace(char* p, const char* rw, const char* with) {
	int move = strlen(with) - strlen(rw);
	char* w = strstr(p, rw);
	while (!is_null(w)) {
		if (move < 0) {
			// 'with' shorter or equal in length to 'rw'
			memmove(w, w - move, strlen(w - move) + 1);
		} else if (move > 0) {
			// 'with' longer than 'rw'
			memmove(w + move, w, strlen(w) + 1);
		}
		if (with[0]) {
			strncpy(w, with, strlen(with));
		}
		w = strstr(p, rw);
	}
}

void p_replace(std::string& p, const std::string& rw, const std::string& with) {
	size_t i = 0;
	const size_t l = rw.length(), l2 = with.length();
	forever {
		i = p.find(rw, i);
		if (i == std::string::npos) {
			break;
		}
		p.erase(i, l);
		p.insert(i, with);
		i += l2;
	}
#if 0
	size_t i = 0;
	const size_t l = rw.length(), l2 = with.length();
	forever {
		i = p.find(rw, i);
		if (i == std::string::npos) {
			break;
		}

		p.replace(i, l, with);
		i += l2;
	}
#endif
}

const char* next_of_two(const char* w, char c1, char c2) {
	forever {
		if (0 == *w)
			break;
		if (c1 == *w)
			return w;
		if (c2 == *w)
			return w;
		++w;
	}
	return nullptr;
}

const char* next_of_three(const char* w, char c1, char c2, char c3) {
	forever {
		if (0 == *w)
			break;
		if (c1 == *w)
			return w;
		if (c2 == *w)
			return w;
		if (c3 == *w)
			return w;
		++w;
	}
	return nullptr;
}

// end misc.cpp
