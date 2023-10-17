/***

	port.h

	Some stuff, like drop-in darray replacements, used to help port old c_library code to libcodehappy.

	Copyright (c) 2022 Chris Street.

***/
#ifndef ___PORT_H___
#define ___PORT_H___

#include <vector>

// Porting the darray C container to C++ std::vector.

// Declaring a dynamic array.
#define	darray(typname)		std::vector< typname >
// Appending to a dynamic array.
#define	darray_append(vec, dat)	vec.push_back(dat)
#define	darray_push(vec, dat)	vec.push_back(dat)
// Initialization.
#define	darray_init(vec)	vec.clear()
// Number of elements.
#define	darray_size(vec)	vec.size()
// Element access.
#define darray_item(vec, idx)	vec[idx]
// Free
#define	darray_free(vec)	vec.clear()

// OUR_STRCPY() and friends
#ifndef OUR_STRCPY
#define	OUR_STRCPY(x, y)	strcpy(x, y)
#endif
#ifndef OUR_STRDUP
#define OUR_STRDUP(x)		strdup(x)
#endif
#ifndef OUR_MEMCPY
#define	OUR_MEMCPY(x, y, z)	memcpy(x, y, z)	
#endif
#ifndef	OUR_MALLOC
#define OUR_MALLOC(x)		malloc(x)
#endif
#ifdef CODEHAPPY_WINDOWS
#define	__stricmp(x, y)		stricmp(x, y)
#endif
#ifndef UNSAFE_FUNCTION
#define	UNSAFE_FUNCTION(str)	{ puts(str); exit(1); }
#endif

#endif  // ___PORT_H___
