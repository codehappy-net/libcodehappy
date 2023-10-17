/**
 * typesafe_cb - macros for safe callbacks.
 *
 * The basis of the typesafe_cb header is typesafe_cb_cast(): a
 * conditional cast macro.   If an expression exactly matches a given
 * type, it is cast to the target type, otherwise it is left alone.
 *
 * This allows us to create functions which take a small number of
 * specific types, rather than being forced to use a void *.  In
 * particular, it is useful for creating typesafe callbacks as the
 * helpers typesafe_cb(), typesafe_cb_preargs() and
 * typesafe_cb_postargs() demonstrate.
 * 
 * The standard way of passing arguments to callback functions in C is
 * to use a void pointer, which the callback then casts back to the
 * expected type.  This unfortunately subverts the type checking the
 * compiler would perform if it were a direct call.  Here's an example:
 *
 *	static void my_callback(void *_obj)
 *	{
 *		struct obj *obj = _obj;
 *		...
 *	}
 *	...
 *		register_callback(my_callback, &my_obj);
 *
 * If we wanted to use the natural type for my_callback (ie. "void
 * my_callback(struct obj *obj)"), we could make register_callback()
 * take a void * as its first argument, but this would subvert all
 * type checking.  We really want register_callback() to accept only
 * the exactly correct function type to match the argument, or a
 * function which takes a void *.
 *
 * This is where typesafe_cb() comes in: it uses typesafe_cb_cast() to
 * cast the callback function if it matches the argument type:
 *
 *	void _register_callback(void (*cb)(void *arg), void *arg);
 *	#define register_callback(cb, arg)				\
 *		_register_callback(typesafe_cb(void, void *, (cb), (arg)), \
 *				   (arg))
 *
 * On compilers which don't support the extensions required
 * typesafe_cb_cast() and friend become an unconditional cast, so your
 * code will compile but you won't get type checking.
 *
 * Example:
 *	#include <ccan/typesafe_cb/typesafe_cb.h>
 *	#include <stdlib.h>
 *	#include <stdio.h>
 *
 *	// Generic callback infrastructure.
 *	struct callback {
 *		struct callback *next;
 *		int value;
 *		int (*callback)(int value, void *arg);
 *		void *arg;
 *	};
 *	static struct callback *callbacks;
 *	
 *	static void _register_callback(int value, int (*cb)(int, void *),
 *				       void *arg)
 *	{
 *		struct callback *new = malloc(sizeof(*new));
 *		new->next = callbacks;
 *		new->value = value;
 *		new->callback = cb;
 *		new->arg = arg;
 *		callbacks = new;
 *	}
 *	#define register_callback(value, cb, arg)			\
 *		_register_callback(value,				\
 *				   typesafe_cb_preargs(int, void *,	\
 *						       (cb), (arg), int),\
 *				   (arg))
 *	
 *	static struct callback *find_callback(int value)
 *	{
 *		struct callback *i;
 *	
 *		for (i = callbacks; i; i = i->next)
 *			if (i->value == value)
 *				return i;
 *		return NULL;
 *	}   
 *
 *	// Define several silly callbacks.  Note they don't use void *!
 *	#define DEF_CALLBACK(name, op)			\
 *		static int name(int val, int *arg)	\
 *		{					\
 *			printf("%s", #op);		\
 *			return val op *arg;		\
 *		}
 *	DEF_CALLBACK(multiply, *);
 *	DEF_CALLBACK(add, +);
 *	DEF_CALLBACK(divide, /);
 *	DEF_CALLBACK(sub, -);
 *	DEF_CALLBACK(or, |);
 *	DEF_CALLBACK(and, &);
 *	DEF_CALLBACK(xor, ^);
 *	DEF_CALLBACK(assign, =);
 *
 *	// Silly game to find the longest chain of values.
 *	int main(int argc, char *argv[])
 *	{
 *		int i, run = 1, num = argv[1] ? atoi(argv[1]) : 0;
 *	
 *		for (i = 1; i < 1024;) {
 *			// Since run is an int, compiler checks "add" does too.
 *			register_callback(i++, add, &run);
 *			register_callback(i++, divide, &run);
 *			register_callback(i++, sub, &run);
 *			register_callback(i++, multiply, &run);
 *			register_callback(i++, or, &run);
 *			register_callback(i++, and, &run);
 *			register_callback(i++, xor, &run);
 *			register_callback(i++, assign, &run);
 *		}
 *	
 *		printf("%i ", num);
 *		while (run < 56) {
 *			struct callback *cb = find_callback(num % i);
 *			if (!cb) {
 *				printf("-> STOP\n");
 *				return 1;
 *			}
 *			num = cb->callback(num, cb->arg);
 *			printf("->%i ", num);
 *			run++;
 *		}
 *		printf("-> Winner!\n");
 *		return 0;
 *	}
 *
 * License: CC0 (Public domain)
 * Author: Rusty Russell <rusty@rustcorp.com.au>
 */

/* CC0 (Public domain) - see LICENSE file for details */
#ifndef CCAN_TYPESAFE_CB_H
#define CCAN_TYPESAFE_CB_H


#ifdef __cplusplus
#define BUILTINS_OK	0
#else
#if __GNUC
#define BUILTINS_OK	1
#else
#define BUILTINS_OK	0
#endif
#endif

#if BUILTINS_OK
/**
 * typesafe_cb_cast - only cast an expression if it matches a given type
 * @desttype: the type to cast to
 * @oktype: the type we allow
 * @expr: the expression to cast
 *
 * This macro is used to create functions which allow multiple types.
 * The result of this macro is used somewhere that a @desttype type is
 * expected: if @expr is exactly of type @oktype, then it will be
 * cast to @desttype type, otherwise left alone.
 *
 * This macro can be used in static initializers.
 *
 * This is merely useful for warnings: if the compiler does not
 * support the primitives required for typesafe_cb_cast(), it becomes an
 * unconditional cast, and the @oktype argument is not used.  In
 * particular, this means that @oktype can be a type which uses the
 * "typeof": it will not be evaluated if typeof is not supported.
 *
 * Example:
 *	// We can take either an unsigned long or a void *.
 *	void _set_some_value(void *val);
 *	#define set_some_value(e)			\
 *		_set_some_value(typesafe_cb_cast(void *, (e), unsigned long))
 */
#define typesafe_cb_cast(desttype, oktype, expr)			\
	__builtin_choose_expr(						\
		__builtin_types_compatible_p(__typeof__(0?(expr):(expr)), \
					     oktype),			\
		(desttype)(expr), (expr))
#else
#define typesafe_cb_cast(desttype, oktype, expr) ((desttype)(expr))
#endif

/**
 * typesafe_cb_cast3 - only cast an expression if it matches given types
 * @desttype: the type to cast to
 * @ok1: the first type we allow
 * @ok2: the second type we allow
 * @ok3: the third type we allow
 * @expr: the expression to cast
 *
 * This is a convenient wrapper for multiple typesafe_cb_cast() calls.
 * You can chain them inside each other (ie. use typesafe_cb_cast()
 * for expr) if you need more than 3 arguments.
 *
 * Example:
 *	// We can take either a long, unsigned long, void * or a const void  *
 *	void _set_some_value(void *val);
 *	#define set_some_value(expr)					\
 *		_set_some_value(typesafe_cb_cast3(void *,,		\
 *					    long, unsigned long, const void *,\
 *					    (expr)))
 */
#define typesafe_cb_cast3(desttype, ok1, ok2, ok3, expr)		\
	typesafe_cb_cast(desttype, ok1,					\
			 typesafe_cb_cast(desttype, ok2,		\
					  typesafe_cb_cast(desttype, ok3, \
							   (expr))))

/**
 * typesafe_cb - cast a callback function if it matches the arg
 * @rtype: the return type of the callback function
 * @atype: the (pointer) type which the callback function expects.
 * @fn: the callback function to cast
 * @arg: the (pointer) argument to hand to the callback function.
 *
 * If a callback function takes a single argument, this macro does
 * appropriate casts to a function which takes a single atype argument if the
 * callback provided matches the @arg.
 *
 * It is assumed that @arg is of pointer type: usually @arg is passed
 * or assigned to a void * elsewhere anyway.
 *
 * Example:
 *	void _register_callback(void (*fn)(void *arg), void *arg);
 *	#define register_callback(fn, arg) \
 *		_register_callback(typesafe_cb(void, (fn), void*, (arg)), (arg))
 */
#define typesafe_cb(rtype, atype, fn, arg)			\
	typesafe_cb_cast(rtype (*)(atype),			\
			 rtype (*)(__typeof__(arg)),		\
			 (fn))

/**
 * typesafe_cb_preargs - cast a callback function if it matches the arg
 * @rtype: the return type of the callback function
 * @atype: the (pointer) type which the callback function expects.
 * @fn: the callback function to cast
 * @arg: the (pointer) argument to hand to the callback function.
 *
 * This is a version of typesafe_cb() for callbacks that take other arguments
 * before the @arg.
 *
 * Example:
 *	void _register_callback(void (*fn)(int, void *arg), void *arg);
 *	#define register_callback(fn, arg)				   \
 *		_register_callback(typesafe_cb_preargs(void, void *,	   \
 *				   (fn), (arg), int),			   \
 *				   (arg))
 */
#define typesafe_cb_preargs(rtype, atype, fn, arg, ...)			\
	typesafe_cb_cast(rtype (*)(__VA_ARGS__, atype),			\
			 rtype (*)(__VA_ARGS__, __typeof__(arg)),	\
			 (fn))

/**
 * typesafe_cb_postargs - cast a callback function if it matches the arg
 * @rtype: the return type of the callback function
 * @atype: the (pointer) type which the callback function expects.
 * @fn: the callback function to cast
 * @arg: the (pointer) argument to hand to the callback function.
 *
 * This is a version of typesafe_cb() for callbacks that take other arguments
 * after the @arg.
 *
 * Example:
 *	void _register_callback(void (*fn)(void *arg, int), void *arg);
 *	#define register_callback(fn, arg) \
 *		_register_callback(typesafe_cb_postargs(void, (fn), void *, \
 *				   (arg), int),				    \
 *				   (arg))
 */
#define typesafe_cb_postargs(rtype, atype, fn, arg, ...)		\
	typesafe_cb_cast(rtype (*)(atype, __VA_ARGS__),			\
			 rtype (*)(__typeof__(arg), __VA_ARGS__),	\
			 (fn))
#endif /* CCAN_CAST_IF_TYPE_H */
