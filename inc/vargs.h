/***

	__vargs.h

	Implement a variable-length argument list (like va_list) portably. Also indicates the
	types of the arguments (unlike va_list). Works with POD types and pointers; you
	can use custom types and structs by passing a pointer to the varg_list (as you probably 
	would if you were calling a normal function with a structure argument anyway.)
	
	Use to pass a variable number of arguments to functions with some extra type safety.
	(Or use it to return multiple values from a function! It's all up to you.)

	C. M. Street

***/
#ifndef	__VARGS_CODEHAPPY
#define	__VARGS_CODEHAPPY

/* implementation details are hidden */
typedef	void *
		varg_list;

/*** Determine the total number of arguments in the varg list. ***/
extern u32 varg_list_total(varg_list vargs);

/*** Determine the total number of remaining arguments in the varg list. ***/
extern u32 varg_list_remaining(varg_list vargs);

/*** Rewind the varg_list to the previous argument (if we are not at the start of the list.) Returns
	true iff we successfully backed up. ***/
extern bool varg_list_rewind(varg_list vargs);

/*** Restart the varg_list from the beginning. ***/
extern void varg_list_reset(varg_list vargs);

/*** Skip the current argument and advance the varg_list. Returns true iff we succeeded (i.e. we
	weren't already at the end of the varg_list.) ***/
extern bool varg_list_skip(varg_list vargs);

/*** Try and parse the next argument as the specified type. If the return type is true, we succeeded: the
	argument "ptr" is set to the value of the next argument, and the varg_list is advanced (so the next call
	will retrieve the next element), and we return true. If we return false, the argument type was wrong, and
	we do not advance the varg_list. ***/
extern bool varg_list_int(varg_list vargs, int* ptr);
extern bool varg_list_uint(varg_list vargs, uint* ptr);
extern bool varg_list_i64(varg_list vargs, i64* ptr);
extern bool varg_list_u64(varg_list vargs, u64* ptr);
extern bool varg_list_schar(varg_list vargs, schar* ptr);
extern bool varg_list_uchar(varg_list vargs, uchar* ptr);
extern bool varg_list_float(varg_list vargs, float* ptr);
extern bool varg_list_double(varg_list vargs, double* ptr);
extern bool varg_list_ptr(varg_list vargs, void **ptr);
extern bool varg_list_long_double(varg_list vargs, long double* ptr);

/*** Construct a new varg_list. ***/
extern varg_list varg_list_new(void);

/*** Add arguments of the specified type to the end of the varg_list. ***/
extern void varg_list_add_int(varg_list vargs, int arg);
extern void varg_list_add_uint(varg_list vargs, uint arg);
extern void varg_list_add_i64(varg_list vargs, i64 arg);
extern void varg_list_add_u64(varg_list vargs, u64 arg);
extern void varg_list_add_schar(varg_list vargs, schar arg);
extern void varg_list_add_uchar(varg_list vargs, uchar arg);
extern void varg_list_add_float(varg_list vargs, float arg);
extern void varg_list_add_double(varg_list vargs, double arg);
extern void varg_list_add_ptr(varg_list vargs, void* arg);
extern void varg_list_add_long_double(varg_list vargs, long double arg);

/*** Create an entire varg list in one swoop. You don't even need to free these; they are managed internally.
	Allows fun stuff like "variable_function(p, NULL, create_varg_list_ints(2, 3, 4));". ***/
extern varg_list create_varg_list_empty(void);
extern varg_list create_varg_list_ints(int nargs, ...);
extern varg_list create_varg_list_ptrs(int nargs, ...);
extern varg_list create_varg_list_i64s(int nargs, ...);

/*** some macros for create_varg_list_ints ***/
#define	VA_INT1(i)			create_varg_list_ints(1, (i))
#define	VA_INT2(i, j)		create_varg_list_ints(2, (i), (j))
#define	VA_INT3(i, j, k)	create_varg_list_ints(3, (i), (j), (k))
#define	VA_INT4(i, j, k, l)	create_varg_list_ints(4, (i), (j), (k), (l))
#define	VA_INT(i)			VA_INT1(i)

// TODO: support more variable argument types as needed.

/*** Concatenate two varg lists into a third varg list that is managed internally (you don't need to free yourself).
	Can be used with the above create_varg_list() functions to make complicated, multiple-argument varg_lists. ***/
extern varg_list varg_list_concat(varg_list v1, varg_list v2);

// TODO: C++ template class?

#endif  // __VARGS_CODEHAPPY
