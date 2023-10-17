/***

	permute.cpp

	Functions for dealing with permutations, sub-sets, and
	n choose k.

	Copyright (c) 2014-2022 C. M. Street

***/
#include "libcodehappy.h"

static i32 __call_all_permutations_helper(u32 flags, u32 chosen, int array[], int parray[], u32 n, SubsetCallback callback, void* args) {
	u32 ret;
	u32 e;
	
	if (chosen == n) {
		ret = callback(parray, n, args);
		return(ret);
	}

	for (e = 0; e < n; ++e) {
		if ((flags & (1 << e)) == 0) {
			parray[chosen] = array[e];
			ret = __call_all_permutations_helper(flags | (1 << e), chosen + 1, array, parray, n, callback, args);
			if (ret != 0L)
				return(ret);
		}
	}

	return(0L);
}

/***

	Call the specified callback function for each permutation
	of the integers in the array.

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
i32 call_all_permutations(int array[], u32 n, SubsetCallback callback, void* args) {
	i32 ret;
	int parray[32];

	// 32! is pretty darn big anyway
	if (n >= 32)
		return(-1);

	ret = __call_all_permutations_helper(0UL, 0UL, array, parray, n, callback, args);

	return(ret);
}

// TODO: call all _unique_ permutations or n choose ks?

static u32 __make_subset_helper(int array[], int parray[], u32 flags) {
	int e;
	u32 m;
	u32 c;

	m = 1UL;
	c = 0UL;
	for (e = 0; e < 32; ++e) {
		if ((flags & m) != 0) {
			parray[c] = array[e];
			++c;
		}

		m <<= 1;
	}

	return(c);
}

/***

	Call the specified callback function for each subset
	of the integers in the array

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
i32 call_all_subsets(int array[], u32 n, SubsetCallback callback, void* args) {
	u32 mask;
	u32 mask_max;
	int parray[32];
	u32 nn;
	i32 ret;
	
	// 2^32 is pretty big, too
	if (n > 31)
		return(-1);

	mask_max = (1 << n);
	mask = 0UL;

	while (mask < mask_max) {
		nn = __make_subset_helper(array, parray, mask);
		ret = callback(parray, nn, args);
		if (0 != ret)
			return(ret);
		++mask;
	}

	return(0L);
}

/***

	Call the specified callback function for each k-element subset
	of the n integers in the array.

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
i32 call_all_nchoosek(int array[], u32 n, u32 k, SubsetCallback callback, void* args) {
	u32 mask;
	u32 mask_max;
	int parray[32];
	i32 ret;

	if (n > 31 || k > 31)
		return(-1);
	if (k > n)
		return(-1);

	mask = (1 << k) - 1;
	mask_max = 1 << n;

	forever {
		u32 maskn;

		__make_subset_helper(array, parray, mask);

		ret = callback(parray, k, args);
		if (0L != ret)
			return(ret);
		
		maskn = next_same_number_1_bits(mask);
		if (maskn <= mask)
			break;
		if (maskn >= mask_max)
			break;
		mask = maskn;
	}

	return(0L);
}

/*** end permute.cpp ***/
