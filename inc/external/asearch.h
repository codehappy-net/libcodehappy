/**
 * asearch - typesafe binary search (bsearch)
 *
 * An ordered array of objects can be efficiently searched using a binary
 * search algorithm; the time taken is around log(number of elements).
 *
 * This version uses macros to be typesafe on platforms which support it.
 *
 * Author: Rusty Russell <rusty@rustcorp.com.au>
 *
 * Example:
 *	#include <ccan/asearch/asearch.h>
 *	#include <stdio.h>
 *	#include <string.h>
 *	
 *	static int cmp(const char *key, char *const *elem)
 *	{
 *		return strcmp(key, *elem);
 *	}
 *	
 *	int main(int argc, char *argv[])
 *	{
 *		char **p;
 *	
 *		if (argc < 2) {
 *			fprintf(stderr, "Usage: %s <key> <list>...\n"
 *				"Print position of key in (sorted) list\n",
 *				argv[0]);
 *			exit(1);
 *		}
 *	
 *		p = asearch(argv[1], &argv[2], argc-2, cmp);
 *		if (!p) {
 *			printf("Not found!\n");
 *			return 1;
 *		}
 *		printf("%u\n", (int)(p - &argv[2]));
 *		return 0;
 *	}
 */

#ifndef CCAN_ASEARCH_H
#define CCAN_ASEARCH_H
#include "typesafe_cb.h"

/**
 * asearch - search an array of elements
 * @key: pointer to item being searched for
 * @base: pointer to data to search
 * @num: number of elements
 * @cmp: pointer to comparison function
 *
 * This function does a binary search on the given array.  The
 * contents of the array should already be in ascending sorted order
 * under the provided comparison function.
 *
 * Note that the key need not have the same type as the elements in
 * the array, e.g. key could be a string and the comparison function
 * could compare the string with the struct's name field.  However, if
 * the key and elements in the array are of the same type, you can use
 * the same comparison function for both sort() and asearch().
 */
#if __GNUC__
#define asearch(key, base, num, cmp)					\
	((__typeof__(*(base))*)(bsearch((key), (base), (num), sizeof(*(base)), \
		typesafe_cb_cast(int (*)(const void *, const void *),	\
				 int (*)(const __typeof__(*(key)) *,	\
					 const __typeof__(*(base)) *),	\
				 (cmp)))))

#else
#define asearch(key, base, num, cmp)				\
	(bsearch((key), (base), (num), sizeof(*(base)),		\
		 (int (*)(const void *, const void *))(cmp)))
#endif

#endif /* CCAN_ASEARCH_H */
