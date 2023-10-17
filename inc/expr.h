/***

	expr.h

	Expression parser for libcodehappy.

	Functions and operators can be defined. Can be used as an in-app
	programming/macro language, a programming language interpreter,
	for rich input, even JIT code.

	Copyright (c) 2022 C. M. Street

***/
#ifndef __EXPR_H__
#define __EXPR_H__

/* Function (operator) syntax. */
enum FuncSynx {
	FUNCTION_INFIX = 0,
	FUNCTION_PREFIX,
	FUNCTION_POSTFIX,
	FUNCTION_PAREN,
	FUNCTION_SQBRACKETS,
	FUNCTION_CBRACKETS,
};

/* Operators. */
struct ExprOp {
	ExprOp();
	~ExprOp();
	// id/bytecode representation of this operator.
	u32 opcode;
	// text that the tokenizer can convert into this operator.
	const char* text;
	// this operator's function syntax
	FuncSynx fsyx;
	// precedence (order of operations): lowest gets evaluated first
	int prec;
	// number of arguments
	int carg;
	// enabled?
	bool enabled;
};

/* This enum corresponds to the op-codes for built-in operators. They come
   with a standard syntax, but you can modify the ExprOp directly. */
enum BuiltInOps {
	BUILTIN_ADD = 0,
	BUILTIN_SUB,
	BUILTIN_MUL,
	BUILTIN_DIV,
	BUILTIN_NEG,
	BUILTIN_EXP,
	BUILTIN_LOGAND,
	BUILTIN_LOGOR,
	BUILTIN_LOGXOR,
	BUILTIN_LOGNOT,
	BUILTIN_BWAND,
	BUILTIN_BWOR,
	BUILTIN_BWXOR,
	BUILTIN_BWNOT,

	// not a built-in op, this just gives the first non-builtin opcode
	NON_BUILTIN_START,
};

/* Representing an expression. Can be parsed from string input. */
class Expr {
};

#endif  // __EXPR_H__
/* end expr.h */