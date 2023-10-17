/***

	sprintf.cpp

	A version of sprintf() that doesn't suck. Allocates the memory needed for the 
	formatted string and returns it.

	Copyright (c) 2014-2022 C. M. Street

***/

char *mysprintf(const char *fmt, ...) {
	u32 len;
	va_list args;
	char *buf;
	char *str;
	const char* w;
	union {
		int i;
		long l;
		long long ll;
		short s;
		double g;
		long double lg;
		char* str;
		char c;
		int *pi;
		} data;

	va_start(args, fmt);

	len = strlen(fmt) * 2 + 256UL;
	w = fmt;
	while (*w)
		{
		if ('%' == *w)
			{
			bool numeric = false, numericd = false;
			bool decimal = false;
			i32 numval;
			i32 decval;
			i32 lenmod = 0;
#define	RESET_STATE()		do { numeric = numericd = decimal = false; lenmod = decval = numval = 0; } while (0)
#define	INVALID_LENMOD()	((lenmod == 3) || (lenmod > 8) || (lenmod > 4 && (lenmod & 3) > 0) || (lenmod < 0))
LAgain:
			++w;
			switch (*w)
				{
			case '%':
				break;
				
			case 0:
				goto LDone;

			case '*':
			case '+':
			case '-':
			case ' ':
			case '#':
				goto LAgain;
			case '.':
				decimal = true;
				goto LAgain;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (!numeric)
					{
					numeric = true;
					numval = atoi(w);
					if (numval > 2048)
						return(NULL);	// ???
					}
				else if (decimal && !numericd)
					{
					numericd = true;
					decval = atoi(w);
					if (decval > 2048)
						return(NULL);
					}
				goto LAgain;

			case 'h':
				lenmod++;
				if (INVALID_LENMOD())
					return(NULL);
				goto LAgain;

			case 'l':
			case 'L':
				lenmod += 4;
				if (INVALID_LENMOD())
					return(NULL);
				goto LAgain;

			case 'j':
				lenmod = 8;
				goto LAgain;

			case 'z':
				if (sizeof(size_t) == sizeof(long))
					lenmod = 4;
				else if (sizeof(size_t) == sizeof(long long))
					lenmod = 8;
				goto LAgain;

			case 't':
				if (sizeof(ptrdiff_t) == sizeof(long))
					lenmod = 4;
				else if (sizeof(ptrdiff_t) == sizeof(long long))
					lenmod = 8;
				goto LAgain;

			case 'n':
				data.pi = va_arg(args, int*);
				break;

			case 'd':
			case 'i':
			case 'u':
			case 'o':
			case 'x':
			case 'X':
				/* integer types */
				switch (lenmod)
					{
				case 0:
					data.i = va_arg(args, int);
					break;
				case 1:
					// short is actually stored as an int in the argument list.
					data.s = va_arg(args, int);
					break;
				case 2:
					// char is actually stored as an int in the argument list.
					data.c = va_arg(args, int);
					break;
				case 4:
					data.l = va_arg(args, long);
					break;
				case 8:
					data.ll = va_arg(args, long long);
					break;
				default:
					return(NULL);
					}
				if (numeric)
					len += max_int(32, numval);
				else
					len += 32;
				RESET_STATE();
				break;

			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 'g':
			case 'G':
			case 'a':
			case 'A':
				// floating point
				switch (lenmod)
					{
				case 4:
				case 8:
					data.lg = va_arg(args, long double);
					break;
				default:
					data.g = va_arg(args, double);
					break;
					}
				if (numeric && numericd)
					len += max_int(32, numval + decval + 1);
				else if (numericd)
					len += max_int(32, decval + 1);
				else if (numeric)
					len += max_int(32, numval);
				else
					len += 32;
				RESET_STATE();
				break;

			case 'c':
				data.c = va_arg(args, int);
				len += 16;
				RESET_STATE();
				break;

			case 's':
				data.str = va_arg(args, char*);
				len += (strlen(data.str) + 1);
				RESET_STATE();
				break;
				}
			}
		++w;
		}

LDone:
	va_end(args);

	buf = new char [len + 1];
	if (unlikely(is_null(buf)))
		return(NULL);

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	str = new char [strlen(buf) + 1];
	if (unlikely(is_null(str)))
		return(buf);
	strcpy(str, buf);
	delete [] buf;

	return(str);
}

/*** end sprintf.cpp ***/
