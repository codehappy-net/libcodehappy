/***

	general.h

	Generally useful macros and typedefs.

	C. M. Street

***/
#ifndef GENERAL_H
#define GENERAL_H

#include <stdint.h>

/*** forever -- a loop with no default termination condition ***/
#ifndef forever
#define	forever	for(;;) 
#endif  // forever
#ifndef	FOREVER
#define	FOREVER	for(;;)
#endif  // FOREVER

/*** implies -- logical macro meaning if x, then y ***/
#ifndef implies
#define	implies(x, y)			(falsity(x) || truth(y))
#endif
#ifndef IMPLIES
#define	IMPLIES(x, y)			(!(zo1(x)) || (zo1(y)))
#endif

/*** iff -- logical macro meaning x and y are BFF and live together ***/
#ifndef iff
#define	iff(x, y)			((implies(x, y)) && (implies(y, x)))
#endif
#ifndef IFF
#define	IFF(x, y)			((IMPLIES(x, y) && (IMPLIES(y, x))))
#endif

/*** ignore_return_val() -- a no-op, but it documents that, yes, you are deliberately
	ignoring the return value ***/
#ifndef	ignore_return_val
#define	ignore_return_val(x)		x
#endif

/*** struct_field_offset -- return the offset of a field in a struct ***/
#ifndef struct_field_offset
#define	struct_field_offset(structname, fieldname)	((unsigned int)(&(((structname *)NULL)->fieldname)))
#endif

/*** allocate, free, and reallocate memory; macros allow us to redefine the allocator ***/
#ifndef NEW
#define	NEW(_typename)	(new _typename)
#endif
#ifndef NEW_ARRAY
#define NEW_ARRAY(_typename, n)	(new _typename [n])
#endif
#ifndef	NEW_STR
#define	NEW_STR(n)		(new char [n + 1])
#endif

#undef likely
#undef unlikely
#undef unused
#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define unused		
#else  // !GNUC
#define likely(x)       x
#define unlikely(x)     x
#define unused
#pragma warning(disable : 4996) /* For fscanf */
#endif

/*** if the given ptr is NULL, return (the specified value) ***/
#define	NOT_NULL_OR_RETURN(ptr, retval)		if (unlikely(is_null(ptr)))	return(retval)
#define	NOT_NULL_OR_RETURN_VOID(ptr)		if (unlikely(is_null(ptr)))	return

/*** if the given memory is NULL, goto the specified label (good for jumping to a cleanup subroutine, etc.) ***/
#define	NOT_NULL_OR_GOTO(ptr, label)		if (unlikely(is_null(ptr)))	goto label

/*** if the given memory is NULL, break or continue. ***/
#define NOT_NULL_OR_BREAK(ptr)			if (unlikely(is_null(ptr)))	break
#define NOT_NULL_OR_CONTINUE(ptr)		if (unlikely(is_null(ptr)))	continue

/*** SWAP -- swaps values of x and y (that are of type t) ***/
#ifndef SWAP
#define SWAP(x, y, t)			do { t x__tmp = (x); (x) = (y); (y) = x__tmp; } while (0)
#endif

/*** SORT2 -- sorts x and y (of type t) ***/
#ifndef	SORT2
#define	SORT2(x, y, t)			do { if ((x) > (y))	SWAP(x, y, t);	} while (0)
#endif

/*** is_null and not_null ***/
#ifndef is_null
#define	is_null(x)	((x) == NULL)
#endif
#ifndef not_null
#define not_null(x)	((x) != NULL)
#endif

/*** finite state machine (FSM) macros ***/
#ifndef FSM
#define	FSM		forever
#endif
#ifndef	STATE
#define	STATE(x)		__##_state
#endif
#ifndef	NEXT_STATE
#define	NEXT_STATE(x)	goto __##_state
#endif

/*** concatenation macro ***/
#ifndef CONCATENATE
#define CONCATENATE(arg1, arg2)  arg1##arg2
#endif

/*** malloc_len -- length needed to malloc the C string ***/
#ifndef malloc_len
#define	malloc_len(x)	(strlen((x)) + 1)
#endif

/*** return a pointer to the terminator of the C string ***/
#ifndef	TERMINATOR
#define	TERMINATOR(str)	((str) + strlen(str))
#endif
/*** ...and the last character of the C string -- note, returns the terminator for the empty string ***/
#ifndef END_OF_STR
#define	END_OF_STR(str)	(((str)[0] == 0) ? (str) : ((str) + strlen(str) - 1))
#endif
/*** is the C string empty? ***/
#ifndef EMPTY_STR
#define	EMPTY_STR(str)	(is_null(str) || (str)[0] == 0)
#endif

/*** documents that falling through is intended ***/
#define	fallthrough

/*** max_int() and min_int() ***/
#define	max_int(x, y)	(((x) > (y)) ? (x) : (y))
#define min_int(x, y)	(((x) < (y)) ? (x) : (y))

/*** starts_with(): string starts-with operation ***/
#define	starts_with(str, pfx)	(strncmp((str), (pfx), strlen(pfx)) == 0)

/*** is_between (inclusive between) ***/
#ifndef is_between
#define	is_between(_val, _min, _max)	(((_val) >= (_min)) && ((_val) <= (_max)))
/*** The following three versions of is_between() _might_ be slightly faster, but require the size of the types ***/
#define	is_between32(_val, _min, _max)	(((u32)((_val) - (_min))) <= (u32)((_max) - (_min)))
#define	is_between64(_val, _min, _max)	(((u64)((_val) - (_min))) <= (u64)((_max) - (_min)))
#define	is_between_int(_val, _min, _max) (((unsigned int)((_val) - (_min))) <= (unsigned int)((_max) - (_min)))
#endif

/*** CLAMP (forces a value to be between [i, j] -- if less than i it becomes i, if greater than j it becomes j) ***/
#ifndef	CLAMP
#define	CLAMP(val, i, j)	(((val) < (i)) ? (i) : (((val) > (j)) ? (j) : (val)))
#endif

/*** unless and until ***/
#ifndef unless
#define	unless(x)		if (!(x))
#endif
#ifndef until
#define	until(x)		while (!(x))
#endif

/*** is_odd and is_even ***/
#ifndef is_odd
#define	is_odd(val)		((val) & 1)
#endif
#ifndef is_even
#define	is_even(val)	(((val) & 1) == 0)
#endif

/*** zero_or_one (or zo1) ***/
#ifndef zero_or_one
#define	zero_or_one(x)	(!!(x))
#endif
#ifndef zo1
#define	zo1(x)			(!!(x))
#endif
#ifndef	truth
#define	truth(x)		(!!(x))
#endif
#ifndef falsity
#define	falsity(x)		(!(x))
#endif

/*** if a number is even or odd and between to values ***/
#define	EVEN_BETWEEN(x, b1, b2)	((((x) & 1) == 0) && (x) >= (b1) && (x) <= (b2))
#define	ODD_BETWEEN(x, b1, b2)	((((x) & 1) == 1) && (x) >= (b1) && (x) <= (b2))

/***

	The following array macros allow easy use of linear one-dimensional arrays for
	multi-dimensional arrays. This avoids the extra allocations and initialization of this method:


		int **array_2d;
		int i;
		
		array_2d = NEW(int *, x);
		for (i = 0; i < x; ++i)
			array_2d[i] = NEW(int, y);

	at the sacrifice of a slightly more involved syntax for accessing elements (you must provide
	the size of all but one dimension of the array to ARRAY_xD_INDEX().)

	It also may increase memory locality, and it _may_ be slightly faster to access individual array
	elements thanks to removing a layer of indirection (though optimizing compilers these days
	are pretty smart about avoiding superfluous array offset computations.)

***/

#define	NEW_ARRAY_2D(t, x, y)			NEW_ARRAY(t, (x) * (y))
#define	NEW_ARRAY_3D(t, x, y, z)		NEW_ARRAY(t, (x) * (y) * (z))
#define	NEW_ARRAY_4D(t, x, y, z, a)		NEW_ARRAY(t, (x) * (y) * (z) * (a))

#define	ARRAY_2D_INDEX(i, j, x)				(((j) * (x)) + (i))
#define	ARRAY_3D_INDEX(i, j, k, x, y)		(((k) * (x) * (y)) + ((j) * (x)) + (i))
#define	ARRAY_4D_INDEX(i, j, k, l, x, y, z)	(((l) * (x) * (y) * (z)) + ((k) * (x) * (y)) + ((j) * (x)) + (i))

/*** for loop shorthand ***/
#define FOR(i,n) 				for (i = 0;i < (n);++i)

#define array_size(a)	(sizeof(a) / sizeof((a)[0]))

/*** is the macro argument zero? ***/
#ifndef iszero
#define	iszero(x)					(0 == (x))
#endif
/*** or not? ***/
#ifndef	nonzero
#define	nonzero(x)				(0 != (x))
#endif

/*** some useful bitmasks (for bytes) ***/
#define	HIGH_1		128
#define	HIGH_2		192
#define	HIGH_3		224
#define	HIGH_4		240
#define	HIGH_5		248
#define	HIGH_6		252
#define	HIGH_7		254
#define	LOW_1		1
#define	LOW_2		3
#define	LOW_3		7
#define	LOW_4		15
#define	LOW_5		31
#define	LOW_6		63
#define	LOW_7		127

/*** ifeq -- if equal macro, and friends ***/
#ifndef ifeq
#define	ifeq(x, y)		if ((x) == (y))
#endif
#ifndef ifne
#define	ifne(x, y)		if ((x) != (y))
#endif
#ifndef ifgt
#define	ifgt(x, y)		if ((x) > (y))
#endif
#ifndef iflt
#define	iflt(x, y)		if ((x) < (y))
#endif
#ifndef ifgte
#define	ifgte(x, y)		if ((x) >= (y))
#endif
#ifndef iflte
#define	iflte(x, y)		if ((x) <= (y))
#endif
#ifndef	ifz
#define	ifz(x)			if ((x) == 0)
#endif
#ifndef ifnz
#define	ifnz(x)			if ((x) != 0)
#endif

/*** IS_DEFINED() -- is a macro defined? Expands to 1 if the macro is defined (as 1), 0 otherwise. ***/
#define IS_DEFINED(macro) IS_DEFINED_(macro)
#define MACROTEST_1 ,
#define IS_DEFINED_(value) IS_DEFINED__(MACROTEST_##value)
#define IS_DEFINED__(comma) IS_DEFINED___(comma 1, 0)
#define IS_DEFINED___(_, v, ...) v

/*** round a floating-point number to integer ***/
#define	ROUND_FLOAT_TO_INT(x)	((int)(floor((x) + 0.5)))

#define	no_op
#define check_or_die(x)	{ if (!(x)) { exit(1); } }
#define check_mem_or_die(ptr)	{ if (is_null(ptr)) { exit(1); } }

/*** Ship assert ***/
#ifndef ship_assert
extern const char __assert_failed_msg[];
#define ship_assert(x)	if (x) {} else { fprintf(stderr, "%s -- %s, line %d\n", __assert_failed_msg, __FILE__, __LINE__); exit(999); }
#define	SHIP_ASSERT(x)	if (x) {} else { fprintf(stderr, "%s -- %s, line %d\n", __assert_failed_msg, __FILE__, __LINE__); exit(999); }
#endif

/*** Integer multiple? ***/
#ifndef integer_multiple
#define integer_multiple(X, Y)	((((X) / (Y)) * (Y)) == (X))
#endif

/*** TBI (To Be Implemented) ***/
#ifndef TBI
extern const char __impl_error_msg[];
#define TBI()		{ fputs(__impl_error_msg, stderr); exit(777); }
#endif

/*** Break, if null ***/
#ifndef BREAK_NULL
#define BREAK_NULL(w) if (nullptr == (w)) break
#endif

/*** Output to null ***/
#ifndef DEV_NULL
#ifdef CODEHAPPY_WINDOWS
#define DEV_NULL	" >nul"
#else
#define DEV_NULL	" >/dev/null"
#endif
#endif

/*** Useful typedefs ***/

typedef unsigned char 
	u8;

typedef signed char
	s8;

typedef signed char
	i8;

typedef uint16_t
	u16;

typedef int16_t
	s16;

typedef int16_t
	i16;

typedef uint32_t
	u32;

typedef int32_t
	s32;

typedef int32_t
	i32;

typedef uint64_t
	u64;

typedef int64_t
	s64;

typedef int64_t
	i64;

typedef	unsigned int
	uint;

typedef unsigned short
	ushort;

typedef unsigned char
	uchar;

#endif  // GENERAL_H
