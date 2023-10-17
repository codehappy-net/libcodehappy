/***

	extern.h

	Declarations and macros for miscellaneous functions from extern.cpp.

	Part of libcodehappy, by C. M. Street.

***/
#ifndef __EXTERN_H__
#define __EXTERN_H__

extern int32_t ymd_to_jdn(int y, int m, int d, int julian, int papal);
extern void jdn_to_ymd(long jdn, int *yy, int *mm, int *dd, int julian, int papal); 	

/* C snippets library */
extern size_t commafmt(char* buf, int bufsize, int64_t N);
extern unsigned short crc16(char *data_p, unsigned short length);
extern void solve_cubic(double a, double b, double c, double d, int* nsol, double* roots);
extern const char *eng_format(double value, int places);
extern long hexorint(const char *string);
extern int isisbn(const char *str);

extern void lv1ws(char *str);
extern char *rmallws(char *str);
extern int moon_age(int month, int day, int year);
extern float MSBINToIEEE(float f);
extern float IEEEToMSBIN(float f);
extern int permutation_index(char pit[], int size);

extern int scanfrac (const char buf[], double *f);
extern char *soundex(char *instr, char *outstr);
extern char *str27seg(char *string);
extern char *strreplace_us(char *Str, char *OldStr, char *NewStr);
extern char *translate_string_c_literal(char *string);
extern char *trim_whitespace(char *str);

extern const char *ordinal_text_suffix(int number);
extern char *rmlead(char *str);
extern char *rmtrail(char *str);
extern void easter(int year,int *easter_month, int *easter_day);

/* Solar time functions -- incl. sunrise/sunset calculation */

/* Following are some macros around the "workhorse" function __daylen__ */
/* They mainly fill in the desired values for the reference altitude    */
/* below the horizon, and also selects whether this altitude should     */
/* refer to the Sun's center or its upper limb.                         */


/* This macro computes the length of the day, from sunrise to sunset. */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define day_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -35.0/60.0, 1 )

/* This macro computes the length of the day, including civil twilight. */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define day_civil_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -6.0, 0 )

/* This macro computes the length of the day, incl. nautical twilight.  */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define day_nautical_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -12.0, 0 )

/* This macro computes the length of the day, incl. astronomical twilight. */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define day_astronomical_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -18.0, 0 )


/* This macro computes times for sunrise/sunset.                      */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define sun_rise_set(year,month,day,lon,lat,rise,set)  \
        __sunriset__( year, month, day, lon, lat, -35.0/60.0, 1, rise, set )

/* This macro computes the start and end times of civil twilight.       */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define civil_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -6.0, 0, start, end )

/* This macro computes the start and end times of nautical twilight.    */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define nautical_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -12.0, 0, start, end )

/* This macro computes the start and end times of astronomical twilight.   */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define astronomical_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -18.0, 0, start, end )

extern double __daylen__( int year, int month, int day, double lon, double lat,
                   double altit, int upper_limb );

extern int __sunriset__( int year, int month, int day, double lon, double lat,
                  double altit, int upper_limb, double *rise, double *set );

extern void sunpos(double d, double *lon, double *r);
extern void sun_RA_dec(double d, double *RA, double *dec, double *r);
extern double revolution(double x);
extern double rev180(double x);
extern double GMST0(double d);

/* EBCDIC / ASCII conversion */

char ebcdic_to_ascii(const char c);
char ascii_to_ebcdic(const char c);
void cstr_ebcdic_to_ascii(char* c);
void cstr_ascii_to_ebcdic(char* c);

/* libtable */

#ifndef LIBTABLE_H_INCLUDED
#define LIBTABLE_H_INCLUDED

/***

libtable

libtable is a tiny C library for generating pretty-printed ascii tables.

Example usage

#include <table.h>

int main(void) {
        struct table t;

        table_init(
                &t,
                "Name",  "%s",
                "Age",   "%d",
                "Score", "%.2f",
                NULL
        );

        table_add(&t, "Amet fugiat commodi eligendi possimus harum earum. "
                      "Sequi quidem ab commodi tempore mollitia provident. "
                      "Iusto incidunt consequuntur rem eligendi illum. "
                      "Nisi odit soluta dolorum vero enim neque id. Hic magni? "
                      "foo bar baz", 36, 3.1);
        table_add(&t, "Ametfugiatcommodieligendipossimusharumearum."
                      "Sequiquidemabcommoditemporemollitiaprovident."
                      "Iustoinciduntconsequunturremeligendiillum."
                      "Nisioditsolutadolorumveroenimnequeid.Hicmagni?"
                      "foobarbaz",36,3.1);
        table_add(&t, "?es?ep???t??????f???aßde???µ?a", 36, 3.1);
        table_add(&t, "?es?ep??? t?? ????f???a ßde???µ?a", 36, 3.1);
        table_add(&t, "Bob", 18, 1.3123);
        table_add(&t, "Alice", 20, 6.43);
        table_add(&t, "Roger", 18, 12.45);
        table_add(&t, "Larry", 59, 12.52);
        table_add(&t, "? ? ? ? ? ? ? ? ?", 21, 14.12312312);

        table_print(&t, 60, stdout);

        table_free(&t);

        return 0;
}

Output

+--------------------------------------------------------+
| Name                                     | Age | Score |
*--------------------------------------------------------*
| Amet fugiat commodi eligendi possimus    | 36  | 3.10  |
| harum earum. Sequi quidem ab commodi     |     |       |
| tempore mollitia provident. Iusto        |     |       |
| incidunt consequuntur rem eligendi       |     |       |
| illum. Nisi odit soluta dolorum vero     |     |       |
| enim neque id. Hic magni? foo bar baz    |     |       |
|------------------------------------------|-----|-------|
| Ametfugiatcommodieligendipossimusharume- | 36  | 3.10  |
| arum.Sequiquidemabcommoditemporemolliti- |     |       |
| aprovident.Iustoinciduntconsequunturrem- |     |       |
| eligendiillum.Nisioditsolutadolorumvero- |     |       |
| enimnequeid.Hicmagni?foobarbaz           |     |       |
|------------------------------------------|-----|-------|
| ?es?ep???t??????f???aßde???µ?a           | 36  | 3.10  |
|------------------------------------------|-----|-------|
| ?es?ep??? t?? ????f???a ßde???µ?a        | 36  | 3.10  |
|------------------------------------------|-----|-------|
| Bob                                      | 18  | 1.31  |
|------------------------------------------|-----|-------|
| Alice                                    | 20  | 6.43  |
|------------------------------------------|-----|-------|
| Roger                                    | 18  | 12.45 |
|------------------------------------------|-----|-------|
| Larry                                    | 59  | 12.52 |
|------------------------------------------|-----|-------|
| ? ? ? ? ? ? ? ? ?                        | 21  | 14.12 |
+--------------------------------------------------------+

MIT license
by Bradley Garagan

***/

struct table {
        size_t cols;
        size_t rows;
        size_t alloc;
        size_t *max;
        char ***data;
        char **headers;
        char *fmt;
};

extern bool table_init(struct table *t, ...);
extern bool table_add(struct table *t, ...);
extern bool table_print(struct table const *t, size_t maxwidth, FILE *f);
extern void table_free(struct table *t);
extern void fputnc(int c, size_t n, FILE *f);

#endif

/* Create a temporary file. */

extern FILE *tmpfileplus(const char *dir, const char *prefix, const char* suffix, char **pathname, int keep);

#define TMPFILE_KEEP 1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibMd5
//
//  Implementation of MD5 hash function. Originally written by Alexander Peslyak. Modified by WaterJuice retaining
//  Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _LibMd5_h_
#define _LibMd5_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Md5Context - This must be initialised using Md5Initialised. Do not modify the contents of this structure directly.
struct Md5Context {
    uint32_t     lo;
    uint32_t     hi;
    uint32_t     a;
    uint32_t     b;
    uint32_t     c;
    uint32_t     d;
    uint8_t      buffer[64];
    uint32_t     block[16];
};

#define MD5_HASH_SIZE           ( 128 / 8 )

struct MD5_HASH {
    uint8_t      bytes [MD5_HASH_SIZE];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
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
    );

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
    );

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
    );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_LibMd5_h_

/* Levenshtein distance between two strings. */

/**
 * Returns an unsigned integer, depicting
 * the difference between `a` and `b`.
 *
 * See http://en.wikipedia.org/wiki/Levenshtein_distance
 * for more information.
 */
extern unsigned int levenshtein(const char *a, const char *b);

/* Bloom filters. */

/**
 * @file bloom-filter.h
 *
 * @brief Bloom filter
 *
 * A bloom filter is a space efficient data structure that can be
 * used to test whether a given element is part of a set.  Lookups
 * will occasionally generate false positives, but never false 
 * negatives. 
 *
 * To create a bloom filter, use @ref bloom_filter_new.  To destroy a 
 * bloom filter, use @ref bloom_filter_free.
 *
 * To insert a value into a bloom filter, use @ref bloom_filter_insert.
 *
 * To query whether a value is part of the set, use 
 * @ref bloom_filter_query.
 */
/*

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

/** 
 * A bloom filter structure.
 */

typedef struct _BloomFilter BloomFilter;

/**
 * A value stored in a @ref BloomFilter.
 */

typedef void *BloomFilterValue;

/**
 * Hash function used to generate hash values for values inserted into a 
 * bloom filter.
 *
 * @param data   The value to generate a hash value for.
 * @return       The hash value.
 */

typedef unsigned long (*BloomFilterHashFunc)(BloomFilterValue data);

/**
 * Create a new bloom filter.
 *
 * @param table_size       The size of the bloom filter.  The greater
 *                         the table size, the more elements can be
 *                         stored, and the lesser the chance of false
 *                         positives.
 * @param hash_func        Hash function to use on values stored in the
 *                         filter.
 * @param num_functions    Number of hash functions to apply to each
 *                         element on insertion.  This running time for
 *                         insertion and queries is proportional to this
 *                         value.  The more functions applied, the lesser
 *                         the chance of false positives.  The maximum
 *                         number of functions is 64.
 * @return                 A new hash table structure, or NULL if it 
 *                         was not possible to allocate the new bloom
 *                         filter.
 */

extern BloomFilter *bloom_filter_new(unsigned int table_size, 
                              BloomFilterHashFunc hash_func,
                              unsigned int num_functions);

/**
 * Destroy a bloom filter.
 *
 * @param bloomfilter      The bloom filter to destroy.
 */

extern void bloom_filter_free(BloomFilter *bloomfilter);

/**
 * Insert a value into a bloom filter.
 *
 * @param bloomfilter          The bloom filter.
 * @param value                The value to insert.
 */

extern void bloom_filter_insert(BloomFilter *bloomfilter, BloomFilterValue value);

/**
 * Query a bloom filter for a particular value.
 *
 * @param bloomfilter          The bloom filter.
 * @param value                The value to look up.
 * @return                     Zero if the value was definitely not 
 *                             inserted into the filter.  Non-zero
 *                             indicates that it either may or may not
 *                             have been inserted.
 */

extern int bloom_filter_query(BloomFilter *bloomfilter, BloomFilterValue value);

/**
 * Read the contents of a bloom filter into an array.
 *
 * @param bloomfilter          The bloom filter.
 * @param array                Pointer to the array to read into.  This
 *                             should be (table_size + 7) / 8 bytes in 
 *                             length.
 */

extern void bloom_filter_read(BloomFilter *bloomfilter, unsigned char *array);

/**
 * Load the contents of a bloom filter from an array.
 * The data loaded should be the output read from @ref bloom_filter_read,
 * from a bloom filter created using the same arguments used to create
 * the original filter.
 *
 * @param bloomfilter          The bloom filter.
 * @param array                Pointer to the array to load from.  This 
 *                             should be (table_size + 7) / 8 bytes in 
 *                             length.
 */

extern void bloom_filter_load(BloomFilter *bloomfilter, unsigned char *array);

/** 
 * Find the union of two bloom filters.  Values are present in the 
 * resulting filter if they are present in either of the original 
 * filters.
 *
 * Both of the original filters must have been created using the 
 * same parameters to @ref bloom_filter_new.
 *
 * @param filter1              The first filter.
 * @param filter2              The second filter.
 * @return                     A new filter which is an intersection of the
 *                             two filters, or NULL if it was not possible
 *                             to allocate memory for the new filter, or
 *                             if the two filters specified were created
 *                             with different parameters. 
 */

extern BloomFilter *bloom_filter_union(BloomFilter *filter1, 
                                BloomFilter *filter2);

/** 
 * Find the intersection of two bloom filters.  Values are only ever 
 * present in the resulting filter if they are present in both of the
 * original filters.
 *
 * Both of the original filters must have been created using the 
 * same parameters to @ref bloom_filter_new.
 *
 * @param filter1              The first filter.
 * @param filter2              The second filter.
 * @return                     A new filter which is an intersection of the
 *                             two filters, or NULL if it was not possible
 *                             to allocate memory for the new filter, or
 *                             if the two filters specified were created
 *                             with different parameters. 
 */

extern BloomFilter *bloom_filter_intersection(BloomFilter *filter1, 
                                       BloomFilter *filter2);

//
// SpookyHash: a 128-bit noncryptographic hash function
// By Bob Jenkins, public domain
//   Oct 31 2010: alpha, framework + SpookyHash::Mix appears right
//   Oct 31 2011: alpha again, Mix only good to 2^^69 but rest appears right
//   Dec 31 2011: beta, improved Mix, tested it for 2-bit deltas
//   Feb  2 2012: production, same bits as beta
//   Feb  5 2012: adjusted definitions of uint* to be more portable
//   Mar 30 2012: 3 bytes/cycle, not 4.  Alpha was 4 but wasn't thorough enough.
//   August 5 2012: SpookyV2 (different results)
// 
// Up to 3 bytes/cycle for long messages.  Reasonably fast for short messages.
// All 1 or 2 bit deltas achieve avalanche within 1% bias per output bit.
//
// This was developed for and tested on 64-bit x86-compatible processors.
// It assumes the processor is little-endian.  There is a macro
// controlling whether unaligned reads are allowed (by default they are).
// This should be an equally good hash on big-endian machines, but it will
// compute different results on them than on little-endian machines.
//
// Google's CityHash has similar specs to SpookyHash, and CityHash is faster
// on new Intel boxes.  MD4 and MD5 also have similar specs, but they are orders
// of magnitude slower.  CRCs are two or more times slower, but unlike 
// SpookyHash, they have nice math for combining the CRCs of pieces to form 
// the CRCs of wholes.  There are also cryptographic hashes, but those are even 
// slower than MD5.
//

#include <stddef.h>

#ifdef _MSC_VER
# define INLINE __forceinline
  typedef  unsigned __int64 uint64;
  typedef  unsigned __int32 uint32;
  typedef  unsigned __int16 uint16;
  typedef  unsigned __int8  uint8;
#else
# include <stdint.h>
# define INLINE inline
  typedef  uint64_t  uint64;
  typedef  uint32_t  uint32;
  typedef  uint16_t  uint16;
  typedef  uint8_t   uint8;
#endif


class SpookyHash
{
public:
    //
    // SpookyHash: hash a single message in one call, produce 128-bit output
    //
    static void Hash128(
        const void *message,  // message to hash
        size_t length,        // length of message in bytes
        uint64 *hash1,        // in/out: in seed 1, out hash value 1
        uint64 *hash2);       // in/out: in seed 2, out hash value 2

    //
    // Hash64: hash a single message in one call, return 64-bit output
    //
    static uint64 Hash64(
        const void *message,  // message to hash
        size_t length,        // length of message in bytes
        uint64 seed)          // seed
    {
        uint64 hash1 = seed;
        Hash128(message, length, &hash1, &seed);
        return hash1;
    }

    //
    // Hash32: hash a single message in one call, produce 32-bit output
    //
    static uint32 Hash32(
        const void *message,  // message to hash
        size_t length,        // length of message in bytes
        uint32 seed)          // seed
    {
        uint64 hash1 = seed, hash2 = seed;
        Hash128(message, length, &hash1, &hash2);
        return (uint32)hash1;
    }

    //
    // Init: initialize the context of a SpookyHash
    //
    void Init(
        uint64 seed1,       // any 64-bit value will do, including 0
        uint64 seed2);      // different seeds produce independent hashes
    
    //
    // Update: add a piece of a message to a SpookyHash state
    //
    void Update(
        const void *message,  // message fragment
        size_t length);       // length of message fragment in bytes


    //
    // Final: compute the hash for the current SpookyHash state
    //
    // This does not modify the state; you can keep updating it afterward
    //
    // The result is the same as if SpookyHash() had been called with
    // all the pieces concatenated into one message.
    //
    void Final(
        uint64 *hash1,    // out only: first 64 bits of hash value.
        uint64 *hash2);   // out only: second 64 bits of hash value.

    //
    // left rotate a 64-bit value by k bytes
    //
    static INLINE uint64 Rot64(uint64 x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }

    //
    // This is used if the input is 96 bytes long or longer.
    //
    // The internal state is fully overwritten every 96 bytes.
    // Every input bit appears to cause at least 128 bits of entropy
    // before 96 other bytes are combined, when run forward or backward
    //   For every input bit,
    //   Two inputs differing in just that input bit
    //   Where "differ" means xor or subtraction
    //   And the base value is random
    //   When run forward or backwards one Mix
    // I tried 3 pairs of each; they all differed by at least 212 bits.
    //
    static INLINE void Mix(
        const uint64 *data, 
        uint64 &s0, uint64 &s1, uint64 &s2, uint64 &s3,
        uint64 &s4, uint64 &s5, uint64 &s6, uint64 &s7,
        uint64 &s8, uint64 &s9, uint64 &s10,uint64 &s11)
    {
      s0 += data[0];    s2 ^= s10;    s11 ^= s0;    s0 = Rot64(s0,11);    s11 += s1;
      s1 += data[1];    s3 ^= s11;    s0 ^= s1;    s1 = Rot64(s1,32);    s0 += s2;
      s2 += data[2];    s4 ^= s0;    s1 ^= s2;    s2 = Rot64(s2,43);    s1 += s3;
      s3 += data[3];    s5 ^= s1;    s2 ^= s3;    s3 = Rot64(s3,31);    s2 += s4;
      s4 += data[4];    s6 ^= s2;    s3 ^= s4;    s4 = Rot64(s4,17);    s3 += s5;
      s5 += data[5];    s7 ^= s3;    s4 ^= s5;    s5 = Rot64(s5,28);    s4 += s6;
      s6 += data[6];    s8 ^= s4;    s5 ^= s6;    s6 = Rot64(s6,39);    s5 += s7;
      s7 += data[7];    s9 ^= s5;    s6 ^= s7;    s7 = Rot64(s7,57);    s6 += s8;
      s8 += data[8];    s10 ^= s6;    s7 ^= s8;    s8 = Rot64(s8,55);    s7 += s9;
      s9 += data[9];    s11 ^= s7;    s8 ^= s9;    s9 = Rot64(s9,54);    s8 += s10;
      s10 += data[10];    s0 ^= s8;    s9 ^= s10;    s10 = Rot64(s10,22);    s9 += s11;
      s11 += data[11];    s1 ^= s9;    s10 ^= s11;    s11 = Rot64(s11,46);    s10 += s0;
    }

    //
    // Mix all 12 inputs together so that h0, h1 are a hash of them all.
    //
    // For two inputs differing in just the input bits
    // Where "differ" means xor or subtraction
    // And the base value is random, or a counting value starting at that bit
    // The final result will have each bit of h0, h1 flip
    // For every input bit,
    // with probability 50 +- .3%
    // For every pair of input bits,
    // with probability 50 +- 3%
    //
    // This does not rely on the last Mix() call having already mixed some.
    // Two iterations was almost good enough for a 64-bit result, but a
    // 128-bit result is reported, so End() does three iterations.
    //
    static INLINE void EndPartial(
        uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3,
        uint64 &h4, uint64 &h5, uint64 &h6, uint64 &h7, 
        uint64 &h8, uint64 &h9, uint64 &h10,uint64 &h11)
    {
        h11+= h1;    h2 ^= h11;   h1 = Rot64(h1,44);
        h0 += h2;    h3 ^= h0;    h2 = Rot64(h2,15);
        h1 += h3;    h4 ^= h1;    h3 = Rot64(h3,34);
        h2 += h4;    h5 ^= h2;    h4 = Rot64(h4,21);
        h3 += h5;    h6 ^= h3;    h5 = Rot64(h5,38);
        h4 += h6;    h7 ^= h4;    h6 = Rot64(h6,33);
        h5 += h7;    h8 ^= h5;    h7 = Rot64(h7,10);
        h6 += h8;    h9 ^= h6;    h8 = Rot64(h8,13);
        h7 += h9;    h10^= h7;    h9 = Rot64(h9,38);
        h8 += h10;   h11^= h8;    h10= Rot64(h10,53);
        h9 += h11;   h0 ^= h9;    h11= Rot64(h11,42);
        h10+= h0;    h1 ^= h10;   h0 = Rot64(h0,54);
    }

    static INLINE void End(
        const uint64 *data, 
        uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3,
        uint64 &h4, uint64 &h5, uint64 &h6, uint64 &h7, 
        uint64 &h8, uint64 &h9, uint64 &h10,uint64 &h11)
    {
        h0 += data[0];   h1 += data[1];   h2 += data[2];   h3 += data[3];
        h4 += data[4];   h5 += data[5];   h6 += data[6];   h7 += data[7];
        h8 += data[8];   h9 += data[9];   h10 += data[10]; h11 += data[11];
        EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        EndPartial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
    }

    //
    // The goal is for each bit of the input to expand into 128 bits of 
    //   apparent entropy before it is fully overwritten.
    // n trials both set and cleared at least m bits of h0 h1 h2 h3
    //   n: 2   m: 29
    //   n: 3   m: 46
    //   n: 4   m: 57
    //   n: 5   m: 107
    //   n: 6   m: 146
    //   n: 7   m: 152
    // when run forwards or backwards
    // for all 1-bit and 2-bit diffs
    // with diffs defined by either xor or subtraction
    // with a base of all zeros plus a counter, or plus another bit, or random
    //
    static INLINE void ShortMix(uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3)
    {
        h2 = Rot64(h2,50);  h2 += h3;  h0 ^= h2;
        h3 = Rot64(h3,52);  h3 += h0;  h1 ^= h3;
        h0 = Rot64(h0,30);  h0 += h1;  h2 ^= h0;
        h1 = Rot64(h1,41);  h1 += h2;  h3 ^= h1;
        h2 = Rot64(h2,54);  h2 += h3;  h0 ^= h2;
        h3 = Rot64(h3,48);  h3 += h0;  h1 ^= h3;
        h0 = Rot64(h0,38);  h0 += h1;  h2 ^= h0;
        h1 = Rot64(h1,37);  h1 += h2;  h3 ^= h1;
        h2 = Rot64(h2,62);  h2 += h3;  h0 ^= h2;
        h3 = Rot64(h3,34);  h3 += h0;  h1 ^= h3;
        h0 = Rot64(h0,5);   h0 += h1;  h2 ^= h0;
        h1 = Rot64(h1,36);  h1 += h2;  h3 ^= h1;
    }

    //
    // Mix all 4 inputs together so that h0, h1 are a hash of them all.
    //
    // For two inputs differing in just the input bits
    // Where "differ" means xor or subtraction
    // And the base value is random, or a counting value starting at that bit
    // The final result will have each bit of h0, h1 flip
    // For every input bit,
    // with probability 50 +- .3% (it is probably better than that)
    // For every pair of input bits,
    // with probability 50 +- .75% (the worst case is approximately that)
    //
    static INLINE void ShortEnd(uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3)
    {
        h3 ^= h2;  h2 = Rot64(h2,15);  h3 += h2;
        h0 ^= h3;  h3 = Rot64(h3,52);  h0 += h3;
        h1 ^= h0;  h0 = Rot64(h0,26);  h1 += h0;
        h2 ^= h1;  h1 = Rot64(h1,51);  h2 += h1;
        h3 ^= h2;  h2 = Rot64(h2,28);  h3 += h2;
        h0 ^= h3;  h3 = Rot64(h3,9);   h0 += h3;
        h1 ^= h0;  h0 = Rot64(h0,47);  h1 += h0;
        h2 ^= h1;  h1 = Rot64(h1,54);  h2 += h1;
        h3 ^= h2;  h2 = Rot64(h2,32);  h3 += h2;
        h0 ^= h3;  h3 = Rot64(h3,25);  h0 += h3;
        h1 ^= h0;  h0 = Rot64(h0,63);  h1 += h0;
    }
    
private:

    //
    // Short is used for messages under 192 bytes in length
    // Short has a low startup cost, the normal mode is good for long
    // keys, the cost crossover is at about 192 bytes.  The two modes were
    // held to the same quality bar.
    // 
    static void Short(
        const void *message,  // message (array of bytes, not necessarily aligned)
        size_t length,        // length of message (in bytes)
        uint64 *hash1,        // in/out: in the seed, out the hash value
        uint64 *hash2);       // in/out: in the seed, out the hash value

    // number of uint64's in internal state
    static const size_t sc_numVars = 12;

    // size of the internal state
    static const size_t sc_blockSize = sc_numVars*8;

    // size of buffer of unhashed data, in bytes
    static const size_t sc_bufSize = 2*sc_blockSize;

    //
    // sc_const: a constant which:
    //  * is not zero
    //  * is odd
    //  * is a not-very-regular mix of 1's and 0's
    //  * does not need any other special mathematical properties
    //
    static const uint64 sc_const = 0xdeadbeefdeadbeefLL;

    uint64 m_data[2*sc_numVars];   // unhashed data, for partial messages
    uint64 m_state[sc_numVars];  // internal state of the hash
    size_t m_length;             // total length of the input so far
    uint8  m_remainder;          // length of unhashed data stashed in m_data
};

/* id3.h

   Header file for reading and writing ID3 tags
   By Matt Craven
   (c)1999/2000 Hedgehog Software
*/

#define ID3_GENRE_MAX 150	/* Count of ID3 genres */

/* ID3 tag structure - contains all the fields of the ID3 tag */

struct ID3_TAG {
   char title[31];
   char artist[31];
   char album[31];
   char year[5];
   char comment[30];
   char track;
   char genre;
};

/* Declare the array of ID3 genres */
extern const char *id3_genre_string[ID3_GENRE_MAX];

/* Function prototypes */
extern int id3_read(char *filename, ID3_TAG *infotag);
extern int id3_write(char *filename, ID3_TAG tag);
extern void id3_tag_to_string(ID3_TAG tag, char *txt);

/**
 * ilog - Integer logarithm.
 *
 * ilog_32() and ilog_64() compute the minimum number of bits required to store
 * an unsigned 32-bit or 64-bit value without any leading zero bits.
 *
 * This can also be thought of as the location of the highest set bit, with
 * counting starting from one (so that 0 returns 0, 1 returns 1, and 2**31
 * returns 32).
 *
 * When the value is known to be non-zero ilog32_nz() and ilog64_nz() can
 * compile into as few as two instructions, one of which may get optimized out
 * later.
 *
 * STATIC_ILOG_32 and STATIC_ILOG_64 allow computation on compile-time
 * constants, so other compile-time constants can be derived from them.
 *
 * Example:
 *  #include <stdio.h>
 *  #include <limits.h>
 *  #include <ccan/ilog/ilog.h>
 *
 *  int main(void){
 *    int i;
 *    printf("ilog32(0x%08X)=%i\n",0,ilog32(0));
 *    for(i=1;i<=STATIC_ILOG_32(USHRT_MAX);i++){
 *      uint32_t v;
 *      v=(uint32_t)1U<<(i-1);
 *      //Here we know v is non-zero, so we can use ilog32_nz().
 *      printf("ilog32(0x%08X)=%i\n",v,ilog32_nz(v));
 *    }
 *    return 0;
 *  }
 *
 * License: CC0 (Public domain)
 * Author: Timothy B. Terriberry <tterribe@xiph.org>
 */



/* CC0 (Public domain) - see LICENSE file for details */
#if !defined(_ilog_H)
# define _ilog_H (1)

/**
 * ilog32 - Integer binary logarithm of a 32-bit value.
 * @_v: A 32-bit value.
 * Returns floor(log2(_v))+1, or 0 if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * Note that many uses will resolve to the fast macro version instead.
 *
 * See Also:
 *	ilog32_nz(), ilog64()
 *
 * Example:
 *	// Rounds up to next power of 2 (if not a power of 2).
 *	static uint32_t round_up32(uint32_t i)
 *	{
 *		assert(i != 0);
 *		return 1U << ilog32(i-1);
 *	}
 */
extern int ilog32(uint32_t _v);

/**
 * ilog32_nz - Integer binary logarithm of a non-zero 32-bit value.
 * @_v: A 32-bit value.
 * Returns floor(log2(_v))+1, or undefined if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * Note that many uses will resolve to the fast macro version instead.
 * See Also:
 *	ilog32(), ilog64_nz()
 * Example:
 *	// Find Last Set (ie. highest bit set, 0 to 31).
 *	static uint32_t fls32(uint32_t i)
 *	{
 *		assert(i != 0);
 *		return ilog32_nz(i) - 1;
 *	}
 */
extern int ilog32_nz(uint32_t _v);

/**
 * ilog64 - Integer binary logarithm of a 64-bit value.
 * @_v: A 64-bit value.
 * Returns floor(log2(_v))+1, or 0 if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * Note that many uses will resolve to the fast macro version instead.
 * See Also:
 *	ilog64_nz(), ilog32()
 */
extern int ilog64(uint64_t _v);

/**
 * ilog64_nz - Integer binary logarithm of a non-zero 64-bit value.
 * @_v: A 64-bit value.
 * Returns floor(log2(_v))+1, or undefined if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * Note that many uses will resolve to the fast macro version instead.
 * See Also:
 *	ilog64(), ilog32_nz()
 */
extern int ilog64_nz(uint64_t _v);

/**
 * STATIC_ILOG_32 - The integer logarithm of an (unsigned, 32-bit) constant.
 * @_v: A non-negative 32-bit constant.
 * Returns floor(log2(_v))+1, or 0 if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * This macro should only be used when you need a compile-time constant,
 * otherwise ilog32 or ilog32_nz are just as fast and more flexible.
 *
 * Example:
 *	#define MY_PAGE_SIZE	4096
 *	#define MY_PAGE_BITS	(STATIC_ILOG_32(PAGE_SIZE) - 1)
 */
#define STATIC_ILOG_32(_v) (STATIC_ILOG5((uint32_t)(_v)))

/**
 * STATIC_ILOG_64 - The integer logarithm of an (unsigned, 64-bit) constant.
 * @_v: A non-negative 64-bit constant.
 * Returns floor(log2(_v))+1, or 0 if _v==0.
 * This is the number of bits that would be required to represent _v in two's
 *  complement notation with all of the leading zeros stripped.
 * This macro should only be used when you need a compile-time constant,
 * otherwise ilog64 or ilog64_nz are just as fast and more flexible.
 */
#define STATIC_ILOG_64(_v) (STATIC_ILOG6((uint64_t)(_v)))

/* Private implementation details */

/*Note the casts to (int) below: this prevents "upgrading"
   the type of an entire expression to an (unsigned) size_t.*/
#if INT_MAX>=2147483647 && __GNUC__
#define builtin_ilog32_nz(v) \
	(((int)sizeof(unsigned)*CHAR_BIT) - __builtin_clz(v))
#elif LONG_MAX>=2147483647L && __GNUC__
#define builtin_ilog32_nz(v) \
	(((int)sizeof(unsigned)*CHAR_BIT) - __builtin_clzl(v))
#endif

#if INT_MAX>=9223372036854775807LL && __GNUC__
#define builtin_ilog64_nz(v) \
	(((int)sizeof(unsigned)*CHAR_BIT) - __builtin_clz(v))
#elif LONG_MAX>=9223372036854775807LL && __GNUC__
#define builtin_ilog64_nz(v) \
	(((int)sizeof(unsigned long)*CHAR_BIT) - __builtin_clzl(v))
#elif __GNUC__
#define builtin_ilog64_nz(v) \
	(((int)sizeof(unsigned long long)*CHAR_BIT) - __builtin_clzll(v))
#endif

#ifdef builtin_ilog32_nz
#define ilog32(_v) (builtin_ilog32_nz(_v)&-!!(_v))
#define ilog32_nz(_v) builtin_ilog32_nz(_v)
#else
#define ilog32_nz(_v) ilog32(_v)
#define ilog32(_v) (IS_COMPILE_CONSTANT(_v) ? STATIC_ILOG_32(_v) : ilog32(_v))
#endif /* builtin_ilog32_nz */

#ifdef builtin_ilog64_nz
#define ilog64(_v) (builtin_ilog64_nz(_v)&-!!(_v))
#define ilog64_nz(_v) builtin_ilog64_nz(_v)
#else
#define ilog64_nz(_v) ilog64(_v)
#define ilog64(_v) (IS_COMPILE_CONSTANT(_v) ? STATIC_ILOG_64(_v) : ilog64(_v))
#endif /* builtin_ilog64_nz */

/* Macros for evaluating compile-time constant ilog. */
# define STATIC_ILOG0(_v) (!!(_v))
# define STATIC_ILOG1(_v) (((_v)&0x2)?2:STATIC_ILOG0(_v))
# define STATIC_ILOG2(_v) (((_v)&0xC)?2+STATIC_ILOG1((_v)>>2):STATIC_ILOG1(_v))
# define STATIC_ILOG3(_v) \
 (((_v)&0xF0)?4+STATIC_ILOG2((_v)>>4):STATIC_ILOG2(_v))
# define STATIC_ILOG4(_v) \
 (((_v)&0xFF00)?8+STATIC_ILOG3((_v)>>8):STATIC_ILOG3(_v))
# define STATIC_ILOG5(_v) \
 (((_v)&0xFFFF0000)?16+STATIC_ILOG4((_v)>>16):STATIC_ILOG4(_v))
# define STATIC_ILOG6(_v) \
 (((_v)&0xFFFFFFFF00000000ULL)?32+STATIC_ILOG5((_v)>>32):STATIC_ILOG5(_v))

#endif /* _ilog_H */

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

#ifndef _LibSha1_h_
#define _LibSha1_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sha1Context - This must be initialised using Sha1Initialised. Do not modify the contents of this structure directly.
typedef struct
{
    uint32_t        State[5];
    uint32_t        Count[2];
    uint8_t         Buffer[64];
} Sha1Context;

#define SHA1_HASH_SIZE           ( 160 / 8 )

typedef struct
{
    uint8_t      bytes [SHA1_HASH_SIZE];
} SHA1_HASH;

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
    );

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
    );

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
    );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_LibSha1_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha256
//
//  Implementation of SHA256 hash function.
//  Original author: Tom St Denis, tomstdenis@gmail.com, http://libtom.org
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _LibSha256_h_
#define _LibSha256_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    uint64_t    length;
    uint32_t    state[8];
    uint32_t    curlen;
    uint8_t     buf[64];
} Sha256Context;

#define SHA256_HASH_SIZE           ( 256 / 8 )

typedef struct
{
    uint8_t      bytes [SHA256_HASH_SIZE];
} SHA256_HASH;

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
    );

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
    );

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
    );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_LibSha256_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha512
//
//  Implementation of SHA512 hash function.
//  Original author: Tom St Denis, tomstdenis@gmail.com, http://libtom.org
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _LibSha512_h_
#define _LibSha512_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint64_t    length;
    uint64_t    state[8];
    uint32_t    curlen;
    uint8_t     buf[128];
} Sha512Context;

#define SHA512_HASH_SIZE           ( 512 / 8 )

typedef struct {
    uint8_t      bytes [SHA512_HASH_SIZE];
} SHA512_HASH;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha512Initialise
//
//  Initialises a SHA512 Context. Use this to initialise/reset a context.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sha512Initialise
    (
        Sha512Context*          Context
    );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Sha512Update
//
//  Adds data to the SHA512 context. This will process the data and update the internal state of the context. Keep on
//  calling this function until all the data has been added. Then call Sha512Finalise to calculate the hash.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sha512Update
    (
        Sha512Context*      Context,
        void*               Buffer,
        uint32_t            BufferSize
    );

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
    );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_LibSha512_h_

/**
 * bdelta - Generate and apply binary deltas
 *
 * This library takes two strings containing binary data, and produces a
 * "patch" that can be applied to the first one to produce the second one.
 * It can be used to save bandwidth and disk space when many texts differing
 * by a small number of bytes need to be transmitted or stored.
 *
 * Patches produced by this version of the library can be applied using future
 * versions, but not past versions.
 *
 * bdelta implements the algorithm described in
 * An O(ND) Difference Algorithm and Its Variations by Eugene W. Myers.
 * Because its memory usage and expected running time are O(N + D^2),
 * it works well only when the strings differ by a small number of bytes.
 * This implementation stops trying when the strings differ by more than
 * 1000 bytes, and falls back to producing a patch that simply emits the new
 * string.
 *
 * Thus, bdelta does not save any space when given two strings that differ by
 * more than 1000 bytes.  This may be improved in a future version of the
 * library.
 *
 * Example:
 *	#include <ccan/bdelta/bdelta.h>
 *	#include <stdio.h>
 *	#include <stdlib.h>
 *	#include <string.h>
 *
 *	static void gulp(const char *filename, void **data_out, size_t *size_out);
 *
 *	static int usage(const char *prog)
 *	{
 *		fprintf(
 *			stderr,
 *			"Usage: %s diff  <old> <new>    >  <patch>\n"
 *			"       %s patch <old> <patch>  >  <new>\n",
 *			prog, prog
 *		);
 *		return 1;
 *	}
 *
 *	int main(int argc, char *argv[])
 *	{
 *		void *old, *new_, *patch;
 *		size_t old_size, new_size, patch_size;
 *		BDELTAcode rc;
 *
 *		if (argc != 4)
 *			return usage(argv[0]);
 *
 *		if (strcmp(argv[1], "diff") == 0) {
 *			gulp(argv[2], &old, &old_size);
 *			gulp(argv[3], &new_, &new_size);
 *
 *			rc = bdelta_diff(old, old_size, new_, new_size, &patch, &patch_size);
 *			if (rc != BDELTA_OK) {
 *				bdelta_perror("bdelta_diff", rc);
 *				return 1;
 *			}
 *
 *			if (fwrite(patch, 1, patch_size, stdout) != patch_size) {
 *				perror("stdout");
 *				return 1;
 *			}
 *		} else if (strcmp(argv[1], "patch") == 0) {
 *			gulp(argv[2], &old, &old_size);
 *			gulp(argv[3], &patch, &patch_size);
 *
 *			rc = bdelta_patch(old, old_size, patch, patch_size, &new_, &new_size);
 *			if (rc != BDELTA_OK) {
 *				bdelta_perror("bdelta_patch", rc);
 *				return 1;
 *			}
 *
 *			if (fwrite(new_, 1, new_size, stdout) != new_size) {
 *				perror("stdout");
 *				return 1;
 *			}
 *		} else {
 *			return usage(argv[0]);
 *		}
 *
 *		free(old);
 *		free(new_);
 *		free(patch);
 *		return 0;
 *	}
 *
 *	static void gulp(const char *filename, void **data_out, size_t *size_out)
 *	{
 *		FILE *f = fopen(filename, "rb");
 *		size_t size = 0;
 *		size_t alloc = 16;
 *		char *data = malloc(alloc);
 *
 *		if (f == NULL || data == NULL)
 *			goto error;
 *
 *		for (;;) {
 *			size += fread(data + size, 1, alloc - size, f);
 *			if (size < alloc) {
 *				if (!feof(f))
 *					goto error;
 *				break;
 *			}
 *			data = realloc(data, alloc *= 2);
 *			if (data == NULL)
 *				goto error;
 *		}
 *
 *		if (fclose(f) != 0)
 *			goto error;
 *
 *		*data_out = data;
 *		*size_out = size;
 *		return;
 *
 *	error:
 *		perror(filename);
 *		exit(EXIT_FAILURE);
 *	}
 *
 * Author: Joey Adams <joeyadams3.14159@gmail.com>
 * License: MIT
 * Version: 0.1.1
 */


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

#ifndef CCAN_BDELTA_H
#define CCAN_BDELTA_H

typedef enum {
	BDELTA_OK               = 0,  /* Operation succeeded. */

	BDELTA_MEMORY           = 1,  /* Memory allocation failed. */
	BDELTA_PATCH_INVALID    = 2,  /* Patch is malformed. */
	BDELTA_PATCH_MISMATCH   = 3,  /* Patch applied to wrong original string. */
	
	/* Internal error codes.  These will never be returned by API functions. */
	BDELTA_INTERNAL_DMAX_EXCEEDED    = -10,
	BDELTA_INTERNAL_INPUTS_TOO_LARGE = -11,
} BDELTAcode;

/*
 * bdelta_diff - Given two byte strings, generate a "patch" (also a byte string)
 * that describes how to transform the old string into the new string.
 *
 * On success, returns BDELTA_OK, and passes a malloc'd block
 * and its size through *patch_out and *patch_size_out.
 *
 * On failure, returns an error code, and clears *patch_out and *patch_size_out.
 *
 * Example:
 *	const char *old = "abcabba";
 *	const char *new_ = "cbabac";
 *	void *patch;
 *	size_t patch_size;
 *	BDELTAcode rc;
 *
 *	rc = bdelta_diff(old, strlen(old), new_, strlen(new_), &patch, &patch_size);
 *	if (rc != BDELTA_OK) {
 *		bdelta_perror("bdelta_diff", rc);
 *		return;
 *	}
 *	...
 *	free(patch);
 */
BDELTAcode bdelta_diff(
	const void  *old,       size_t  old_size,
	const void  *new_,      size_t  new_size,
	void       **patch_out, size_t *patch_size_out
);

/*
 * bdelta_patch - Apply a patch produced by bdelta_diff to the
 * old string to recover the new string.
 *
 * On success, returns BDELTA_OK, and passes a malloc'd block
 * and its size through *new_out and *new_size_out.
 *
 * On failure, returns an error code, and clears *new_out and *new_size_out.
 *
 * Example:
 *	const char *old = "abcabba";
 *	void *new_;
 *	size_t new_size;
 *	BDELTAcode rc;
 *
 *	rc = bdelta_patch(old, strlen(old), patch, patch_size, &new_, &new_size);
 *	if (rc != BDELTA_OK) {
 *		bdelta_perror("bdelta_patch", rc);
 *		return;
 *	}
 *	fwrite(new_, 1, new_size, stdout);
 *	putchar('\n');
 *	free(new_);
 */
BDELTAcode bdelta_patch(
	const void  *old,     size_t  old_size,
	const void  *patch,   size_t  patch_size,
	void       **new_out, size_t *new_size_out
);

/*
 * bdelta_strerror - Return a string describing a bdelta error code.
 */
const char *bdelta_strerror(BDELTAcode code);

/*
 * bdelta_perror - Print a bdelta error message to stderr.
 *
 * This function handles @s the same way perror does.
 */
void bdelta_perror(const char *s, BDELTAcode code);

#endif  // CCAN_BDELTA_H


/**
 * build_assert - routines for build-time assertions
 *
 * This code provides routines which will cause compilation to fail should some
 * assertion be untrue: such failures are preferable to run-time assertions,
 * but much more limited since they can only depends on compile-time constants.
 *
 * These assertions are most useful when two parts of the code must be kept in
 * sync: it is better to avoid such cases if possible, but seconds best is to
 * detect invalid changes at build time.
 *
 * For example, a tricky piece of code might rely on a certain element being at
 * the start of the structure.  To ensure that future changes don't break it,
 * you would catch such changes in your code like so:
 *
 * Example:
 *	#include <stddef.h>
 *	#include <ccan/build_assert/build_assert.h>
 *
 *	struct foo {
 *		char string[5];
 *		int x;
 *	};
 *
 *	static char *foo_string(struct foo *foo)
 *	{
 *		// This trick requires that the string be first in the structure
 *		BUILD_ASSERT(offsetof(struct foo, string) == 0);
 *		return (char *)foo;
 *	}
 *
 * License: CC0 (Public domain)
 * Author: Rusty Russell <rusty@rustcorp.com.au>
 */

/* CC0 (Public domain) - see LICENSE file for details */
#ifndef CCAN_BUILD_ASSERT_H
#define CCAN_BUILD_ASSERT_H

/**
 * BUILD_ASSERT - assert a build-time dependency.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can only be used within a function.
 *
 * Example:
 *	#include <stddef.h>
 *	...
 *	static char *foo_to_char(struct foo *foo)
 *	{
 *		// This code needs string to be at start of foo.
 *		BUILD_ASSERT(offsetof(struct foo, string) == 0);
 *		return (char *)foo;
 *	}
 */
#define BUILD_ASSERT(cond) \
	do { (void) sizeof(char [1 - 2*!(cond)]); } while(0)

/**
 * BUILD_ASSERT_OR_ZERO - assert a build-time dependency, as an expression.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can be used in an expression: its value is "0".
 *
 * Example:
 *	#define foo_to_char(foo)					\
 *		 ((char *)(foo)						\
 *		  + BUILD_ASSERT_OR_ZERO(offsetof(struct foo, string) == 0))
 */
#define BUILD_ASSERT_OR_ZERO(cond) \
	(sizeof(char [1 - 2*!(cond)]) - 1)

#endif /* CCAN_BUILD_ASSERT_H */

/**
 * cpuid - a CPUID instruction parser for x86/x86_64 CPUs.
 *
 * This module tries to keep-it-simple to get information about the CPU
 * from the CPU.
 *
 * Example:
 * #include <ccan/cpuid/cpuid.h>
 * #include <stdio.h>
 *
 * int main(void)
 * {
 * 	uint32_t highest;
 * 	cpuid(CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED, &highest);
 * 	printf("Highest extended function supported: %d\n", highest);
 *
 *      return 0;
 * }
 *
 * Author: Ahmed Samy <f.fallen45@gmail.com>
 * License: MIT
 * Version: 0.1
 */

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
 */
#ifndef CCAN_CPUID_H
#define CCAN_CPUID_H

/**
 * enum cpuid - stuff to get information about from the CPU.
 *
 * This is used as a parameter in cpuid().
 *
 * %CPUID_VENDORID:
 * 	The CPU's Vendor ID.
 *
 * %CPUID_PROCINFO_AND_FEATUREBITS:
 * 	Processor information and feature bits (SSE, etc.).
 *
 * %CPUID_CACHE_AND_TLBD_INFO
 * 	Cache and TLBD Information.
 * 	For AMD: Use CPUID_EXTENDED_L2_CACHE_FEATURES
 *
 * %CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED:
 * 	Highest extended function supported address.
 * 	Can be like 0x80000008.
 *
 * %CPUID_EXTENDED_PROC_INFO_FEATURE_BITS:
 * 	Extended processor information and feature bits (64bit etc.)
 *
 * %CPUID_PROC_BRAND_STRING:
 * 	The Processor's brand string.
 *
 * %CPUID_L1_CACHE_AND_TLB_IDS:
 * 	L1 Cache and TLB Identifications.
 *	AMD Only.
 *
 * %CPUID_EXTENDED_L2_CACHE_FEATURES:
 * 	Extended L2 Cache features.
 *
 * %CPUID_ADV_POWER_MGT_INFO:
 * 	Advaned power management information.
 *
 * %CPUID_VIRT_PHYS_ADDR_SIZES:
 * 	Virtual and physical address sizes.
 */

typedef enum cpuid {
	CPUID_VENDORID 					= 0,
	CPUID_PROCINFO_AND_FEATUREBITS 			= 1,
	CPUID_CACHE_AND_TLBD_INFO 			= 2,

	CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED 	= 0x80000000,
	CPUID_EXTENDED_PROC_INFO_FEATURE_BITS 		= 0x80000001,
	CPUID_PROC_BRAND_STRING 			= 0x80000002,
	CPUID_L1_CACHE_AND_TLB_IDS 			= 0x80000005,
	CPUID_EXTENDED_L2_CACHE_FEATURES 		= 0x80000006,
	CPUID_ADV_POWER_MGT_INFO 			= 0x80000007,
	CPUID_VIRT_PHYS_ADDR_SIZES 			= 0x80000008
} cpuid_t;

enum {
	CPUID_FEAT_ECX_SSE3         = 1 << 0, 
	CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
	CPUID_FEAT_ECX_DTES64       = 1 << 2,
	CPUID_FEAT_ECX_MONITOR      = 1 << 3,  
	CPUID_FEAT_ECX_DS_CPL       = 1 << 4,  
	CPUID_FEAT_ECX_VMX          = 1 << 5,  
	CPUID_FEAT_ECX_SMX          = 1 << 6,  
	CPUID_FEAT_ECX_EST          = 1 << 7,  
	CPUID_FEAT_ECX_TM2          = 1 << 8,  
	CPUID_FEAT_ECX_SSSE3        = 1 << 9,  
	CPUID_FEAT_ECX_CID          = 1 << 10,
	CPUID_FEAT_ECX_FMA          = 1 << 12,
	CPUID_FEAT_ECX_CX16         = 1 << 13, 
	CPUID_FEAT_ECX_ETPRD        = 1 << 14, 
	CPUID_FEAT_ECX_PDCM         = 1 << 15, 
	CPUID_FEAT_ECX_DCA          = 1 << 18, 
	CPUID_FEAT_ECX_SSE4_1       = 1 << 19, 
	CPUID_FEAT_ECX_SSE4_2       = 1 << 20, 
	CPUID_FEAT_ECX_x2APIC       = 1 << 21, 
	CPUID_FEAT_ECX_MOVBE        = 1 << 22, 
	CPUID_FEAT_ECX_POPCNT       = 1 << 23, 
	CPUID_FEAT_ECX_AES          = 1 << 25, 
	CPUID_FEAT_ECX_XSAVE        = 1 << 26, 
	CPUID_FEAT_ECX_OSXSAVE      = 1 << 27, 
	CPUID_FEAT_ECX_AVX          = 1 << 28,

	CPUID_FEAT_ECX_ALL 	    = CPUID_FEAT_ECX_SSE3 | CPUID_FEAT_ECX_PCLMUL | CPUID_FEAT_ECX_DTES64
					| CPUID_FEAT_ECX_MONITOR | CPUID_FEAT_ECX_DS_CPL | CPUID_FEAT_ECX_VMX
					| CPUID_FEAT_ECX_SMX | CPUID_FEAT_ECX_EST | CPUID_FEAT_ECX_TM2
					| CPUID_FEAT_ECX_SSSE3 | CPUID_FEAT_ECX_CID | CPUID_FEAT_ECX_FMA
					| CPUID_FEAT_ECX_CX16 | CPUID_FEAT_ECX_ETPRD | CPUID_FEAT_ECX_PDCM
					| CPUID_FEAT_ECX_DCA | CPUID_FEAT_ECX_SSE4_1 | CPUID_FEAT_ECX_SSE4_2
					| CPUID_FEAT_ECX_x2APIC | CPUID_FEAT_ECX_MOVBE | CPUID_FEAT_ECX_POPCNT
					| CPUID_FEAT_ECX_AES | CPUID_FEAT_ECX_XSAVE | CPUID_FEAT_ECX_OSXSAVE
					| CPUID_FEAT_ECX_AVX,

	CPUID_FEAT_EDX_FPU          = 1 << 0,  
	CPUID_FEAT_EDX_VME          = 1 << 1,  
	CPUID_FEAT_EDX_DE           = 1 << 2,  
	CPUID_FEAT_EDX_PSE          = 1 << 3,  
	CPUID_FEAT_EDX_TSC          = 1 << 4,  
	CPUID_FEAT_EDX_MSR          = 1 << 5,  
	CPUID_FEAT_EDX_PAE          = 1 << 6,  
	CPUID_FEAT_EDX_MCE          = 1 << 7,  
	CPUID_FEAT_EDX_CX8          = 1 << 8,  
	CPUID_FEAT_EDX_APIC         = 1 << 9,  
	CPUID_FEAT_EDX_SEP          = 1 << 11, 
	CPUID_FEAT_EDX_MTRR         = 1 << 12, 
	CPUID_FEAT_EDX_PGE          = 1 << 13, 
	CPUID_FEAT_EDX_MCA          = 1 << 14, 
	CPUID_FEAT_EDX_CMOV         = 1 << 15, 
	CPUID_FEAT_EDX_PAT          = 1 << 16, 
	CPUID_FEAT_EDX_PSE36        = 1 << 17, 
	CPUID_FEAT_EDX_PSN          = 1 << 18, 
	CPUID_FEAT_EDX_CLF          = 1 << 19, 
	CPUID_FEAT_EDX_DTES         = 1 << 21, 
	CPUID_FEAT_EDX_ACPI         = 1 << 22, 
	CPUID_FEAT_EDX_MMX          = 1 << 23, 
	CPUID_FEAT_EDX_FXSR         = 1 << 24, 
	CPUID_FEAT_EDX_SSE          = 1 << 25, 
	CPUID_FEAT_EDX_SSE2         = 1 << 26, 
	CPUID_FEAT_EDX_SS           = 1 << 27, 
	CPUID_FEAT_EDX_HTT          = 1 << 28, 
	CPUID_FEAT_EDX_TM1          = 1 << 29, 
	CPUID_FEAT_EDX_IA64         = 1 << 30,
	CPUID_FEAT_EDX_PBE          = 1 << 31,

	CPUID_FEAT_EDX_ALL 	    = CPUID_FEAT_EDX_FPU | CPUID_FEAT_EDX_VME | CPUID_FEAT_EDX_DE
					| CPUID_FEAT_EDX_PSE | CPUID_FEAT_EDX_TSC | CPUID_FEAT_EDX_MSR
					| CPUID_FEAT_EDX_PAE | CPUID_FEAT_EDX_MCE | CPUID_FEAT_EDX_CX8
					| CPUID_FEAT_EDX_APIC | CPUID_FEAT_EDX_SEP | CPUID_FEAT_EDX_MTRR
					| CPUID_FEAT_EDX_PGE | CPUID_FEAT_EDX_MCA | CPUID_FEAT_EDX_CMOV
					| CPUID_FEAT_EDX_PAT | CPUID_FEAT_EDX_PSE36 | CPUID_FEAT_EDX_PSN
					| CPUID_FEAT_EDX_CLF | CPUID_FEAT_EDX_DTES | CPUID_FEAT_EDX_ACPI
					| CPUID_FEAT_EDX_MMX | CPUID_FEAT_EDX_FXSR | CPUID_FEAT_EDX_SSE
					| CPUID_FEAT_EDX_SSE2 | CPUID_FEAT_EDX_SS | CPUID_FEAT_EDX_HTT
					| CPUID_FEAT_EDX_TM1 | CPUID_FEAT_EDX_IA64 | CPUID_FEAT_EDX_PBE
};

enum {
	CPUID_EXTFEAT_ECX_LAHF_LM 		= 1 << 0,
	CPUID_EXTFEAT_ECX_CMP_LEGACY 		= 1 << 1,
	CPUID_EXTFEAT_ECX_SVM 			= 1 << 2,
	CPUID_EXTFEAT_ECX_EXTAPIC 		= 1 << 3,
	CPUID_EXTFEAT_ECX_CR8_LEGACY 		= 1 << 4,
	CPUID_EXTFEAT_ECX_ABM 			= 1 << 5,
	CPUID_EXTFEAT_ECX_SSE4A 		= 1 << 6,
	CPUID_EXTFEAT_ECX_MISALIGNSSE 		= 1 << 7,
	CPUID_EXTFEAT_ECX_3DNOWPREFETCH 	= 1 << 8,
	CPUID_EXTFEAT_ECX_OSVW 			= 1 << 9,
	CPUID_EXTFEAT_ECX_IBS 			= 1 << 10,
	CPUID_EXTFEAT_ECX_XOP			= 1 << 11,
	CPUID_EXTFEAT_ECX_SKINIT 		= 1 << 12,
	CPUID_EXTFEAT_ECX_WDT 			= 1 << 13,
	CPUID_EXTFEAT_ECX_LWP 			= 1 << 15,
	CPUID_EXTFEAT_ECX_FMA4 			= 1 << 16,
	CPUID_EXTFEAT_ECX_TCE 			= 1 << 17,
	CPUID_EXTFEAT_ECX_NODEIDE_MSR 		= 1 << 19,
	CPUID_EXTFEAT_ECX_TBM 			= 1 << 21,
	CPUID_EXTFEAT_ECX_TOPOEXT 		= 1 << 22,
	CPUID_EXTFEAT_ECX_PERFXTR_CORE 		= 1 << 23,
	CPUID_EXTFEAT_ECX_PERFCTR_NB 		= 1 << 24,

	CPUID_EXTFEAT_EDX_FPU 			= 1 << 0,
	CPUID_EXTFEAT_EDX_VME 			= 1 << 1,
	CPUID_EXTFEAT_EDX_DE 			= 1 << 2,
	CPUID_EXTFEAT_EDX_PSE 			= 1 << 3,
	CPUID_EXTFEAT_EDX_TSC 			= 1 << 4,
	CPUID_EXTFEAT_EDX_MSR 			= 1 << 5,
	CPUID_EXTFEAT_EDX_PAE 			= 1 << 6,
	CPUID_EXTFEAT_EDX_MCE 			= 1 << 7,
	CPUID_EXTFEAT_EDX_CX8 			= 1 << 8,
	CPUID_EXTFEAT_EDX_APIC 			= 1 << 9,
	CPUID_EXTFEAT_EDX_SYSCALL 		= 1 << 11,
	CPUID_EXTFEAT_EDX_MTRR 			= 1 << 12,
	CPUID_EXTFEAT_EDX_PGE 			= 1 << 13,
	CPUID_EXTFEAT_EDX_MCA 			= 1 << 14,
	CPUID_EXTFEAT_EDX_CMOV 			= 1 << 15,
	CPUID_EXTFEAT_EDX_PAT 			= 1 << 16,
	CPUID_EXTFEAT_EDX_PSE36 		= 1 << 17,
	CPUID_EXTFEAT_EDX_MP 			= 1 << 19,
	CPUID_EXTFEAT_EDX_NX 			= 1 << 20,
	CPUID_EXTFEAT_EDX_MMXEXT 		= 1 << 22,
	CPUID_EXTFEAT_EDX_MMX 			= 1 << 23,
	CPUID_EXTFEAT_EDX_FXSR 			= 1 << 24,
	CPUID_EXTFEAT_EDX_FXSR_OPT 		= 1 << 25,
	CPUID_EXTFEAT_EDX_PDPE1GB 		= 1 << 26,
	CPUID_EXTFEAT_EDX_RDTSCP		= 1 << 27,
	CPUID_EXTFEAT_EDX_LM 			= 1 << 29,
	CPUID_EXTFEAT_EDX_3DNOWEXT 		= 1 << 30,
	CPUID_EXTFEAT_EDX_3DNOW 		= 1 << 31
};

typedef enum cputype {
	CT_NONE,
	CT_AMDK5,
	CT_AMD,
	CT_CENTAUR,
	CT_CYRIX,
	CT_INTEL,
	CT_TRANSMETA,
	CT_NATIONAL_SEMICONDUCTOR,
	CT_NEXGEN,
	CT_RISE,
	CT_SIS,
	CT_UMC,
	CT_VIA,
	CT_VORTEX,
	CT_KVM
} cputype_t;

#if defined(__i386__) || defined(__i386) || defined(__x86_64) \
	|| defined(_M_AMD64) || defined(__M_X64)

/**
 * cpuid_get_cpu_type - Get CPU Type
 *
 * Returns the CPU Type as cputype_t.
 *
 * See also: cpuid_get_cpu_type_string()
 */
#define is_intel_cpu() 	cpuid_get_cpu_type() == CT_INTEL
#define is_amd_cpu() 	cpuid_get_cpu_type() == CT_AMDK5 || cpuid_get_cpu_type() == CT_AMD
cputype_t cpuid_get_cpu_type(void);

/**
 * cpuid_sprintf_cputype - Get CPU Type string
 * @cputype: a char of atleast 12 bytes in it.
 *
 * Returns true on success, false on failure
 */
bool cpuid_sprintf_cputype(const cputype_t cputype, char *buf);

/**
 * cpuid_is_supported - test if the CPUID instruction is supported
 *
 * CPUID is not supported by old CPUS.
 *
 * Returns true if the cpuid instruction is supported, false otherwise.
 *
 * See also: cpuid()
 */
bool cpuid_is_supported(void);

/**
 * cpuid_highest_ext_func_supported - Get the highest extended function supported
 *
 *
 * Returns the highest extended function supported.
 *
 * This is the same as calling:
 * 	cpuid(CPUID_HIGHEST_EEXTENDED_FUNCTION_SUPPORTED, &highest);
 *
 * This is made visible to the linker because it's easier to call it
 * instead of calling cpuid with less type-checking.  cpuid calls this.
 *
 * See also: cpuid()
 */
uint32_t cpuid_highest_ext_func_supported(void);

/**
 * cpuid - Get Some information from the CPU.
 *
 * This function expects buf to be a valid pointer to a string/int/...
 * depending on the requested information.
 *
 * For CPUID_VENDOR_ID:
 * 	Returns a string into buf.
 *
 * For CPUID_PROCINFO_AND_FEATUREBITS:
 * 	buf[0]: Stepping
 * 	buf[1]: Model
 * 	buf[2]: Family
 * 	buf[3]: Extended Model
 * 	buf[4]: Extended Family
 * 	buf[5]: Brand Index
 * 	buf[6]: CL Flush Line Size
 * 	buf[7]: Logical Processors
 * 	buf[8]: Initial APICID
 *
 * For CPUID_L1_CACHE_AND_TLB_IDS:
 *	buf[0] to buf[3]: 2M+4M page TLB info
 * 		0: Inst count
 * 		1: Inst Assoc
 * 		2: Data Count
 * 		3: Data Assoc
 * 	buf[4] to buf[7]: 4k page TLB info
 * 		0: Inst count
 * 		1: Inst Assoc
 * 		2: Data Count
 * 		3: Data Assoc
 * 	buf[8] to buf[11]: L1 data cache information
 *		0: Line Size
 * 		1: LinesPerTag
 * 		2: Associativity
 * 		3: CacheSize
 * 	buf[12] to buf[15]: L1 instruction cache info
 * 		0: Line Size
 * 		1: LinesPerTag
 * 		2: Associativity
 * 		3: CacheSize
 *
 * For CPUID_HIGHEST_EXTENDED_FUNCTION_SUPPORTED:
 * 	Returns the highest supported function in *buf (expects an integer ofc)
 *
 * For CPUID_EXTENDED_PROC_INFO_FEATURE_BITS:
 * 	Returns them in buf[0] and buf[1].
 *
 * For CPUID_EXTENDED_L2_CACHE_FEATURES:
 * 	buf[0]: Line size
 * 	buf[1]: Associativity
 * 	buf[2]: Cache size.
 *
 * For CPUID_VIRT_PHYS_ADDR_SIZES:
 * 	buf[0]: Physical
 * 	buf[1]: Virtual
 *
 * For CPUID_PROC_BRAND_STRING:
 * 	Have a char array with at least 48 bytes assigned to it.
 *
 * If an invalid flag has been passed a 0xbaadf00d is returned in *buf.
 */
void cpuid(cpuid_t info, uint32_t *buf);

/**
 * cpuid_write_info - Write specified CPU information to a file.
 * @info: Bit set of information to write.
 * @featureset: Bit set of features to write.
 * @outfile: Output filename (Max 256).
 *
 * If @outfile is NULL, a name is choosen in the following format:
 * 	CPUVENDOR_PROCESSORBRAND.cpuid
 *
 * Returns true on success, false otherwise.
 *
 * Example usage:
 * 	if (!cpuid_write_info(CPUID_VENDORID | CPUID_PROC_BRAND_STRING,
 * 				CPUID_FEAT_ECX_SSE3 | CPUID_FEAT_EDX_FPU,
 * 				"cpuinfo.cpuid"))
 * 		... error ...
 */
bool cpuid_write_info(uint32_t info, uint32_t featureset, const char *outfile);

/**
 * cpuid_test_feature - Test if @feature is available
 *
 * Returns true if feature is supported, false otherwise.
 *
 * The feature parameter must be >= CPUID_EXTENDED_PROC_INFO_FEATURE_BITS
 *  and <= CPUID_VIRT_PHYS_ADDR_SIZES.
 */
bool cpuid_test_feature(cpuid_t feature);

/**
 * cpuid_has_feature - Test if @feature is supported
 *
 * Test if the CPU supports MMX/SSE* etc.
 * This is split into two parts:
 * 	cpuid_has_ecxfeature and
 * 	cpuid_has_edxfeature.
 * See the enum for more information.
 *
 *
 * Returns true if the feature is available, false otherwise.
 */
bool cpuid_has_ecxfeature(int feature);
bool cpuid_has_edxfeature(int feature);

/**
 * cpuid_has_extfeature - Test if @extfeature is supported
 * @extfeature: the extended feature to test.
 *
 * This is split into two parts:
 * 	cpuid_has_ecxfeature_ext and
 * 	cpuid_has_edxfeature_ext.
 * See the enum for more information.
 *
 * Test if the CPU supports this feature.
 * Returns true on success, false otherwise.
 */
bool cpuid_has_ecxfeature_ext(int extfeature);
bool cpuid_has_edxfeature_ext(int extfeature);

#else

#define cpuid_get_cpu_type() 				BUILD_ASSERT_OR_ZERO(0)
#define cpuid_get_cpu_type_string() 			BUILD_ASSERT_OR_ZERO(0)

#define cpuid_is_supported() 				BUILD_ASSERT_OR_ZERO(0)
#define cpuid(info, buf) 				BUILD_ASSERT_OR_ZERO(0)
#define cpuid_write_info(info, featureset, outfile)	BUILD_ASSERT_OR_ZERO(0)

#define cpuid_highest_ext_func_supported() 		BUILD_ASSERT_OR_ZERO(0)
#define cpuid_test_feature(feature) 			BUILD_ASSERT_OR_ZERO(0)
#define cpuid_has_ecxfeature(feature) 			BUILD_ASSERT_OR_ZERO(0)
#define cpuid_has_edxfeature(feature) 			BUILD_ASSERT_OR_ZERO(0)

#endif  // !x86/x64
#endif  // CCAN_CPUID_H

/**
 * strcount - Count number of (non-overlapping) occurrences of a substring.
 * @haystack: a C string
 * @needle: a substring
 *
 * Example:
 *      assert(strcount("aaa aaa", "a") == 6);
 *      assert(strcount("aaa aaa", "ab") == 0);
 *      assert(strcount("aaa aaa", "aa") == 2);
 */
extern size_t strcount(const char *haystack, const char *needle);

/**
 * ptrint - Encoding integers in pointer values
 *
 * Library (standard or ccan) functions which take user supplied
 * callbacks usually have the callback supplied with a void * context
 * pointer.  For simple cases, it's sometimes sufficient to pass a
 * simple integer cast into a void *, rather than having to allocate a
 * context structure.  This module provides some helper macros to do
 * this relatively safely and portably.
 *
 * The key characteristics of these functions are:
 *	ptr2int(int2ptr(val)) == val
 * and
 *      !int2ptr(val) == !val
 * (i.e. the transformation preserves truth value).
 *
 * Example:
 *	#include <ccan/ptrint/ptrint.h>
 *
 *	static void callback(void *opaque)
 *	{
 *		int val = ptr2int(opaque);
 *		printf("Value is %d\n", val);
 *	}
 *
 *	void (*cb)(void *opaque) = callback;
 *
 *	int main(int argc, char *argv[])
 *	{
 *		int val = 17;
 *
 *		(*cb)(int2ptr(val));
 *		exit(0);
 *	}
 *
 * License: CC0 (Public domain)
 * Author: David Gibson <david@gibson.dropbear.id.au>
 */

/* CC0 (Public domain) - see LICENSE file for details */
#ifndef CCAN_PTRINT_H
#define CCAN_PTRINT_H

/*
 * This is a deliberately incomplete type, because it should never be
 * dereferenced - instead it marks pointer values which are actually
 * encoding integers
 */
typedef struct ptrint ptrint_t;

extern ptrdiff_t ptr2int(const ptrint_t* p);
extern ptrint_t* int2ptr(ptrdiff_t i);

#endif /* CCAN_PTRINT_H */


/**
 * tally - running tally of integers
 *
 * The tally module implements simple analysis of a stream of integers.
 * Numbers are fed in via tally_add(), and then the mean, median, mode and
 * a histogram can be read out.
 *
 * Example:
 *	#include <stdio.h>
 *	#include <err.h>
 *	#include <ccan/tally/tally.h>
 *
 *	int main(int argc, char *argv[])
 *	{
 *		struct tally *t;
 *		unsigned int i;
 *		size_t err;
 *		ssize_t val;
 *		char *histogram;
 *
 *		if (argc < 2)
 *			errx(1, "Usage: %s <number>...\n", argv[0]);
 *
 *		t = tally_new(100);
 *		for (i = 1; i < argc; i++)
 *			tally_add(t, atol(argv[i]));
 *
 *		printf("Mean = %zi\n", tally_mean(t));
 *		val = tally_approx_median(t, &err);
 *		printf("Median = %zi (+/- %zu)\n", val, err);
 *		val = tally_approx_mode(t, &err);
 *		printf("Mode = %zi (+/- %zu)\n", val, err);
 *		histogram = tally_histogram(t, 50, 10);
 *		printf("Histogram:\n%s", histogram);
 *		free(histogram);
 *		return 0;
 *	}
 *
 * Author: Rusty Russell <rusty@rustcorp.com.au>
 */

#ifndef CCAN_TALLY_H
#define CCAN_TALLY_H

struct tally;

/**
 * tally_new - allocate the tally structure.
 * @buckets: the number of frequency buckets.
 *
 * This allocates a tally structure using malloc().  The greater the value
 * of @buckets, the more accurate tally_approx_median() and tally_approx_mode()
 * and tally_histogram() will be, but more memory is consumed.  If you want
 * to use tally_histogram(), the optimal bucket value is the same as that
 * @height argument.
 */
struct tally *tally_new(unsigned int buckets);

/**
 * tally_add - add a value.
 * @tally: the tally structure.
 * @val: the value to add.
 */
void tally_add(struct tally *tally, ssize_t val);

/**
 * tally_num - how many times as tally_add been called?
 * @tally: the tally structure.
 */
size_t tally_num(const struct tally *tally);

/**
 * tally_min - the minimum value passed to tally_add.
 * @tally: the tally structure.
 *
 * Undefined if tally_num() == 0.
 */
ssize_t tally_min(const struct tally *tally);

/**
 * tally_max - the maximum value passed to tally_add.
 * @tally: the tally structure.
 *
 * Undefined if tally_num() == 0.
 */
ssize_t tally_max(const struct tally *tally);

/**
 * tally_mean - the mean value passed to tally_add.
 * @tally: the tally structure.
 *
 * Undefined if tally_num() == 0, but will not crash.
 */
ssize_t tally_mean(const struct tally *tally);

/**
 * tally_total - the total value passed to tally_add.
 * @tally: the tally structure.
 * @overflow: the overflow value (or NULL).
 *
 * If your total can't overflow a ssize_t, you don't need @overflow.
 * Otherwise, @overflow is the upper ssize_t, and the return value should
 * be treated as the lower size_t (ie. the sign bit is in @overflow).
 */
ssize_t tally_total(const struct tally *tally, ssize_t *overflow);

/**
 * tally_approx_median - the approximate median value passed to tally_add.
 * @tally: the tally structure.
 * @err: the error in the returned value (ie. real median is +/- @err).
 *
 * Undefined if tally_num() == 0, but will not crash.  Because we
 * don't reallocate, we don't store all values, so this median cannot be
 * exact.
 */
ssize_t tally_approx_median(const struct tally *tally, size_t *err);

/**
 * tally_approx_mode - the approximate mode value passed to tally_add.
 * @tally: the tally structure.
 * @err: the error in the returned value (ie. real mode is +/- @err).
 *
 * Undefined if tally_num() == 0, but will not crash.  Because we
 * don't reallocate, we don't store all values, so this mode cannot be
 * exact.  It could well be a value which was never passed to tally_add!
 */
ssize_t tally_approx_mode(const struct tally *tally, size_t *err);

#define TALLY_MIN_HISTO_WIDTH 8
#define TALLY_MIN_HISTO_HEIGHT 3

/**
 * tally_graph - return an ASCII image of the tally_add distribution
 * @tally: the tally structure.
 * @width: the maximum string width to use (>= TALLY_MIN_HISTO_WIDTH)
 * @height: the maximum string height to use (>= TALLY_MIN_HISTO_HEIGHT)
 *
 * Returns a malloc()ed string which draws a multi-line graph of the
 * distribution of values.  On out of memory returns NULL.
 */
char *tally_histogram(const struct tally *tally,
		      unsigned width, unsigned height);
#endif /* CCAN_TALLY_H */

/* CC0 (Public domain) - see LICENSE file for details */
#ifndef CCAN_COMPILER_H
#define CCAN_COMPILER_H

#ifndef COLD
#if __GNUC__
/**
 * COLD - a function is unlikely to be called.
 *
 * Used to mark an unlikely code path and optimize appropriately.
 * It is usually used on logging or error routines.
 *
 * Example:
 * static void COLD moan(const char *reason)
 * {
 *	fprintf(stderr, "Error: %s (%s)\n", reason, strerror(errno));
 * }
 */
#define COLD __attribute__((__cold__))
#else
#define COLD
#endif
#endif

#ifndef NORETURN
#if __GNUC__
/**
 * NORETURN - a function does not return
 *
 * Used to mark a function which exits; useful for suppressing warnings.
 *
 * Example:
 * static void NORETURN fail(const char *reason)
 * {
 *	fprintf(stderr, "Error: %s (%s)\n", reason, strerror(errno));
 *	exit(1);
 * }
 */
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif
#endif

#ifndef PRINTF_FMT
#if __GNUC__
/**
 * PRINTF_FMT - a function takes printf-style arguments
 * @nfmt: the 1-based number of the function's format argument.
 * @narg: the 1-based number of the function's first variable argument.
 *
 * This allows the compiler to check your parameters as it does for printf().
 *
 * Example:
 * void PRINTF_FMT(2,3) my_printf(const char *prefix, const char *fmt, ...);
 */
#define PRINTF_FMT(nfmt, narg) \
	__attribute__((format(__printf__, nfmt, narg)))
#else
#define PRINTF_FMT(nfmt, narg)
#endif
#endif

#ifndef CONST_FUNCTION
#if __GNUC__
/**
 * CONST_FUNCTION - a function's return depends only on its argument
 *
 * This allows the compiler to assume that the function will return the exact
 * same value for the exact same arguments.  This implies that the function
 * must not use global variables, or dereference pointer arguments.
 */
#define CONST_FUNCTION __attribute__((__const__))
#else
#define CONST_FUNCTION
#endif

#ifndef PURE_FUNCTION
#if __GNUC__
/**
 * PURE_FUNCTION - a function is pure
 *
 * A pure function is one that has no side effects other than it's return value
 * and uses no inputs other than it's arguments and global variables.
 */
#define PURE_FUNCTION __attribute__((__pure__))
#else
#define PURE_FUNCTION
#endif
#endif
#endif

#if __GNUC__
#ifndef UNNEEDED
/**
 * UNNEEDED - a variable/function may not be needed
 *
 * This suppresses warnings about unused variables or functions, but tells
 * the compiler that if it is unused it need not emit it into the source code.
 *
 * Example:
 * // With some preprocessor options, this is unnecessary.
 * static UNNEEDED int counter;
 *
 * // With some preprocessor options, this is unnecessary.
 * static UNNEEDED void add_to_counter(int add)
 * {
 *	counter += add;
 * }
 */
#define UNNEEDED __attribute__((__unused__))
#endif

#ifndef NEEDED
#if __GNUC__
/**
 * NEEDED - a variable/function is needed
 *
 * This suppresses warnings about unused variables or functions, but tells
 * the compiler that it must exist even if it (seems) unused.
 *
 * Example:
 *	// Even if this is unused, these are vital for debugging.
 *	static NEEDED int counter;
 *	static NEEDED void dump_counter(void)
 *	{
 *		printf("Counter is %i\n", counter);
 *	}
 */
#define NEEDED __attribute__((__used__))
#else
/* Before used, unused functions and vars were always emitted. */
#define NEEDED __attribute__((__unused__))
#endif
#endif

#ifndef UNUSED
/**
 * UNUSED - a parameter is unused
 *
 * Some compilers (eg. gcc with -W or -Wunused) warn about unused
 * function parameters.  This suppresses such warnings and indicates
 * to the reader that it's deliberate.
 *
 * Example:
 *	// This is used as a callback, so needs to have this prototype.
 *	static int some_callback(void *unused UNUSED)
 *	{
 *		return 0;
 *	}
 */
#define UNUSED __attribute__((__unused__))
#endif
#else
#ifndef UNNEEDED
#define UNNEEDED
#endif
#ifndef NEEDED
#define NEEDED
#endif
#ifndef UNUSED
#define UNUSED
#endif
#endif

#ifndef IS_COMPILE_CONSTANT
#if __GNUC__
/**
 * IS_COMPILE_CONSTANT - does the compiler know the value of this expression?
 * @expr: the expression to evaluate
 *
 * When an expression manipulation is complicated, it is usually better to
 * implement it in a function.  However, if the expression being manipulated is
 * known at compile time, it is better to have the compiler see the entire
 * expression so it can simply substitute the result.
 *
 * This can be done using the IS_COMPILE_CONSTANT() macro.
 *
 * Example:
 *	enum greek { ALPHA, BETA, GAMMA, DELTA, EPSILON };
 *
 *	// Out-of-line version.
 *	const char *greek_name(enum greek greek);
 *
 *	// Inline version.
 *	static inline const char *_greek_name(enum greek greek)
 *	{
 *		switch (greek) {
 *		case ALPHA: return "alpha";
 *		case BETA: return "beta";
 *		case GAMMA: return "gamma";
 *		case DELTA: return "delta";
 *		case EPSILON: return "epsilon";
 *		default: return "**INVALID**";
 *		}
 *	}
 *
 *	// Use inline if compiler knows answer.  Otherwise call function
 *	// to avoid copies of the same code everywhere.
 *	#define greek_name(g)						\
 *		 (IS_COMPILE_CONSTANT(greek) ? _greek_name(g) : greek_name(g))
 */
#define IS_COMPILE_CONSTANT(expr) __builtin_constant_p(expr)
#else
/* If we don't know, assume it's not. */
#define IS_COMPILE_CONSTANT(expr) 0
#endif
#endif

#ifndef WARN_UNUSED_RESULT
#if __GNUC__
/**
 * WARN_UNUSED_RESULT - warn if a function return value is unused.
 *
 * Used to mark a function where it is extremely unlikely that the caller
 * can ignore the result, eg realloc().
 *
 * Example:
 * // buf param may be freed by this; need return value!
 * static char *WARN_UNUSED_RESULT enlarge(char *buf, unsigned *size)
 * {
 *	return realloc(buf, (*size) *= 2);
 * }
 */
#define WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#else
#define WARN_UNUSED_RESULT
#endif
#endif
#endif /* CCAN_COMPILER_H */

/*** Regular expression functions: this implementation avoids degenerate recursive performance issues matching patterns. ***/

#define NSUBEXP  32
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;
#undef NSUBEXP

/*** Compiles a regular expression (exp) into a regex structure for use with other regex functions. ***/
extern regexp* regcomp(char* exp);

/*** Match string against the regexp prog. Returns 1 on match, 0 on no match. ***/
extern int regexec(regexp* prog, char* string);

/*** use to perform substitutions using regexp ***/
extern void regsub(regexp* prog, char* source, char* dest);

/*** prints regular expression error ***/
extern void regerror(const char* msg);

/*** incomplete beta function and the Student t cumulative distribution function ***/
extern double incbeta(double a, double b, double x);
extern double student_t_cdf(double t, double v);

#endif  // __EXTERN_H__
/* end extern.h */