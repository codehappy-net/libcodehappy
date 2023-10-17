/***

	extern.cpp

	A collection of miscellaneous functions, originally written by others, that
	were released to the public domain or otherwise incorporated into the library
	with permission.

	For other external libraries incorporated into libcodehappy, see the inc/external/
	folder.

	Part of libcodehappy, copyright 2022 Chris Street

***/

#include "libcodehappy.h"
#include <math.h>

/* 
** __julian.c (originally jdn.c) -- Julian Day Number computation
**
** public domain Julian Day Number functions
**
** Based on formulae originally posted by
**    Tom Van Flandern / Washington, DC / metares@well.sf.ca.us
**       in the UseNet newsgroup sci.astro.
**    Reposted 14 May 1991 in FidoNet C Echo conference by
**       Paul Schlyter (Stockholm)
** Minor corrections, added JDN to julian, and recast into C by
**    Raymond Gardner  Englewood, Colorado
** Modified for use with the codehappy library by C. M. Street.
**    Original documentation comments below; note that instead of
**    #defining the Catholic or Protestant dates of Gregorian calendar
**    adoption, this is now specified with an argument "papal" to
**    each function.
**
** Synopsis:
**      long ymd_to_jdn(int year, int month, int day, int julian_flag)
**      void jdn_to_ymd(long jdn, int *year, int *month, int *day,
**                                                      int julian_flag)
**      year is negative if BC
**      if julian_flag is >  0, use Julian calendar
**      if julian_flag is == 0, use Gregorian calendar
**      if julian_flag is <  0, routines decide based on date
**
** These routines convert Gregorian and Julian calendar dates to and 
** from Julian Day Numbers.  Julian Day Numbers (JDN) are used by 
** astronomers as a date/time measure independent of calendars and 
** convenient for computing the elapsed time between dates.  The JDN 
** for any date/time is the number of days (including fractional 
** days) elapsed since noon, 1 Jan 4713 BC.  Julian Day Numbers were 
** originated by Joseph Scaliger in 1582 and named after his father 
** Julius, not after Julius Caesar.  They are not related to the 
** Julian calendar. 
** 
** For dates from 1 Jan 4713 BC thru 12 Dec Feb 32766 AD, ymd_to_jdn() 
** will give the JDN for noon on that date.  jdn_to_ymd() will compute 
** the year, month, and day from the JDN.  Years BC are given (and 
** returned) as negative numbers.  Note that there is no year 0 BC; 
** the day before 1 Jan 1 AD is 31 Dec 1 BC.  Note also that 1 BC, 
** 5 BC, etc. are leap years.
** 
** Pope Gregory XIII decreed that the Julian calendar would end on 
** 4 Oct 1582 AD and that the next day would be 15 Oct 1582 in the 
** Gregorian Calendar.  The only other change is that centesimal 
** years (years ending in 00) would no longer be leap years 
** unless divisible by 400.  Britain and its possessions and 
** colonies continued to use the Julian calendar up until 2 Sep 
** 1752, when the next day became 14 Sep 1752 in the Gregorian 
** Calendar.  These routines can be compiled to use either 
** convention.  By default, the British convention will be used.  
** Simply #define PAPAL to use Pope Gregory's convention. 
** 
** Each routine takes, as its last argument, a flag to indicate 
** whether to use the Julian or Gregorian calendar convention.  If 
** this flag is negative, the routines decide based on the date 
** itself, using the changeover date described in the preceding 
** paragraph.  If the flag is zero, Gregorian conventions will be used, 
** and if the flag is positive, Julian conventions will be used. 
*/


/* Pope Gregory XIII's decree */
#define LASTJULDATE_PAPAL	15821004L   /* last day to use Julian calendar */
#define LASTJULJDN_PAPAL	2299160L    /* jdn of same */
/* British-American usage */
#define LASTJULDATE_BRITISH 	17520902L   /* last day to use Julian calendar */
#define LASTJULJDN_BRITISH	2361221L    /* jdn of same */
 
int32_t ymd_to_jdn(int y, int m, int d, int julian, int papal) {
        long jdn;

        if (julian < 0)         /* set Julian flag if auto set */
                julian = (((y * 100L) + m) * 100 + d  <= (papal ? LASTJULDATE_PAPAL : LASTJULDATE_BRITISH));

        if (y < 0)              /* adjust BC year */
                y++;

        if (julian)
                jdn = 367L * y - 7 * (y + 5001L + (m - 9) / 7) / 4
                + 275 * m / 9 + d + 1729777L;
        else
                jdn = (long)(d - 32076)
                + 1461L * (y + 4800L + (m - 14) / 12) / 4
                + 367 * (m - 2 - (m - 14) / 12 * 12) / 12
                - 3 * ((y + 4900L + (m - 14) / 12) / 100) / 4
                + 1;            /* correction by rdg */

        return jdn;
}


void jdn_to_ymd(long jdn, int *yy, int *mm, int *dd, int julian, int papal) {
        long x, z, m, d, y;
        long daysPer400Years = 146097L;
        long fudgedDaysPer4000Years = 1460970L + 31;

        if (julian < 0)                 /* set Julian flag if auto set */
                julian = (jdn <= (papal ? LASTJULJDN_PAPAL : LASTJULJDN_BRITISH));

        x = jdn + 68569L;
        if ( julian )
        {
                x += 38;
                daysPer400Years = 146100L;
                fudgedDaysPer4000Years = 1461000L + 1;
        }
        z = 4 * x / daysPer400Years;
        x = x - (daysPer400Years * z + 3) / 4;
        y = 4000 * (x + 1) / fudgedDaysPer4000Years;
        x = x - 1461 * y / 4 + 31;
        m = 80 * x / 2447;
        d = x - 2447 * m / 80;
        x = m / 11;
        m = m + 2 - 12 * x;
        y = 100 * (z - 49) + y + x;

        *yy = (int)y;
        *mm = (int)m;
        *dd = (int)d;

        if (*yy <= 0)                   /* adjust BC years */
                (*yy)--;
}


/*
**  COMMAFMT.C
**
**  Public domain by Bob Stout
**
**  Notes:  1. Use static buffer to eliminate error checks on buffer overflow
**             and reduce code size.
**          2. By making the numeric argument a long and prototyping it before
**             use, passed numeric arguments will be implicitly cast to longs
**             thereby avoiding int overflow.
**          3. Use the thousands grouping and thousands separator from the
**             ANSI locale to make this more robust.
*/

size_t commafmt(char   *buf,            /* Buffer for formatted string  */
                int     bufsize,        /* Size of buffer               */
                int64_t    N)           /* Number to convert            */
{
        int len = 1, posn = 1, sign = 1;
        char *ptr = buf + bufsize - 1;

        if (2 > bufsize)
        {
ABORT:          *buf = '\000';
                return 0;
        }

        *ptr-- = '\000';
        --bufsize;
        if (0LL > N)
        {
                sign = -1;
                N = -N;
        }

        for ( ; len <= bufsize; ++len, ++posn)
        {
                *ptr-- = (char)((N % 10LL) + '0');
                if (0LL == (N /= 10LL))
                        break;
                if (0 == (posn % 3))
                {
                        *ptr-- = ',';
                        ++len;
                }
                if (len >= bufsize)
                        goto ABORT;
        }

        if (0 > sign)
        {
                if (0 == bufsize)
                        goto ABORT;
                *ptr-- = '-';
                ++len;
        }

        strcpy(buf, ++ptr);
        return (size_t)len;
}

#define POLY 0x8408

/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

unsigned short crc16(char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8; 
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}

/*
**  CUBIC.C - Solve a cubic polynomial
**  public domain by Ross Cottrell
*/

void solve_cubic(double  a,
                double  b,
                double  c,
                double  d,
                int    *solutions,
                double *x)
{
      long double    a1 = b/a, a2 = c/a, a3 = d/a;
      long double    Q = (a1*a1 - 3.0*a2)/9.0;
      long double R = (2.0*a1*a1*a1 - 9.0*a1*a2 + 27.0*a3)/54.0;
      double    R2_Q3 = R*R - Q*Q*Q;

      double    theta;

      if (R2_Q3 <= 0)
      {
            *solutions = 3;
            theta = acos(R/sqrt(Q*Q*Q));
            x[0] = -2.0*sqrt(Q)*cos(theta/3.0) - a1/3.0;
            x[1] = -2.0*sqrt(Q)*cos((theta+2.0*M_PI)/3.0) - a1/3.0;
            x[2] = -2.0*sqrt(Q)*cos((theta+4.0*M_PI)/3.0) - a1/3.0;
      }
      else
      {
            *solutions = 1;
            x[0] = pow(sqrt(R2_Q3)+fabs(R), 1/3.0);
            x[0] += Q/x[0];
            x[0] *= (R < 0.0) ? 1 : -1;
            x[0] -= a1/3.0;
      }
}

/* ENG.C - Format floating point in engineering notation          */
/* Released to public domain by author, David Harmon, Jan. 1994   */ 
const char *eng_format(double value, int places) {
      const char * const prefixes[] = {
            "a", "f", "p", "n", "æ", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"
            };
      int p = 6;
      static char result[30];
      char *res = result;

      if (value < 0.)
      {
            *res++ = '-';
            value = -value;
      }
      while (value != 0 && value < 1. && p > 0)
      {
            value *= 1000.;
            p--;
      }
      while (value != 0 && value > 1000. && p < 14 )
      {
            value /= 1000.;
            p++;
      }
      if (value > 100.)
            places--;
      if (value > 10.)
            places--;
      sprintf(res, "%.*f %s", places-1, value, prefixes[p]);
      return result;
}

/*
**  HEXORINT.C - Detect if a string denotes a hex or decimal
**  number by detecting a leading "0X" or trailing "H" string.
**
**  public domain demo by Bob Stout
*/

#define LAST_CHAR(s) (((char *)s)[strlen(s) - 1])

/*
**  Let strtol() do most of the work
*/

long hexorint(const char *string) {
      int radix = 0;
      char valstr[128];

      // some improvements: added null check and leading '#' indicating hexadecimal -- CS
      if (is_null(string))
	  	return(0L);

      if (string[0] == '#')
      	{
      	strcpy(valstr, string + 1);
		radix = 16;
      	}
	  else
	  	{
        strcpy(valstr, string);
	  	}
	  
      if (strchr("Hh", LAST_CHAR(valstr)))
      {
            LAST_CHAR(valstr) = '\000';
            radix = 16;
      }
      return strtol(valstr, NULL, radix);
}

/*
**  ISISBN.C - Validate International Standard Book Numbers (ISBNs)
**
**  public domain by Maynard Hogg
*/

int isisbn(const char *str)
{
      int i = 0;
      int test = 0;
      int c; 

      while ('\0' != (c = *str++))
      {
            if (isdigit(c))
                  c -= '0';
            else if (i == 9 && 'X' == c)
                  c = 10;
            else continue;
            test += c * ++i;
      }
      return (i == 10 && test % 11 == 0);
}

/*
**  Originally published as part of the MicroFirm Function Library
**
**  Copyright 1987-88, Robert B.Stout
**
**  Subset version released to the public domain, 1992
**
**  Makes all whitespace single spaces. Passed a string, lv1ws()
**  converts all multiple whitespace characters to single spaces.
*/

void lv1ws(char *str)
{
      char *ibuf = str, *obuf = str;
      int i = 0, cnt = 0;

      while(*ibuf)
      {
            if(isspace(*ibuf) && cnt)
                  ibuf++;
            else
            {
                  if (!isspace(*ibuf))
                        cnt = 0;
                  else
                  {
                        *ibuf = ' ';
                        cnt = 1;
                  }
                  obuf[i++] = *ibuf++;
            }
      }
      obuf[i] = '\0';
}

/* PD by Michelangelo Jones, 1:1/124. */

/*
**  Returns 0 for new moon, 15 for full moon,
**  29 for the day before new, and so forth.
*/

/*
**  This routine sometimes gets "off" by a few days,
**  but is self-correcting.
*/

int moon_age(int month, int day, int year)
{
      static short int ages[] =
            {18, 0, 11, 22, 3, 14, 25, 6, 17,
             28, 9, 20, 1, 12, 23, 4, 15, 26, 7};
      static short int offsets[] =
            {-1, 1, 0, 1, 2, 3, 4, 5, 7, 7, 9, 9};

	if (month < 1 || month > 12)
		return(-1);

      if (day == 31)
            day = 1;
      return ((ages[(year + 1) % 19] + ((day + offsets[month-1]) % 30) +
            (year < 1900)) % 30);
}

/***  MSBIN conversion routines    ***/
/***  public domain by Jeffery Foy ***/

union Converter {
      unsigned char uc[10];
      unsigned int  ui[5];
      unsigned long ul[2];
      float          f[2];
      double         d[1];
};

/* MSBINToIEEE - Converts an MSBIN floating point number */
/*               to IEEE floating point format           */
/*                                                       */
/*  Input: f - floating point number in MSBIN format     */
/* Output: Same number in IEEE format                    */

float MSBINToIEEE(float f)
{
      union Converter t;
      int sign, exp;       /* sign and exponent */

      t.f[0] = f;

      /* extract the sign & move exponent bias from 0x81 to 0x7f */

      sign = t.uc[2] / 0x80;
      exp  = (t.uc[3] - 0x81 + 0x7f) & 0xff;

      /* reassemble them in IEEE 4 byte real number format */

      t.ui[1] = (t.ui[1] & 0x7f) | (exp << 7) | (sign << 15);
      return t.f[0];
} /* End of MSBINToIEEE */


/* IEEEToMSBIN - Converts an IEEE floating point number  */
/*               to MSBIN floating point format          */
/*                                                       */
/*  Input: f - floating point number in IEEE format      */
/* Output: Same number in MSBIN format                   */

float IEEEToMSBIN(float f)
{
      union Converter t;
      int sign, exp;       /* sign and exponent */

      t.f[0] = f;

      /* extract sign & change exponent bias from 0x7f to 0x81 */

      sign = t.uc[3] / 0x80;
      exp  = ((t.ui[1] >> 7) - 0x7f + 0x81) & 0xff;

      /* reassemble them in MSBIN format */

      t.ui[1] = (t.ui[1] & 0x7f) | (sign << 7) | (exp << 8);
      return t.f[0];
} /* End of IEEEToMSBIN */

/*
** Determine the permutation index for a given permutation list.
** Written by Thad Smith III, Boulder, CO  8/31/91
** Hereby contributed to the Public Domain.
**
** The following function computes the ordinal of the given permutation,
** which is index of the permutation in sorting order:
**  1, 2, ..., n-1, n   is index 0
**  1, 2, ..., n, n-1   is index 1
**  ...
**  n, n-1, ..., 2, 1   is index n! -1
**
** The actual values of the elements are immaterial, only the relative
** ordering of the values is used.
**
** pit[] is the array of elements of length size.
** The return value is the permutation index.
*/

int permutation_index (char pit[], int size)
{
      int i;
      int j, ball;
      int index = 0;

      for (i = 1; i < size; i++)
      {
            ball = pit[i-1];
            for (j = i; j < size; j++)
            {
                  if (ball > pit[j])
                        index ++;
            }
            index *= size - i;
      }
      return index;
}

/*
**  Originally published as part of the MicroFirm Function Library
**
**  Copyright 1986, S.E. Margison
**  Copyright 1989, Robert B.Stout
**
**  Subset version released to the public domain, 1991
**
**  remove all whitespace from a string
*/

char *rmallws(char *str)
{
      char *obuf, *nbuf;

      for (obuf = str, nbuf = str; *obuf && obuf; ++obuf)
      {
            if (!isspace(*obuf))
                  *nbuf++ = *obuf;
      }
      *nbuf = '\000';
      return str;
}

/* function scanfrac - scan an input string for a numeric value.
**
** Written in ANSI C and contributed to the public domain by
** Thad Smith III, Boulder, CO.     August 5, 1991
*/

/*******************************************************************
** scanfrac() scans an input string for a numeric value, which can
** be specified as:
**  1. an integer,               5
**  2. a floating point value,   5.1
**  3. a fraction, or            3/4
**  4. a mixed fraction.         5 3/4  or  5-3/4
**
** Conditions:
**  1. Preceeding whitespace is allowed.
**  2. The input number may be signed.
**  3. The fractional part of a mixed fraction (but not pure fraction)
**     must be less than 1.
**  4. The numerator and denominator of a fraction or mixed fraction
**     must be less than 2^31.
**
** Parameters:
**  1. Input buffer containing value.
**  2. Pointer to double to receive return value.
**
** Return status:
**  0 = OK, value returned in f,
**  1 = bad input format,
**  2 = can't allocate memory
*/

int scanfrac (const char buf[], double *f)
{
      char *tbuf = (char*)malloc (strlen(buf) +2);   /* input + terminator     */
      static char term[] = "\a";              /* terminator flag        */
      char t1,t2,t3;                          /* separator chars        */
      char sign;                              /* possible sign          */
      int nc;                                 /* # conversions          */
      long int b,c;                           /* 2nd & 3rd inputs       */

      if (!tbuf)                          /* couldn't allocate memory   */
            return 2;

      /* Copy the input to a temporary buffer and append a terminator
      ** character.  This terminator is used to determine whether the
      ** scanning of the input field by sscanf() was terminated by end
      ** of input or by an invalid character.  If terminated properly,
      ** the terminator character picked up in t1, t2, or t3.
      */

      strcat (strcpy(tbuf, buf), term);       /* input + term flag      */
      nc = sscanf (tbuf, " %lf%c %ld %c %ld %c",
                           f,&t1,&b,&t2,&c,&t3);
      free (tbuf);

      switch (nc)                   /* number of sscanf() conversions   */
      {
      case 2:         /* single floating value: a */
            if (t1 == *term) return 0;
            break;
      case 4:         /* pure fraction: a/b */
            if (t1 == '/' && t2 == *term && fmod (*f,1.0) == 0.0 && b > 0)
            {
                  *f /= b;
                  return 0;
            }
            break;
      case 6:         /* mixed fraction: a b/c  or  a-b/c */
            if ((t1 == ' ' || t1 == '-') && t2 == '/' && t3 == *term &&
                  fmod (*f,1.0) == 0.0 && b >= 0 && c > b)
            {
                  /* get first non-blank character so that
                  ** -0 b/c will be neg
                  */

#ifdef __ZTC__  /* fix for missing const in sscanf() declaration */
                  sscanf ((char*)buf, " %c", &sign);
#else
                  sscanf (buf, " %c", &sign);
#endif
                  if (sign == '-')
                        *f -= (double)b/c;
                  else  *f += (double)b/c;
                  return 0;
            }
      }
      return 1;
}

/*
** from Bob Jarvis
*/

char *soundex(char *instr, char *outstr)
{                   /* ABCDEFGHIJKLMNOPQRSTUVWXYZ */
        const char table[] = "01230120022455012623010202";
        int count = 0;

        while(!isalpha(instr[0]) && instr[0])
                ++instr;

        if(!instr[0])     /* Hey!  Where'd the string go? */
                return(NULL);

        if(toupper(instr[0]) == 'P' && toupper(instr[1]) == 'H')
        {
                instr[0] = 'F';
                instr[1] = 'A';
        }

        *outstr++ = (char)toupper(*instr++);

        while(*instr && count < 5)
        {
                if(isalpha(*instr) && *instr != *(instr-1))
                {
                        *outstr = table[toupper(instr[0]) - 'A'];
                        if(*outstr != '0')
                        {
                                ++outstr;
                                ++count;
                        }
                }
                ++instr;
        }

        *outstr = '\0';
        return(outstr);
}

/*
**  STR27SEG.C - Convert numeric strings to 7-segment strings.
**
**  Public domain by Bob Stout
**
**  Input:  A string (NUL-delimited char array) containing only digits
**          ('0' - '9' chars).
**
**  Output: The same string with each digit converted to a 7-segment
**          representation. Returns NULL on error.
*/

#define CAST(new_type,old_object) (*((new_type *)&old_object))
#define DISP(str) fputs((str), stdout)

/*
**  Define the bit significance
**
**     a
**    ---
**   |   |
**  f|   |b
**   | g |
**    ---
**   |   |
**  e|   |c
**   |   |
**    ---
**     d
*/

struct Seg7disp {
      unsigned seg_a : 1;
      unsigned seg_b : 1;
      unsigned seg_c : 1;
      unsigned seg_d : 1;
      unsigned seg_e : 1;
      unsigned seg_f : 1;
      unsigned seg_g : 1;
} Seg7digits[10] = {
      { 1, 1, 1, 1, 1, 1, 0 },      /* 0 */
      { 0, 1, 1, 0, 0, 0, 0 },      /* 1 */
      { 1, 1, 0, 1, 1, 0, 1 },      /* 2 */
      { 1, 1, 1, 1, 0, 0, 1 },      /* 3 */
      { 0, 1, 1, 0, 0, 1, 1 },      /* 4 */
      { 1, 0, 1, 1, 0, 1, 1 },      /* 5 */
      { 1, 0, 1, 1, 1, 1, 1 },      /* 6 */
      { 1, 1, 1, 0, 0, 0, 0 },      /* 7 */
      { 1, 1, 1, 1, 1, 1, 1 },      /* 8 */
      { 1, 1, 1, 1, 0, 1, 1 }       /* 9 */
};

char *str27seg(char *string)
{
      char *str;
      int ch;

      for (str = string ; *str; ++str)
      {
            if (!isdigit(*str))
                  return NULL;
            ch = CAST(int, Seg7digits[*str - '0']);
            *str = (char)(ch & 0xff);
      }
      return string;
}

/*
   --------------------------------------------------------------------
   Module:     REPLACE.C
   Author:     Gilles Kohl
   Started:    09.06.1992   12:16:47
   Modified:   09.06.1992   12:41:41
   Subject:    Replace one string by another in a given buffer.
               This code is public domain. Use freely.
   --------------------------------------------------------------------
*/

/*
 * StrReplace: Replace OldStr by NewStr in string Str.
 *
 * Str should have enough allocated space for the replacement, no check
 * is made for this. Str and OldStr/NewStr should not overlap.
 * The empty string ("") is found at the beginning of every string.
 *
 * Returns: pointer to first location behind where NewStr was inserted
 * or NULL if OldStr was not found.
 * This is useful for multiple replacements, see example in main() below
 * (be careful not to replace the empty string this way !)
 */

char *strreplace_us(char *Str, char *OldStr, char *NewStr)
{
      int OldLen, NewLen;
      char *p, *q;

      if(NULL == (p = strstr(Str, OldStr)))
            return p;
      OldLen = strlen(OldStr);
      NewLen = strlen(NewStr);
      memmove(q = p+NewLen, p+OldLen, strlen(p+OldLen)+1);
      memcpy(p, NewStr, NewLen);
      return q;
}

/*
** Public Domain by Jerry Coffin.
**
** Interpets a string in a manner similar to that the compiler
** does string literals in a program.  All escape sequences are
** longer than their translated equivalant, so the string is
** translated in place and either remains the same length or
** becomes shorter.
*/

char *translate_string_c_literal(char *string)
{
      char *here=string;
      size_t len=strlen(string);
      int num;
      int numlen;

      while (NULL!=(here=strchr(here,'\\')))
      {
            numlen=1;
            switch (here[1])
            {
            case '\\':
                  break;

            case 'r':
                  *here = '\r';
                  break;

            case 'n':
                  *here = '\n';
                  break;

            case 't':
                  *here = '\t';
                  break;

            case 'v':
                  *here = '\v';
                  break;

            case 'a':
                  *here = '\a';
                  break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                  numlen = sscanf(here,"%o",&num);
                  *here = (char)num;
                  break;

            case 'x':
                  numlen = sscanf(here,"%x",&num);
                  *here = (char) num;
                  break;
            }
            num = here - string + numlen;
            here++;
            memmove(here,here+numlen,len-num );
      }
      return string;
}

/*
**  TRIM.C - Remove leading, trailing, & excess embedded spaces
**
**  public domain by Bob Stout
*/

char *trim_whitespace(char *str)
{
      char *ibuf = str, *obuf = str;
      int i = 0, cnt = 0;

      /*
      **  Trap NULL
      */

      if (str)
      {
            /*
            **  Remove leading spaces (from RMLEAD.C)
            */

            for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
                  ;
            if (str != ibuf)
                  memmove(str, ibuf, ibuf - str);

            /*
            **  Collapse embedded spaces (from LV1WS.C)
            */

            while (*ibuf)
            {
                  if (isspace(*ibuf) && cnt)
                        ibuf++;
                  else
                  {
                        if (!isspace(*ibuf))
                              cnt = 0;
                        else
                        {
                              *ibuf = ' ';
                              cnt = 1;
                        }
                        obuf[i++] = *ibuf++;
                  }
            }
            obuf[i] = '\000';

            /*
            **  Remove trailing spaces (from RMTRAIL.C)
            */

            while (--i >= 0)
            {
                  if (!isspace(obuf[i]))
                        break;
            }
            obuf[++i] = '\000';
      }
      return str;
}

/*
**  Originally published as part of the MicroFirm Function Library
**
**  Copyright 1991, Robert B.Stout
**
**  Subset version with modifications suggested by Maynard Hogg
**  released to the public domain, 1992
**
**  Function to return ordinal text.
*/

static const char *text[] = {"th", "st", "nd", "rd"};

const char *ordinal_text_suffix(int number) {
      if (((number %= 100) > 9 && number < 20) || (number %= 10) > 3)
            number = 0;
      return text[number];
}


/*
**
**  remove leading whitespace from a string
**
*/

char *rmlead(char *str) {
      char *obuf;

      for (obuf = str; obuf && *obuf && isspace(*obuf); ++obuf)
            ;
      if (str != obuf)
            strcpy(str, obuf);
      return str;
}

/*
**
**  remove trailing whitespace from a string
**
*/

char *rmtrail(char *str) {
      int i;

      if (0 != (i = strlen(str)))
      {
            while (--i >= 0)
            {
                  if (!isspace(str[i]))
                        break;
            }
            str[++i] = '\000';
      }
      return str;
}


/*
**  EASTER.C - Determine the date of Easter for any given year
**
**  public domain by Ed Bernal
*/

void easter(int year,int *easter_month, int *easter_day)
{
      int a,b,c,e,g,h,i,k,u,x,z;
      div_t f;

      /*
      **  Gauss' famous algorithm (I don't know how or why it works,
      **  so there's no commenting)
      */

      a = year % 19;
      f = div(year,100);
      b = f.quot;
      c = f.rem;
      f = div(b,4);
      z = f.quot;
      e = f.rem;
      f = div((8*b + 13),25);
      g = f.quot;
      f = div((19*a + b - z - g + 15),30);
      h = f.rem;
      f = div((a + 11*h),319);
      u = f.quot;
      f = div(c,4);
      i = f.quot;
      k = f.rem;
      f = div((2*e + 2*i - k - h + u + 32),7);
      x = f.rem;
      f = div((h-u+x+90),25);
      *easter_month = f.quot;
      f = div((h-u+x + *easter_month +19),32);
      *easter_day = f.rem;
}

/*

SUNRISET.C - computes Sun rise/set times, start/end of twilight, and
             the length of the day at any date and latitude

Written as DAYLEN.C, 1989-08-16

Modified to SUNRISET.C, 1992-12-01

(c) Paul Schlyter, 1989, 1992

Released to the public domain by Paul Schlyter, December 1992

Modified for use with the codehappy library, C. M. Street

*/

/* A macro to compute the number of days elapsed since 2000 Jan 0.0 */
/* (which is equal to 1999 Dec 31, 0h UT)                           */

#define days_since_2000_Jan_0(y,m,d) \
    (367L*(y)-((7*((y)+(((m)+9)/12)))/4)+((275*(m))/9)+(d)-730530L)

/* Some conversion factors between radians and degrees */

#ifndef PI
 #define PI        3.1415926535897932384
#endif

#define RADEG     ( 180.0 / PI )
#define DEGRAD    ( PI / 180.0 )

/* The trigonometric functions in degrees */

#define sind(x)  sin((x)*DEGRAD)
#define cosd(x)  cos((x)*DEGRAD)
#define tand(x)  tan((x)*DEGRAD)

#define atand(x)    (RADEG*atan(x))
#define asind(x)    (RADEG*asin(x))
#define acosd(x)    (RADEG*acos(x))
#define atan2d(y,x) (RADEG*atan2(y,x))


#if 0
/* A small test program */

void main(void)
{
      int year,month,day;
      double lon, lat;
      double daylen, civlen, nautlen, astrlen;
      double rise, set, civ_start, civ_end, naut_start, naut_end,
             astr_start, astr_end;
      int    rs, civ, naut, astr;

      printf( "Longitude (+ is east) and latitude (+ is north) : " );
      scanf( "%lf %lf", &lon, &lat );

      for(;;)
      {
            printf( "Input date ( yyyy mm dd ) (ctrl-C exits): " );
            scanf( "%d %d %d", &year, &month, &day );

            daylen  = day_length(year,month,day,lon,lat);
            civlen  = day_civil_twilight_length(year,month,day,lon,lat);
            nautlen = day_nautical_twilight_length(year,month,day,lon,lat);
            astrlen = day_astronomical_twilight_length(year,month,day,
                  lon,lat);

            printf( "Day length:                 %5.2f hours\n", daylen );
            printf( "With civil twilight         %5.2f hours\n", civlen );
            printf( "With nautical twilight      %5.2f hours\n", nautlen );
            printf( "With astronomical twilight  %5.2f hours\n", astrlen );
            printf( "Length of twilight: civil   %5.2f hours\n",
                  (civlen-daylen)/2.0);
            printf( "                  nautical  %5.2f hours\n",
                  (nautlen-daylen)/2.0);
            printf( "              astronomical  %5.2f hours\n",
                  (astrlen-daylen)/2.0);

            rs   = sun_rise_set         ( year, month, day, lon, lat,
                                          &rise, &set );
            civ  = civil_twilight       ( year, month, day, lon, lat,
                                          &civ_start, &civ_end );
            naut = nautical_twilight    ( year, month, day, lon, lat,
                                          &naut_start, &naut_end );
            astr = astronomical_twilight( year, month, day, lon, lat,
                                          &astr_start, &astr_end );

            printf( "Sun at south %5.2fh UT\n", (rise+set)/2.0 );

            switch( rs )
            {
                case 0:
                    printf( "Sun rises %5.2fh UT, sets %5.2fh UT\n",
                             rise, set );
                    break;
                case +1:
                    printf( "Sun above horizon\n" );
                    break;
                case -1:
                    printf( "Sun below horizon\n" );
                    break;
            }

            switch( civ )
            {
                case 0:
                    printf( "Civil twilight starts %5.2fh, "
                            "ends %5.2fh UT\n", civ_start, civ_end );
                    break;
                case +1:
                    printf( "Never darker than civil twilight\n" );
                    break;
                case -1:
                    printf( "Never as bright as civil twilight\n" );
                    break;
            }

            switch( naut )
            {
                case 0:
                    printf( "Nautical twilight starts %5.2fh, "
                            "ends %5.2fh UT\n", naut_start, naut_end );
                    break;
                case +1:
                    printf( "Never darker than nautical twilight\n" );
                    break;
                case -1:
                    printf( "Never as bright as nautical twilight\n" );
                    break;
            }

            switch( astr )
            {
                case 0:
                    printf( "Astronomical twilight starts %5.2fh, "
                            "ends %5.2fh UT\n", astr_start, astr_end );
                    break;
                case +1:
                    printf( "Never darker than astronomical twilight\n" );
                    break;
                case -1:
                    printf( "Never as bright as astronomical twilight\n" );
                    break;
            }

      }
}
#endif

/* The "workhorse" function for sun rise/set times */

int __sunriset__( int year, int month, int day, double lon, double lat,
                  double altit, int upper_limb, double *trise, double *tset )
/***************************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value IS critical in this function!            */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing rise/set     */
/*               times, and to zero when computing start/end of       */
/*               twilight.                                            */
/*        *rise = where to store the rise time                        */
/*        *set  = where to store the set  time                        */
/*                Both times are relative to the specified altitude,  */
/*                and thus this function can be used to comupte       */
/*                various twilight times, as well as rise/set times   */
/* Return value:  0 = sun rises/sets this day, times stored at        */
/*                    *trise and *tset.                               */
/*               +1 = sun above the specified "horizon" 24 hours.     */
/*                    *trise set to time when the sun is at south,    */
/*                    minus 12 hours while *tset is set to the south  */
/*                    time plus 12 hours. "Day" length = 24 hours     */
/*               -1 = sun is below the specified "horizon" 24 hours   */
/*                    "Day" length = 0 hours, *trise and *tset are    */
/*                    both set to the time when the sun is at south.  */
/*                                                                    */
/**********************************************************************/
{
      double  d,  /* Days since 2000 Jan 0.0 (negative before) */
      sr,         /* Solar distance, astronomical units */
      sRA,        /* Sun's Right Ascension */
      sdec,       /* Sun's declination */
      sradius,    /* Sun's apparent radius */
      t,          /* Diurnal arc */
      tsouth,     /* Time when Sun is at south */
      sidtime;    /* Local sidereal time */

      int rc = 0; /* Return cde from function - usually 0 */

      /* Compute d of 12h local mean solar time */
      d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

      /* Compute local sideral time of this moment */
      sidtime = revolution( GMST0(d) + 180.0 + lon );

      /* Compute Sun's RA + Decl at this moment */
      sun_RA_dec( d, &sRA, &sdec, &sr );

      /* Compute time when Sun is at south - in hours UT */
      tsouth = 12.0 - rev180(sidtime - sRA)/15.0;

      /* Compute the Sun's apparent radius, degrees */
      sradius = 0.2666 / sr;

      /* Do correction to upper limb, if necessary */
      if ( upper_limb )
            altit -= sradius;

      /* Compute the diurnal arc that the Sun traverses to reach */
      /* the specified altitide altit: */
      {
            double cost;
            cost = ( sind(altit) - sind(lat) * sind(sdec) ) /
                  ( cosd(lat) * cosd(sdec) );
            if ( cost >= 1.0 )
                  rc = -1, t = 0.0;       /* Sun always below altit */
            else if ( cost <= -1.0 )
                  rc = +1, t = 12.0;      /* Sun always above altit */
            else
                  t = acosd(cost)/15.0;   /* The diurnal arc, hours */
      }

      /* Store rise and set times - in hours UT */
      *trise = tsouth - t;
      *tset  = tsouth + t;

      return rc;
}  /* __sunriset__ */



/* The "workhorse" function */


double __daylen__( int year, int month, int day, double lon, double lat,
                   double altit, int upper_limb )
/**********************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value is not critical. Set it to the correct   */
/*       longitude if you're picky, otherwise set to to, say, 0.0     */
/*       The latitude however IS critical - be sure to get it correct */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing day length   */
/*               and to zero when computing day+twilight length.      */
/**********************************************************************/
{
      double  d,  /* Days since 2000 Jan 0.0 (negative before) */
      obl_ecl,    /* Obliquity (inclination) of Earth's axis */
      sr,         /* Solar distance, astronomical units */
      slon,       /* True solar longitude */
      sin_sdecl,  /* Sine of Sun's declination */
      cos_sdecl,  /* Cosine of Sun's declination */
      sradius,    /* Sun's apparent radius */
      t;          /* Diurnal arc */

      /* Compute d of 12h local mean solar time */
      d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

      /* Compute obliquity of ecliptic (inclination of Earth's axis) */
      obl_ecl = 23.4393 - 3.563E-7 * d;

      /* Compute Sun's position */
      sunpos( d, &slon, &sr );

      /* Compute sine and cosine of Sun's declination */
      sin_sdecl = sind(obl_ecl) * sind(slon);
      cos_sdecl = sqrt( 1.0 - sin_sdecl * sin_sdecl );

      /* Compute the Sun's apparent radius, degrees */
      sradius = 0.2666 / sr;

      /* Do correction to upper limb, if necessary */
      if ( upper_limb )
            altit -= sradius;

      /* Compute the diurnal arc that the Sun traverses to reach */
      /* the specified altitide altit: */
      {
            double cost;
            cost = ( sind(altit) - sind(lat) * sin_sdecl ) /
                  ( cosd(lat) * cos_sdecl );
            if ( cost >= 1.0 )
                  t = 0.0;                      /* Sun always below altit */
            else if ( cost <= -1.0 )
                  t = 24.0;                     /* Sun always above altit */
            else  t = (2.0/15.0) * acosd(cost); /* The diurnal arc, hours */
      }
      return t;
}  /* __daylen__ */


/* This function computes the Sun's position at any instant */

void sunpos( double d, double *lon, double *r )
/******************************************************/
/* Computes the Sun's ecliptic longitude and distance */
/* at an instant given in d, number of days since     */
/* 2000 Jan 0.0.  The Sun's ecliptic latitude is not  */
/* computed, since it's always very near 0.           */
/******************************************************/
{
      double M,         /* Mean anomaly of the Sun */
             w,         /* Mean longitude of perihelion */
                        /* Note: Sun's mean longitude = M + w */
             e,         /* Eccentricity of Earth's orbit */
             E,         /* Eccentric anomaly */
             x, y,      /* x, y coordinates in orbit */
             v;         /* True anomaly */

      /* Compute mean elements */
      M = revolution( 356.0470 + 0.9856002585 * d );
      w = 282.9404 + 4.70935E-5 * d;
      e = 0.016709 - 1.151E-9 * d;

      /* Compute true longitude and radius vector */
      E = M + e * RADEG * sind(M) * ( 1.0 + e * cosd(M) );
            x = cosd(E) - e;
      y = sqrt( 1.0 - e*e ) * sind(E);
      *r = sqrt( x*x + y*y );              /* Solar distance */
      v = atan2d( y, x );                  /* True anomaly */
      *lon = v + w;                        /* True solar longitude */
      if ( *lon >= 360.0 )
            *lon -= 360.0;                   /* Make it 0..360 degrees */
}

void sun_RA_dec( double d, double *RA, double *dec, double *r )
{
      double lon, obl_ecl, x, y, z;

      /* Compute Sun's ecliptical coordinates */
      sunpos( d, &lon, r );

      /* Compute ecliptic rectangular coordinates (z=0) */
      x = *r * cosd(lon);
      y = *r * sind(lon);

      /* Compute obliquity of ecliptic (inclination of Earth's axis) */
      obl_ecl = 23.4393 - 3.563E-7 * d;

      /* Convert to equatorial rectangular coordinates - x is uchanged */
      z = y * sind(obl_ecl);
      y = y * cosd(obl_ecl);

      /* Convert to spherical coordinates */
      *RA = atan2d( y, x );
      *dec = atan2d( z, sqrt(x*x + y*y) );

}  /* sun_RA_dec */


/******************************************************************/
/* This function reduces any angle to within the first revolution */
/* by subtracting or adding even multiples of 360.0 until the     */
/* result is >= 0.0 and < 360.0                                   */
/******************************************************************/

#define INV360    ( 1.0 / 360.0 )

double revolution( double x )
/*****************************************/
/* Reduce angle to within 0..360 degrees */
/*****************************************/
{
      return( x - 360.0 * floor( x * INV360 ) );
}  /* revolution */

double rev180( double x )
/*********************************************/
/* Reduce angle to within +180..+180 degrees */
/*********************************************/
{
      return( x - 360.0 * floor( x * INV360 + 0.5 ) );
}  /* revolution */


/*******************************************************************/
/* This function computes GMST0, the Greenwhich Mean Sidereal Time */
/* at 0h UT (i.e. the sidereal time at the Greenwhich meridian at  */
/* 0h UT).  GMST is then the sidereal time at Greenwich at any     */
/* time of the day.  I've generelized GMST0 as well, and define it */
/* as:  GMST0 = GMST - UT  --  this allows GMST0 to be computed at */
/* other times than 0h UT as well.  While this sounds somewhat     */
/* contradictory, it is very practical:  instead of computing      */
/* GMST like:                                                      */
/*                                                                 */
/*  GMST = (GMST0) + UT * (366.2422/365.2422)                      */
/*                                                                 */
/* where (GMST0) is the GMST last time UT was 0 hours, one simply  */
/* computes:                                                       */
/*                                                                 */
/*  GMST = GMST0 + UT                                              */
/*                                                                 */
/* where GMST0 is the GMST "at 0h UT" but at the current moment!   */
/* Defined in this way, GMST0 will increase with about 4 min a     */
/* day.  It also happens that GMST0 (in degrees, 1 hr = 15 degr)   */
/* is equal to the Sun's mean longitude plus/minus 180 degrees!    */
/* (if we neglect aberration, which amounts to 20 seconds of arc   */
/* or 1.33 seconds of time)                                        */
/*                                                                 */
/*******************************************************************/

double GMST0( double d )
{
      double sidtim0;
      /* Sidtime at 0h UT = L (Sun's mean longitude) + 180.0 degr  */
      /* L = M + w, as defined in sunpos().  Since I'm too lazy to */
      /* add these numbers, I'll let the C compiler do it for me.  */
      /* Any decent C compiler will add the constants at compile   */
      /* time, imposing no runtime or code overhead.               */
      sidtim0 = revolution( ( 180.0 + 356.0470 + 282.9404 ) +
                          ( 0.9856002585 + 4.70935E-5 ) * d );
      return sidtim0;
}  /* GMST0 */

static unsigned char __a2e[256] = {
          0,  1,  2,  3, 55, 45, 46, 47, 22,  5, 37, 11, 12, 13, 14, 15,
         16, 17, 18, 19, 60, 61, 50, 38, 24, 25, 63, 39, 28, 29, 30, 31,
         64, 79,127,123, 91,108, 80,125, 77, 93, 92, 78,107, 96, 75, 97,
        240,241,242,243,244,245,246,247,248,249,122, 94, 76,126,110,111,
        124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,214,
        215,216,217,226,227,228,229,230,231,232,233, 74,224, 90, 95,109,
        121,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
        151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,  7,
         32, 33, 34, 35, 36, 21,  6, 23, 40, 41, 42, 43, 44,  9, 10, 27,
         48, 49, 26, 51, 52, 53, 54,  8, 56, 57, 58, 59,  4, 20, 62,225,
         65, 66, 67, 68, 69, 70, 71, 72, 73, 81, 82, 83, 84, 85, 86, 87,
         88, 89, 98, 99,100,101,102,103,104,105,112,113,114,115,116,117,
        118,119,120,128,138,139,140,141,142,143,144,154,155,156,157,158,
        159,160,170,171,172,173,174,175,176,177,178,179,180,181,182,183,
        184,185,186,187,188,189,190,191,202,203,204,205,206,207,218,219,
        220,221,222,223,234,235,236,237,238,239,250,251,252,253,254,255
};

static unsigned char __e2a[256] = {
          0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
         16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
        128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
        144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
         32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
         38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
         45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
        186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
        195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
        202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
        209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
        216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
        123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
        125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
         92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
         48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
};

char ebcdic_to_ascii(const char c) {
	return (char)(__e2a[(unsigned char)(c)]);
}

char ascii_to_ebcdic(const char c) {
	return (char)(__a2e[(unsigned char)(c)]);
}

void cstr_ebcdic_to_ascii(char* c) {
	while (*c) {
		*c = ebcdic_to_ascii(*c);
		c++;
	}
}

void cstr_ascii_to_ebcdic(char* c) {
	while (*c) {
		*c = ascii_to_ebcdic(*c);
		c++;
	}
}

#define CPSTART(c) (((unsigned char) (c)) >> (CHAR_BIT - 2) != 2)

static char const *DELIM = "\x1f";
static char const CORNER = '+';
static char const INTERSECT = '*';

static inline size_t max(size_t a, size_t b)
{
        return a > b ? a : b;
}

static size_t utf8len(char const *s)
{
        size_t len = 0;
        char const *c;

        for (c = s; *c; ++c) {
                if (CPSTART(*c)) {
                        len += 1;
                }
        }

        return len;
}

static size_t b2cp(char const *s, size_t n)
{
        size_t cp, i;

        for (cp = i = 0; s[i] && n > 0; ++i, --n) {
                if (CPSTART(s[i])) {
                        cp += 1;
                }
        }

        return cp;
}

void fputnc(int c, size_t n, FILE *f)
{
        while (n --> 0) {
                fputc(c, f);
        }
}

static size_t find_break(char const *s, size_t max, bool *hyphen)
{
        char *c;
        size_t brk, cp;

        if (!*s) {
                *hyphen = false;
                return 0;
        }

        for (c = (char*)s, cp = brk = 0; *c && cp < max; ++c) {
                brk += 1;
                if (CPSTART(*c)) {
                        cp += 1;
                }
        }

        while (!CPSTART(*c)) {
                c += 1;
                brk += 1;
        }

        while (*c && c != s && !isspace(*c)) {
                c -= 1;
        }

        if (c == s) {
                *hyphen = true;
                while (!CPSTART(s[--brk]));
                return brk;
        }

        *hyphen = false;

        return c - s;
}

static void print_row(char * const *data, size_t *max, size_t *remaining, size_t cols, FILE *f)
{
        static size_t alloc = 0;
        static char const **cells = NULL;

        size_t i, n, pad;
        bool hyphen, finished;

        if (cols > alloc) {
                char const **tmp;
                alloc = cols;
                tmp = (const char**)realloc(cells, alloc * sizeof *cells);
                if (tmp == NULL) {
                        return;
                }

                cells = tmp;

        }

        for (i = 0; i < cols; ++i) {
                cells[i] = data[i];
                remaining[i] = utf8len(cells[i]);
        }

        for (finished = false; !finished;) {
                finished = true;
                for (i = 0; i < cols; ++i) {
                        fputs("| ", f);

                        n = find_break(cells[i], max[i], &hyphen);
                        fwrite(cells[i], 1, n, f);

                        if (hyphen) {
                                fputc('-', f);
                        } else if (isspace(cells[i][n])) {
                                fputc(' ', f);
                                remaining[i] -= 1;
                                cells[i] += 1;
                        } else {
                                fputc(' ', f);
                        }

                        remaining[i] -= b2cp(cells[i], n);

                        if (remaining[i] != 0) {
                                finished = false;
                        }

                        pad = max[i] - b2cp(cells[i], n);
                        fputnc(' ', pad, f);

                        cells[i] += n;

                        if (i + 1 == cols) {
                                fputs("|\n", f);
                        }
                }
        }
}

bool table_init(struct table *t, ...)
{
        va_list ap;
        char *header, *fmt, *tmp, **tmph;
        size_t i, fmtlen, totalfmtlen;

        totalfmtlen = 0;
        
        t->rows     = 0;
        t->cols     = 0;
        t->alloc    = 0;
        t->data     = NULL;
        t->fmt      = NULL;
        t->headers  = NULL;

        va_start(ap, t);

        for (;; ++t->cols) {
                header = va_arg(ap, char *);
                if (header == NULL) {
                        break;
                }
                fmt    = va_arg(ap, char *);

                tmph = (char **)realloc(t->headers, (t->cols + 1) * sizeof (char *));
                if (tmph == NULL) {
                        free(t->headers);
                        return false;
                }
                t->headers = tmph;

                t->headers[t->cols] = header;

                fmtlen = strlen(fmt);
                tmp = (char *)realloc(t->fmt, totalfmtlen + fmtlen + 2);
                if (tmp == NULL) {
                        free(t->fmt);
                        free(t->headers);
                        return false;
                }
                t->fmt = tmp;
                strcpy(t->fmt + totalfmtlen, fmt);
                totalfmtlen += fmtlen;
                strcpy(t->fmt + totalfmtlen, DELIM);
                totalfmtlen += 1;
        }

        va_end(ap);

        t->max = (size_t *)malloc(t->cols * sizeof *t->max);
        if (t->max == NULL) {
                free(t->headers);
                free(t->fmt);
                return false;
        }

        for (i = 0; i < t->cols; ++i) {
                t->max[i] = utf8len(t->headers[i]);
        }

        return true;
}

bool table_add(struct table *t, ...)
{
        va_list ap;
        char *field, **row, buffer[512];
        size_t i;

        if (t->rows == t->alloc) {
                char ***tmp;
                t->alloc = t->alloc * 2 + 4;
                tmp      = (char ***)realloc(t->data, t->alloc * sizeof (char **));
                if (tmp == NULL) {
                        return false;
                }
                t->data = tmp;
        }

        row = (t->data[t->rows++] = (char **)malloc(t->cols * sizeof (char *)));

        if (row == NULL) {
                t->rows -= 1;
                return false;
        }



        va_start(ap, t);
        vsnprintf(buffer, sizeof buffer, t->fmt, ap);
        va_end(ap);

        field = strtok(buffer, DELIM);

        for (i = 0; field != NULL; ++i, field = strtok(NULL, DELIM)) {
                assert(&row[i] == &t->data[t->rows - 1][i]);
                row[i] = (char *)malloc(strlen(field) + 1);
                if (row[i] == NULL) {
                        goto err;
                }
                strcpy(row[i], field);

                t->max[i] = max(t->max[i], utf8len(field));
        }

        return true;

err:
        while (i --> 0) {
                free(row[i]);
        }
        
        return false;
}

bool table_print(struct table const *t, size_t n, FILE *f)
{
        size_t i, width, avg, trimthrshld, *max, *remaining;

        if (n < t->cols * 3 + 4) {
                /* Not enough space */
                return false;
        }

        n -= 2;

        max = (size_t *)malloc(t->cols * sizeof *max);
        if (max == NULL) {
                return false;
        }

        remaining = (size_t *)malloc(t-> cols * sizeof *remaining);
        if (remaining == NULL) {
                free(max);
                return false;
        }

        width = t->cols * 3 + 1;
        for (i = 0; i < t->cols; ++i) {
                max[i] = t->max[i];
                width += t->max[i];
        }

        avg = n / t->cols;
        trimthrshld = 0;

        while (width > n) {
                bool none = true;
                for (i = 0; i < t->cols; ++i) {
                        if (max[i] + trimthrshld > avg) {
                                max[i] -= 1;
                                width -= 1;
                                none = false;
                        }
                }

                if (none) {
                        trimthrshld += 1;
                }
        }

        fputc(CORNER, f);
        fputnc('-', width - 2, f);
        fputc(CORNER, f);
        fputc('\n', f);

        print_row(t->headers, max, remaining, t->cols, f);

        fputc(INTERSECT, f);
        fputnc('-', width - 2, f);
        fputc(INTERSECT, f);
        fputc('\n', f);

        for (i = 0; i < t->rows; ++i) {
                print_row(t->data[i], max, remaining, t->cols, f);

                if (i + 1 < t->rows) {
                        size_t j;
                        fputc('|', f);
                        for (j = 0; j < t->cols; ++j) {
                                fputnc('-', max[j] + 2, f);
                                fputc('|', f);
                        }
                        fputc('\n', f);
                }
        }

        fputc(CORNER, f);
        fputnc('-', width - 2, f);
        fputc(CORNER, f);
        fputc('\n', f);

        return true;
}

void table_free(struct table *t)
{
        size_t i, j;

        for (i = 0; i < t->rows; ++i) {
                for (j = 0; j < t->cols; ++j) {
                        free(t->data[i][j]);
                }
        }

        for (i = 0; i < t->cols; ++i) {
                free(t->data[i]);
        }

        free(t->data);
        free(t->headers);
        free(t->max);
        free(t->fmt);
}

/* $Id: tmpfileplus.c $ */
/*
 * $Date: 2013-05-16 08:07Z $
 * $Revision: 1.0.1 $
 * $Author: dai $
 */

/**************************** COPYRIGHT NOTICE ********************************
  This code was originally written by David Ireland and is copyright (C)
  2012-13 DI Management Services Pty Ltd <www.di-mgt.com.au>. It is provided
  "as is" with no warranties. Use at your own risk. You must make your own
  assessment of its accuracy and suitability for your own purposes. It is
  not to be altered or distributed, except as part of an application. You
  are free to use it in any application, provided this copyright notice is
  left unchanged.  
************************* END OF COPYRIGHT NOTICE ****************************/

/*
* NAME
*        tmpfileplus - create a unique temporary file
* 
* SYNOPSIS
*        FILE *tmpfileplus(const char *dir, const char *prefix, char **pathname, int keep)
*
* DESCRIPTION
*        The tmpfileplus() function opens a unique temporary file in binary
*        read/write (w+b) mode. The file is opened with the O_EXCL flag,
*        guaranteeing that the caller is the only user. The filename will consist
*        of the string given by `prefix` followed by 10 random characters. If
*        `prefix` is NULL, then the string "tmp." will be used instead. The file
*        will be created in an appropriate directory chosen by the first
*        successful attempt in the following sequence:
*        
*        a) The directory given by the `dir` argument (so the caller can specify
*        a secure directory to take precedence).
*        
*        b) The directory name in the environment variables:
*        
*          (i)   "TMP" [Windows only]   
*          (ii)  "TEMP" [Windows only]   
*          (iii) "TMPDIR" [Unix only]
*        
*        c) `P_tmpdir` as defined in <stdio.h> [Unix only] (in Windows, this is
*        usually "\", which is no good).
*        
*        d) The current working directory.
*        
*        If a file cannot be created in any of the above directories, then the
*        function fails and NULL is returned. 
*        
*        If the argument `pathname` is not a null pointer, then it will point to
*        the full pathname of the file. The pathname is allocated using `malloc`
*        and therefore should be freed by `free`.
*        
*        If `keep` is nonzero and `pathname` is not a null pointer, then the file
*        will be kept after it is closed. Otherwise the file will be
*        automatically deleted when it is closed or the program terminates. 
*
*
* RETURN VALUE
*        The tmpfileplus() function returns a pointer to the open file stream, 
*        or NULL if a unique file cannot be opened.
*
*
* ERRORS
*        ENOMEM Not enough memory to allocate filename.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* Non-ANSI include files that seem to work in both MSVC and Linux */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#endif

#ifdef _WIN32
/* MSVC nags to enforce ISO C++ conformant function names with leading "_",
 * so we define our own function names to avoid whingeing compilers...
 */
#define OPEN_ _open
#define FDOPEN_ _fdopen
#else
#define OPEN_ open
#define FDOPEN_ fdopen
#endif


/* DEBUGGING STUFF */
#if defined(_DEBUG) && defined(SHOW_DPRINTF)
#define DPRINTF1(s, a1) printf(s, a1)
#else
#define DPRINTF1(s, a1)
#endif


#ifdef _WIN32
#define FILE_SEPARATOR "\\"
#else
#define FILE_SEPARATOR "/"
#endif

#define RANDCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define NRANDCHARS  (sizeof(RANDCHARS) - 1)

/** Replace each byte in string s with a random character from TEMPCHARS */
static char *set_randpart(char *s)
{
  size_t i;
  unsigned int r;
  static unsigned int seed; /* NB static */

  if (seed == 0)
  { /* First time set our seed using current time and clock */
    seed = ((unsigned)time(NULL)<<16) ^ (unsigned)clock();
  }
  srand(seed++);  
  for (i = 0; i < strlen(s); i++)
  {
    r = rand() % NRANDCHARS;
    s[i] = (RANDCHARS)[r];
  }
  return s;
}

/** Return 1 if path is a valid directory otherwise 0 */
static int is_valid_dir(const char *path)
{
  struct stat st;
  if ((stat(path, &st) == 0) && (st.st_mode & S_IFDIR))
    return 1;
  
  return 0;
}

/** Call getenv and save a copy in buf */
static char *getenv_save(const char *varname, char *buf, size_t bufsize)
{
  char *ptr = getenv(varname);
  buf[0] = '\0';
  if (ptr)
  {
    strncpy(buf, ptr, bufsize);
    buf[bufsize-1] = '\0';
    return buf;
  }
  return NULL;
}

/** 
 * Try and create a randomly-named file in directory `tmpdir`. 
 * If successful, allocate memory and set `tmpname_ptr` to full filepath, and return file pointer;
 * otherwise return NULL.
 * If `keep` is zero then create the file as temporary and it should not exist once closed.
 */
static FILE *mktempfile_internal(const char *tmpdir, const char *pfx, const char* sfx, char **tmpname_ptr, int keep)
/* PRE: 
 * pfx is not NULL and points to a valid null-terminated string
 * tmpname_ptr is not NULL.
 */
{
  FILE *fp;
  int fd;
  char randpart[] = "1234567890";
  size_t lentempname;
  int i;
  char *tmpname = NULL;
  int oflag, pmode;

/* In Windows, we use the _O_TEMPORARY flag with `open` to ensure the file is deleted when closed.
 * In Unix, we use the unlink function after opening the file. (This does not work in Windows,
 * which does not allow an open file to be unlinked.)
 */
#ifdef _WIN32
  /* MSVC flags */
  oflag =  _O_BINARY|_O_CREAT|_O_EXCL|_O_RDWR;
  if (!keep)
    oflag |= _O_TEMPORARY;
  pmode = _S_IREAD | _S_IWRITE;
#else 
  /* Standard POSIX flags */
  oflag = O_CREAT|O_EXCL|O_RDWR;
  pmode = S_IRUSR|S_IWUSR;
#endif

  if (!tmpdir || !is_valid_dir(tmpdir))
    return NULL;

  lentempname = strlen(tmpdir) + strlen(FILE_SEPARATOR) + strlen(pfx) + strlen(sfx) + strlen(randpart);
  DPRINTF1("lentempname=%d\n", lentempname);
  tmpname = (char *)malloc(lentempname + 1);
  if (!tmpname)
  {
    errno = ENOMEM;
    return NULL;
  }
  /* If we don't manage to create a file after 10 goes, there is something wrong... */
  for (i = 0; i < 10; i++)
  {
    sprintf(tmpname, "%s%s%s%s%s", tmpdir, FILE_SEPARATOR, pfx, set_randpart(randpart), sfx);
    DPRINTF1("[%s]\n", tmpname);
    fd = OPEN_(tmpname, oflag, pmode);
    if (fd != -1) break;
  }
  DPRINTF1("strlen(tmpname)=%d\n", strlen(tmpname));
  if (fd != -1)
  { /* Success, so return user a proper ANSI C file pointer */
    fp = FDOPEN_(fd, "w+b");

#ifndef _WIN32
    /* [Unix only] And make sure the file will be deleted once closed */
    if (!keep) unlink(tmpname);
#endif

  }
  else
  { /* We failed */
    fp = NULL;
  }
  if (!fp)
  {
    free(tmpname);
    tmpname = NULL;
  }

  *tmpname_ptr = tmpname;
  return fp;
}

/*********************/
/* EXPORTED FUNCTION */
/*********************/

FILE *tmpfileplus(const char *dir, const char *prefix, const char* suffix, char **pathname, int keep)
{
  FILE *fp = NULL;
  char *tmpname = NULL;
  char *tmpdir = NULL;
  const char *pfx = (prefix ? prefix : "tmp.");
  const char *sfx = (suffix ? suffix : "");
  static char period[] = ".";
  char *tempdirs[12] = { 0 };
  char env1[FILENAME_MAX+1] = { 0 };
  char env2[FILENAME_MAX+1] = { 0 };
  char env3[FILENAME_MAX+1] = { 0 };
  int ntempdirs = 0;
  int i;

  /* Set up a list of temp directories we will try in order */
  i = 0;
  tempdirs[i++] = (char *)dir;
#ifdef _WIN32
  tempdirs[i++] = getenv_save("TMP", env1, sizeof(env1));
  tempdirs[i++] = getenv_save("TEMP", env2, sizeof(env2));
#else
  tempdirs[i++] = getenv_save("TMPDIR", env3, sizeof(env3));
  tempdirs[i++] = (char *)P_tmpdir;
#endif
  tempdirs[i++] = (char *)period;
  ntempdirs = i;

  errno = 0;

  /* Work through list we set up before, and break once we are successful */
  for (i = 0; i < ntempdirs; i++)
  {
    tmpdir = tempdirs[i];
    DPRINTF1("Trying tmpdir=[%s]\n", tmpdir);
    fp = mktempfile_internal(tmpdir, pfx, sfx, &tmpname, keep);
    if (fp) break;
  }
  /* If we succeeded and the user passed a pointer, set it to the alloc'd pathname: the user must free this */
  if (fp && pathname)
    *pathname = tmpname;
  else  /* Otherwise, free the alloc'd memory */
    free(tmpname);

  return fp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibMd5
//
//  Implementation of MD5 hash function. Originally written by Alexander Peslyak. Modified by WaterJuice retaining
//  Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <memory.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  F, G, H, I
//
//  The basic MD5 functions. F and G are optimized compared to their RFC 1321 definitions for architectures that lack
//  an AND-NOT instruction, just like in Colin Plumb's implementation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define F( x, y, z )            ( (z) ^ ((x) & ((y) ^ (z))) )
#define G( x, y, z )            ( (y) ^ ((z) & ((x) ^ (y))) )
#define H( x, y, z )            ( (x) ^ (y) ^ (z) )
#define I( x, y, z )            ( (y) ^ ((x) | ~(z)) )


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  STEP
//
//  The MD5 transformation for all four rounds.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STEP( f, a, b, c, d, x, t, s )                          \
    (a) += f((b), (c), (d)) + (x) + (t);                        \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));  \
    (a) += (b);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  SET, GET
//
//  SET reads 4 input bytes in little-endian byte order and stores them in a properly aligned word in host byte order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SET(n)      (*(uint32_t *)&ptr[(n) * 4])
#define GET(n)      SET(n)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  This processes one or more 64-byte data blocks, but does NOT update the bit counters. There are no alignment
//  requirements.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static
void*
    TransformFunctionMd5
    (
        Md5Context*     ctx,
        void*           data,
        uintmax_t       size
    )
{
    uint8_t*     ptr;
    uint32_t     a;
    uint32_t     b;
    uint32_t     c;
    uint32_t     d;
    uint32_t     saved_a;
    uint32_t     saved_b;
    uint32_t     saved_c;
    uint32_t     saved_d;

    ptr = (uint8_t*)data;

    a = ctx->a;
    b = ctx->b;
    c = ctx->c;
    d = ctx->d;

    do
    {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

        // Round 1
        STEP( F, a, b, c, d, SET(0),  0xd76aa478, 7 )
        STEP( F, d, a, b, c, SET(1),  0xe8c7b756, 12 )
        STEP( F, c, d, a, b, SET(2),  0x242070db, 17 )
        STEP( F, b, c, d, a, SET(3),  0xc1bdceee, 22 )
        STEP( F, a, b, c, d, SET(4),  0xf57c0faf, 7 )
        STEP( F, d, a, b, c, SET(5),  0x4787c62a, 12 )
        STEP( F, c, d, a, b, SET(6),  0xa8304613, 17 )
        STEP( F, b, c, d, a, SET(7),  0xfd469501, 22 )
        STEP( F, a, b, c, d, SET(8 ),  0x698098d8, 7 )
        STEP( F, d, a, b, c, SET(9 ),  0x8b44f7af, 12 )
        STEP( F, c, d, a, b, SET(10 ), 0xffff5bb1, 17 )
        STEP( F, b, c, d, a, SET(11 ), 0x895cd7be, 22 )
        STEP( F, a, b, c, d, SET(12 ), 0x6b901122, 7 )
        STEP( F, d, a, b, c, SET(13 ), 0xfd987193, 12 )
        STEP( F, c, d, a, b, SET(14 ), 0xa679438e, 17 )
        STEP( F, b, c, d, a, SET(15 ), 0x49b40821, 22 )

        // Round 2
        STEP( G, a, b, c, d, GET(1),  0xf61e2562, 5 )
        STEP( G, d, a, b, c, GET(6),  0xc040b340, 9 )
        STEP( G, c, d, a, b, GET(11), 0x265e5a51, 14 )
        STEP( G, b, c, d, a, GET(0),  0xe9b6c7aa, 20 )
        STEP( G, a, b, c, d, GET(5),  0xd62f105d, 5 )
        STEP( G, d, a, b, c, GET(10), 0x02441453, 9 )
        STEP( G, c, d, a, b, GET(15), 0xd8a1e681, 14 )
        STEP( G, b, c, d, a, GET(4),  0xe7d3fbc8, 20 )
        STEP( G, a, b, c, d, GET(9),  0x21e1cde6, 5 )
        STEP( G, d, a, b, c, GET(14), 0xc33707d6, 9 )
        STEP( G, c, d, a, b, GET(3),  0xf4d50d87, 14 )
        STEP( G, b, c, d, a, GET(8),  0x455a14ed, 20 )
        STEP( G, a, b, c, d, GET(13), 0xa9e3e905, 5 )
        STEP( G, d, a, b, c, GET(2),  0xfcefa3f8, 9 )
        STEP( G, c, d, a, b, GET(7),  0x676f02d9, 14 )
        STEP( G, b, c, d, a, GET(12), 0x8d2a4c8a, 20 )

        // Round 3
        STEP( H, a, b, c, d, GET(5),  0xfffa3942, 4 )
        STEP( H, d, a, b, c, GET(8),  0x8771f681, 11 )
        STEP( H, c, d, a, b, GET(11), 0x6d9d6122, 16 )
        STEP( H, b, c, d, a, GET(14), 0xfde5380c, 23 )
        STEP( H, a, b, c, d, GET(1),  0xa4beea44, 4 )
        STEP( H, d, a, b, c, GET(4),  0x4bdecfa9, 11 )
        STEP( H, c, d, a, b, GET(7),  0xf6bb4b60, 16 )
        STEP( H, b, c, d, a, GET(10), 0xbebfbc70, 23 )
        STEP( H, a, b, c, d, GET(13), 0x289b7ec6, 4 )
        STEP( H, d, a, b, c, GET(0),  0xeaa127fa, 11 )
        STEP( H, c, d, a, b, GET(3),  0xd4ef3085, 16 )
        STEP( H, b, c, d, a, GET(6),  0x04881d05, 23 )
        STEP( H, a, b, c, d, GET(9),  0xd9d4d039, 4 )
        STEP( H, d, a, b, c, GET(12), 0xe6db99e5, 11 )
        STEP( H, c, d, a, b, GET(15), 0x1fa27cf8, 16 )
        STEP( H, b, c, d, a, GET(2),  0xc4ac5665, 23 )

        // Round 4
        STEP( I, a, b, c, d, GET(0),  0xf4292244, 6 )
        STEP( I, d, a, b, c, GET(7),  0x432aff97, 10 )
        STEP( I, c, d, a, b, GET(14), 0xab9423a7, 15 )
        STEP( I, b, c, d, a, GET(5),  0xfc93a039, 21 )
        STEP( I, a, b, c, d, GET(12), 0x655b59c3, 6 )
        STEP( I, d, a, b, c, GET(3),  0x8f0ccc92, 10 )
        STEP( I, c, d, a, b, GET(10), 0xffeff47d, 15 )
        STEP( I, b, c, d, a, GET(1),  0x85845dd1, 21 )
        STEP( I, a, b, c, d, GET(8),  0x6fa87e4f, 6 )
        STEP( I, d, a, b, c, GET(15), 0xfe2ce6e0, 10 )
        STEP( I, c, d, a, b, GET(6),  0xa3014314, 15 )
        STEP( I, b, c, d, a, GET(13), 0x4e0811a1, 21 )
        STEP( I, a, b, c, d, GET(4),  0xf7537e82, 6 )
        STEP( I, d, a, b, c, GET(11), 0xbd3af235, 10 )
        STEP( I, c, d, a, b, GET(2),  0x2ad7d2bb, 15 )
        STEP( I, b, c, d, a, GET(9),  0xeb86d391, 21 )

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while( size -= 64 );

    ctx->a = a;
    ctx->b = b;
    ctx->c = c;
    ctx->d = d;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  EXPORTED FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Md5Initialise
//
//  Initialises an MD5 Context. Use this to initialise/reset a context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Md5Initialise
    (
        Md5Context*     Context
    )
{
    Context->a = 0x67452301;
    Context->b = 0xefcdab89;
    Context->c = 0x98badcfe;
    Context->d = 0x10325476;

    Context->lo = 0;
    Context->hi = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Md5Update
//
//  Adds data to the MD5 context. This will process the data and update the internal state of the context. Keep on
//  calling this function until all the data has been added. Then call Md5Finalise to calculate the hash.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Md5Update
    (
        Md5Context*         Context,
        void*               Buffer,
        uint32_t            BufferSize
    )
{
    uint32_t    saved_lo;
    uint32_t    used;
    uint32_t    free;

    saved_lo = Context->lo;
    if( (Context->lo = (saved_lo + BufferSize) & 0x1fffffff) < saved_lo )
    {
        Context->hi++;
    }
    Context->hi += (uint32_t)( BufferSize >> 29 );

    used = saved_lo & 0x3f;

    if( used )
    {
        free = 64 - used;

        if( BufferSize < free )
        {
            memcpy( &Context->buffer[used], Buffer, BufferSize );
            return;
        }

        memcpy( &Context->buffer[used], Buffer, free );
        Buffer = (uint8_t*)Buffer + free;
        BufferSize -= free;
        TransformFunctionMd5(Context, Context->buffer, 64);
    }

    if( BufferSize >= 64 )
    {
        Buffer = TransformFunctionMd5( Context, Buffer, BufferSize & ~(unsigned long)0x3f );
        BufferSize &= 0x3f;
    }

    memcpy( Context->buffer, Buffer, BufferSize );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Md5Finalise
//
//  Performs the final calculation of the hash and returns the digest (16 byte buffer containing 128bit hash). After
//  calling this, Md5Initialised must be used to reuse the context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Md5Finalise
    (
        Md5Context*         Context,
        MD5_HASH*           Digest
    )
{
    uint32_t    used;
    uint32_t    free;

    used = Context->lo & 0x3f;

    Context->buffer[used++] = 0x80;

    free = 64 - used;

    if(free < 8)
    {
        memset( &Context->buffer[used], 0, free );
        TransformFunctionMd5( Context, Context->buffer, 64 );
        used = 0;
        free = 64;
    }

    memset( &Context->buffer[used], 0, free - 8 );

    Context->lo <<= 3;
    Context->buffer[56] = (uint8_t)( Context->lo );
    Context->buffer[57] = (uint8_t)( Context->lo >> 8 );
    Context->buffer[58] = (uint8_t)( Context->lo >> 16 );
    Context->buffer[59] = (uint8_t)( Context->lo >> 24 );
    Context->buffer[60] = (uint8_t)( Context->hi );
    Context->buffer[61] = (uint8_t)( Context->hi >> 8 );
    Context->buffer[62] = (uint8_t)( Context->hi >> 16 );
    Context->buffer[63] = (uint8_t)( Context->hi >> 24 );

    TransformFunctionMd5( Context, Context->buffer, 64 );

    Digest->bytes[0]  = (uint8_t)( Context->a );
    Digest->bytes[1]  = (uint8_t)( Context->a >> 8 );
    Digest->bytes[2]  = (uint8_t)( Context->a >> 16 );
    Digest->bytes[3]  = (uint8_t)( Context->a >> 24 );
    Digest->bytes[4]  = (uint8_t)( Context->b );
    Digest->bytes[5]  = (uint8_t)( Context->b >> 8 );
    Digest->bytes[6]  = (uint8_t)( Context->b >> 16 );
    Digest->bytes[7]  = (uint8_t)( Context->b >> 24 );
    Digest->bytes[8]  = (uint8_t)( Context->c );
    Digest->bytes[9]  = (uint8_t)( Context->c >> 8 );
    Digest->bytes[10] = (uint8_t)( Context->c >> 16 );
    Digest->bytes[11] = (uint8_t)( Context->c >> 24 );
    Digest->bytes[12] = (uint8_t)( Context->d );
    Digest->bytes[13] = (uint8_t)( Context->d >> 8 );
    Digest->bytes[14] = (uint8_t)( Context->d >> 16 );
    Digest->bytes[15] = (uint8_t)( Context->d >> 24 );
}

/**
 * `levenshtein.c` - levenshtein
 *
 * MIT licensed.
 *
 * Copyright (c) 2015 Titus Wormer <tituswormer@gmail.com>
 */

/**
 * Returns an unsigned integer, depicting
 * the difference between `a` and `b`.
 *
 * See http://en.wikipedia.org/wiki/Levenshtein_distance
 * for more information.
 */

unsigned int
levenshtein(const char *a, const char *b) {
    unsigned int length = strlen(a);
    unsigned int bLength = strlen(b);
    unsigned int *cache = (unsigned int *) calloc(length, sizeof(unsigned int));
    unsigned int index = 0;
    unsigned int bIndex = 0;
    unsigned int distance;
    unsigned int bDistance;
    unsigned int result;
    char code;

    /*
     * Shortcut optimizations / degenerate cases.
     */

    if (a == b) {
        return 0;
    }

    if (length == 0) {
        return bLength;
    }

    if (bLength == 0) {
        return length;
    }

    /*
     * initialize the vector.
     */

    while (index < length) {
        cache[index] = index + 1;
        index++;
    }

    /*
     * Loop.
     */

    while (bIndex < bLength) {
        code = b[bIndex];
        result = distance = bIndex++;
        index = -1;

        while (++index < length) {
            bDistance = code == a[index] ? distance : distance + 1;
            distance = cache[index];

            cache[index] = result = distance > result
                ? bDistance > result
                    ? result + 1
                    : bDistance
                : bDistance > distance
                    ? distance + 1
                    : bDistance;
        }
    }

    free(cache);

    return result;
}

/*

bloom_filter

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software 
for any purpose with or without fee is hereby granted, provided 
that the above copyright notice and this permission notice appear 
in all copies. 

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE 
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN      
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 

 */

struct _BloomFilter {
	BloomFilterHashFunc hash_func;
	unsigned char *table;
	unsigned int table_size;
	unsigned int num_functions;
};

/* Salt values.  These salts are XORed with the output of the hash
 * function to give multiple unique hashes. */

static const unsigned int salts[] = {
	0x5cee4612, 0xb5587b1c, 0xa250f2b0, 0xa3bf6d2a, 
	0x7a81bd1a, 0x92888d7f, 0x1dc977c7, 0xedc96624, 
	0x920c85d9, 0xf16066b3, 0xc6f0d4b3, 0x2b76eb86, 
	0xcacb3893, 0x493d81c5, 0xf5a133ac, 0x039740bf, 
	0x162b8224, 0xf841de90, 0xc3e5090d, 0x3bce93a7, 
	0xf1860334, 0xe832b5f1, 0xf5b6535b, 0xe4cf4fa6, 
	0x8357b769, 0x1442b07a, 0x21c5863d, 0xabc0d846, 
	0x6dc0d77a, 0x23a3992c, 0xe12179ba, 0xd81d1e23, 
	0xcff4727b, 0xe957ecfb, 0xee8f391a, 0x426efa23, 
	0x3a34ff2c, 0x8b875d94, 0x34fd0f63, 0xf159daae, 
	0xaabab8b3, 0xa83a07ba, 0x4e54fb33, 0xfb82fab8, 
	0x2ae2888f, 0xd1a307a8, 0xbe33322d, 0x87c73f86, 
	0x7270fa7e, 0x68673c55, 0x2c8026d0, 0xead8e422, 
	0xa3ee5132, 0xecb67767, 0x1c3b1ae5, 0x47adf5b6, 
	0xf4518d30, 0x46e62797, 0x9889aa76, 0x1405aadf, 
	0xf62f9124, 0x5c435ac5, 0x35b8dfe3, 0x651c08c5, 
};

BloomFilter *bloom_filter_new(unsigned int table_size, 
                              BloomFilterHashFunc hash_func,
                              unsigned int num_functions)
{
	BloomFilter *filter;

	/* There is a limit on the number of functions which can be 
	 * applied, due to the table size */

	if (num_functions > sizeof(salts) / sizeof(*salts)) {
		return NULL;
	}

	/* Allocate bloom filter structure */

	filter = (BloomFilter *)malloc(sizeof(BloomFilter));

	if (filter == NULL) {
		return NULL;
	}

	/* Allocate table, each entry is one bit; these are packed into
	 * bytes.  When allocating we must round the length up to the nearest
	 * byte. */

	filter->table = (unsigned char *) calloc((table_size + 7) / 8, 1);

	if (filter->table == NULL) {
		free(filter);
		return NULL;
	}

	filter->hash_func = hash_func;
	filter->num_functions = num_functions;
	filter->table_size = table_size;

	return filter;
}

void bloom_filter_free(BloomFilter *bloomfilter)
{
	free(bloomfilter->table);
	free(bloomfilter);
}

void bloom_filter_insert(BloomFilter *bloomfilter, BloomFilterValue value)
{
	unsigned long hash;
	unsigned long subhash;
	unsigned int index;
	unsigned int i;

	/* Generate hash of the value to insert */

	hash = bloomfilter->hash_func(value);

	/* Generate multiple unique hashes by XORing with values in the
	 * salt table. */

	for (i=0; i<bloomfilter->num_functions; ++i) {

		/* Generate a unique hash */

		subhash = hash ^ salts[i];

		/* Find the index into the table */

		index = subhash % bloomfilter->table_size;

		/* Insert into the table.  
		 * index / 8 finds the byte index of the table,
		 * index % 8 gives the bit index within that byte to set. */

		bloomfilter->table[index / 8] |= 1 << (index % 8);
	}
}

int bloom_filter_query(BloomFilter *bloomfilter, BloomFilterValue value)
{
	unsigned long hash;
	unsigned long subhash;
	unsigned int index;
	unsigned int i;
	unsigned char b;
	int bit;

	/* Generate hash of the value to lookup */

	hash = bloomfilter->hash_func(value);

	/* Generate multiple unique hashes by XORing with values in the
	 * salt table. */

	for (i=0; i<bloomfilter->num_functions; ++i) {

		/* Generate a unique hash */

		subhash = hash ^ salts[i];

		/* Find the index into the table to test */

		index = subhash % bloomfilter->table_size;

		/* The byte at index / 8 holds the value to test */

		b = bloomfilter->table[index / 8];
		bit = 1 << (index % 8);

		/* Test if the particular bit is set; if it is not set,
		 * this value can not have been inserted. */

		if ((b & bit) == 0) {
			return 0;
		}
	}

	/* All necessary bits were set.  This may indicate that the value
	 * was inserted, or the values could have been set through other
	 * insertions. */

	return 1;
}

void bloom_filter_read(BloomFilter *bloomfilter, unsigned char *array)
{
	unsigned int array_size;

	/* The table is an array of bits, packed into bytes.  Round up
	 * to the nearest byte. */

	array_size = (bloomfilter->table_size + 7) / 8;

	/* Copy into the buffer of the calling routine. */

	memcpy(array, bloomfilter->table, array_size);
}

void bloom_filter_load(BloomFilter *bloomfilter, unsigned char *array)
{
	unsigned int array_size;

	/* The table is an array of bits, packed into bytes.  Round up
	 * to the nearest byte. */

	array_size = (bloomfilter->table_size + 7) / 8;

	/* Copy from the buffer of the calling routine. */

	memcpy(bloomfilter->table, array, array_size);
}

BloomFilter *bloom_filter_union(BloomFilter *filter1, BloomFilter *filter2)
{
	BloomFilter *result;
	unsigned int i;
	unsigned int array_size;

	/* To perform this operation, both filters must be created with
	 * the same values. */

	if (filter1->table_size != filter2->table_size
	 || filter1->num_functions != filter2->num_functions
	 || filter1->hash_func != filter2->hash_func) {
		return NULL;
	}

	/* Create a new bloom filter for the result */

	result = bloom_filter_new(filter1->table_size, 
	                          filter1->hash_func, 
	                          filter1->num_functions);

	if (result == NULL) {
		return NULL;
	}

	/* The table is an array of bits, packed into bytes.  Round up
	 * to the nearest byte. */

	array_size = (filter1->table_size + 7) / 8;

	/* Populate the table of the new filter */

	for (i=0; i<array_size; ++i) {
		result->table[i] = filter1->table[i] | filter2->table[i];
	}

	return result;
}

BloomFilter *bloom_filter_intersection(BloomFilter *filter1, 
                                       BloomFilter *filter2)
{
	BloomFilter *result;
	unsigned int i;
	unsigned int array_size;

	/* To perform this operation, both filters must be created with
	 * the same values. */

	if (filter1->table_size != filter2->table_size
	 || filter1->num_functions != filter2->num_functions
	 || filter1->hash_func != filter2->hash_func) {
		return NULL;
	}

	/* Create a new bloom filter for the result */

	result = bloom_filter_new(filter1->table_size, 
	                          filter1->hash_func, 
	                          filter1->num_functions);

	if (result == NULL) {
		return NULL;
	}

	/* The table is an array of bits, packed into bytes.  Round up
	 * to the nearest byte. */

	array_size = (filter1->table_size + 7) / 8;

	/* Populate the table of the new filter */

	for (i=0; i<array_size; ++i) {
		result->table[i] = filter1->table[i] & filter2->table[i];
	}

	return result;
}

// Spooky Hash
// A 128-bit noncryptographic hash, for checksums and table lookup
// By Bob Jenkins.  Public domain.
//   Oct 31 2010: published framework, disclaimer ShortHash isn't right
//   Nov 7 2010: disabled ShortHash
//   Oct 31 2011: replace End, ShortMix, ShortEnd, enable ShortHash again
//   April 10 2012: buffer overflow on platforms without unaligned reads
//   July 12 2012: was passing out variables in final to in/out in short
//   July 30 2012: I reintroduced the buffer overflow
//   August 5 2012: SpookyV2: d = should be d += in short hash, and remove extra mix from long hash

#define ALLOW_UNALIGNED_READS 1

//
// short hash ... it could be used on any message, 
// but it's used by Spooky just for short messages.
//
void SpookyHash::Short(
    const void *message,
    size_t length,
    uint64 *hash1,
    uint64 *hash2)
{
    uint64 buf[2*sc_numVars];
    union 
    { 
        const uint8 *p8; 
        uint32 *p32;
        uint64 *p64; 
        size_t i; 
    } u;

    u.p8 = (const uint8 *)message;
    
    if (!ALLOW_UNALIGNED_READS && (u.i & 0x7))
    {
        memcpy(buf, message, length);
        u.p64 = buf;
    }

    size_t remainder = length%32;
    uint64 a=*hash1;
    uint64 b=*hash2;
    uint64 c=sc_const;
    uint64 d=sc_const;

    if (length > 15)
    {
        const uint64 *end = u.p64 + (length/32)*4;
        
        // handle all complete sets of 32 bytes
        for (; u.p64 < end; u.p64 += 4)
        {
            c += u.p64[0];
            d += u.p64[1];
            ShortMix(a,b,c,d);
            a += u.p64[2];
            b += u.p64[3];
        }
        
        //Handle the case of 16+ remaining bytes.
        if (remainder >= 16)
        {
            c += u.p64[0];
            d += u.p64[1];
            ShortMix(a,b,c,d);
            u.p64 += 2;
            remainder -= 16;
        }
    }
    
    // Handle the last 0..15 bytes, and its length
    d += ((uint64)length) << 56;
    switch (remainder)
    {
    case 15:
    d += ((uint64)u.p8[14]) << 48;
    case 14:
        d += ((uint64)u.p8[13]) << 40;
    case 13:
        d += ((uint64)u.p8[12]) << 32;
    case 12:
        d += u.p32[2];
        c += u.p64[0];
        break;
    case 11:
        d += ((uint64)u.p8[10]) << 16;
    case 10:
        d += ((uint64)u.p8[9]) << 8;
    case 9:
        d += (uint64)u.p8[8];
    case 8:
        c += u.p64[0];
        break;
    case 7:
        c += ((uint64)u.p8[6]) << 48;
    case 6:
        c += ((uint64)u.p8[5]) << 40;
    case 5:
        c += ((uint64)u.p8[4]) << 32;
    case 4:
        c += u.p32[0];
        break;
    case 3:
        c += ((uint64)u.p8[2]) << 16;
    case 2:
        c += ((uint64)u.p8[1]) << 8;
    case 1:
        c += (uint64)u.p8[0];
        break;
    case 0:
        c += sc_const;
        d += sc_const;
    }
    ShortEnd(a,b,c,d);
    *hash1 = a;
    *hash2 = b;
}




// do the whole hash in one call
void SpookyHash::Hash128(
    const void *message, 
    size_t length, 
    uint64 *hash1, 
    uint64 *hash2)
{
    if (length < sc_bufSize)
    {
        Short(message, length, hash1, hash2);
        return;
    }

    uint64 h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11;
    uint64 buf[sc_numVars];
    uint64 *end;
    union 
    { 
        const uint8 *p8; 
        uint64 *p64; 
        size_t i; 
    } u;
    size_t remainder;
    
    h0=h3=h6=h9  = *hash1;
    h1=h4=h7=h10 = *hash2;
    h2=h5=h8=h11 = sc_const;
    
    u.p8 = (const uint8 *)message;
    end = u.p64 + (length/sc_blockSize)*sc_numVars;

    // handle all whole sc_blockSize blocks of bytes
    if (ALLOW_UNALIGNED_READS || ((u.i & 0x7) == 0))
    {
        while (u.p64 < end)
        { 
            Mix(u.p64, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
	    u.p64 += sc_numVars;
        }
    }
    else
    {
        while (u.p64 < end)
        {
            memcpy(buf, u.p64, sc_blockSize);
            Mix(buf, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
	    u.p64 += sc_numVars;
        }
    }

    // handle the last partial block of sc_blockSize bytes
    remainder = (length - ((const uint8 *)end-(const uint8 *)message));
    memcpy(buf, end, remainder);
    memset(((uint8 *)buf)+remainder, 0, sc_blockSize-remainder);
    ((uint8 *)buf)[sc_blockSize-1] = remainder;
    
    // do some final mixing 
    End(buf, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
    *hash1 = h0;
    *hash2 = h1;
}



// init spooky state
void SpookyHash::Init(uint64 seed1, uint64 seed2)
{
    m_length = 0;
    m_remainder = 0;
    m_state[0] = seed1;
    m_state[1] = seed2;
}


// add a message fragment to the state
void SpookyHash::Update(const void *message, size_t length)
{
    uint64 h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11;
    size_t newLength = length + m_remainder;
    uint8  remainder;
    union 
    { 
        const uint8 *p8; 
        uint64 *p64; 
        size_t i; 
    } u;
    const uint64 *end;
    
    // Is this message fragment too short?  If it is, stuff it away.
    if (newLength < sc_bufSize)
    {
        memcpy(&((uint8 *)m_data)[m_remainder], message, length);
        m_length = length + m_length;
        m_remainder = (uint8)newLength;
        return;
    }
    
    // init the variables
    if (m_length < sc_bufSize)
    {
        h0=h3=h6=h9  = m_state[0];
        h1=h4=h7=h10 = m_state[1];
        h2=h5=h8=h11 = sc_const;
    }
    else
    {
        h0 = m_state[0];
        h1 = m_state[1];
        h2 = m_state[2];
        h3 = m_state[3];
        h4 = m_state[4];
        h5 = m_state[5];
        h6 = m_state[6];
        h7 = m_state[7];
        h8 = m_state[8];
        h9 = m_state[9];
        h10 = m_state[10];
        h11 = m_state[11];
    }
    m_length = length + m_length;
    
    // if we've got anything stuffed away, use it now
    if (m_remainder)
    {
        uint8 prefix = sc_bufSize-m_remainder;
        memcpy(&(((uint8 *)m_data)[m_remainder]), message, prefix);
        u.p64 = m_data;
        Mix(u.p64, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        Mix(&u.p64[sc_numVars], h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        u.p8 = ((const uint8 *)message) + prefix;
        length -= prefix;
    }
    else
    {
        u.p8 = (const uint8 *)message;
    }
    
    // handle all whole blocks of sc_blockSize bytes
    end = u.p64 + (length/sc_blockSize)*sc_numVars;
    remainder = (uint8)(length-((const uint8 *)end-u.p8));
    if (ALLOW_UNALIGNED_READS || (u.i & 0x7) == 0)
    {
        while (u.p64 < end)
        { 
            Mix(u.p64, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
	    u.p64 += sc_numVars;
        }
    }
    else
    {
        while (u.p64 < end)
        { 
            memcpy(m_data, u.p8, sc_blockSize);
            Mix(m_data, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
	    u.p64 += sc_numVars;
        }
    }

    // stuff away the last few bytes
    m_remainder = remainder;
    memcpy(m_data, end, remainder);
    
    // stuff away the variables
    m_state[0] = h0;
    m_state[1] = h1;
    m_state[2] = h2;
    m_state[3] = h3;
    m_state[4] = h4;
    m_state[5] = h5;
    m_state[6] = h6;
    m_state[7] = h7;
    m_state[8] = h8;
    m_state[9] = h9;
    m_state[10] = h10;
    m_state[11] = h11;
}


// report the hash for the concatenation of all message fragments so far
void SpookyHash::Final(uint64 *hash1, uint64 *hash2)
{
    // init the variables
    if (m_length < sc_bufSize)
    {
        *hash1 = m_state[0];
        *hash2 = m_state[1];
        Short( m_data, m_length, hash1, hash2);
        return;
    }
    
    const uint64 *data = (const uint64 *)m_data;
    uint8 remainder = m_remainder;
    
    uint64 h0 = m_state[0];
    uint64 h1 = m_state[1];
    uint64 h2 = m_state[2];
    uint64 h3 = m_state[3];
    uint64 h4 = m_state[4];
    uint64 h5 = m_state[5];
    uint64 h6 = m_state[6];
    uint64 h7 = m_state[7];
    uint64 h8 = m_state[8];
    uint64 h9 = m_state[9];
    uint64 h10 = m_state[10];
    uint64 h11 = m_state[11];

    if (remainder >= sc_blockSize)
    {
        // m_data can contain two blocks; handle any whole first block
        Mix(data, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        data += sc_numVars;
        remainder -= sc_blockSize;
    }

    // mix in the last partial block, and the length mod sc_blockSize
    memset(&((uint8 *)data)[remainder], 0, (sc_blockSize-remainder));

    ((uint8 *)data)[sc_blockSize-1] = remainder;
    
    // do some final mixing
    End(data, h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);

    *hash1 = h0;
    *hash2 = h1;
}


/* id3.c

   ID3 reading and writing code
   By Matt Craven
   (c)2000 Hedgehog Software
*/

/* Define the ID3 genre strings */

const char *id3_genre_string[ID3_GENRE_MAX] = {
              "Blues", "Classic Rock", "Country",
              "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz",
              "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap",
              "Reggae", "Rock", "Techno", "Industrial", "Alternative",
              "Ska", "Death Metal", "Pranks", "Soundtrack", "Euro-Techno",
              "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion",
              "Trance", "Classical", "Instrumental", "Acid", "House", "Game",
              "Sound Clip", "Gospel", "Noise", "Alternative Rock", "Bass",
              "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
              "Instrumental Rock", "Ethnic", "Gothic", "Darkwave",
              "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance",
              "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40",
              "Christian Rap", "Pop/Funk", "Jungle", "Native US", "Cabaret",
              "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer",
              "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro",
              "Musical", "Rock & Roll", "Hard Rock",

              /* now the additional winamp ones */

              "Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion",
              "Bebop", "Latin", "Revival", "Celtic", "Bluegrass",
              "Avantgarde", "Gothic Rock", "Progressive Rock",
              "Psychadelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
              "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech",
              "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony",
              "Booty Bass", "Primus", "Porn Groove", "Satire",
              "Slow Jam", "Club", "Tabgo", "Samba", "Folklore",
              "Ballad", "Power Ballad", "Ryhtmic Soul", "Freestyle", "Duet",
              "Punk Rock", "Drum Solo","Acapella", "Euro-House",
              "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore",
              "Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk",
              "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal",
              "Crossover", "Contemporary Christian", "Christian Rock",
              "Merengue", "Salsa", "Trash Metal", "Anime", "Jpop",
              "Synthpop",

			/* some additional genres (CMS) */
			  "Library", "KPop", 
				};

/*===================================================================
  id3_read(filename, tag)

  Gets the ID3 tag from the file given in "filename", storing tag
  info in "tag".

  If there was an ID3 tag it returns 1, otherwise it returns 0
  =================================================================*/

int id3_read(char *filename, ID3_TAG *infotag)
{
    char inbuf[256];
    FILE *fp;      
    int tagflag;
    int x;

    /* Initialise the tag to empty */

    for(x=0;x<30;x++) infotag->title[x] = ' ';
    for(x=0;x<30;x++) infotag->artist[x] = ' ';
    for(x=0;x<30;x++) infotag->album[x] = ' ';
    for(x=0;x<4;x++) infotag->year[x] = ' ';
    for(x=0;x<29;x++) infotag->comment[x] = ' ';

    /* Open the file */

    fp = fopen(filename,"rb");
    if(fp==NULL)
    {
       /* Quit if there was a problem */

       printf("Error opening file: %s\n",filename);    
       exit(-1);
    }

    /* look for ID tag (128 bytes from the end of the file) */

    if(fseek(fp,-128,SEEK_END))
    {
      /* Return if file smaller than 128 bytes */

      fclose(fp);
      return(0);
    }

    /* Read the (potential) ID3 tag */
    fread(inbuf,sizeof(char),128,fp);

    inbuf[129]=0;
    fclose(fp);

    /* check there actually is a tag: */

    tagflag=0;
    if( (inbuf[0]=='T' || inbuf[0]=='t')
    &&  (inbuf[1]=='A' || inbuf[1]=='a')
    &&  (inbuf[2]=='G' || inbuf[2]=='g'))
      tagflag=1;

    if(!tagflag)
    {
      /* there was no tag, so return */

      return(0);
    }

    /* Put the tag info into the tag structure */

    for(x=0;x<30;x++) infotag->title[x] = inbuf[3+x];
    for(x=0;x<30;x++) infotag->artist[x] = inbuf[33+x];
    for(x=0;x<30;x++) infotag->album[x] = inbuf[63+x];
    for(x=0;x<4;x++) infotag->year[x] = inbuf[93+x];
    for(x=0;x<29;x++) infotag->comment[x] = inbuf[97+x];
    infotag->track = inbuf[126];
    infotag->genre = inbuf[127];

    /* null-terminate the strings (in case we want to print them later */

    infotag->title[30]=0;
    infotag->artist[30]=0;
    infotag->album[30]=0;
    infotag->year[4]=0;
    infotag->comment[29]=0;

    /* Return the fact that there was an ID3 tag */

    return 1;
}

/*===================================================================
  id3_tag_to_string(tag, txt)

  Converts the given ID3 "tag" to a 128-character string, storing it
  in "txt"
  =================================================================*/

void id3_tag_to_string(ID3_TAG tag, char *txt)
{
  int x;

  /* Initialise the string */

  for(x=0;x<128;x++)
     txt[x] = 32;

  /* TAG, title, artist, album, year, comment, track, genre */

  txt[0] = 'T'; txt[1] = 'A'; txt[2] = 'G';

  for(x=0;x<30;x++) txt[3+x] = tag.title[x];
  for(x=0;x<30;x++) txt[33+x] = tag.artist[x];
  for(x=0;x<30;x++) txt[63+x] = tag.album[x];
  for(x=0;x<4;x++) txt[93+x] = tag.year[x];
  for(x=0;x<29;x++) txt[97+x] = tag.comment[x];
  txt[126] = tag.track;
  txt[127] = tag.genre;
}

/*===================================================================
  id3_write(filename, tag)

  Writes the given tag onto the given file (replacing an existing
  tag if there was one)
  =================================================================*/

int id3_write(char *filename, ID3_TAG tag)
{
   int tagged;
   ID3_TAG dummy;
   FILE *fp;
   char id3_string[128];

   /* See if it's already tagged */

   tagged = id3_read(filename, &dummy);

   /* Open the file for binary append */

   fp=fopen(filename,"ab");

   /* Return if something went wrong */

   if(fp==NULL)
      return 0;

   /* If it's already tagged, seek back 128 bytes from end of file */

   if(tagged)
   {
     if(fseek(fp,-128,SEEK_END))
     {
       fclose(fp);
       return 0;
     }
   }

   /* convert the tag to a string */

   id3_tag_to_string(tag, id3_string);

   /* write the tag */

   fwrite(id3_string,sizeof(char),128,fp);

   fclose(fp);

   /* say we succeeded */

   return 1;
}

/* ilog.c */
/*(C) Timothy B. Terriberry (tterribe@xiph.org) 2001-2009 CC0 (Public domain).
 * See LICENSE file for details. */

/*The fastest fallback strategy for platforms with fast multiplication appears
   to be based on de Bruijn sequences~\cite{LP98}.
  Tests confirmed this to be true even on an ARM11, where it is actually faster
   than using the native clz instruction.
  Define ILOG_NODEBRUIJN to use a simpler fallback on platforms where
   multiplication or table lookups are too expensive.

  @UNPUBLISHED{LP98,
    author="Charles E. Leiserson and Harald Prokop",
    title="Using de {Bruijn} Sequences to Index a 1 in a Computer Word",
    month=Jun,
    year=1998,
    note="\url{http://supertech.csail.mit.edu/papers/debruijn.pdf}"
  }*/
static const unsigned char DEBRUIJN_IDX32[32]={
   0, 1,28, 2,29,14,24, 3,30,22,20,15,25,17, 4, 8,
  31,27,13,23,21,19,16, 7,26,12,18, 6,11, 5,10, 9
};

/* We always compile these in, in case someone takes address of function. */
#undef ilog32_nz
#undef ilog32
#undef ilog64_nz
#undef ilog64

int ilog32(uint32_t _v){
/*On a Pentium M, this branchless version tested as the fastest version without
   multiplications on 1,000,000,000 random 32-bit integers, edging out a
   similar version with branches, and a 256-entry LUT version.*/
# if defined(ILOG_NODEBRUIJN)
  int ret;
  int m;
  ret=_v>0;
  m=(_v>0xFFFFU)<<4;
  _v>>=m;
  ret|=m;
  m=(_v>0xFFU)<<3;
  _v>>=m;
  ret|=m;
  m=(_v>0xFU)<<2;
  _v>>=m;
  ret|=m;
  m=(_v>3)<<1;
  _v>>=m;
  ret|=m;
  ret+=_v>1;
  return ret;
/*This de Bruijn sequence version is faster if you have a fast multiplier.*/
# else
  int ret;
  ret=_v>0;
  _v|=_v>>1;
  _v|=_v>>2;
  _v|=_v>>4;
  _v|=_v>>8;
  _v|=_v>>16;
  _v=(_v>>1)+1;
  ret+=DEBRUIJN_IDX32[_v*0x77CB531U>>27&0x1F];
  return ret;
# endif
}

int ilog32_nz(uint32_t _v) {
  return ilog32(_v);
}

int ilog64(uint64_t _v){
# if defined(ILOG_NODEBRUIJN)
  uint32_t v;
  int      ret;
  int      m;
  ret=_v>0;
  m=(_v>0xFFFFFFFFU)<<5;
  v=(uint32_t)(_v>>m);
  ret|=m;
  m=(v>0xFFFFU)<<4;
  v>>=m;
  ret|=m;
  m=(v>0xFFU)<<3;
  v>>=m;
  ret|=m;
  m=(v>0xFU)<<2;
  v>>=m;
  ret|=m;
  m=(v>3)<<1;
  v>>=m;
  ret|=m;
  ret+=v>1;
  return ret;
# else
/*If we don't have a 64-bit word, split it into two 32-bit halves.*/
#  if LONG_MAX<9223372036854775807LL
  uint32_t v;
  int      ret;
  int      m;
  ret=_v>0;
  m=(_v>0xFFFFFFFFU)<<5;
  v=(uint32_t)(_v>>m);
  ret|=m;
  v|=v>>1;
  v|=v>>2;
  v|=v>>4;
  v|=v>>8;
  v|=v>>16;
  v=(v>>1)+1;
  ret+=DEBRUIJN_IDX32[v*0x77CB531U>>27&0x1F];
  return ret;
/*Otherwise do it in one 64-bit operation.*/
#  else
  static const unsigned char DEBRUIJN_IDX64[64]={
     0, 1, 2, 7, 3,13, 8,19, 4,25,14,28, 9,34,20,40,
     5,17,26,38,15,46,29,48,10,31,35,54,21,50,41,57,
    63, 6,12,18,24,27,33,39,16,37,45,47,30,53,49,56,
    62,11,23,32,36,44,52,55,61,22,43,51,60,42,59,58
  };
  int ret;
  ret=_v>0;
  _v|=_v>>1;
  _v|=_v>>2;
  _v|=_v>>4;
  _v|=_v>>8;
  _v|=_v>>16;
  _v|=_v>>32;
  _v=(_v>>1)+1;
  ret+=DEBRUIJN_IDX64[_v*0x218A392CD3D5DBF>>58&0x3F];
  return ret;
#  endif
# endif
}

int ilog64_nz(uint64_t _v) {
  return ilog64(_v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha1
//
//  Implementation of SHA1 hash function.
//  Original author:  Steve Reid <sreid@sea-to-sky.net>
//  Contributions by: James H. Brown <jbrown@burgoyne.com>, Saul Kravitz <Saul.Kravitz@celera.com>,
//  and Ralph Giles <giles@ghostscript.com>
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef union {
    uint8_t     c [64];
    uint32_t    l [16];
} CHAR64LONG16;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

// blk0() and blk() perform the initial expand.
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

// (R0+R1), R2, R3, R4 are the different operations used in SHA1
#define R0(v,w,x,y,z,i)  z += ((w&(x^y))^y)     + blk0(i)+ 0x5A827999 + rol(v,5); w=rol(w,30);
#define R1(v,w,x,y,z,i)  z += ((w&(x^y))^y)     + blk(i) + 0x5A827999 + rol(v,5); w=rol(w,30);
#define R2(v,w,x,y,z,i)  z += (w^x^y)           + blk(i) + 0x6ED9EBA1 + rol(v,5); w=rol(w,30);
#define R3(v,w,x,y,z,i)  z += (((w|x)&y)|(w&x)) + blk(i) + 0x8F1BBCDC + rol(v,5); w=rol(w,30);
#define R4(v,w,x,y,z,i)  z += (w^x^y)           + blk(i) + 0xCA62C1D6 + rol(v,5); w=rol(w,30);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  Hash a single 512-bit block. This is the core of the algorithm
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static
void
    TransformFunctionSha1
    (
        uint32_t            state[5],
        const uint8_t       buffer[64]
    )
{
    uint32_t            a;
    uint32_t            b;
    uint32_t            c;
    uint32_t            d;
    uint32_t            e;
    uint8_t             workspace[64];
    CHAR64LONG16*       block = (CHAR64LONG16*) workspace;

    memcpy( block, buffer, 64 );

    // Copy context->state[] to working vars
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    // 4 rounds of 20 operations each. Loop unrolled.
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    // Add the working vars back into context.state[]
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha1Initialise
//
//  Initialises an SHA1 Context. Use this to initialise/reset a context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha1Initialise
    (
        Sha1Context*                Context
    )
{
    // SHA1 initialization constants
    Context->State[0] = 0x67452301;
    Context->State[1] = 0xEFCDAB89;
    Context->State[2] = 0x98BADCFE;
    Context->State[3] = 0x10325476;
    Context->State[4] = 0xC3D2E1F0;
    Context->Count[0] = 0;
    Context->Count[1] = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha1Update
//
//  Adds data to the SHA1 context. This will process the data and update the internal state of the context. Keep on
//  calling this function until all the data has been added. Then call Sha1Finalise to calculate the hash.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha1Update
    (
        Sha1Context*        Context,
        void*               Buffer,
        uint32_t            BufferSize
    )
{
    uint32_t    i;
    uint32_t    j;

    j = (Context->Count[0] >> 3) & 63;
    if( (Context->Count[0] += BufferSize << 3) < (BufferSize << 3) )
    {
        Context->Count[1]++;
    }

    Context->Count[1] += (BufferSize >> 29);
    if( (j + BufferSize) > 63 )
    {
        i = 64 - j;
        memcpy( &Context->Buffer[j], Buffer, i );
        TransformFunctionSha1(Context->State, Context->Buffer);
        for( ; i + 63 < BufferSize; i += 64 )
        {
            TransformFunctionSha1(Context->State, (uint8_t*)Buffer + i);
        }
        j = 0;
    }
    else
    {
        i = 0;
    }

    memcpy( &Context->Buffer[j], &((uint8_t*)Buffer)[i], BufferSize - i );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha1Finalise
//
//  Performs the final calculation of the hash and returns the digest (20 byte buffer containing 160bit hash). After
//  calling this, Sha1Initialised must be used to reuse the context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha1Finalise
    (
        Sha1Context*                Context,
        SHA1_HASH*                  Digest
    )
{
    uint32_t    i;
    uint8_t     finalcount[8];

    for( i=0; i<8; i++ )
    {
        finalcount[i] = (unsigned char)((Context->Count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  // Endian independent
    }
    Sha1Update( Context, (uint8_t*)"\x80", 1 );
    while( (Context->Count[0] & 504) != 448 )
    {
        Sha1Update( Context, (uint8_t*)"\0", 1 );
    }

    Sha1Update( Context, finalcount, 8 );  // Should cause a Sha1TransformFunction()
    for( i=0; i<SHA1_HASH_SIZE; i++ )
    {
        Digest->bytes[i] = (uint8_t)((Context->State[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha256
//
//  Implementation of SHA256 hash function.
//  Original author: Tom St Denis, tomstdenis@gmail.com, http://libtom.org
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ror(value, bits) (((value) >> (bits)) | ((value) << (32 - (bits))))

#ifndef MIN
#define MIN(x, y) ( ((x)<(y))?(x):(y) )
#endif

#define STORE32H(x, y)                                                                     \
     { (y)[0] = (uint8_t)(((x)>>24)&255); (y)[1] = (uint8_t)(((x)>>16)&255);   \
       (y)[2] = (uint8_t)(((x)>>8)&255); (y)[3] = (uint8_t)((x)&255); }

#define LOAD32H(x, y)                            \
     { x = ((uint32_t)((y)[0] & 255)<<24) | \
           ((uint32_t)((y)[1] & 255)<<16) | \
           ((uint32_t)((y)[2] & 255)<<8)  | \
           ((uint32_t)((y)[3] & 255)); }

#define STORE64H(x, y)                                                                     \
   { (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);     \
     (y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);     \
     (y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);     \
     (y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The K array
static const uint32_t K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
    0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
    0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
    0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
    0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
    0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
    0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
    0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
    0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

#define BLOCK_SIZE          64

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Various logical functions
#define Ch( x, y, z )     (z ^ (x & (y ^ z)))
#define Maj( x, y, z )    (((x | y) & z) | (x & y))
#define S( x, n )         ror((x),(n))
#define R( x, n )         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0( x )       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1( x )       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0( x )       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1( x )       (S(x, 17) ^ S(x, 19) ^ R(x, 10))

#define Sha256Round( a, b, c, d, e, f, g, h, i )       \
     t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];   \
     t1 = Sigma0(a) + Maj(a, b, c);                    \
     d += t0;                                          \
     h  = t0 + t1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  Compress 512-bits
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static
void
    TransformFunctionSha256
    (
        Sha256Context*      Context,
        uint8_t*            Buffer
    )
{
    uint32_t    S[8];
    uint32_t    W[64];
    uint32_t    t0;
    uint32_t    t1;
    uint32_t    t;
    int         i;

    // Copy state into S
    for( i=0; i<8; i++ )
    {
        S[i] = Context->state[i];
    }

    // Copy the state into 512-bits into W[0..15]
    for( i=0; i<16; i++ )
    {
        LOAD32H( W[i], Buffer + (4*i) );
    }

    // Fill W[16..63]
    for( i=16; i<64; i++ )
    {
        W[i] = Gamma1( W[i-2]) + W[i-7] + Gamma0( W[i-15] ) + W[i-16];
    }

    // Compress
    for( i=0; i<64; i++ )
    {
        Sha256Round( S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], i );
        t = S[7];
        S[7] = S[6];
        S[6] = S[5];
        S[5] = S[4];
        S[4] = S[3];
        S[3] = S[2];
        S[2] = S[1];
        S[1] = S[0];
        S[0] = t;
    }

    // Feedback
    for( i=0; i<8; i++ )
    {
        Context->state[i] = Context->state[i] + S[i];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha256Initialise
//
//  Initialises a SHA256 Context. Use this to initialise/reset a context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sha256Initialise
    (
        Sha256Context*          Context
    )
{
    Context->curlen = 0;
    Context->length = 0;
    Context->state[0] = 0x6A09E667UL;
    Context->state[1] = 0xBB67AE85UL;
    Context->state[2] = 0x3C6EF372UL;
    Context->state[3] = 0xA54FF53AUL;
    Context->state[4] = 0x510E527FUL;
    Context->state[5] = 0x9B05688CUL;
    Context->state[6] = 0x1F83D9ABUL;
    Context->state[7] = 0x5BE0CD19UL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha256Update
//
//  Adds data to the SHA256 context. This will process the data and update the internal state of the context. Keep on
//  calling this function until all the data has been added. Then call Sha256Finalise to calculate the hash.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sha256Update
    (
        Sha256Context*      Context,
        void*               Buffer,
        uint32_t            BufferSize
    )
{
    uint32_t    n
        ;
    if( Context->curlen > sizeof(Context->buf) )
    {
       return;
    }

    while( BufferSize > 0 )
    {
        if( Context->curlen == 0 && BufferSize >= BLOCK_SIZE )
        {
           TransformFunctionSha256( Context, (uint8_t*)Buffer );
           Context->length += BLOCK_SIZE * 8;
           Buffer = (uint8_t*)Buffer + BLOCK_SIZE;
           BufferSize -= BLOCK_SIZE;
        }
        else
        {
           n = MIN( BufferSize, (BLOCK_SIZE - Context->curlen) );
           memcpy( Context->buf + Context->curlen, Buffer, (size_t)n );
           Context->curlen += n;
           Buffer = (uint8_t*)Buffer + n;
           BufferSize -= n;
           if( Context->curlen == BLOCK_SIZE )
           {
              TransformFunctionSha256( Context, Context->buf );
              Context->length += 8*BLOCK_SIZE;
              Context->curlen = 0;
           }
       }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha256Finalise
//
//  Performs the final calculation of the hash and returns the digest (32 byte buffer containing 256bit hash). After
//  calling this, Sha256Initialised must be used to reuse the context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha256Finalise
    (
        Sha256Context*          Context,
        SHA256_HASH*            Digest
    )
{
    int i;

    if( Context->curlen >= sizeof(Context->buf) )
    {
       return;
    }

    // Increase the length of the message
    Context->length += Context->curlen * 8;

    // Append the '1' bit
    Context->buf[Context->curlen++] = (uint8_t)0x80;

    // if the length is currently above 56 bytes we append zeros
    // then compress.  Then we can fall back to padding zeros and length
    // encoding like normal.
    if( Context->curlen > 56 )
    {
        while( Context->curlen < 64 )
        {
            Context->buf[Context->curlen++] = (uint8_t)0;
        }
        TransformFunctionSha256(Context, Context->buf);
        Context->curlen = 0;
    }

    // Pad up to 56 bytes of zeroes
    while( Context->curlen < 56 )
    {
        Context->buf[Context->curlen++] = (uint8_t)0;
    }

    // Store length
    STORE64H( Context->length, Context->buf+56 );
    TransformFunctionSha256( Context, Context->buf );

    // Copy output
    for( i=0; i<8; i++ )
    {
        STORE32H( Context->state[i], Digest->bytes+(4*i) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha512
//
//  Implementation of SHA512 hash function.
//  Original author: Tom St Denis, tomstdenis@gmail.com, http://libtom.org
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ROR64( value, bits ) (((value) >> (bits)) | ((value) << (64 - (bits))))

#define LOAD64H( x, y )                                                      \
   { x = (((uint64_t)((y)[0] & 255))<<56)|(((uint64_t)((y)[1] & 255))<<48) | \
         (((uint64_t)((y)[2] & 255))<<40)|(((uint64_t)((y)[3] & 255))<<32) | \
         (((uint64_t)((y)[4] & 255))<<24)|(((uint64_t)((y)[5] & 255))<<16) | \
         (((uint64_t)((y)[6] & 255))<<8)|(((uint64_t)((y)[7] & 255))); }

#define STORE64H( x, y )                                                                     \
   { (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);     \
     (y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);     \
     (y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);     \
     (y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The K array
static const uint64_t KK[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

#undef  BLOCK_SIZE
#define BLOCK_SIZE          128

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef Ch
#undef Maj
#undef S
#undef R
#undef Sigma0
#undef Sigma1
#undef Gamma0
#undef Gamma1

// Various logical functions
#define Ch( x, y, z )     (z ^ (x & (y ^ z)))
#define Maj(x, y, z )     (((x | y) & z) | (x & y))
#define S( x, n )         ROR64( x, n )
#define R( x, n )         (((x)&0xFFFFFFFFFFFFFFFFULL)>>((uint64_t)n))
#define Sigma0( x )       (S(x, 28) ^ S(x, 34) ^ S(x, 39))
#define Sigma1( x )       (S(x, 14) ^ S(x, 18) ^ S(x, 41))
#define Gamma0( x )       (S(x, 1) ^ S(x, 8) ^ R(x, 7))
#define Gamma1( x )       (S(x, 19) ^ S(x, 61) ^ R(x, 6))

#define Sha512Round( a, b, c, d, e, f, g, h, i )       \
     t0 = h + Sigma1(e) + Ch(e, f, g) + KK[i] + W[i];   \
     t1 = Sigma0(a) + Maj(a, b, c);                    \
     d += t0;                                          \
     h  = t0 + t1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  Compress 1024-bits
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static
void
    TransformFunctionSha512
    (
        Sha512Context*          Context,
        uint8_t*                Buffer
    )
{
    uint64_t    S[8];
    uint64_t    W[80];
    uint64_t    t0;
    uint64_t    t1;
    int         i;

    // Copy state into S
    for( i=0; i<8; i++ )
    {
        S[i] = Context->state[i];
    }

    // Copy the state into 1024-bits into W[0..15]
    for( i=0; i<16; i++ )
    {
        LOAD64H(W[i], Buffer + (8*i));
    }

    // Fill W[16..79]
    for( i=16; i<80; i++ )
    {
        W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
    }

    // Compress
     for( i=0; i<80; i+=8 )
     {
         Sha512Round(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i+0);
         Sha512Round(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i+1);
         Sha512Round(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i+2);
         Sha512Round(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i+3);
         Sha512Round(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i+4);
         Sha512Round(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i+5);
         Sha512Round(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i+6);
         Sha512Round(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i+7);
     }

    // Feedback
    for( i=0; i<8; i++ )
    {
        Context->state[i] = Context->state[i] + S[i];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha512Initialise
//
//  Initialises a SHA512 Context. Use this to initialise/reset a context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void 
    Sha512Initialise
    (
        Sha512Context*          Context
    )
{
    Context->curlen = 0;
    Context->length = 0;
    Context->state[0] = 0x6a09e667f3bcc908ULL;
    Context->state[1] = 0xbb67ae8584caa73bULL;
    Context->state[2] = 0x3c6ef372fe94f82bULL;
    Context->state[3] = 0xa54ff53a5f1d36f1ULL;
    Context->state[4] = 0x510e527fade682d1ULL;
    Context->state[5] = 0x9b05688c2b3e6c1fULL;
    Context->state[6] = 0x1f83d9abfb41bd6bULL;
    Context->state[7] = 0x5be0cd19137e2179ULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha512Update
//
//  Adds data to the SHA512 context. This will process the data and update the internal state of the context. Keep on
//  calling this function until all the data has been added. Then call Sha512Finalise to calculate the hash.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha512Update
    (
        Sha512Context*      Context,
        void*               Buffer,
        uint32_t            BufferSize
    )
{
    uint32_t    n;

    if( Context->curlen > sizeof(Context->buf) )
    {
       return;
    }

    while( BufferSize > 0 )
    {
        if( Context->curlen == 0 && BufferSize >= BLOCK_SIZE )
        {
           TransformFunctionSha512( Context, (uint8_t *)Buffer );
           Context->length += BLOCK_SIZE * 8;
           Buffer = (uint8_t*)Buffer + BLOCK_SIZE;
           BufferSize -= BLOCK_SIZE;
        }
        else
        {
           n = MIN( BufferSize, (BLOCK_SIZE - Context->curlen) );
           memcpy( Context->buf + Context->curlen, Buffer, (size_t)n );
           Context->curlen += n;
           Buffer = (uint8_t*)Buffer + n;
           BufferSize -= n;
           if( Context->curlen == BLOCK_SIZE )
           {
              TransformFunctionSha512( Context, Context->buf );
              Context->length += 8*BLOCK_SIZE;
              Context->curlen = 0;
           }
       }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha512Finalise
//
//  Performs the final calculation of the hash and returns the digest (64 byte buffer containing 512bit hash). After
//  calling this, Sha512Initialised must be used to reuse the context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
    Sha512Finalise
    (
        Sha512Context*          Context,
        SHA512_HASH*            Digest
    )
{
    int i;

    if( Context->curlen >= sizeof(Context->buf) )
    {
       return;
    }

    // Increase the length of the message
    Context->length += Context->curlen * 8ULL;

    // Append the '1' bit
    Context->buf[Context->curlen++] = (uint8_t)0x80;

    // If the length is currently above 112 bytes we append zeros
    // then compress.  Then we can fall back to padding zeros and length
    // encoding like normal.
    if( Context->curlen > 112 )
    {
        while( Context->curlen < 128 )
        {
            Context->buf[Context->curlen++] = (uint8_t)0;
        }
        TransformFunctionSha512( Context, Context->buf );
        Context->curlen = 0;
    }

    // Pad up to 120 bytes of zeroes
    // note: that from 112 to 120 is the 64 MSB of the length.  We assume that you won't hash
    // > 2^64 bits of data... :-)
    while( Context->curlen < 120 )
    {
        Context->buf[Context->curlen++] = (uint8_t)0;
    }

    // Store length
    STORE64H( Context->length, Context->buf+120 );
    TransformFunctionSha512( Context, Context->buf );

    // Copy output
    for( i=0; i<8; i++ ) 
    {
        STORE64H( Context->state[i], Digest->bytes+(8*i) );
    }
}

#undef BLOCK_SIZE
#undef Ch
#undef Maj
#undef S
#undef R
#undef Sigma0
#undef Sigma1
#undef Gamma0
#undef Gamma1

/*
 * Copyright (C) 2011 Joseph Adams <joeyadams3.14159@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned char *cur;    /* End of string; insertion point for new bytes */
	unsigned char *end;    /* End of buffer */
	unsigned char *start;  /* Beginning of string */
} SB;

/* sb is evaluated multiple times in these macros. */
#define sb_size(sb)  ((size_t)((sb)->cur - (sb)->start))
#define sb_avail(sb) ((size_t)((sb)->end - (sb)->cur))

/* sb and need may be evaluated multiple times. */
#define sb_need(sb, need) do {           \
		if (sb_avail(sb) < (need))       \
			if (sb_grow(sb, need) != 0)  \
				goto out_of_memory;      \
	} while (0)

static int sb_init(SB *sb) {
	sb->start = (unsigned char*)malloc(17);
	if (sb->start == NULL)
		return -1;
	sb->cur = sb->start;
	sb->end = sb->start + 16;
	return 0;
}

static int sb_grow(SB *sb, size_t need) {
	size_t length = sb->cur - sb->start;
	size_t alloc = sb->end - sb->start;
	unsigned char *tmp;
	
	do {
		alloc *= 2;
	} while (alloc < length + need);
	
	tmp = (unsigned char*)realloc(sb->start, alloc + 1);
	if (tmp == NULL)
		return -1;
	sb->start = tmp;
	sb->cur = tmp + length;
	sb->end = tmp + alloc;
	return 0;
}

static int sb_putc(SB *sb, unsigned char c) {
	sb_need(sb, 1);
	*sb->cur++ = c;
	return 0;

out_of_memory:
	return -1;
}

static int sb_write(SB *sb, const void *data, size_t size) {
	sb_need(sb, size);
	memcpy(sb->cur, data, size);
	sb->cur += size;
	return 0;
	
out_of_memory:
	return -1;
}

static void sb_return(SB *sb, void **data_out, size_t *length_out) {
	*sb->cur = 0;
	if (data_out)
		*data_out = sb->start;
	else
		free(sb->start);
	if (length_out)
		*length_out = sb->cur - sb->start;
}

static void sb_discard(SB *sb, void **data_out, size_t *length_out) {
	free(sb->start);
	if (data_out)
		*data_out = NULL;
	if (length_out)
		*length_out = 0;
}

/*
 * The first byte in a patch is the "patch type", which indicates how the
 * patch is formatted.  This keeps the patch format flexible while retaining
 * backward compatibility.  Patches produced with an older version of
 * the library can be applied with a newer version.
 *
 * PT_LITERAL
 *     Contains nothing more than the content of the new text.
 *
 * PT_CSI32
 *     A string of copy, skip, and insert instructions for generating the new
 *     string from the old.
 *
 *         copy(size):   Copy @size bytes from old to new.
 *         skip(size):   Skip @size bytes of old.
 *         insert(text): Insert @text into new.
 *
 *     The syntax is as follows:
 *
 *         copy:   instruction_byte(1) size
 *         skip:   instruction_byte(2) size
 *         insert: instruction_byte(3) size $size*OCTET
 *
 *         0 <= size_param_length <= 4
 *         instruction_byte(op) = op | size_param_length << 2
 *         size: $size_param_length*OCTET
 *               -- size is an unsigned integer encoded in big endian.
 *               -- However, if size_param_length is 0, the operation size is 1.
 *
 *     Simply put, an instruction starts with an opcode and operation size.
 *     An insert instruction is followed by the bytes to be inserted.
 */
#define PT_LITERAL   10
#define PT_CSI32     11

#define OP_COPY      1
#define OP_SKIP      2
#define OP_INSERT    3

static unsigned int bytes_needed_for_size(uint32_t size) {
	if (size == 1)
		return 0;
	else if (size <= 0xFF)
		return 1;
	else if (size <= 0xFFFF)
		return 2;
	else if (size <= 0xFFFFFF)
		return 3;
	else
		return 4;
}

/*
 * Return values:
 *
 *  BDELTA_OK:      Success
 *  BDELTA_MEMORY:  Memory allocation failed
 */
static BDELTAcode csi32_emit_op(SB *patch_out, int op, uint32_t size, const char **new_) {
	unsigned int i;
	unsigned int size_param_length;
	size_t need;
	uint32_t tmp;
	
	assert(op >= 1 && op <= 3);
	
	if (size == 0)
		return BDELTA_OK;
	size_param_length = bytes_needed_for_size(size);
	
	need = 1 + size_param_length;
	if (op == OP_INSERT)
		need += size;
	sb_need(patch_out, need);
	
	*patch_out->cur++ = (unsigned int)op | size_param_length << 2;
	for (i = size_param_length, tmp = size; i-- > 0; tmp >>= 8)
		patch_out->cur[i] = tmp & 0xFF;
	patch_out->cur += size_param_length;
	
	switch (op) {
		case OP_COPY:
			*new_ += size;
			break;
		case OP_SKIP:
			break;
		case OP_INSERT:
			memcpy(patch_out->cur, *new_, size);
			patch_out->cur += size;
			*new_ += size;
			break;
		default:
			assert(0);
	}
	
	return BDELTA_OK;

out_of_memory:
	return BDELTA_MEMORY;
}

/*
 * On success, returns 1, advances *sp past the parsed text, and sets *op_out and *size_out.
 * On error or EOF, returns 0.
 */
static int csi32_parse_op(
	const unsigned char **sp, const unsigned char *e,
	int *op_out, uint32_t *size_out) {
	const unsigned char *s = *sp;
	int op;
	unsigned int i;
	unsigned int size_param_length;
	uint32_t size;
	
	if (s >= e)
		return 0;
	op = *s & 3;
	size_param_length = *s >> 2;
	s++;
	if (op == 0 || size_param_length > 4)
		return 0;
	
	if (size_param_length == 0) {
		size = 1;
	} else {
		if ((size_t)(e - s) < size_param_length)
			return 0;
		size = 0;
		for (i = 0; i < size_param_length; i++) {
			size <<= 8;
			size |= *s++ & 0xFF;
		}
	}
	
	/* Make sure insert data fits in the patch, but don't consume it. */
	if (op == OP_INSERT && (size_t)(e - s) < size)
		return 0;
	
	*op_out = op;
	*size_out = size;
	*sp = s;
	return 1;
}

/*
 * bdelta uses the algorithm described in:
 *
 *     Myers, E. (1986). An O(ND) Difference Algorithm and Its Variations.
 *     Retrieved from http://www.xmailserver.org/diff2.pdf
 *
 * The pseudocode in Myers' paper (Figure 2) uses an array called V,
 * where (V[k], V[k] - k) is the endpoint of the furthest-reaching
 * D-path ending on diagonal k.
 *
 * The structure below holds the V array for every iteration of the outer loop.
 * Because each iteration produces D+1 values, a triangle is formed:
 *
 *                      k
 *        -5 -4 -3 -2 -1  0  1  2  3  4  5
 *      ----------------------------------
 *    0 |                 0                (copy 0)
 *      |                   \              skip 1
 *    1 |              0     1
 *      |                      \           skip 1, then copy 1
 *    2 |           2     2     3
 *  D   |                      /           insert 1, then copy 2
 *    3 |        3     4     5     5
 *      |                     \            skip 1, then copy 1
 *    4 |     3     4     5     7     7
 *      |                      /           insert 1
 *    5 |  3     4     5     7     -     -
 *
 * @data will literally contain: 0 0 1 2 2 3 3 4 5 5 3 4 5 7 7 3 4 5 7
 *
 * To convert this to an edit script, we first climb back to the top,
 * using the same procedure as was used when the triangle was generated:
 *
 *     If k = -D, climb right (the only way we can go).
 *     If k = +D, climb left  (the only way we can go).
 *     Otherwise, favor the greater number.
 *     If the numbers are the same, climb left.
 *
 * Finally, we convert the descent to the solution to a patch script:
 *
 *     The top number n corresponds to:
 *         copy   n
 *
 *     A descent left from a to b corresponds to:
 *         insert 1
 *         copy   b-a
 *
 *     A descent right from a to b corresponds to:
 *         skip   1
 *         copy   b-a-1
 */
typedef struct {
	uint32_t *data;
	int solution_d;
	int solution_k;
	uint32_t *solution_ptr;
} Triangle;

/*
 * Return values:
 *
 *  BDELTA_OK:                      Success
 *  BDELTA_MEMORY:                  Memory allocation failed
 *  BDELTA_INTERNAL_DMAX_EXCEEDED:  d_max exceeded (strings are too different)
 */
static BDELTAcode build_triangle(
	const char *old,  uint32_t old_size,
	const char *new_, uint32_t new_size,
	int d_max,
	Triangle *triangle_out)
{
	int d, k;
	uint32_t x, y;
	uint32_t *data;
	uint32_t *vprev; /* position within previous row */
	uint32_t *v;     /* position within current row */
	uint32_t *vcur;  /* beginning of current row */
	size_t data_alloc = 16;
	
	memset(triangle_out, 0, sizeof(*triangle_out));
	
	data = (u32 *)malloc(data_alloc * sizeof(*data));
	if (data == NULL)
		return BDELTA_MEMORY;
	
	/* Allow dmax < 0 to mean "no limit". */
	if (d_max < 0)
		d_max = old_size + new_size;
	
	/*
	 * Compute the farthest-reaching 0-path so the loop after this
	 * will have a "previous" row to start with.
	 */
	for (x = 0; x < old_size && x < new_size && old[x] == new_[x]; )
		x++;
	*data = x;
	if (x >= old_size && x >= new_size) {
		/* Strings are equal, so return a triangle with one row (a dot). */
		assert(x == old_size && x == new_size);
		triangle_out->data = data;
		triangle_out->solution_d = 0;
		triangle_out->solution_k = 0;
		triangle_out->solution_ptr = data;
		return BDELTA_OK;
	}
	vprev = data;
	vcur = v = data + 1;
	
	/*
	 * Here is the core of the Myers diff algorithm.
	 *
	 * This is a direct translation of the pseudocode in Myers' paper,
	 * with implementation-specific adaptations:
	 *
	 *  * Every V array is preserved per iteration of the outer loop.
	 *    This is necessary so we can determine the actual patch, not just
	 *    the length of the shortest edit string.  See the coment above
	 *    the definition of Triangle for an in-depth explanation.
	 *
	 *  * Array items are stored consecutively so as to not waste space.
	 *
	 *  * The buffer holding the V arrays is expanded dynamically.
	 */
	for (d = 1; d <= d_max; d++, vprev = vcur, vcur = v) {
		/* Ensure that the buffer has enough room for this row. */
		if ((size_t)(v - data + d + 1) > data_alloc) {
			size_t vprev_idx = vprev - data;
			size_t v_idx     = v     - data;
			size_t vcur_idx  = vcur  - data;
			uint32_t *tmp;
			
			do {
				data_alloc *= 2;
			} while ((size_t)(v - data + d + 1) > data_alloc);
			
			tmp = (u32 *)realloc(data, data_alloc * sizeof(*data));
			if (tmp == NULL) {
				free(data);
				return BDELTA_MEMORY;
			}
			data = tmp;
			
			/* Relocate pointers to the buffer we just expanded. */
			vprev = data + vprev_idx;
			v     = data + v_idx;
			vcur  = data + vcur_idx;
		}
		
		for (k = -d; k <= d; k += 2, vprev++) {
			if (k == -d || (k != d && vprev[-1] < vprev[0]))
				x = vprev[0];
			else
				x = vprev[-1] + 1;
			y = x - k;
			while (x < old_size && y < new_size && old[x] == new_[y])
				x++, y++;
			*v++ = x;
			if (x >= old_size && y >= new_size) {
				/* Shortest edit string found. */
				assert(x == old_size && y == new_size);
				triangle_out->data = data;
				triangle_out->solution_d = d;
				triangle_out->solution_k = k;
				triangle_out->solution_ptr = v - 1;
				return BDELTA_OK;
			}
		}
	}
	
	free(data);
	return BDELTA_INTERNAL_DMAX_EXCEEDED;
}

/*
 * Trace a solution back to the top, returning a string of instructions
 * for descending from the top to the solution.
 *
 * An instruction is one of the following:
 *
 *  -1: Descend left.
 *  +1: Descend right.
 *   0: Finished.  You should be at the solution now.
 *
 * If memory allocation fails, this function will return NULL.
 */
static signed char *climb_triangle(const Triangle *triangle)
{
	signed char *descent;
	int d, k;
	uint32_t *p;
	
	assert(triangle->solution_d >= 0);
	
	descent = (signed char *)malloc(triangle->solution_d + 1);
	if (descent == NULL)
		return NULL;
	d = triangle->solution_d;
	k = triangle->solution_k;
	p = triangle->solution_ptr;
	descent[d] = 0;
	
	while (d > 0) {
		if (k == -d || (k != d && *(p-d-1) < *(p-d))) {
			/* Climb right */
			k++;
			p = p - d;
			descent[--d] = -1;
		} else {
			/* Climb left */
			k--;
			p = p - d - 1;
			descent[--d] = 1;
		}
	}
	
	return descent;
}

/*
 * Generate the actual patch, given data produced by build_triangle and
 * climb_triangle.  new_ is needed for the content of the insertions.
 *
 * See the comment above the definition of Triangle.  It concisely documents
 * how a descent down the triangle corresponds to a patch script.
 *
 * The resulting patch, including the patch type byte, is appended to patch_out.
 *
 * Return values:
 *
 *  BDELTA_OK:      Success
 *  BDELTA_MEMORY:  Memory allocation failed
 */
static BDELTAcode descent_to_patch(
	const signed char *descent,
	const Triangle *triangle,
	const char *new_, uint32_t new_size,
	SB *patch_out)
{
	const char *new_end = new_ + new_size;
	uint32_t *p = triangle->data;
	uint32_t *p2;
	int d = 0;
	int k = 0;
	int pending_op = 0;
	int current_op;
	uint32_t pending_length = 0;
	uint32_t copy_length;
	
	if (sb_putc(patch_out, PT_CSI32) != 0)
		return BDELTA_MEMORY;
	
	if (*p > 0) {
		if (csi32_emit_op(patch_out, OP_COPY, *p, &new_) != BDELTA_OK)
			return BDELTA_MEMORY;
	}
	
	for (; *descent != 0; descent++, p = p2) {
		if (*descent < 0) {
			/* Descend left. */
			d++;
			k--;
			p2 = p + d;
			current_op = OP_INSERT;
			assert(*p2 >= *p);
			copy_length = *p2 - *p;
		} else {
			/* Descend right. */
			d++;
			k++;
			p2 = p + d + 1;
			current_op = OP_SKIP;
			assert(*p2 > *p);
			copy_length = *p2 - *p - 1;
		}
		
		if (pending_op == current_op) {
			pending_length++;
		} else {
			if (pending_op != 0) {
				if (csi32_emit_op(patch_out, pending_op, pending_length, &new_) != BDELTA_OK)
					return BDELTA_MEMORY;
			}
			pending_op = current_op;
			pending_length = 1;
		}
		
		if (copy_length > 0) {
			if (csi32_emit_op(patch_out, pending_op, pending_length, &new_) != BDELTA_OK)
				return BDELTA_MEMORY;
			pending_op = 0;
			if (csi32_emit_op(patch_out, OP_COPY, copy_length, &new_) != BDELTA_OK)
				return BDELTA_MEMORY;
		}
	}
	assert(d == triangle->solution_d);
	assert(k == triangle->solution_k);
	assert(p == triangle->solution_ptr);
	
	/* Emit the last pending op, unless it's a skip. */
	if (pending_op != 0 && pending_op != OP_SKIP) {
		if (csi32_emit_op(patch_out, pending_op, pending_length, &new_) != BDELTA_OK)
			return BDELTA_MEMORY;
	}
	
	assert(new_ == new_end);
	return BDELTA_OK;
}

/*
 * Generate a patch using Myers' O(ND) algorithm.
 *
 * The patch is appended to @patch_out, which must be initialized before calling.
 *
 * Return values:
 *
 *  BDELTA_OK:                         Success
 *  BDELTA_MEMORY:                     Memory allocation failed
 *  BDELTA_INTERNAL_INPUTS_TOO_LARGE:  Input sizes are too large
 *  BDELTA_INTERNAL_DMAX_EXCEEDED:     d_max exceeded (strings are too different)
 */
static BDELTAcode diff_myers(
	const char *old,  size_t old_size,
	const char *new_, size_t new_size,
	SB *patch_out)
{
	Triangle triangle;
	signed char *descent;
	BDELTAcode rc;
	
	/* Make sure old_size + new_size does not overflow int or uint32_t. */
	if (old_size >= UINT32_MAX ||
	    new_size >= UINT32_MAX - old_size ||
	    old_size >= (unsigned int)INT_MAX ||
	    new_size >= (unsigned int)INT_MAX - old_size)
		return BDELTA_INTERNAL_INPUTS_TOO_LARGE;
	
	rc = build_triangle(old, old_size, new_, new_size, 1000, &triangle);
	if (rc != BDELTA_OK)
		return rc;
	
	descent = climb_triangle(&triangle);
	if (descent == NULL)
		goto oom1;
	
	if (descent_to_patch(descent, &triangle, new_, new_size, patch_out) != BDELTA_OK)
		goto oom2;
	
	free(descent);
	free(triangle.data);
	return BDELTA_OK;
	
oom2:
	free(descent);
oom1:
	free(triangle.data);
	return BDELTA_MEMORY;
}

BDELTAcode bdelta_diff(
	const void  *old,       size_t  old_size,
	const void  *new_,      size_t  new_size,
	void       **patch_out, size_t *patch_size_out)
{
	SB patch;
	
	if (sb_init(&patch) != 0)
		goto out_of_memory;
	
	if (new_size == 0)
		goto emit_new_literally;
	
	if (diff_myers((const char*)old, old_size, (const char*)new_, new_size, &patch) != BDELTA_OK)
		goto emit_new_literally;
	
	if (sb_size(&patch) > new_size) {
		/*
		 * A literal copy of new is no longer than this patch.
		 * All that for nothing.
		 */
		goto emit_new_literally;
	}
	
	/*
	 * Verify that patch, when applied to old, produces the correct text.
	 * If it doesn't, it's a bug, but fall back to a simple emit
	 * to avert data corruption.
	 */
	{
		void *result;
		size_t result_size;
		BDELTAcode rc;
		int correct;
		
		rc = bdelta_patch(
			old, old_size,
			patch.start, patch.cur - patch.start,
			&result, &result_size
		);
		
		switch (rc) {
			case BDELTA_OK:
				correct = (result_size == new_size &&
				           memcmp(result, new_, new_size) == 0);
				free(result);
				break;
			
			case BDELTA_MEMORY:
				goto out_of_memory;
			
			default:
				correct = 0;
				break;
		}
		
		if (!correct) {
			assert(0);
			goto emit_new_literally;
		}
	}
	
	sb_return(&patch, patch_out, patch_size_out);
	return BDELTA_OK;

emit_new_literally:
	if (patch.cur != patch.start) {
		free(patch.start);
		if (sb_init(&patch) != 0)
			goto out_of_memory;
	}
	if (sb_putc(&patch, PT_LITERAL) != 0 || sb_write(&patch, new_, new_size) != 0)
		goto out_of_memory;
	sb_return(&patch, patch_out, patch_size_out);
	return BDELTA_OK;

out_of_memory:
	sb_discard(&patch, patch_out, patch_size_out);
	return BDELTA_MEMORY;
}

/*
 * Return values:
 *
 *  BDELTA_OK:              Success
 *  BDELTA_PATCH_INVALID:   Patch is malformed
 *  BDELTA_PATCH_MISMATCH:  Old string is too small
 *  BDELTA_MEMORY:          Memory allocation failed
 */
static BDELTAcode patch_csi32(
	const unsigned char *o, const unsigned char *oe,
	const unsigned char *p, const unsigned char *pe,
	SB *new_out)
{
	int op;
	uint32_t size;
	
	while (csi32_parse_op(&p, pe, &op, &size)) {
		if ((op == OP_COPY || op == OP_SKIP) && (size_t)(oe - o) < size) {
			/* Copy or skip instruction exceeds length of old string. */
			return BDELTA_PATCH_MISMATCH;
		}
		if (op == OP_COPY || op == OP_INSERT)
			sb_need(new_out, size);
		
		switch (op) {
			case OP_COPY:  /* Copy @size bytes from old string. */
				memcpy(new_out->cur, o, size);
				new_out->cur += size;
				o += size;
				break;
			
			case OP_SKIP:  /* Skip @size bytes of old string. */
				o += size;
				break;
			
			case OP_INSERT:  /* Insert @size new bytes (from the patch script). */
				memcpy(new_out->cur, p, size);
				new_out->cur += size;
				p += size;
				break;
			
			default:
				assert(0);
		}
	}
	if (p != pe)
		return BDELTA_PATCH_INVALID;
	
	return BDELTA_OK;

out_of_memory:
	return BDELTA_MEMORY;
}

BDELTAcode bdelta_patch(
	const void  *old,     size_t  old_size,
	const void  *patch,   size_t  patch_size,
	void       **new_out, size_t *new_size_out)
{
	const unsigned char *o = (unsigned char *)old;
	const unsigned char *oe = o + old_size;
	const unsigned char *p = (unsigned char *)patch;
	const unsigned char *pe = p + patch_size;
	SB result;
	BDELTAcode rc;
	
	if (sb_init(&result) != 0) {
		rc = BDELTA_MEMORY;
		goto discard;
	}
	
	if (p >= pe) {
		rc = BDELTA_PATCH_INVALID;
		goto discard;
	}
	
	switch (*p++) {
		case PT_LITERAL:
			if (sb_write(&result, p, pe - p) != 0) {
				rc = BDELTA_MEMORY;
				goto discard;
			}
			break;
		
		case PT_CSI32:
			rc = patch_csi32(o, oe, p, pe, &result);
			if (rc != BDELTA_OK)
				goto discard;
			break;
		
		default:
			rc = BDELTA_PATCH_INVALID;
			goto discard;
	}
	
	sb_return(&result, new_out, new_size_out);
	return BDELTA_OK;

discard:
	sb_discard(&result, new_out, new_size_out);
	return rc;
}

const char *bdelta_strerror(BDELTAcode code)
{
	switch (code) {
		case BDELTA_OK:
			return "Success";
		case BDELTA_MEMORY:
			return "Could not allocate memory";
		case BDELTA_PATCH_INVALID:
			return "Patch is invalid";
		case BDELTA_PATCH_MISMATCH:
			return "Patch applied to wrong data";
		
		case BDELTA_INTERNAL_DMAX_EXCEEDED:
			return "Difference threshold exceeded (internal error)";
		case BDELTA_INTERNAL_INPUTS_TOO_LARGE:
			return "Inputs are too large (internal error)";
		
		default:
			return "Invalid error code";
	}
}

void bdelta_perror(const char *s, BDELTAcode code)
{
	if (s != NULL && *s != '\0')
		fprintf(stderr, "%s: %s\n", s, bdelta_strerror(code));
	else
		fprintf(stderr, "%s\n", bdelta_strerror(code));
}

/* end bdelta.c */

/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file has been written with some help from wikipedia:
 * 	http://en.wikipedia.org/wiki/CPUID
 */

/* Only compile this file if we're on a x86 machine.  */
#if defined(__i386__) || defined(__i386) || defined(__x86_64) \
	|| defined(_M_AMD64) || defined(__M_X64)

enum {
	CPUID_PROC_BRAND_STRING_INTERNAL0  		= 0x80000003,
	CPUID_PROC_BRAND_STRING_INTERNAL1 		= 0x80000004
};

#ifndef _MSC_VER
static void ___cpuid(cpuid_t info, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
	__asm__(
		"xchg %%ebx, %%edi\n\t" 	/* 32bit PIC: Don't clobber ebx.  */
		"cpuid\n\t"
		"xchg %%ebx, %%edi\n\t"
		: "=a"(*eax), "=D"(*ebx), "=c"(*ecx), "=d"(*edx)
		: "0" (info)
	);
}
#else
#include <intrin.h>

static void ___cpuid(cpuid_t info, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
	uint32_t registers[4];
	__cpuid(registers, info);

	*eax = registers[0];
	*ebx = registers[1];
	*ecx = registers[2];
	*edx = registers[3];
}
#endif

bool cpuid_is_supported(void)
{
	int ret = 0;
#if defined(__GNUC__) || defined(__clang__)
	/* The following assembly code uses EAX as the return value,
	 * but we store the value of EAX into ret since GCC uses EAX
	 * as the return register for every C function.  That's a double
	 * operation, but there's no other way to do this unless doing this
	 * function entirely in assembly.
	 *
	 * The following assembly code has been shamelessly stolen from:
	 * 	http://wiki.osdev.org/CPUID
	 * and converted to work with AT&T syntax.
	 *
	 * This check is to make sure that the compiler is actually compiling
	 * for 64-bit.
	 *
	 * The compiler can be 32-bit and the system 64-bit so the 
	 * following would be true:
	 * 	#if defined(__x86_64) ...
	 */

#if UINTPTR_MAX == 0xffffffffffffffff

	asm volatile(
		"pushfq\n\t"
		"popq %%rax\n\t"
		"movl %%eax, %%ecx\n\t"
		"xorl $0x200000, %%eax\n\t"
		"pushq %%rax\n\t"
		"popfq\n\t"
		"pushfq\n\t"
		"popq %%rax\n\t"
		"xorl %%ecx, %%eax\n\t"
		"shrl $21, %%eax\n\t"
		"andl $1, %%eax\n\t"
		"pushq %%rcx\n\t"
		"popfq\n\t"
		: "=a" (ret)
	);

#elif UINTPTR_MAX == 0xffffffff

	asm volatile(
		"pushfl\n\t"
		"popl %%eax\n\t"
		"movl %%eax, %%ecx\n\t"
		"xorl $0x200000, %%eax\n\t"
		"pushl %%eax\n\t"
		"popfl\n\t"
		"pushfl\n\t"
		"popl %%eax\n\t"
		"xorl %%ecx, %%eax\n\t"
		"shrl $21, %%eax\n\t"
		"andl $1, %%eax\n\t"
		"pushl %%ecx\n\t"
		"popfl\n\t"
		: "=a" (ret)
	);

#endif

#elif defined _MSC_VER
	__asm {
		pushfd
		pop eax
		mov ecx, eax
		xor eax, 0x200000
		push eax
		popfd

		pushfd
		pop eax
		xor eax, ecx
		shr eax, 21
		and eax, 1
		push ecx
		popfd

		mov eax, ret
	};
#endif
	return !!ret;
}

bool cpuid_test_feature(cpuid_t feature)
{
	if (feature > CPUID_VIRT_PHYS_ADDR_SIZES || feature < CPUID_EXTENDED_PROC_INFO_FEATURE_BITS)
		return false;

	return (feature <= cpuid_highest_ext_func_supported());
}

#if defined(__GNUC__) || defined(__clang__)
static uint32_t fetch_ecx(uint32_t what)
{
	static uint32_t ecx;
	if (ecx == 0) {
		asm volatile(
			"cpuid\n\t"
			: "=c" (ecx)
			: "a" (what)
		);
	}

	return ecx;
}

static uint32_t fetch_edx(uint32_t what)
{
	static uint32_t edx;
	if (edx == 0) {
		asm volatile(
			"cpuid\n\t"
			: "=d" (edx)
			: "a" (what)
		);
	}

	return edx;
}
#elif defined(_MSC_VER)
static uint32_t fetch_ecx(uint32_t what)
{
	static uint32_t _ecx;
	if (_ecx == 0) {
		__asm {
			mov eax, what
			cpuid
			mov _ecx, ecx
		};
	}

	return _ecx;
}

static uint32_t fetch_edx(uint32_t what)
{
	static uint32_t _edx;
	if (_edx == 0) {
		__asm {
			mov eax, what
			cpuid
			mov _edx, edx
		};
	}

	return _edx;
}
#endif

#define DEFINE_FEATURE_FUNC(NAME, REGISTER, TYPE) \
	bool cpuid_has_##NAME(int feature) \
	{ \
		static uint32_t REGISTER; \
		if (REGISTER == 0) \
			REGISTER = fetch_##REGISTER(TYPE); \
		return (REGISTER & feature) == feature; \
	}

DEFINE_FEATURE_FUNC(ecxfeature, ecx, CPUID_PROCINFO_AND_FEATUREBITS)
DEFINE_FEATURE_FUNC(edxfeature, edx, CPUID_PROCINFO_AND_FEATUREBITS)

DEFINE_FEATURE_FUNC(ecxfeature_ext, ecx, CPUID_EXTENDED_PROC_INFO_FEATURE_BITS)
DEFINE_FEATURE_FUNC(edxfeature_ext, edx, CPUID_EXTENDED_PROC_INFO_FEATURE_BITS)

#undef DEFINE_FEATURE_FUNC

static const char *const cpuids[] = {
	"Nooooooooone",
	"AMDisbetter!",
	"AuthenticAMD",
	"CentaurHauls",
	"CyrixInstead",
	"GenuineIntel",
	"TransmetaCPU",
	"GeniuneTMx86",
	"Geode by NSC",
	"NexGenDriven",
	"RiseRiseRise",
	"SiS SiS SiS ",
	"UMC UMC UMC ",
	"VIA VIA VIA ",
	"Vortex86 SoC",
	"KVMKVMKVMKVM"
};

cputype_t cpuid_get_cpu_type(void)
{
	static cputype_t cputype;
	if (cputype == CT_NONE) {
		union {
			char buf[12];
			uint32_t bufu32[3];
		} u;
		uint32_t i;

		___cpuid(CPUID_VENDORID, &i, &u.bufu32[0], &u.bufu32[2], &u.bufu32[1]);
		for (i = 0; i < sizeof(cpuids) / sizeof(cpuids[0]); ++i) {
			if (strncmp(cpuids[i], u.buf, 12) == 0) {
				cputype = (cputype_t)i;
				break;
			}
		}
	}

	return cputype;
}

bool cpuid_sprintf_cputype(const cputype_t cputype, char *buf)
{
	if (cputype == CT_NONE)
		return false;

	memcpy(buf, cpuids[(int)cputype], 12);
	buf[12] = '\0';
	return true;
}

uint32_t cpuid_highest_ext_func_supported(void)
{
	static uint32_t highest;

	if (!highest) {
#if defined(__GNUC__) || defined(__clang__)
		asm volatile(
			"cpuid\n\t"
			: "=a" (highest)
			: "a" (CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED)
		);
#elif defined _MSC_VER
		__asm {
			mov eax, CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED
			cpuid
			mov highest, eax
		};
#endif
	}

	return highest;
}

void cpuid(cpuid_t info, uint32_t *buf)
{
	/* Sanity checks, make sure we're not trying to do something
	 * invalid or we are trying to get information that isn't supported
	 * by the CPU.  */
	if (info > CPUID_VIRT_PHYS_ADDR_SIZES || (info > CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED
		&& !cpuid_test_feature(info)))
		return;

	if (info == CPUID_PROC_BRAND_STRING) {
		static char cached[48] = { 0 };
		if (cached[0] == '\0') {
			___cpuid(CPUID_PROC_BRAND_STRING,	    &buf[0], &buf[1], &buf[2],  &buf[3] );
			___cpuid((cpuid_t)(CPUID_PROC_BRAND_STRING_INTERNAL0), &buf[4], &buf[5], &buf[6],  &buf[7] );
			___cpuid((cpuid_t)(CPUID_PROC_BRAND_STRING_INTERNAL1), &buf[8], &buf[9], &buf[10], &buf[11]);

			memcpy(cached, buf, sizeof cached);
		} else
			buf = (uint32_t *)cached;

		return;
	} else if (info == CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED) {
		*buf = cpuid_highest_ext_func_supported();
		return;
	}

	uint32_t eax, ebx, ecx, edx;
	___cpuid(info, &eax, &ebx, &ecx, &edx);

	switch (info) {
		case CPUID_VENDORID:
			buf[0] = ebx;
			buf[1] = edx;
			buf[2] = ecx;
			break;
		case CPUID_PROCINFO_AND_FEATUREBITS:
			buf[0] = (eax & 0x0F);		/* Stepping  */
			buf[1] = (eax >> 4)  & 0x0F; 	/* Model  */
			buf[2] = (eax >> 8)  & 0x0F; 	/* Family  */
			buf[3] = (eax >> 16) & 0x0F; 	/* Extended Model.  */
			buf[4] = (eax >> 24) & 0x0F; 	/* Extended Family.  */

			/* Additional Feature information.  */
			buf[5] = ebx & 0xFF;
			buf[6] = (ebx >> 8) & 0xFF;
			buf[7] = (ebx >> 16) & 0xFF;
			buf[8] = ebx >> 24;
			break;
		case CPUID_CACHE_AND_TLBD_INFO:
			buf[0] = eax;
			buf[1] = ebx;
			buf[2] = ecx;
			buf[3] = edx;
			break;
		case CPUID_EXTENDED_PROC_INFO_FEATURE_BITS:
			buf[0] = edx;
			buf[1] = ecx;
			break;
		case CPUID_L1_CACHE_AND_TLB_IDS:
			buf[0] = eax & 0xFF;
			buf[1] = (eax >> 8) & 0xFF;
			buf[2] = (eax >> 16) & 0xFF;
			buf[3] = eax >> 24;

			buf[4] = ebx & 0xFF;
			buf[5] = (ebx >> 8) & 0xFF;
			buf[6] = (ebx >> 16) & 0xFF;
			buf[7] = ebx >> 24;

			buf[8] = ecx & 0xFF;
			buf[9] = (ecx >> 8) & 0xFF;
			buf[10] = (ecx >> 16) & 0xFF;
			buf[11] = ecx >> 24;

			buf[12] = edx & 0xFF;
			buf[13] = (edx >> 8) & 0xFF;
			buf[14] = (edx >> 16) & 0xFF;
			buf[15] = edx >> 24;
			break;
		case CPUID_EXTENDED_L2_CACHE_FEATURES:
			buf[0] = ecx & 0xFF; 		/* Line size.  */
			buf[1] = (ecx >> 12) & 0xFF; 	/* Associativity.  */
			buf[2] = ecx >> 16; 		/* Cache size.  */
			break;
		case CPUID_ADV_POWER_MGT_INFO:
			*buf = edx;
			break;
		case CPUID_VIRT_PHYS_ADDR_SIZES:
			buf[0] = eax & 0xFF; 		/* physical.  */
			buf[1] = (eax >> 8) & 0xFF; 	/* virtual.  */
			break;
		default:
			*buf = 0xbaadf00d;
			break;
	}
}

bool cpuid_write_info(uint32_t info, uint32_t featureset, const char *outfile)
{
	FILE *file;
	char filename[256];
	char cpu_information[64];

	if (!cpuid_sprintf_cputype(cpuid_get_cpu_type(), cpu_information))
		return false;

	char brand[48];
	cpuid(CPUID_PROC_BRAND_STRING, (uint32_t *)brand);

	cpu_information[12] = '_';
	memcpy(&cpu_information[13], brand, sizeof brand);

	if (!outfile)
		strncpy(filename, cpu_information, sizeof cpu_information);
	else
		strncpy(filename, outfile, sizeof filename);

	file = fopen(filename, "w");
	if (!file)
		return false;

	fprintf(file, "-- CPU Information for CPU: %s --\n\n", cpu_information);

	if (info & CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED)
		fprintf(file, "Highest extended function supported: %#010x\n\n", cpuid_highest_ext_func_supported());

	if (info & CPUID_EXTENDED_L2_CACHE_FEATURES) {
		uint32_t l2c[3];
		cpuid(CPUID_EXTENDED_L2_CACHE_FEATURES, l2c);

		fprintf(file, "-- Extended L2 Cache features --\nL2 Line size: %u bytes\nAssociativity: %02xh\nCache Size: %u KB\n\n",
			l2c[0], l2c[1], l2c[2]);
	}

	if (info & CPUID_VIRT_PHYS_ADDR_SIZES) {
		uint32_t phys_virt[2];
		cpuid(CPUID_VIRT_PHYS_ADDR_SIZES, phys_virt);

		fprintf(file, "-- Virtual and Physical address sizes --\n"
				"Physical address size: %d\nVirtual address size: %d\n\n", phys_virt[0], phys_virt[1]);
	}

	if (info & CPUID_PROCINFO_AND_FEATUREBITS) {
		uint32_t procinfo[9];
		cpuid(CPUID_PROCINFO_AND_FEATUREBITS, procinfo);

		fputs("-- Processor information and feature bits --\n", file	);
		fprintf(file, "Stepping: %d\nModel: 0x%X\nFamily: %d\nExtended model: %d\nExtended family: %d\n",
			procinfo[0], procinfo[1], procinfo[2], procinfo[3], procinfo[4]);
		fprintf(file, "\nBrand Index: %d\nCL Flush Line Size: %d\nLogical Processors: %d\nInitial APICID: %d\n\n",
			procinfo[5], procinfo[6], procinfo[7], procinfo[8]);
	}

	if (featureset != 0)
		fputs("-- CPU FEATURES --\n\n", file);

	bool
		sse3    = cpuid_has_ecxfeature(CPUID_FEAT_ECX_SSE3),
		pclmul  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_PCLMUL),
		dtes64  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_DTES64),
		monitor = cpuid_has_ecxfeature(CPUID_FEAT_ECX_MONITOR),
		ds_cpl  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_DS_CPL),
		vmx     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_VMX),
		smx     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_SMX),
		est     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_EST),
		tm2     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_TM2),
		ssse3   = cpuid_has_ecxfeature(CPUID_FEAT_ECX_SSSE3),
		cid     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_CID),
		fma     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_FMA),
		cx16    = cpuid_has_ecxfeature(CPUID_FEAT_ECX_CX16),
		etprd   = cpuid_has_ecxfeature(CPUID_FEAT_ECX_ETPRD),
		pdcm    = cpuid_has_ecxfeature(CPUID_FEAT_ECX_PDCM),
		dca     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_DCA),
		sse4_1  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_SSE4_1),
		sse4_2  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_SSE4_2),
		x2_apic = cpuid_has_ecxfeature(CPUID_FEAT_ECX_x2APIC),
		movbe   = cpuid_has_ecxfeature(CPUID_FEAT_ECX_MOVBE),
		popcnt  = cpuid_has_ecxfeature(CPUID_FEAT_ECX_POPCNT),
		aes     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_AES),
		xsave   = cpuid_has_ecxfeature(CPUID_FEAT_ECX_XSAVE),
		osxsave = cpuid_has_ecxfeature(CPUID_FEAT_ECX_OSXSAVE),
		avx     = cpuid_has_ecxfeature(CPUID_FEAT_ECX_AVX);

#define YON(v)	(v) ? "Yes" : "No"
	if (featureset & CPUID_FEAT_ECX_ALL) {
		fputs("-- ECX Features --\n", file);
		fprintf(file, "SSE3:    %s\n"
			      "PCMUL:   %s\n"
			      "DTES64:  %s\n"
			      "MONITOR: %s\n"
			      "DS_CPL:  %s\n"
			      "VMX:     %s\n"
			      "SMX:     %s\n"
			      "EST:     %s\n"
			      "TM2:     %s\n"
			      "SSSE3:   %s\n"
			      "CID:     %s\n"
			      "FMA:     %s\n"
			      "CX16:    %s\n"
			      "ETPRD:   %s\n"
			      "PDCM:    %s\n"
			      "DCA:     %s\n"
			      "SSE4_1:  %s\n"
			      "SSE$_2:  %s\n"
			      "X2_APIC: %s\n"
			      "MOVBE:   %s\n"
			      "POPCNT:  %s\n"
			      "AES:     %s\n"
			      "XSAVE:   %s\n"
			      "OSXSAVE: %s\n"
			      "AVS:     %s\n\n",
			YON(sse3), YON(pclmul), YON(dtes64), YON(monitor), YON(ds_cpl),
			YON(vmx), YON(smx), YON(est), YON(tm2), YON(ssse3), YON(cid),
			YON(fma), YON(cx16), YON(etprd), YON(pdcm), YON(dca), YON(sse4_1),
			YON(sse4_2), YON(x2_apic), YON(movbe), YON(popcnt), YON(aes),
			YON(xsave), YON(osxsave), YON(avx)
		);
	} else {
		if (featureset & CPUID_FEAT_ECX_SSE3)
			fprintf(file, "SSE3:    %s\n", YON(sse3));
		if (featureset & CPUID_FEAT_ECX_PCLMUL)
			fprintf(file, "PCLMUL:    %s\n", YON(pclmul));
		if (featureset & CPUID_FEAT_ECX_DTES64)
			fprintf(file, "DTES64:    %s\n", YON(dtes64));
		if (featureset & CPUID_FEAT_ECX_MONITOR)
			fprintf(file, "Monitor:    %s\n", YON(monitor));
		if (featureset & CPUID_FEAT_ECX_DS_CPL)
			fprintf(file, "DS CPL:    %s\n", YON(ds_cpl));
		if (featureset & CPUID_FEAT_ECX_VMX)
			fprintf(file, "VMX:    %s\n", YON(vmx));
		if (featureset & CPUID_FEAT_ECX_SMX)
			fprintf(file, "SMX:    %s\n", YON(smx));
		if (featureset & CPUID_FEAT_ECX_EST)
			fprintf(file, "EST:    %s\n", YON(est));
		if (featureset & CPUID_FEAT_ECX_TM2)
			fprintf(file, "TM2:    %s\n", YON(tm2));
		if (featureset & CPUID_FEAT_ECX_SSSE3)
			fprintf(file, "SSSE3:    %s\n", YON(ssse3));
		if (featureset & CPUID_FEAT_ECX_CID)
			fprintf(file, "CID:    %s\n", YON(cid));
		if (featureset & CPUID_FEAT_ECX_FMA)
			fprintf(file, "FMA:    %s\n", YON(fma));
		if (featureset & CPUID_FEAT_ECX_CX16)
			fprintf(file, "CX16:    %s\n", YON(cx16));
		if (featureset & CPUID_FEAT_ECX_ETPRD)
			fprintf(file, "ETPRD:    %s\n", YON(etprd));
		if (featureset & CPUID_FEAT_ECX_PDCM)
			fprintf(file, "PDCM:    %s\n", YON(pdcm));
		if (featureset & CPUID_FEAT_ECX_DCA)
			fprintf(file, "DCA:    %s\n", YON(dca));
		if (featureset & CPUID_FEAT_ECX_SSE4_1)
			fprintf(file, "SSE4_1:    %s\n", YON(sse4_1));
		if (featureset & CPUID_FEAT_ECX_SSE4_2)
			fprintf(file, "SSE4_2:    %s\n", YON(sse4_2));
		if (featureset & CPUID_FEAT_ECX_x2APIC)
			fprintf(file, "x2APIC:    %s\n", YON(x2_apic));
		if (featureset & CPUID_FEAT_ECX_MOVBE)
			fprintf(file, "MOVBE:    %s\n", YON(movbe));
		if (featureset & CPUID_FEAT_ECX_POPCNT)
			fprintf(file, "POPCNT:    %s\n", YON(popcnt));
		if (featureset & CPUID_FEAT_ECX_AES)
			fprintf(file, "AES:    %s\n", YON(aes));
		if (featureset & CPUID_FEAT_ECX_XSAVE)
			fprintf(file, "XSAVE:    %s\n", YON(xsave));
		if (featureset & CPUID_FEAT_ECX_OSXSAVE)
			fprintf(file, "OSXSAVE:    %s\n", YON(osxsave));
		if (featureset & CPUID_FEAT_ECX_AVX)
			fprintf(file, "AVX:    %s\n", YON(avx));
	}

	bool
		fpu = cpuid_has_edxfeature(CPUID_FEAT_EDX_FPU),
		vme   = cpuid_has_edxfeature(CPUID_FEAT_EDX_VME),
		de    = cpuid_has_edxfeature(CPUID_FEAT_EDX_DE),
		pse   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PSE),
		tsc   = cpuid_has_edxfeature(CPUID_FEAT_EDX_TSC),
		msr   = cpuid_has_edxfeature(CPUID_FEAT_EDX_MSR),
		pae   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PAE),
		mce   = cpuid_has_edxfeature(CPUID_FEAT_EDX_MCE),
		cx8   = cpuid_has_edxfeature(CPUID_FEAT_EDX_CX8),
		apic  = cpuid_has_edxfeature(CPUID_FEAT_EDX_APIC),
		sep   = cpuid_has_edxfeature(CPUID_FEAT_EDX_SEP),
		mtrr  = cpuid_has_edxfeature(CPUID_FEAT_EDX_MTRR),
		pge   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PGE),
		mca   = cpuid_has_edxfeature(CPUID_FEAT_EDX_MCA),
		cmov  = cpuid_has_edxfeature(CPUID_FEAT_EDX_CMOV),
		pat   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PAT),
		pse36 = cpuid_has_edxfeature(CPUID_FEAT_EDX_PSE36),
		psn   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PSN),
		clf   = cpuid_has_edxfeature(CPUID_FEAT_EDX_CLF),
		dtes  = cpuid_has_edxfeature(CPUID_FEAT_EDX_DTES),
		acpi  = cpuid_has_edxfeature(CPUID_FEAT_EDX_ACPI),
		mmx   = cpuid_has_edxfeature(CPUID_FEAT_EDX_MMX),
		fxsr  = cpuid_has_edxfeature(CPUID_FEAT_EDX_FXSR),
		sse   = cpuid_has_edxfeature(CPUID_FEAT_EDX_SSE),
		sse2  = cpuid_has_edxfeature(CPUID_FEAT_EDX_SSE2),
		ss    = cpuid_has_edxfeature(CPUID_FEAT_EDX_SS),
		htt   = cpuid_has_edxfeature(CPUID_FEAT_EDX_HTT),
		tm1   = cpuid_has_edxfeature(CPUID_FEAT_EDX_TM1),
		ia64  = cpuid_has_edxfeature(CPUID_FEAT_EDX_IA64),
		pbe   = cpuid_has_edxfeature(CPUID_FEAT_EDX_PBE);

	if (featureset & CPUID_FEAT_EDX_ALL) {
		fputs("-- EDX FEATURES --\n", file);
		fprintf(file, "FPU:   %s\n"
			      "VME:   %s\n"
			      "DE:    %s\n"
			      "PSE:   %s\n"
			      "TSC:   %s\n"
			      "MSR:   %s\n"
			      "PAE:   %s\n"
			      "MCE:   %s\n"
			      "CX8:   %s\n"
			      "APIC:  %s\n"
			      "SEP:   %s\n"
			      "MTRR:  %s\n"
			      "PGE:   %s\n"
			      "MCA:   %s\n"
			      "CMOV:  %s\n"
			      "PAT:   %s\n"
			      "PSE36: %s\n"
			      "PSN:   %s\n"
			      "CLF:   %s\n"
			      "DTES:  %s\n"
			      "ACPI:  %s\n"
			      "MMX:   %s\n"
			      "FXSR:  %s\n"
			      "SSE:   %s\n"
			      "SSE2:  %s\n"
			      "SS:    %s\n"
			      "HTT:   %s\n"
			      "TM1:   %s\n"
			      "IA64:  %s\n"
			      "PBE:   %s\n\n",
			YON(fpu), YON(vme), YON(de), YON(pse), YON(tsc), YON(msr),
			YON(pae), YON(mce), YON(cx8), YON(apic), YON(sep), YON(mtrr),
			YON(pge), YON(mca), YON(cmov), YON(pat), YON(pse36), YON(psn),
			YON(clf), YON(dtes), YON(acpi), YON(mmx), YON(fxsr), YON(sse),
			YON(sse2), YON(ss), YON(htt), YON(tm1), YON(ia64), YON(pbe)
		);
	} else {
		if (featureset & CPUID_FEAT_EDX_FPU)
			fprintf(file, "FPU:   %s\n", YON(fpu));
		if (featureset & CPUID_FEAT_EDX_VME)
			fprintf(file, "VME:   %s\n", YON(vme));
		if (featureset & CPUID_FEAT_EDX_DE)
			fprintf(file, "DE: %s\n", YON(de));
		if (featureset & CPUID_FEAT_EDX_PSE)
			fprintf(file, "PSE:   %s\n", YON(pse));
		if (featureset & CPUID_FEAT_EDX_TSC)
			fprintf(file, "TSC:   %s\n", YON(tsc));
		if (featureset & CPUID_FEAT_EDX_MSR)
			fprintf(file, "MSR:   %s\n", YON(msr));
		if (featureset & CPUID_FEAT_EDX_PAE)
			fprintf(file, "PAE:   %s\n", YON(pae));
		if (featureset & CPUID_FEAT_EDX_MCE)
			fprintf(file, "MCE:   %s\n", YON(mce));
		if (featureset & CPUID_FEAT_EDX_CX8)
			fprintf(file, "CX8:   %s\n", YON(cx8));
		if (featureset & CPUID_FEAT_EDX_APIC)
			fprintf(file, "APIC: %s\n", YON(apic));
		if (featureset & CPUID_FEAT_EDX_SEP)
			fprintf(file, "SEP:   %s\n", YON(sep));
		if (featureset & CPUID_FEAT_EDX_MTRR)
			fprintf(file, "MTRR: %s\n", YON(mtrr));
		if (featureset & CPUID_FEAT_EDX_PGE)
			fprintf(file, "PGE:   %s\n", YON(pge));
		if (featureset & CPUID_FEAT_EDX_MCA)
			fprintf(file, "MCA:   %s\n", YON(mca));
		if (featureset & CPUID_FEAT_EDX_CMOV)
			fprintf(file, "CMOV: %s\n", YON(cmov));
		if (featureset & CPUID_FEAT_EDX_PAT)
			fprintf(file, "PAT:   %s\n", YON(pat));
		if (featureset & CPUID_FEAT_EDX_PSE36)
			fprintf(file, "PSE36: %s\n", YON(pse36));
		if (featureset & CPUID_FEAT_EDX_PSN)
			fprintf(file, "PSN:   %s\n", YON(psn));
		if (featureset & CPUID_FEAT_EDX_CLF)
			fprintf(file, "CLF:   %s\n", YON(clf));
		if (featureset & CPUID_FEAT_EDX_DTES)
			fprintf(file, "DTES:%s\n", YON(dtes));
		if (featureset & CPUID_FEAT_EDX_ACPI)
			fprintf(file, "ACPI: %s\n", YON(acpi));
		if (featureset & CPUID_FEAT_EDX_MMX)
			fprintf(file, "MMX:   %s\n", YON(mmx));
		if (featureset & CPUID_FEAT_EDX_FXSR)
			fprintf(file, "FXSR: %s\n", YON(fxsr));
		if (featureset & CPUID_FEAT_EDX_SSE)
			fprintf(file, "SSE:   %s\n", YON(sse));
		if (featureset & CPUID_FEAT_EDX_SSE2)
			fprintf(file, "SSE2: %s\n", YON(sse2));
		if (featureset & CPUID_FEAT_EDX_SS)
			fprintf(file, "SS: %s\n", YON(ss));
		if (featureset & CPUID_FEAT_EDX_HTT)
			fprintf(file, "HTT:   %s\n", YON(htt));
		if (featureset & CPUID_FEAT_EDX_TM1)
			fprintf(file, "TM1:   %s\n", YON(tm1));
		if (featureset & CPUID_FEAT_EDX_IA64)
			fprintf(file, "IA64: %s\n", YON(ia64));
		if (featureset & CPUID_FEAT_EDX_PBE)
			fprintf(file, "PBE:   %s\n", YON(pbe));
	}
#undef YON

	fclose(file);
	return true;
}

#endif  // x86/x64

size_t strcount(const char *haystack, const char *needle) {
	size_t i = 0, nlen = strlen(needle);

	while ((haystack = strstr(haystack, needle)) != NULL) {
		i++;
		haystack += nlen;
	}
	return i;
}

ptrdiff_t ptr2int(const ptrint_t *p) {
	/*
	 * ptrdiff_t is the right size by definition, but to avoid
	 * surprises we want a warning if the user can't fit at least
	 * a regular int in there
	 */
	BUILD_ASSERT(sizeof(int) <= sizeof(ptrdiff_t));
	return (const char *)p - (const char *)NULL;
}

ptrint_t *int2ptr(ptrdiff_t i) {
	return (ptrint_t *)((char *)NULL + i);
}

#include <sys/types.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define SIZET_BITS (sizeof(size_t)*CHAR_BIT)

/* We use power of 2 steps.  I tried being tricky, but it got buggy. */
struct tally {
	ssize_t min, max;
	size_t total[2];
	/* This allows limited frequency analysis. */
	unsigned buckets, step_bits;
	size_t counts[1 /* Actually: [buckets] */ ];
};

struct tally *tally_new(unsigned buckets) {
	struct tally *tally;

	/* There is always 1 bucket. */
	if (buckets == 0) {
		buckets = 1;
	}

	/* Overly cautious check for overflow. */
	if (sizeof(*tally) * buckets / sizeof(*tally) != buckets) {
		return NULL;
	}

	tally = (struct tally *)malloc(
		sizeof(*tally) + sizeof(tally->counts[0])*(buckets-1));
	if (tally == NULL) {
		return NULL;
	}

	tally->max = ((size_t)1 << (SIZET_BITS - 1));
	tally->min = ~tally->max;
	tally->total[0] = tally->total[1] = 0;
	tally->buckets = buckets;
	tally->step_bits = 0;
	memset(tally->counts, 0, sizeof(tally->counts[0])*buckets);
	return tally;
}

static unsigned bucket_of(ssize_t min, unsigned step_bits, ssize_t val) {
	/* Don't over-shift. */
	if (step_bits == SIZET_BITS) {
		return 0;
	}
	assert(step_bits < SIZET_BITS);
	return (size_t)(val - min) >> step_bits;
}

/* Return the min value in bucket b. */
static ssize_t bucket_min(ssize_t min, unsigned step_bits, unsigned b) {
	/* Don't over-shift. */
	if (step_bits == SIZET_BITS) {
		return min;
	}
	assert(step_bits < SIZET_BITS);
	return min + ((ssize_t)b << step_bits);
}

/* Does shifting by this many bits truncate the number? */
static bool shift_overflows(size_t num, unsigned bits) {
	if (bits == 0) {
		return false;
	}

	return ((num << bits) >> 1) != (num << (bits - 1));
}

/* When min or max change, we may need to shuffle the frequency counts. */
static void renormalize(struct tally *tally,
			ssize_t new_min, ssize_t new_max) {
	size_t range, spill;
	unsigned int i, old_min;

	/* Uninitialized?  Don't do anything... */
	if (tally->max < tally->min) {
		goto update;
	}

	/* If we don't have sufficient range, increase step bits until
	 * buckets cover entire range of ssize_t anyway. */
	range = (new_max - new_min) + 1;
	while (!shift_overflows(tally->buckets, tally->step_bits)
	       && range > ((size_t)tally->buckets << tally->step_bits)) {
		/* Collapse down. */
		for (i = 1; i < tally->buckets; i++) {
			tally->counts[i/2] += tally->counts[i];
			tally->counts[i] = 0;
		}
		tally->step_bits++;
	}

	/* Now if minimum has dropped, move buckets up. */
	old_min = bucket_of(new_min, tally->step_bits, tally->min);
	memmove(tally->counts + old_min,
		tally->counts,
		sizeof(tally->counts[0]) * (tally->buckets - old_min));
	memset(tally->counts, 0, sizeof(tally->counts[0]) * old_min);

	/* If we moved boundaries, adjust buckets to that ratio. */
	spill = (tally->min - new_min) % (1 << tally->step_bits);
	for (i = 0; i < tally->buckets-1; i++) {
		size_t adjust = (tally->counts[i] >> tally->step_bits) * spill;
		tally->counts[i] -= adjust;
		tally->counts[i+1] += adjust;
	}

update:
	tally->min = new_min;
	tally->max = new_max;
}

void tally_add(struct tally *tally, ssize_t val) {
	ssize_t new_min = tally->min, new_max = tally->max;
	bool need_renormalize = false;

	if (val < tally->min) {
		new_min = val;
		need_renormalize = true;
	}
	if (val > tally->max) {
		new_max = val;
		need_renormalize = true;
	}
	if (need_renormalize) {
		renormalize(tally, new_min, new_max);
	}

	/* 128-bit arithmetic!  If we didn't want exact mean, we could just
	 * pull it out of counts. */
	if (val > 0 && tally->total[0] + val < tally->total[0]) {
		tally->total[1]++;
	} else if (val < 0 && tally->total[0] + val > tally->total[0]) {
		tally->total[1]--;
	}
	tally->total[0] += val;
	tally->counts[bucket_of(tally->min, tally->step_bits, val)]++;
}

size_t tally_num(const struct tally *tally) {
	size_t i, num = 0;
	for (i = 0; i < tally->buckets; i++) {
		num += tally->counts[i];
	}
	return num;
}

ssize_t tally_min(const struct tally *tally) {
	return tally->min;
}

ssize_t tally_max(const struct tally *tally) {
	return tally->max;
}

/* FIXME: Own ccan module please! */
static unsigned fls64(uint64_t val) {
#if __GNUC__	//HAVE_BUILTIN_CLZL
	if (val <= ULONG_MAX) {
		/* This is significantly faster! */
		return val ? sizeof(long) * CHAR_BIT - __builtin_clzl(val) : 0;
	} else {
#endif
	uint64_t r = 64;

	if (!val) {
		return 0;
	}
	if (!(val & 0xffffffff00000000ull)) {
		val <<= 32;
		r -= 32;
	}
	if (!(val & 0xffff000000000000ull)) {
		val <<= 16;
		r -= 16;
	}
	if (!(val & 0xff00000000000000ull)) {
		val <<= 8;
		r -= 8;
	}
	if (!(val & 0xf000000000000000ull)) {
		val <<= 4;
		r -= 4;
	}
	if (!(val & 0xc000000000000000ull)) {
		val <<= 2;
		r -= 2;
	}
	if (!(val & 0x8000000000000000ull)) {
		val <<= 1;
		r -= 1;
	}
	return r;
#if __GNUC__	//HAVE_BUILTIN_CLZL
	}
#endif
}

/* This is stolen straight from Hacker's Delight. */
static uint64_t divlu64(uint64_t u1, uint64_t u0, uint64_t v) {
	const uint64_t b = 4294967296ULL; /* Number base (32 bits). */
	uint32_t un[4],		  /* Dividend and divisor */
		vn[2];		  /* normalized and broken */
				  /* up into halfwords. */
	uint32_t q[2];		  /* Quotient as halfwords. */
	uint64_t un1, un0,	  /* Dividend and divisor */
		vn0;		  /* as fullwords. */
	uint64_t qhat;		  /* Estimated quotient digit. */
	uint64_t rhat;		  /* A remainder. */
	uint64_t p;		  /* Product of two digits. */
	int64_t s, i, j, t, k;

	if (u1 >= v) {		  /* If overflow, return the largest */
		return (uint64_t)-1; /* possible quotient. */
	}

	s = 64 - fls64(v);		  /* 0 <= s <= 63. */
	vn0 = v << s;		  /* Normalize divisor. */
	vn[1] = vn0 >> 32;	  /* Break divisor up into */
	vn[0] = vn0 & 0xFFFFFFFF; /* two 32-bit halves. */

	// Shift dividend left.
	un1 = ((u1 << s) | (u0 >> (64 - s))) & (-s >> 63);
	un0 = u0 << s;
	un[3] = un1 >> 32;	  /* Break dividend up into */
	un[2] = un1;		  /* four 32-bit halfwords */
	un[1] = un0 >> 32;	  /* Note: storing into */
	un[0] = un0;		  /* halfwords truncates. */

	for (j = 1; j >= 0; j--) {
		/* Compute estimate qhat of q[j]. */
		qhat = (un[j+2]*b + un[j+1])/vn[1];
		rhat = (un[j+2]*b + un[j+1]) - qhat*vn[1];
	again:
		if (qhat >= b || qhat*vn[0] > b*rhat + un[j]) {
			qhat = qhat - 1;
			rhat = rhat + vn[1];
			if (rhat < b) {
				goto again;
			}
		}

		/* Multiply and subtract. */
		k = 0;
		for (i = 0; i < 2; i++) {
			p = qhat*vn[i];
			t = un[i+j] - k - (p & 0xFFFFFFFF);
			un[i+j] = t;
			k = (p >> 32) - (t >> 32);
		}
		t = un[j+2] - k;
		un[j+2] = t;

		q[j] = qhat;		  /* Store quotient digit. */
		if (t < 0) {		  /* If we subtracted too */
			q[j] = q[j] - 1;  /* much, add back. */
			k = 0;
			for (i = 0; i < 2; i++) {
				t = un[i+j] + vn[i] + k;
				un[i+j] = t;
				k = t >> 32;
			}
			un[j+2] = un[j+2] + k;
		}
	} /* End j. */

	return q[1]*b + q[0];
}

static int64_t divls64(int64_t u1, uint64_t u0, int64_t v) {
	int64_t q, uneg, vneg, diff, borrow;

	uneg = u1 >> 63;	  /* -1 if u < 0. */
	if (uneg) {		  /* Compute the absolute */
		u0 = -u0;	  /* value of the dividend u. */
		borrow = (u0 != 0);
		u1 = -u1 - borrow;
	}

	vneg = v >> 63;		  /* -1 if v < 0. */
	v = (v ^ vneg) - vneg;	  /* Absolute value of v. */

	if ((uint64_t)u1 >= (uint64_t)v) {
		goto overflow;
	}

	q = divlu64(u1, u0, v);

	diff = uneg ^ vneg;	  /* Negate q if signs of */
	q = (q ^ diff) - diff;	  /* u and v differed. */

	if ((diff ^ q) < 0 && q != 0) {	   /* If overflow, return the
					      largest */
	overflow:			   /* possible neg. quotient. */
		q = 0x8000000000000000ULL;
	}
	return q;
}

ssize_t tally_mean(const struct tally *tally) {
	size_t count = tally_num(tally);
	if (!count) {
		return 0;
	}

	if (sizeof(tally->total[0]) == sizeof(uint32_t)) {
		/* Use standard 64-bit arithmetic. */
		int64_t total = tally->total[0]
			| (((uint64_t)tally->total[1]) << 32);
		return total / count;
	}
	return divls64(tally->total[1], tally->total[0], count);
}

ssize_t tally_total(const struct tally *tally, ssize_t *overflow) {
	if (overflow) {
		*overflow = tally->total[1];
		return tally->total[0];
	}

	/* If result is negative, make sure we can represent it. */
	if (tally->total[1] & ((size_t)1 << (SIZET_BITS-1))) {
		/* Must have only underflowed once, and must be able to
		 * represent result at ssize_t. */
		if ((~tally->total[1])+1 != 0
		    || (ssize_t)tally->total[0] >= 0) {
			/* Underflow, return minimum. */
			return (ssize_t)((size_t)1 << (SIZET_BITS - 1));
		}
	} else {
		/* Result is positive, must not have overflowed, and must be
		 * able to represent as ssize_t. */
		if (tally->total[1] || (ssize_t)tally->total[0] < 0) {
			/* Overflow.  Return maximum. */
			return (ssize_t)~((size_t)1 << (SIZET_BITS - 1));
		}
	}
	return tally->total[0];
}

static ssize_t bucket_range(const struct tally *tally, unsigned b, size_t *err) {
	ssize_t min, max;

	min = bucket_min(tally->min, tally->step_bits, b);
	if (b == tally->buckets - 1) {
		max = tally->max;
	} else {
		max = bucket_min(tally->min, tally->step_bits, b+1) - 1;
	}

	/* FIXME: Think harder about cumulative error; is this enough?. */
	*err = (max - min + 1) / 2;
	/* Avoid overflow. */
	return min + (max - min) / 2;
}

ssize_t tally_approx_median(const struct tally *tally, size_t *err) {
	size_t count = tally_num(tally), total = 0;
	unsigned int i;

	for (i = 0; i < tally->buckets; i++) {
		total += tally->counts[i];
		if (total * 2 >= count) {
			break;
		}
	}
	return bucket_range(tally, i, err);
}

ssize_t tally_approx_mode(const struct tally *tally, size_t *err) {
	unsigned int i, min_best = 0, max_best = 0;

	for (i = 0; i < tally->buckets; i++) {
		if (tally->counts[i] > tally->counts[min_best]) {
			min_best = max_best = i;
		} else if (tally->counts[i] == tally->counts[min_best]) {
			max_best = i;
		}
	}

	/* We can have more than one best, making our error huge. */
	if (min_best != max_best) {
		ssize_t min, max;
		min = bucket_range(tally, min_best, err);
		max = bucket_range(tally, max_best, err);
		max += *err;
		*err += (size_t)(max - min);
		return min + (max - min) / 2;
	}

	return bucket_range(tally, min_best, err);
}

static unsigned get_max_bucket(const struct tally *tally) {
	unsigned int i;

	for (i = tally->buckets; i > 0; i--) {
		if (tally->counts[i-1]) {
			break;
		}
	}
	return i;
}

char *tally_histogram(const struct tally *tally,
		      unsigned width, unsigned height) {
	unsigned int i, count, max_bucket, largest_bucket;
	struct tally *tmp;
	char *graph, *p;

	assert(width >= TALLY_MIN_HISTO_WIDTH);
	assert(height >= TALLY_MIN_HISTO_HEIGHT);

	/* Ignore unused buckets. */
	max_bucket = get_max_bucket(tally);

	/* FIXME: It'd be nice to smooth here... */
	if (height >= max_bucket) {
		height = max_bucket;
		tmp = NULL;
	} else {
		/* We create a temporary then renormalize so < height. */
		/* FIXME: Antialias properly! */
		tmp = tally_new(tally->buckets);
		if (!tmp) {
			return NULL;
		}
		tmp->min = tally->min;
		tmp->max = tally->max;
		tmp->step_bits = tally->step_bits;
		memcpy(tmp->counts, tally->counts,
		       sizeof(tally->counts[0]) * tmp->buckets);
		while ((max_bucket = get_max_bucket(tmp)) >= height) {
			renormalize(tmp, tmp->min, tmp->max * 2);
		}
		/* Restore max */
		tmp->max = tally->max;
		tally = tmp;
		height = max_bucket;
	}

	/* Figure out longest line, for scale. */
	largest_bucket = 0;
	for (i = 0; i < tally->buckets; i++) {
		if (tally->counts[i] > largest_bucket) {
			largest_bucket = tally->counts[i];
		}
	}

	p = graph = (char *)malloc(height * (width + 1) + 1);
	if (!graph) {
		free(tmp);
		return NULL;
	}

	for (i = 0; i < height; i++) {
		unsigned covered = 1, row;

		/* People expect minimum at the bottom. */
		row = height - i - 1;
		count = (double)tally->counts[row] / largest_bucket * (width-1)+1;

		if (row == 0) {
			covered = snprintf(p, width, "%zi", tally->min);
		} else if (row == height - 1) {
			covered = snprintf(p, width, "%zi", tally->max);
		} else if (row == bucket_of(tally->min, tally->step_bits, 0)) {
			*p = '+';
		} else {
			*p = '|';
		}

		if (covered > width) {
			covered = width;
		}
		p += covered;

		if (count > covered) {
			count -= covered;
			memset(p, '*', count);
		} else {
			count = 0;
		}

		p += count;
		*p = '\n';
		p++;
	}
	*p = '\0';
	free(tmp);
	return graph;
}

/*
	Path: wucfua!wucs1!uunet!lll-winken!lll-tis!ames!mailrus!ulowell!page
	From: page@swan.ulowell.edu (Bob Page)
	Newsgroups: comp.sources.amiga
	Subject: v02i085:  regexp - regular-expression routines, Part01/02
	Message-ID: <10484@swan.ulowell.edu>
	Date: 5 Dec 88 22:05:51 GMT
	Organization: University of Lowell, Computer Science Dept.
	Lines: 736
	Approved: page@swan.ulowell.edu

	Submitted-by: grwalter@watcgl.waterloo.edu
	Posting-number: Volume 2, Issue 85
	Archive-name: unix/regexp.1

This is a reimplementation of the Unix V8 regexp(3) package.  It gives
C programs the ability to use egrep-style regular expressions, and
does it in a much cleaner fashion than the analogous routines in SysV.

This is a nearly-public-domain reimplementation of the V8 regexp(3) package.
It gives C programs the ability to use egrep-style regular expressions, and
does it in a much cleaner fashion than the analogous routines in SysV.

	Copyright (c) 1986 by University of Toronto.
	Written by Henry Spencer.  Not derived from licensed software.

	Permission is granted to anyone to use this software for any
	purpose on any computer system, and to redistribute it freely,
	subject to the following restrictions:

	1. The author is not responsible for the consequences of use of
		this software, no matter how awful, even if they arise
		from defects in it.

	2. The origin of this software must not be misrepresented, either
		by explicit claim or by omission.

	3. Altered versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

Barring a couple of small items in the BUGS list, this implementation is
believed 100% compatible with V8.  It should even be binary-compatible,
sort of, since the only fields in a "struct regexp" that other people have
any business touching are declared in exactly the same way at the same
location in the struct (the beginning).

This implementation is *NOT* AT&T/Bell code, and is not derived from licensed
software.  Even though U of T is a V8 licensee.  This software is based on
a V8 manual page sent to me by Dennis Ritchie (the manual page enclosed
here is a complete rewrite and hence is not covered by AT&T copyright).
The software was nearly complete at the time of arrival of our V8 tape.
I haven't even looked at V8 yet, although a friend elsewhere at U of T has
been kind enough to run a few test programs using the V8 regexp(3) to resolve
a few fine points.  I admit to some familiarity with regular-expression
implementations of the past, but the only one that this code traces any
ancestry to is the one published in Kernighan & Plauger (from which this
one draws ideas but not code).

Simplistically:  put this stuff into a source directory, copy regexp.h into
/usr/include, inspect Makefile for compilation options that need changing
to suit your local environment, and then do "make r".  This compiles the
regexp(3) functions, compiles a test program, and runs a large set of
regression tests.  If there are no complaints, then put regexp.o, regsub.o,
and regerror.o into your C library, and regexp.3 into your manual-pages
directory.

Note that if you don't put regexp.h into /usr/include *before* compiling,
you'll have to add "-I." to CFLAGS before compiling.

The files are:

Makefile	instructions to make everything
regexp.3	manual page
regexp.h	header file, for /usr/include
regexp.c	source for regcomp() and regexec()
regsub.c	source for regsub()
regerror.c	source for default regerror()
regmagic.h	internal header file
try.c		source for test program
timer.c		source for timing program
tests		test list for try and timer

This implementation uses nondeterministic automata rather than the
deterministic ones found in some other implementations, which makes it
simpler, smaller, and faster at compiling regular expressions, but slower
at executing them.  In theory, anyway.  This implementation does employ
some special-case optimizations to make the simpler cases (which do make
up the bulk of regular expressions actually used) run quickly.  In general,
if you want blazing speed you're in the wrong place.  Replacing the insides
of egrep with this stuff is probably a mistake; if you want your own egrep
you're going to have to do a lot more work.  But if you want to use regular
expressions a little bit in something else, you're in luck.  Note that many
existing text editors use nondeterministic regular-expression implementations,
so you're in good company.

This stuff should be pretty portable, given appropriate option settings.
If your chars have less than 8 bits, you're going to have to change the
internal representation of the automaton, although knowledge of the details
of this is fairly localized.  There are no "reserved" char values except for
NUL, and no special significance is attached to the top bit of chars.
The string(3) functions are used a fair bit, on the grounds that they are
probably faster than coding the operations in line.  Some attempts at code
tuning have been made, but this is invariably a bit machine-specific.

	Path: wucfua!wucs1!uunet!lll-winken!lll-tis!ames!mailrus!ulowell!page
	From: page@swan.ulowell.edu (Bob Page)
	Newsgroups: comp.sources.amiga
	Subject: v02i086:  regexp - regular-expression routines, Part02/02
	Message-ID: <10485@swan.ulowell.edu>
	Date: 5 Dec 88 22:06:40 GMT
	Organization: University of Lowell, Computer Science Dept.
	Lines: 1696
	Approved: page@swan.ulowell.edu

	Submitted-by: grwalter@watcgl.waterloo.edu       
	Posting-number: Volume 2, Issue 86
	Archive-name: unix/regexp.2
*/

#include <stdio.h>

#undef	END

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#define NSUBEXP  32

void regerror(const char* s) {
#ifdef ERRAVAIL
    error("regexp: %s", s);
#else
    fprintf(stderr, "regexp(3): %s", s);
    exit(1);
#endif
    /* NOTREACHED */
}

regexp* regcomp(char* exp);
//char *exp;

int regexec(regexp* prog, char* string);
//regexp *prog;
//char *string;

void regsub(regexp* prog, char* source, char* dest);
//regexp *prog;
//char *source;
//char *dest;

void regerror(const char* msg);
//char *msg;

/*
 * regcomp and regexec -- regsub and regerror are elsewhere @(#)regexp.c 1.3
 * of 18 April 87 
 *
 * Copyright (c) 1986 by University of Toronto. Written by Henry Spencer.  Not
 * derived from licensed software. 
 *
 * Permission is granted to anyone to use this software for any purpose on any
 * computer system, and to redistribute it freely, subject to the following
 * restrictions: 
 *
 * 1. The author is not responsible for the consequences of use of this
 * software, no matter how awful, even if they arise from defects in it. 
 *
 * 2. The origin of this software must not be misrepresented, either by explicit
 * claim or by omission. 
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 * misrepresented as being the original software. 
 *
 * Beware that some of this code is subtly aware of the way operator precedence
 * is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink. 
 */

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	0234

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are: 
 *
 * regstart	char that must begin a match; '\0' if none obvious reganch
 * is the match anchored (at beginning-of-line only)? regmust	string
 * (pointer into program) that match must include, or NULL regmlen
 * length of regmust string 
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway. 
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding of
 * a nondeterministic finite-state machine (aka syntax charts or "railroad
 * normal form" in parsing technology).  Each node is an opcode plus a "next"
 * pointer, possibly plus an operand.  "Next" pointers of all nodes except
 * BRANCH implement concatenation; a "next" pointer with a BRANCH on both
 * ends of it is connecting two alternatives.	(Here we have one of the
 * subtle syntax dependencies:	an individual BRANCH (as opposed to a
 * collection of them) is never concatenated with anything because of
 * operator precedence.)  The operand of some types of node is a literal
 * string; for others, it is a node leading into a sub-FSM.  In particular,
 * the operand of a BRANCH node is the first node of the branch. (NB this is
 * *not* a tree structure:  the tail of the branch connects to the thing
 * following the set of BRANCHes.)  The opcodes are: 
 */

/* definition	number	opnd?	meaning */
#define END	0		/* no	End of program. */
#define BOL	1		/* no	Match "" at beginning of line. */
#define EOL	2		/* no	Match "" at end of line. */
#define ANY	3		/* no	Match any one character. */
#define ANYOF	4		/* str	Match any character in this string. */
#define ANYBUT	5		/* str	Match any character not in this
				 * string. */
#define BRANCH	6		/* node Match this alternative, or the
				 * next... */
#define BACK	7		/* no	Match "", "next" ptr points backward. */
#define EXACTLY 8		/* str	Match this string. */
#define NOTHING 9		/* no	Match empty string. */
#define STAR	10		/* node Match this (simple) thing 0 or more
				 * times. */
#define PLUS	11		/* node Match this (simple) thing 1 or more
				 * times. */
#define OPEN	20		/* no	Mark this point in input as start of
				 * #n. */
/* OPEN+1 is number 1, etc. */
#define CLOSE	30		/* no	Analogous to OPEN. */

/*
 * Opcode notes: 
 *
 * BRANCH	The set of branches constituting a single choice are hooked together
 * with their "next" pointers, since precedence prevents anything being
 * concatenated to any individual branch.  The "next" pointer of the last
 * BRANCH in a choice points to the thing following the whole choice.  This
 * is also where the final "next" pointer of each individual branch points;
 * each branch starts with the operand node of a BRANCH node. 
 *
 * BACK 	Normal "next" pointers all implicitly point forward; BACK exists to
 * make loop structures possible. 
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 * BRANCH structures using BACK.  Simple cases (one character per match) are
 * implemented with STAR and PLUS for speed and to minimize recursive
 * plunges. 
 *
 * OPEN,CLOSE	...are numbered at compile time. 
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it. An
 * operand, if any, simply follows the node.  (Note that much of the code
 * generation knows about this implicit relationship.) 
 *
 * Using two bytes for the "next" pointer is vast overkill for most things, but
 * allows patterns to get big without disasters. 
 */
#undef NEXT
#define OP(p)   (*(p))
#define NEXT(p) (((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define OPERAND(p)      ((p) + 3)

/*
 * See regmagic.h for one further detail of program structure. 
 */


/*
 * Utility definitions. 
 */
#ifndef CHARBITS
#define UCHARAT(p)      ((int)*(unsigned char *)(p))
#else
#define UCHARAT(p)      ((int)*(p)&CHARBITS)
#endif

#define FAIL(m) { regerror((char *)m); return(NULL); }
#define ISMULT(c)       ((c) == '*' || (c) == '+' || (c) == '?')
#define META	"^$.[()|?+*\\"

/*
 * Flags to be passed up and down. 
 */
#define HASWIDTH	01	/* Known never to match null string. */
#define SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define SPSTART 	04	/* Starts with * or +. */
#define WORST		0	/* Worst case. */

/*
 * Global work variables for regcomp(). 
 */
static char    *regparse;	/* Input-scan pointer. */
static int      regnpar;	/* () count. */
static char     regdummy;
static char    *regcode;	/* Code-emit pointer; &regdummy = don't. */
static long     regsize;	/* Code size. */

/*
 * Forward declarations for regcomp()'s friends. 
 */
#ifndef STATIC
#define STATIC	static
#endif
STATIC char    *reg(int, int *);
STATIC char    *regbranch(int *);
STATIC char    *regpiece(int *);
STATIC char    *regatom(int *);
STATIC char    *regnode(char);
STATIC char    *regnext(char *);
STATIC void     regc(char);
STATIC void     reginsert(char, char*);
STATIC void     regtail(char *, char *);
STATIC void     regoptail(char *, char *);
#ifdef STRCSPN
int             strcspn(char *, char *);
#endif

/*
 * - regcomp - compile a regular expression into internal code 
 *
 * We can't allocate space until we know how big the compiled form will be, but
 * we can't compile it (and thus know how big it is) until we've got a place
 * to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the code
 * and thus invalidate pointers into it.  (Note that it has to be in one
 * piece because free() must be able to free it all.) 
 *
 * Beware that the optimization-preparation code in here knows about some of the
 * structure of the compiled regexp. 
 */
regexp         *
regcomp(char* exp)
//    char           *exp;
{
    register regexp *r;
    register char  *scan;
    register char  *longest;
    register int    len;
    int             flags;
//    extern char    *malloc();

    if (exp == NULL)
	FAIL("NULL argument");

    /* First pass: determine size, legality. */
    regparse = exp;
    regnpar = 1;
    regsize = 0L;
    regcode = &regdummy;
    regc(MAGIC);
    if (reg(0, &flags) == NULL)
	return (NULL);

    /* Small enough for pointer-storage convention? */
    if (regsize >= 32767L)	/* Probably could be 65535L. */
	FAIL("regexp too big");

    /* Allocate space. */
    r = (regexp *) malloc(sizeof(regexp) + (unsigned) regsize);
    if (r == NULL)
	FAIL("out of space");

    /* Second pass: emit code. */
    regparse = exp;
    regnpar = 1;
    regcode = r->program;
    regc(MAGIC);
    if (reg(0, &flags) == NULL)
	return (NULL);

    /* Dig out information for optimizations. */
    r->regstart = '\0';		/* Worst-case defaults. */
    r->reganch = 0;
    r->regmust = NULL;
    r->regmlen = 0;
    scan = r->program + 1;	/* First BRANCH. */
    if (OP(regnext(scan)) == END) {	/* Only one top-level choice. */
	scan = OPERAND(scan);

	/* Starting-point info. */
	if (OP(scan) == EXACTLY)
	    r->regstart = *OPERAND(scan);
	else if (OP(scan) == BOL)
	    r->reganch++;

	/*
	 * If there's something expensive in the r.e., find the longest
	 * literal string that must appear and make it the regmust.  Resolve
	 * ties in favor of later strings, since the regstart check works
	 * with the beginning of the r.e. and avoiding duplication
	 * strengthens checking.  Not a strong reason, but sufficient in the
	 * absence of others. 
	 */
	if (flags & SPSTART) {
	    longest = NULL;
	    len = 0;
	    for (; scan != NULL; scan = regnext(scan))
		if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
		    longest = OPERAND(scan);
		    len = strlen(OPERAND(scan));
		}
	    r->regmust = longest;
	    r->regmlen = len;
	}
    }
    return (r);
}

/*
 * - reg - regular expression, i.e. main body or parenthesized thing 
 *
 * Caller must absorb opening parenthesis. 
 *
 * Combining parenthesis handling with the base level of regular expression is a
 * trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid. 
 */
static char    *
reg(int paren, int* flagp)
//    int             paren;	/* Parenthesized? */
//    int            *flagp;
{
    register char  *ret;
    register char  *br;
    register char  *ender;
    register int    parno;
    int             flags;

    *flagp = HASWIDTH;		/* Tentatively. */

    /* Make an OPEN node, if parenthesized. */
    if (paren) {
	if (regnpar >= NSUBEXP)
	    FAIL("too many ()");
	parno = regnpar;
	regnpar++;
	ret = regnode(OPEN + parno);
    } else
	ret = NULL;

    /* Pick up the branches, linking them together. */
    br = regbranch(&flags);
    if (br == NULL)
	return (NULL);
    if (ret != NULL)
	regtail(ret, br);	/* OPEN -> first. */
    else
	ret = br;
    if (!(flags & HASWIDTH))
	*flagp &= ~HASWIDTH;
    *flagp |= flags & SPSTART;
    while (*regparse == '|') {
	regparse++;
	br = regbranch(&flags);
	if (br == NULL)
	    return (NULL);
	regtail(ret, br);	/* BRANCH -> BRANCH. */
	if (!(flags & HASWIDTH))
	    *flagp &= ~HASWIDTH;
	*flagp |= flags & SPSTART;
    }

    /* Make a closing node, and hook it on the end. */
    ender = regnode((paren) ? CLOSE + parno : END);
    regtail(ret, ender);

    /* Hook the tails of the branches to the closing node. */
    for (br = ret; br != NULL; br = regnext(br))
	regoptail(br, ender);

    /* Check for proper termination. */
    if (paren && *regparse++ != ')') {
	FAIL("unmatched ()");
    } else if (!paren && *regparse != '\0') {
	if (*regparse == ')') {
	    FAIL("unmatched ()");
	} else
	    FAIL("junk on end");/* "Can't happen". */
	/* NOTREACHED */
    }
    return (ret);
}

/*
 * - regbranch - one alternative of an | operator 
 *
 * Implements the concatenation operator. 
 */
static char    *
regbranch(int* flagp)
//    int            *flagp;
{
    register char  *ret;
    register char  *chain;
    register char  *latest;
    int             flags;

    *flagp = WORST;		/* Tentatively. */

    ret = regnode(BRANCH);
    chain = NULL;
    while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
	latest = regpiece(&flags);
	if (latest == NULL)
	    return (NULL);
	*flagp |= flags & HASWIDTH;
	if (chain == NULL)	/* First piece. */
	    *flagp |= flags & SPSTART;
	else
	    regtail(chain, latest);
	chain = latest;
    }
    if (chain == NULL)		/* Loop ran zero times. */
	(void) regnode(NOTHING);

    return (ret);
}

/*
 * - regpiece - something followed by possible [*+?] 
 *
 * Note that the branching code sequences used for ? and the general cases of *
 * and + are somewhat optimized:  they use the same NOTHING node as both the
 * endmarker for their branch list and the body of the last branch. It might
 * seem that this node could be dispensed with entirely, but the endmarker
 * role is not redundant. 
 */
static char    *
regpiece(int* flagp)
//    int            *flagp;
{
    register char  *ret;
    register char   op;
    register char  *next;
    int             flags;

    ret = regatom(&flags);
    if (ret == NULL)
	return (NULL);

    op = *regparse;
    if (!ISMULT(op)) {
	*flagp = flags;
	return (ret);
    }
    if (!(flags & HASWIDTH) && op != '?')
	FAIL("*+ operand could be empty");
    *flagp = (op != '+') ? (WORST | SPSTART) : (WORST | HASWIDTH);

    if (op == '*' && (flags & SIMPLE))
	reginsert(STAR, ret);
    else if (op == '*') {
	/* Emit x* as (x&|), where & means "self". */
	reginsert(BRANCH, ret);	/* Either x */
	regoptail(ret, regnode(BACK));	/* and loop */
	regoptail(ret, ret);	/* back */
	regtail(ret, regnode(BRANCH));	/* or */
	regtail(ret, regnode(NOTHING));	/* null. */
    } else if (op == '+' && (flags & SIMPLE))
	reginsert(PLUS, ret);
    else if (op == '+') {
	/* Emit x+ as x(&|), where & means "self". */
	next = regnode(BRANCH);	/* Either */
	regtail(ret, next);
	regtail(regnode(BACK), ret);	/* loop back */
	regtail(next, regnode(BRANCH));	/* or */
	regtail(ret, regnode(NOTHING));	/* null. */
    } else if (op == '?') {
	/* Emit x? as (x|) */
	reginsert(BRANCH, ret);	/* Either x */
	regtail(ret, regnode(BRANCH));	/* or */
	next = regnode(NOTHING);/* null. */
	regtail(ret, next);
	regoptail(ret, next);
    }
    regparse++;
    if (ISMULT(*regparse))
	FAIL("nested *?+");

    return (ret);
}

/*
 * - regatom - the lowest level 
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that it
 * can turn them into a single node, which is smaller to store and faster to
 * run.  Backslashed characters are exceptions, each becoming a separate
 * node; the code is simpler that way and it's not worth fixing. 
 */
static char    *
regatom(int* flagp)
//    int            *flagp;
{
    register char  *ret;
    int             flags;

    *flagp = WORST;		/* Tentatively. */

    switch (*regparse++) {
    case '^':
	ret = regnode(BOL);
	break;
    case '$':
	ret = regnode(EOL);
	break;
    case '.':
	ret = regnode(ANY);
	*flagp |= HASWIDTH | SIMPLE;
	break;
    case '[':{
	    register int    _class;
	    register int    classend;

	    if (*regparse == '^') {	/* Complement of range. */
		ret = regnode(ANYBUT);
		regparse++;
	    } else
		ret = regnode(ANYOF);
	    if (*regparse == ']' || *regparse == '-')
		regc(*regparse++);
	    while (*regparse != '\0' && *regparse != ']') {
		if (*regparse == '-') {
		    regparse++;
		    if (*regparse == ']' || *regparse == '\0')
			regc('-');
		    else {
			_class = UCHARAT(regparse - 2) + 1;
			classend = UCHARAT(regparse);
			if (_class > classend + 1)
			    FAIL("invalid [] range");
			for (; _class <= classend; _class++)
			    regc(_class);
			regparse++;
		    }
		} else
		    regc(*regparse++);
	    }
	    regc('\0');
	    if (*regparse != ']')
		FAIL("unmatched []");
	    regparse++;
	    *flagp |= HASWIDTH | SIMPLE;
	}
	break;
    case '(':
	ret = reg(1, &flags);
	if (ret == NULL)
	    return (NULL);
	*flagp |= flags & (HASWIDTH | SPSTART);
	break;
    case '\0':
    case '|':
    case ')':
	FAIL("internal urp");	/* Supposed to be caught earlier. */
	break;
    case '?':
    case '+':
    case '*':
	FAIL("?+* follows nothing");
	break;
    case '\\':
	if (*regparse == '\0')
	    FAIL("trailing \\");
	ret = regnode(EXACTLY);
	regc(*regparse++);
	regc('\0');
	*flagp |= HASWIDTH | SIMPLE;
	break;
    default:{
	    register int    len;
	    register char   ender;

	    regparse--;
	    len = strcspn(regparse, META);
	    if (len <= 0)
		FAIL("internal disaster");
	    ender = *(regparse + len);
	    if (len > 1 && ISMULT(ender))
		len--;		/* Back off clear of ?+* operand. */
	    *flagp |= HASWIDTH;
	    if (len == 1)
		*flagp |= SIMPLE;
	    ret = regnode(EXACTLY);
	    while (len > 0) {
		regc(*regparse++);
		len--;
	    }
	    regc('\0');
	}
	break;
    }

    return (ret);
}

/*
 * - regnode - emit a node 
 */
static char    *		/* Location. */
regnode(char op)
//    char            op;
{
    register char  *ret;
    register char  *ptr;

    ret = regcode;
    if (ret == &regdummy) {
	regsize += 3;
	return (ret);
    }
    ptr = ret;
    *ptr++ = op;
    *ptr++ = '\0';		/* Null "next" pointer. */
    *ptr++ = '\0';
    regcode = ptr;

    return (ret);
}

/*
 * - regc - emit (if appropriate) a byte of code 
 */
static void
regc(char b)
//    char            b;
{
    if (regcode != &regdummy)
	*regcode++ = b;
    else
	regsize++;
}

/*
 * - reginsert - insert an operator in front of already-emitted operand 
 *
 * Means relocating the operand. 
 */
static void
reginsert(char op, char* opnd)
//    char            op;
//    char           *opnd;
{
    register char  *src;
    register char  *dst;
    register char  *place;

    if (regcode == &regdummy) {
	regsize += 3;
	return;
    }
    src = regcode;
    regcode += 3;
    dst = regcode;
    while (src > opnd)
	*--dst = *--src;

    place = opnd;		/* Op node, where operand used to be. */
    *place++ = op;
    *place++ = '\0';
    *place++ = '\0';
}

/*
 * - regtail - set the next-pointer at the end of a node chain 
 */
static void
regtail(char* p, char* val)
//    char           *p;
//    char           *val;
{
    register char  *scan;
    register char  *temp;
    register int    offset;

    if (p == &regdummy)
	return;

    /* Find last node. */
    scan = p;
    for (;;) {
	temp = regnext(scan);
	if (temp == NULL)
	    break;
	scan = temp;
    }

    if (OP(scan) == BACK)
	offset = scan - val;
    else
	offset = val - scan;
    *(scan + 1) = (offset >> 8) & 0377;
    *(scan + 2) = offset & 0377;
}

/*
 * - regoptail - regtail on operand of first argument; nop if operandless 
 */
static void
regoptail(char* p, char* val)
//    char           *p;
//    char           *val;
{
    /* "Operandless" and "op != BRANCH" are synonymous in practice. */
    if (p == NULL || p == &regdummy || OP(p) != BRANCH)
	return;
    regtail(OPERAND(p), val);
}

/*
 * regexec and friends 
 */

/*
 * Global work variables for regexec(). 
 */
static char    *reginput;	/* String-input pointer. */
static char    *regbol;		/* Beginning of input, for ^ check. */
static char   **regstartp;	/* Pointer to startp array. */
static char   **regendp;	/* Ditto for endp. */

/*
 * Forwards. 
 */
STATIC int      regtry(regexp*, char*);
STATIC int      regmatch(char*);
STATIC int      regrepeat(char*);

#ifdef DEBUG
int             regnarrate = 0;
void            regdump(regexp*);
STATIC char    *regprop(char*);
#endif

/*
 * - regexec - match a regexp against a string 
 */
int
regexec(regexp* prog, char* string)
//    register regexp *prog;
//    register char  *string;
{
    register char  *s;

    /* Be paranoid... */
    if (prog == NULL || string == NULL) {
	regerror("NULL parameter");
	return (0);
    }
    /* Check validity of program. */
    if (UCHARAT(prog->program) != MAGIC) {
	regerror("corrupted program");
	return (0);
    }
    /* If there is a "must appear" string, look for it. */
    if (prog->regmust != NULL) {
	s = string;
	while ((s = strchr(s, prog->regmust[0])) != NULL) {
	    if (strncmp(s, prog->regmust, prog->regmlen) == 0)
		break;		/* Found it. */
	    s++;
	}
	if (s == NULL)		/* Not present. */
	    return (0);
    }
    /* Mark beginning of line for ^ . */
    regbol = string;

    /* Simplest case:  anchored match need be tried only once. */
    if (prog->reganch)
	return (regtry(prog, string));

    /* Messy cases:  unanchored match. */
    s = string;
    if (prog->regstart != '\0')
	/* We know what char it must start with. */
	while ((s = strchr(s, prog->regstart)) != NULL) {
	    if (regtry(prog, s))
		return (1);
	    s++;
	}
    else
	/* We don't -- general case. */
	do {
	    if (regtry(prog, s))
		return (1);
	} while (*s++ != '\0');

    /* Failure. */
    return (0);
}

/*
 * - regtry - try match at specific point 
 */
static int			/* 0 failure, 1 success */
regtry(regexp* prog, char* string)
//    regexp         *prog;
//    char           *string;
{
    register int    i;
    register char **sp;
    register char **ep;

    reginput = string;
    regstartp = prog->startp;
    regendp = prog->endp;

    sp = prog->startp;
    ep = prog->endp;
    for (i = NSUBEXP; i > 0; i--) {
	*sp++ = NULL;
	*ep++ = NULL;
    }
    if (regmatch(prog->program + 1)) {
	prog->startp[0] = string;
	prog->endp[0] = reginput;
	return (1);
    } else
	return (0);
}

/*
 * - regmatch - main matching routine 
 *
 * Conceptually the strategy is simple:  check to see whether the current node
 * matches, call self recursively to see whether the rest matches, and then
 * act accordingly.  In practice we make some effort to avoid recursion, in
 * particular by going through "ordinary" nodes (that don't need to know
 * whether the rest of the match failed) by a loop instead of by recursion. 
 */
static int			/* 0 failure, 1 success */
regmatch(char* prog)
//    char           *prog;
{
    register char  *scan;	/* Current node. */
    char           *next;	/* Next node. */

    scan = prog;
#ifdef DEBUG
    if (scan != NULL && regnarrate)
	fprintf(stderr, "%s(\n", regprop(scan));
#endif
    while (scan != NULL) {
#ifdef DEBUG
	if (regnarrate)
	    fprintf(stderr, "%s...\n", regprop(scan));
#endif
	next = regnext(scan);

	switch (OP(scan)) {
	case BOL:
	    if (reginput != regbol)
		return (0);
	    break;
	case EOL:
	    if (*reginput != '\0')
		return (0);
	    break;
	case ANY:
	    if (*reginput == '\0')
		return (0);
	    reginput++;
	    break;
	case EXACTLY:{
		register int    len;
		register char  *opnd;

		opnd = OPERAND(scan);
		/* Inline the first character, for speed. */
		if (*opnd != *reginput)
		    return (0);
		len = strlen(opnd);
		if (len > 1 && strncmp(opnd, reginput, len) != 0)
		    return (0);
		reginput += len;
	    }
	    break;
	case ANYOF:
	    if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) == NULL)
		return (0);
	    reginput++;
	    break;
	case ANYBUT:
	    if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) != NULL)
		return (0);
	    reginput++;
	    break;
	case NOTHING:
	    break;
	case BACK:
	    break;
	case OPEN + 1:
	case OPEN + 2:
	case OPEN + 3:
	case OPEN + 4:
	case OPEN + 5:
	case OPEN + 6:
	case OPEN + 7:
	case OPEN + 8:
	case OPEN + 9:{
		register int    no;
		register char  *save;

		no = OP(scan) - OPEN;
		save = reginput;

		if (regmatch(next)) {
		    /*
		     * Don't set startp if some later invocation of the same
		     * parentheses already has. 
		     */
		    if (regstartp[no] == NULL)
			regstartp[no] = save;
		    return (1);
		} else
		    return (0);
	    }
	    break;
	case CLOSE + 1:
	case CLOSE + 2:
	case CLOSE + 3:
	case CLOSE + 4:
	case CLOSE + 5:
	case CLOSE + 6:
	case CLOSE + 7:
	case CLOSE + 8:
	case CLOSE + 9:{
		register int    no;
		register char  *save;

		no = OP(scan) - CLOSE;
		save = reginput;

		if (regmatch(next)) {
		    /*
		     * Don't set endp if some later invocation of the same
		     * parentheses already has. 
		     */
		    if (regendp[no] == NULL)
			regendp[no] = save;
		    return (1);
		} else
		    return (0);
	    }
	    break;
	case BRANCH:{
		register char  *save;

		if (OP(next) != BRANCH)	/* No choice. */
		    next = OPERAND(scan);	/* Avoid recursion. */
		else {
		    do {
			save = reginput;
			if (regmatch(OPERAND(scan)))
			    return (1);
			reginput = save;
			scan = regnext(scan);
		    } while (scan != NULL && OP(scan) == BRANCH);
		    return (0);
		    /* NOTREACHED */
		}
	    }
	    break;
	case STAR:
	case PLUS:{
		register char   nextch;
		register int    no;
		register char  *save;
		register int    min;

		/*
		 * Lookahead to avoid useless match attempts when we know
		 * what character comes next. 
		 */
		nextch = '\0';
		if (OP(next) == EXACTLY)
		    nextch = *OPERAND(next);
		min = (OP(scan) == STAR) ? 0 : 1;
		save = reginput;
		no = regrepeat(OPERAND(scan));
		while (no >= min) {
		    /* If it could work, try it. */
		    if (nextch == '\0' || *reginput == nextch)
			if (regmatch(next))
			    return (1);
		    /* Couldn't or didn't -- back up. */
		    no--;
		    reginput = save + no;
		}
		return (0);
	    }
	    break;
	case END:
	    return (1);		/* Success! */
	    break;
	default:
	    regerror("memory corruption");
	    return (0);
	    break;
	}

	scan = next;
    }

    /*
     * We get here only if there's trouble -- normally "case END" is the
     * terminating point. 
     */
    regerror("corrupted pointers");
    return (0);
}

/*
 * - regrepeat - repeatedly match something simple, report how many 
 */
static int
regrepeat(char* p)
//    char           *p;
{
    register int    count = 0;
    register char  *scan;
    register char  *opnd;

    scan = reginput;
    opnd = OPERAND(p);
    switch (OP(p)) {
    case ANY:
	count = strlen(scan);
	scan += count;
	break;
    case EXACTLY:
	while (*opnd == *scan) {
	    count++;
	    scan++;
	}
	break;
    case ANYOF:
	while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
	    count++;
	    scan++;
	}
	break;
    case ANYBUT:
	while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
	    count++;
	    scan++;
	}
	break;
    default:			/* Oh dear.  Called inappropriately. */
	regerror("internal foulup");
	count = 0;		/* Best compromise. */
	break;
    }
    reginput = scan;

    return (count);
}

/*
 * - regnext - dig the "next" pointer out of a node 
 */
static char    *
regnext(char* p)
//    register char  *p;
{
    register int    offset;

    if (p == &regdummy)
	return (NULL);

    offset = NEXT(p);
    if (offset == 0)
	return (NULL);

    if (OP(p) == BACK)
	return (p - offset);
    else
	return (p + offset);
}

#ifdef DEBUG

STATIC char    *regprop();

/*
 * - regdump - dump a regexp onto stdout in vaguely comprehensible form 
 */
void
regdump(regexp* r)
//    regexp         *r;
{
    register char  *s;
    register char   op = EXACTLY;	/* Arbitrary non-END op. */
    register char  *next;

    s = r->program + 1;
    while (op != END) {		/* While that wasn't END last time... */
	op = OP(s);
	printf("%2d%s", s - r->program, regprop(s));	/* Where, what. */
	next = regnext(s);
	if (next == NULL)	/* Next ptr. */
	    printf("(0)");
	else
	    printf("(%d)", (s - r->program) + (next - s));
	s += 3;
	if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
	    /* Literal string, where present. */
	    while (*s != '\0') {
		putchar(*s);
		s++;
	    }
	    s++;
	}
	putchar('\n');
    }

    /* Header fields of interest. */
    if (r->regstart != '\0')
	printf("start `%c' ", r->regstart);
    if (r->reganch)
	printf("anchored ");
    if (r->regmust != NULL)
	printf("must have \"%s\"", r->regmust);
    printf("\n");
}

/*
 * - regprop - printable representation of opcode 
 */
static char    *
regprop(char* op)
//    char           *op;
{
    register char  *p;
    static char     buf[50];

    (void) strcpy(buf, ":");

    switch (OP(op)) {
    case BOL:
	p = "BOL";
	break;
    case EOL:
	p = "EOL";
	break;
    case ANY:
	p = "ANY";
	break;
    case ANYOF:
	p = "ANYOF";
	break;
    case ANYBUT:
	p = "ANYBUT";
	break;
    case BRANCH:
	p = "BRANCH";
	break;
    case EXACTLY:
	p = "EXACTLY";
	break;
    case NOTHING:
	p = "NOTHING";
	break;
    case BACK:
	p = "BACK";
	break;
    case END:
	p = "END";
	break;
    case OPEN + 1:
    case OPEN + 2:
    case OPEN + 3:
    case OPEN + 4:
    case OPEN + 5:
    case OPEN + 6:
    case OPEN + 7:
    case OPEN + 8:
    case OPEN + 9:
	sprintf(buf + strlen(buf), "OPEN%d", OP(op) - OPEN);
	p = NULL;
	break;
    case CLOSE + 1:
    case CLOSE + 2:
    case CLOSE + 3:
    case CLOSE + 4:
    case CLOSE + 5:
    case CLOSE + 6:
    case CLOSE + 7:
    case CLOSE + 8:
    case CLOSE + 9:
	sprintf(buf + strlen(buf), "CLOSE%d", OP(op) - CLOSE);
	p = NULL;
	break;
    case STAR:
	p = "STAR";
	break;
    case PLUS:
	p = "PLUS";
	break;
    default:
	regerror("corrupted opcode");
	break;
    }
    if (p != NULL)
	(void) strcat(buf, p);
    return (buf);
}
#endif

/*
 * The following is provided for those people who do not have strcspn() in
 * their C libraries.  They should get off their butts and do something about
 * it; at least one public-domain implementation of those (highly useful)
 * string routines has been published on Usenet. 
 */
#ifdef STRCSPN
/*
 * strcspn - find length of initial segment of s1 consisting entirely of
 * characters not from s2 
 */

static int
strcspn(char* s1, char* s2)
//    char           *s1;
//    char           *s2;
{
    register char  *scan1;
    register char  *scan2;
    register int    count;

    count = 0;
    for (scan1 = s1; *scan1 != '\0'; scan1++) {
	for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
	    if (*scan1 == *scan2++)
		return (count);
	count++;
    }
    return (count);
}
#endif

/*
 * regsub @(#)regsub.c	1.3 of 2 April 86 
 *
 * Copyright (c) 1986 by University of Toronto. Written by Henry Spencer.  Not
 * derived from licensed software. 
 *
 * Permission is granted to anyone to use this software for any purpose on any
 * computer system, and to redistribute it freely, subject to the following
 * restrictions: 
 *
 * 1. The author is not responsible for the consequences of use of this
 * software, no matter how awful, even if they arise from defects in it. 
 *
 * 2. The origin of this software must not be misrepresented, either by explicit
 * claim or by omission. 
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 * misrepresented as being the original software. 
 */

#ifndef CHARBITS
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARBITS)
#endif

/*
 * - regsub - perform substitutions after a regexp match 
 */
void
regsub(regexp* prog, char* source, char* dest)
//    regexp         *prog;
//    char           *source;
//    char           *dest;
{
    register char  *src;
    register char  *dst;
    register char   c;
    register int    no;
    register int    len;

    if (prog == NULL || source == NULL || dest == NULL) {
	regerror("NULL parm to regsub");
	return;
    }
    if (UCHARAT(prog->program) != MAGIC) {
	regerror("damaged regexp fed to regsub");
	return;
    }
    src = source;
    dst = dest;
    while ((c = *src++) != '\0') {
	if (c == '&')
	    no = 0;
	else if (c == '\\' && '0' <= *src && *src <= '9')
	    no = *src++ - '0';
	else
	    no = -1;

	if (no < 0) {		/* Ordinary character. */
	    if (c == '\\' && (*src == '\\' || *src == '&'))
		c = *src++;
	    *dst++ = c;
	} else if (prog->startp[no] != NULL && prog->endp[no] != NULL) {
	    len = prog->endp[no] - prog->startp[no];
	    (void) strncpy(dst, prog->startp[no], len);
	    dst += len;
	    if (len != 0 && *(dst - 1) == '\0') {	/* strncpy hit NUL. */
		regerror("damaged match string");
		return;
	    }
	}
    }
    *dst++ = '\0';
}

#undef MAGIC

/*
 * zlib License
 *
 * Regularized Incomplete Beta Function
 *
 * Copyright (c) 2016, 2017 Lewis Van Winkle
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#define STOP 1.0e-8
#define TINY 1.0e-30

double incbeta(double a, double b, double x) {
    if (x < 0.0 || x > 1.0) return 1.0/0.0;

    /*The continued fraction converges nicely for x < (a+1)/(a+b+2)*/
    if (x > (a+1.0)/(a+b+2.0)) {
        return (1.0-incbeta(b,a,1.0-x)); /*Use the fact that beta is symmetrical.*/
    }

    /*Find the first part before the continued fraction.*/
    const double lbeta_ab = lgamma(a)+lgamma(b)-lgamma(a+b);
    const double front = exp(log(x)*a+log(1.0-x)*b-lbeta_ab) / a;

    /*Use Lentz's algorithm to evaluate the continued fraction.*/
    double f = 1.0, c = 1.0, d = 0.0;

    int i, m;
    for (i = 0; i <= 200; ++i) {
        m = i/2;

        double numerator;
        if (i == 0) {
            numerator = 1.0; /*First numerator is 1.0.*/
        } else if (i % 2 == 0) {
            numerator = (m*(b-m)*x)/((a+2.0*m-1.0)*(a+2.0*m)); /*Even term.*/
        } else {
            numerator = -((a+m)*(a+b+m)*x)/((a+2.0*m)*(a+2.0*m+1)); /*Odd term.*/
        }

        /*Do an iteration of Lentz's algorithm.*/
        d = 1.0 + numerator * d;
        if (fabs(d) < TINY) d = TINY;
        d = 1.0 / d;

        c = 1.0 + numerator / c;
        if (fabs(c) < TINY) c = TINY;

        const double cd = c*d;
        f *= cd;

        /*Check for stop.*/
        if (fabs(1.0-cd) < STOP) {
            return front * (f-1.0);
        }
    }

    return 1.0/0.0; /*Needed more loops, did not converge.*/
}

#undef STOP
#undef TINY

double student_t_cdf(double t, double v) {
    /*The cumulative distribution function (CDF) for Student's t distribution*/
    double x = (t + sqrt(t * t + v)) / (2.0 * sqrt(t * t + v));
    double prob = incbeta(v/2.0, v/2.0, x);
    return prob;
}

/*** end extern.cpp ***/