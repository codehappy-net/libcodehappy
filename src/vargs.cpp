/***

	vargs.cpp

	Implement a variable-length argument list (like va_list) portably. Also indicates the
	types of the arguments (unlike va_list). Works with POD types and pointers; you
	can use custom types and structs by passing a pointer to the varg_list (as you probably 
	would if you were calling a normal function with a structure argument anyway.)
	
	Use to pass a variable number of arguments to functions with some extra type safety.

	Copyright (c) 2014-2022 C. M. Street

***/
#include "libcodehappy.h"

/*** the varg_list type ***/
struct vl {
	int	typ;
	union {
		int		val_i;
		uint		val_u;
		i64		val_i64;
		u64		val_u64;
		float		val_f;
		double		val_g;
		long double	val_lg;
		schar		val_c;
		uchar		val_uc;
		void*		val_ptr;
		// TODO: add bool
	};
	vl*	prev;
	vl*	next;
};

/*** the header of the vl linked list ***/
struct vlhdr {
	vl* start;
	vl* next;
	vl* end;
	u32 len;
};

#define	VLHDR(x)	((vlhdr*)x)
#define	VL_START(x)	(VLHDR(x)->start)
#define	VL_NEXT(x)	(VLHDR(x)->next)
#define	VL_END(x)	(VLHDR(x)->end)

#define	TYPE_INT		0
#define	TYPE_UINT		1
#define	TYPE_I64		2
#define	TYPE_U64		3
#define	TYPE_FLOAT		4
#define	TYPE_DOUBLE		5
#define	TYPE_LDOUBLE	6
#define	TYPE_SCHAR		7
#define	TYPE_UCHAR		8
#define	TYPE_PTR		9
#define	TYPE_BOOL		10

static gc_handle __gc_varg = NULL;

/*** Determine the total number of arguments in the varg list. ***/
u32 varg_list_total(varg_list vargs)
{
	NOT_NULL_OR_RETURN(vargs, 0UL);
	return VLHDR(vargs)->len;
}

/*** Determine the total number of remaining arguments in the varg list. ***/
u32 varg_list_remaining(varg_list vargs)
{
	NOT_NULL_OR_RETURN(vargs, 0UL);
	NOT_NULL_OR_RETURN(VL_START(vargs), 0UL);
	u32 c = 0;
	vl* node = VL_NEXT(vargs);
	while (not_null(node))
		{
		++c;
		node = node->next;
		}
	return(c);
}

/*** Rewind the varg_list to the previous argument (if we are not at the start of the list.) Returns
	true iff we successfully backed up. ***/
bool varg_list_rewind(varg_list vargs)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_START(vargs), false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs)->prev, false);
	VL_NEXT(vargs) = VL_NEXT(vargs)->prev;
	return(true);
}

/*** Restart the varg_list from the beginning. ***/
void varg_list_reset(varg_list vargs)
{
	NOT_NULL_OR_RETURN_VOID(vargs);
	NOT_NULL_OR_RETURN_VOID(VL_START(vargs));
	VL_NEXT(vargs) = VL_START(vargs);
}

/*** Skip the current argument and advance the varg_list. Returns true iff we succeeded (i.e. we
	weren't already at the end of the varg_list.) ***/
bool varg_list_skip(varg_list vargs)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);
	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return(true);
}

// TODO: add stricter versions of the varg_list_*() functions, that return false unless type matches exactly
// TODO: add medium-strict versions, that do not round floats to ints but allow signed/unsigned char/short/int/longlong conversions, etc.

/*** Try and parse the next argument as the specified type. If the return type is true, we succeeded: the
	argument "ptr" is set to the value of the next argument, and the varg_list is advanced (so the next call
	will retrieve the next element). If we return false, the argument type was wrong, and
	we do not advance the varg_list. ***/
bool varg_list_int(varg_list vargs, int* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		if (VL_NEXT(vargs)->val_i64 < INT32_MIN ||
			VL_NEXT(vargs)->val_i64 > INT32_MAX)
			return false;
		*ptr = (int)(VL_NEXT(vargs)->val_i64);
		break;
	case TYPE_U64:
		if (VL_NEXT(vargs)->val_u64 > INT32_MAX)
			return false;
		*ptr = (int)(VL_NEXT(vargs)->val_u64);
		break;
	case TYPE_INT:
		*ptr = VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		if (VL_NEXT(vargs)->val_u > INT32_MAX)
			return false;
		*ptr = (int)(VL_NEXT(vargs)->val_u);
		break;
	case TYPE_FLOAT:
		// TODO: check for over/underflow
		*ptr = ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_f);
		break;
	case TYPE_DOUBLE:
		// TODO: check for over/underflow
		*ptr = ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_g);
		break;
	case TYPE_LDOUBLE:
		// TODO: check for over/underflow
		*ptr = ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_lg);
		break;
	case TYPE_SCHAR:
		*ptr = (int)(VL_NEXT(vargs)->val_c);
		break;
	case TYPE_UCHAR:
		*ptr = (int)(VL_NEXT(vargs)->val_uc);
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_uint(varg_list vargs, uint* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		if (VL_NEXT(vargs)->val_i64 < 0LL ||
			VL_NEXT(vargs)->val_i64 > UINT32_MAX)
			return false;
		*ptr = (uint)(VL_NEXT(vargs)->val_i64);
		break;
	case TYPE_U64:
		if (VL_NEXT(vargs)->val_u64 > UINT32_MAX)
			return false;
		*ptr = (uint)(VL_NEXT(vargs)->val_u64);
		break;
	case TYPE_INT:
		if (VL_NEXT(vargs)->val_i < 0)
			return false;
		*ptr = (uint)(VL_NEXT(vargs)->val_i);
		break;
	case TYPE_UINT:
		*ptr = VL_NEXT(vargs)->val_u;
		break;
	case TYPE_FLOAT:
		// TODO: check for overflow
		if (VL_NEXT(vargs)->val_f < 0.)
			return false;
		*ptr = (uint)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_f);
		break;
	case TYPE_DOUBLE:
		// TODO: check for overflow
		if (VL_NEXT(vargs)->val_g < 0.)
			return false;
		*ptr = (uint)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_g);
		break;
	case TYPE_LDOUBLE:
		// TODO: check for overflow
		if (VL_NEXT(vargs)->val_lg < 0.)
			return false;
		*ptr = (uint)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_lg);
		break;
	case TYPE_SCHAR:
		if (VL_NEXT(vargs)->val_c < 0)
			return false;
		*ptr = (uint)(VL_NEXT(vargs)->val_c);
		break;
	case TYPE_UCHAR:
		*ptr = (uint)(VL_NEXT(vargs)->val_uc);
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_i64(varg_list vargs, i64* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		*ptr = VL_NEXT(vargs)->val_i64;
		break;
	case TYPE_U64:
		if (VL_NEXT(vargs)->val_u64 > INT64_MAX)
			return false;
		*ptr = (i64)(VL_NEXT(vargs)->val_u64);
		break;
	case TYPE_INT:
		*ptr = (i64)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		*ptr = (i64)(VL_NEXT(vargs)->val_u);
		break;
	case TYPE_FLOAT:
		// TODO: floor() is the wrong function here...
		*ptr = (i64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_f);
		break;
	case TYPE_DOUBLE:
		// TODO: floor() is the wrong function here...
		*ptr = (i64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_g);
		break;
	case TYPE_LDOUBLE:
		// TODO: floor() is the wrong function here...
		*ptr = (i64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_lg);
		break;
	case TYPE_SCHAR:
		*ptr = (i64)(VL_NEXT(vargs)->val_c);
		break;
	case TYPE_UCHAR:
		*ptr = (i64)(VL_NEXT(vargs)->val_uc);
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_u64(varg_list vargs, u64* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		if (VL_NEXT(vargs)->val_i64 < 0LL)
			return false;
		*ptr = (u64)(VL_NEXT(vargs)->val_i64);
		break;
	case TYPE_U64:
		*ptr = VL_NEXT(vargs)->val_u64;
		break;
	case TYPE_INT:
		if (VL_NEXT(vargs)->val_i < 0)
			return false;
		*ptr = (u64)(VL_NEXT(vargs)->val_i);
		break;
	case TYPE_UINT:
		*ptr = (u64)VL_NEXT(vargs)->val_u;
		break;
	case TYPE_FLOAT:
		// TODO: floor() is the wrong function here...
		if (VL_NEXT(vargs)->val_f < 0.)
			return false;
		*ptr = (u64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_f);
		break;
	case TYPE_DOUBLE:
		// TODO: floor() is the wrong function here...
		if (VL_NEXT(vargs)->val_g < 0.)
			return false;
		*ptr = (u64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_g);
		break;
	case TYPE_LDOUBLE:
		// TODO: floor() is the wrong function here...
		if (VL_NEXT(vargs)->val_lg < 0.)
			return false;
		*ptr = (u64)ROUND_FLOAT_TO_INT(VL_NEXT(vargs)->val_lg);
		break;
	case TYPE_SCHAR:
		if (VL_NEXT(vargs)->val_c < 0)
			return false;
		*ptr = (u64)(VL_NEXT(vargs)->val_c);
		break;
	case TYPE_UCHAR:
		*ptr = (u64)(VL_NEXT(vargs)->val_uc);
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_schar(varg_list vargs, schar* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
	case TYPE_FLOAT:
	case TYPE_DOUBLE:
	case TYPE_LDOUBLE:
		return false;
	case TYPE_I64:
		if (VL_NEXT(vargs)->val_i64 < -128 ||
			VL_NEXT(vargs)->val_i64 > 127)
			return false;
		*ptr = (schar)(VL_NEXT(vargs)->val_i64);
		break;
	case TYPE_U64:
		if (VL_NEXT(vargs)->val_u64 > 127)
			return false;
		*ptr = (schar)(VL_NEXT(vargs)->val_u64);
		break;
	case TYPE_INT:
		if (VL_NEXT(vargs)->val_i < -128 || VL_NEXT(vargs)->val_i > 127)
			return false;
		*ptr = (schar)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		if (VL_NEXT(vargs)->val_u > 127)
			return false;
		*ptr = (schar)(VL_NEXT(vargs)->val_u);
		break;
	case TYPE_SCHAR:
		*ptr = VL_NEXT(vargs)->val_c;
		break;
	case TYPE_UCHAR:
		if (VL_NEXT(vargs)->val_uc > 127)
			return false;
		*ptr = (schar)(VL_NEXT(vargs)->val_uc);
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_uchar(varg_list vargs, uchar* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
	case TYPE_FLOAT:
	case TYPE_DOUBLE:
	case TYPE_LDOUBLE:
		return false;
	case TYPE_I64:
		if (VL_NEXT(vargs)->val_i64 < 0 ||
			VL_NEXT(vargs)->val_i64 > 255)
			return false;
		*ptr = (uchar)(VL_NEXT(vargs)->val_i64);
		break;
	case TYPE_U64:
		if (VL_NEXT(vargs)->val_u64 > 255)
			return false;
		*ptr = (uchar)(VL_NEXT(vargs)->val_u64);
		break;
	case TYPE_INT:
		if (VL_NEXT(vargs)->val_i < 0 || VL_NEXT(vargs)->val_i > 255)
			return false;
		*ptr = (uchar)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		if (VL_NEXT(vargs)->val_u > 255)
			return false;
		*ptr = (uchar)(VL_NEXT(vargs)->val_u);
		break;
	case TYPE_SCHAR:
		if (VL_NEXT(vargs)->val_c < 0)
			return false;
		*ptr = (uchar)VL_NEXT(vargs)->val_c;
		break;
	case TYPE_UCHAR:
		*ptr = VL_NEXT(vargs)->val_uc;
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_float(varg_list vargs, float* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		*ptr = (float)VL_NEXT(vargs)->val_i64;
		break;
	case TYPE_U64:
		*ptr = (float)VL_NEXT(vargs)->val_u64;
		break;
	case TYPE_INT:
		*ptr = (float)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		*ptr = (float)VL_NEXT(vargs)->val_u;
		break;
	case TYPE_FLOAT:
		*ptr = VL_NEXT(vargs)->val_f;
		break;
	case TYPE_DOUBLE:
		*ptr = (float)VL_NEXT(vargs)->val_g;
		break;
	case TYPE_LDOUBLE:
		*ptr = (float)VL_NEXT(vargs)->val_lg;
		break;
	case TYPE_SCHAR:
		*ptr = (float)VL_NEXT(vargs)->val_c;
		break;
	case TYPE_UCHAR:
		*ptr = (float)VL_NEXT(vargs)->val_uc;
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_double(varg_list vargs, double* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		*ptr = (double)VL_NEXT(vargs)->val_i64;
		break;
	case TYPE_U64:
		*ptr = (double)VL_NEXT(vargs)->val_u64;
		break;
	case TYPE_INT:
		*ptr = (double)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		*ptr = (double)VL_NEXT(vargs)->val_u;
		break;
	case TYPE_FLOAT:
		*ptr = (double)VL_NEXT(vargs)->val_f;
		break;
	case TYPE_DOUBLE:
		*ptr = VL_NEXT(vargs)->val_g;
		break;
	case TYPE_LDOUBLE:
		*ptr = (double)VL_NEXT(vargs)->val_lg;
		break;
	case TYPE_SCHAR:
		*ptr = (double)VL_NEXT(vargs)->val_c;
		break;
	case TYPE_UCHAR:
		*ptr = (double)VL_NEXT(vargs)->val_uc;
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_ptr(varg_list vargs, void **ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		(*ptr) = VL_NEXT(vargs)->val_ptr;
		break;
	case TYPE_I64:
	case TYPE_U64:
	case TYPE_INT:
	case TYPE_UINT:
	case TYPE_FLOAT:
	case TYPE_DOUBLE:
	case TYPE_LDOUBLE:
	case TYPE_SCHAR:
	case TYPE_UCHAR:
		return false;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

bool varg_list_long_double(varg_list vargs, long double* ptr)
{
	NOT_NULL_OR_RETURN(vargs, false);
	NOT_NULL_OR_RETURN(VL_NEXT(vargs), false);

	switch (VL_NEXT(vargs)->typ)
		{
	case TYPE_PTR: 
		return false;
	case TYPE_I64:
		*ptr = (long double)VL_NEXT(vargs)->val_i64;
		break;
	case TYPE_U64:
		*ptr = (long double)VL_NEXT(vargs)->val_u64;
		break;
	case TYPE_INT:
		*ptr = (long double)VL_NEXT(vargs)->val_i;
		break;
	case TYPE_UINT:
		*ptr = (long double)VL_NEXT(vargs)->val_u;
		break;
	case TYPE_FLOAT:
		*ptr = (long double)VL_NEXT(vargs)->val_f;
		break;
	case TYPE_DOUBLE:
		*ptr = (long double)VL_NEXT(vargs)->val_g;
		break;
	case TYPE_LDOUBLE:
		*ptr = VL_NEXT(vargs)->val_lg;
		break;
	case TYPE_SCHAR:
		*ptr = (long double)VL_NEXT(vargs)->val_c;
		break;
	case TYPE_UCHAR:
		*ptr = (long double)VL_NEXT(vargs)->val_uc;
		break;
		}

	VL_NEXT(vargs) = VL_NEXT(vargs)->next;
	return true;
}

/*** Construct a new varg_list. ***/
varg_list varg_list_new(void)
{
	vlhdr* hdr = NEW(vlhdr);
	NOT_NULL_OR_RETURN(hdr, NULL);
	hdr->start = NULL;
	hdr->end = NULL;
	hdr->next = NULL;
	hdr->len = 0UL;
	return hdr;
}

/*** Helper function: allocate a new varg_list using the automatic memory manager. ***/
static varg_list varg_list_new_internal(void)
{
	vlhdr* hdr;
	if (is_null(__gc_varg))
		__gc_varg = gc_new();
	gc_set_queue_length(__gc_varg, 2048);
	hdr = (vlhdr *)gc_alloc(__gc_varg, sizeof(vlhdr));
	NOT_NULL_OR_RETURN(hdr, NULL);
	hdr->start = NULL;
	hdr->end = NULL;
	hdr->next = NULL;
	hdr->len = 0UL;
	return hdr;
}

/*** Helper function: allocate a new vl using the automatic memory manager. ***/
static vl* vl_new_internal(void)
{
	vl* vl_new;
	if (is_null(__gc_varg))
		__gc_varg = gc_new();
	gc_set_queue_length(__gc_varg, 2048);
	vl_new = (vl *)gc_alloc(__gc_varg, sizeof(vl));
	NOT_NULL_OR_RETURN(vl_new, NULL);
	return vl_new;
}

/*** Helper function: add a vl to the end of the linked list and update the vlhdr. ***/
static void __vl_add_to_end(vlhdr* hdr, vl* add_me)
{
	NOT_NULL_OR_RETURN_VOID(hdr);
	NOT_NULL_OR_RETURN_VOID(add_me);

	add_me->next = NULL;
	add_me->prev = hdr->end;
	if (hdr->start == NULL)
		{
		hdr->start = hdr->next = hdr->end = add_me;
		hdr->len = 1UL;
		return;
		}
	hdr->end->next = add_me;
	hdr->end = add_me;
	++hdr->len;
}

/*** Add arguments of the specified type to the end of the varg_list. ***/
void varg_list_add_int(varg_list vargs, int arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_INT;
	node->val_i = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_uint(varg_list vargs, uint arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_UINT;
	node->val_u = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_i64(varg_list vargs, i64 arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_I64;
	node->val_i64 = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_u64(varg_list vargs, u64 arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_U64;
	node->val_u64 = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_schar(varg_list vargs, schar arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_SCHAR;
	node->val_c = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_uchar(varg_list vargs, uchar arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_UCHAR;
	node->val_uc = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_float(varg_list vargs, float arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_FLOAT;
	node->val_f = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_double(varg_list vargs, double arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_DOUBLE;
	node->val_g = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_ptr(varg_list vargs, void* arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_PTR;
	node->val_ptr = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

void varg_list_add_long_double(varg_list vargs, long double arg)
{
	vl* node = vl_new_internal();
	NOT_NULL_OR_RETURN_VOID(node);
	node->typ = TYPE_LDOUBLE;
	node->val_lg = arg;
	__vl_add_to_end(VLHDR(vargs), node);
}

/*** Create an entire varg list in one swoop. You don't even need to free these; they are managed internally.
	Allows fun stuff like "variable_function(p, NULL, create_varg_list_ints(2, 3, 4));". ***/
varg_list create_varg_list_empty(void)
{
	/* inline city */
	return(NULL);
}

varg_list create_varg_list_ints(int nargs, ...)
{
	vlhdr* hdr = (vlhdr *)varg_list_new_internal();
	int e;
	NOT_NULL_OR_RETURN(hdr, NULL);

	va_list list;
	va_start(list, nargs);
	for (e = 0; e < nargs; ++e)
		{
		int i;
		i = va_arg(list, int);
		varg_list_add_int((varg_list)hdr, i);
		}
	va_end(list);

	return (varg_list)hdr;
}

varg_list create_varg_list_ptrs(int nargs, ...)
{
	vlhdr* hdr = (vlhdr *)varg_list_new_internal();
	int e;
	NOT_NULL_OR_RETURN(hdr, NULL);

	va_list list;
	va_start(list, nargs);
	for (e = 0; e < nargs; ++e)
		{
		void* ptr;
		ptr = va_arg(list, void*);
		varg_list_add_ptr((varg_list)hdr, ptr);
		}
	va_end(list);

	return (varg_list)hdr;
}

varg_list create_varg_list_i64s(int nargs, ...)
{
	vlhdr* hdr = (vlhdr *)varg_list_new_internal();
	int e;
	NOT_NULL_OR_RETURN(hdr, NULL);

	va_list list;
	va_start(list, nargs);
	for (e = 0; e < nargs; ++e)
		{
		i64 i;
		i = va_arg(list, i64);
		varg_list_add_i64((varg_list)hdr, i);
		}
	va_end(list);

	return (varg_list)hdr;
}

/*** Concatenate two varg lists into a third varg list that is managed internally (you don't need to free yourself).
	Can be used with the above create_varg_list() functions to make complicated, multiple-argument varg_lists. ***/
varg_list varg_list_concat(varg_list v1, varg_list v2)
{
	NOT_NULL_OR_RETURN(v1, v2);
	NOT_NULL_OR_RETURN(v2, v1);

	vlhdr* hdr = (vlhdr *)varg_list_new_internal();

	// TODO: implement concat
	assert(false);

	return (varg_list)hdr;
}

CODEHAPPY_SOURCE_BOTTOM
/*** end __vargs.c ***/
