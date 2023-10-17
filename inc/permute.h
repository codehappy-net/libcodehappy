/***

	permute.h

	Functions for dealing with permutations, sub-sets, and n choose k.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef PERMUTE_H
#define PERMUTE_H

/*** 

	Subset callback type. 

	The first argument is the subset/permutation.

	The second argument is the number of elements in the subset.

	The third argument is a void pointer provided from the calling function.

	For permutations, the order matters.

	If this callback returns anything other than 0, the iteration will stop.

***/
typedef i32 (*SubsetCallback)(int*, u32, void*);

/***

	Call the specified callback function for each permutation
	of the integers in the array.

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
extern i32 call_all_permutations(int array[], u32 n, SubsetCallback callback, void* args);

/***

	Call the specified callback function for each subset
	of the integers in the array

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
extern i32 call_all_subsets(int array[], u32 n, SubsetCallback callback, void* args);

/***

	Call the specified callback function for each k-element subset
	of the n integers in the array.

	The pointer "args" is passed as-is to the callback function.

	The return value is 0 if all possibilities were run through. If the callback
	function returns a non-zero value to exit early, that becomes the return value of this function.

***/
extern i32 call_all_nchoosek(int array[], u32 n, u32 k, SubsetCallback callback, void* args);

#endif  // PERMUTE_H
