/***

	expeval.h

	Expression evaluator class.

	Supports user-defined operators (in-fix binary, pre-fix or post-fix unary), user-defined functions (with any number
	of arguments), and user-defined variables which can be assigned a value at evaluation time.

	C. M. Street

***/
#ifndef __EXPEVAL_H__
#define __EXPEVAL_H__

#include <stack>
#include <sstream>
#include <cctype>
#include <memory>
#include <functional>
#include <algorithm>

/* error codes that might be returned as values */
enum ErrorCode {
	ERR_DIV_BY_ZERO, ERR_FN_DOMAIN, ERR_OVERFLOW, ERR_OUT_OF_BOUNDS, ERR_UNDEFINED, 
	ERR_USER_DEFINED_1, ERR_USER_DEFINED_2, ERR_USER_DEFINED_3, ERR_USER_DEFINED_4
};

/* operator types */
enum OperatorType {
	OP_PREFIX, OP_INFIX, OP_POSTFIX
};

/* value types */
enum ValueType {
	VALUE_INT, VALUE_DOUBLE, VALUE_ERROR, VALUE_ARRAY
};

struct ExpValue {
	// c'tors/d'tor
	ExpValue() : vt(VALUE_INT), nel_array(1), i_val(0) {}

	ExpValue(int64_t int_value) : vt(VALUE_INT), nel_array(1), i_val(int_value) {}

	ExpValue(double dbl_value) : vt(VALUE_DOUBLE), nel_array(1), d_val(dbl_value) {}

	ExpValue(ErrorCode err_value) : vt(VALUE_ERROR), nel_array(1), e_val(err_value) {}

	ExpValue(size_t nel, ExpValue* array_value) : vt(VALUE_ARRAY), nel_array(nel) {
		a_val = new ExpValue[nel];
		for (size_t i = 0; i < nel; ++i) {
			a_val[i] = array_value[i];
		}
	}
	
	~ExpValue() {
		if (vt == VALUE_ARRAY) {
			delete[] a_val;
		}
	}

	ValueType vt;
	size_t nel_array; // only needed if vt == VALUE_ARRAY. equal to 1 for any other type.
	union {
		int64_t   i_val;
		double    d_val;
		ErrorCode e_val;
		ExpValue* a_val;
	};

	// Array element operator
	ExpValue& operator [](size_t idx) {
		static ExpValue err(ERR_OUT_OF_BOUNDS);
		if (vt == VALUE_ARRAY) {
			if (idx < nel_array) {
				return a_val[idx];
			} else {
				return err;
			}
		} else {
			if (idx == 0) {
				return *this;
			} else {
				return err;
			}
		}
	}

	// Set an integer value directly
	void set_int_value(int64_t new_val) {
		if (vt == VALUE_ARRAY && a_val != nullptr) {
			delete a_val;
		}
		vt = VALUE_INT;
		i_val = new_val;
	}

	// Set floating point value
	void set_double_value(double new_val) {
		if (vt == VALUE_ARRAY && a_val != nullptr) {
			delete a_val;
		}
		vt = VALUE_DOUBLE;
		d_val = new_val;
	}

	// Set error value directly
	void set_error_value(ErrorCode new_val) {
		if (vt == VALUE_ARRAY && a_val != nullptr) {
			delete a_val;
		}
		vt = VALUE_ERROR;
		e_val = new_val;
	}

	// Reserve space for an array
	void reserve_array(size_t nel) {
		if (vt == VALUE_ARRAY) {
			if (nel == nel_array) return;

			ExpValue* new_array = new ExpValue[nel];
			size_t copy_size = std::min(nel, nel_array);
			for (size_t i = 0; i < copy_size; ++i) {
				new_array[i] = a_val[i];
			}
			for (size_t i = copy_size; i < nel; ++i) {
				new_array[i] = ExpValue((int64_t) 0);
			}
			delete[] a_val;
			a_val = new_array;
			nel_array = nel;
		} else {
			ExpValue old_value = *this; // copy current value
			a_val = new ExpValue[nel];
			for (size_t i = 0; i < nel; ++i) {
				if (i == 0) {
					a_val[i] = old_value;
				} else {
					a_val[i] = ExpValue((int64_t) 0);
				}
			}
			nel_array = nel;
			vt = VALUE_ARRAY;
		}
	}

	/* return a string representation of this value */
	std::string str() const {
		switch (vt) {
		case VALUE_ARRAY:
			return "<array>";
		case VALUE_INT:
			return std::to_string(i_val);
		case VALUE_DOUBLE:
			return std::to_string(d_val);
		case VALUE_ERROR:
			switch (e_val) {
			default:
				return "<error>";
			case ERR_DIV_BY_ZERO:
				return "<div-by-zero>";
			case ERR_FN_DOMAIN:
				return "<fn-domain>";
			case ERR_OVERFLOW:
				return "<overflow>";
			case ERR_OUT_OF_BOUNDS:
				return "<out-of-bounds>";
			case ERR_UNDEFINED:
				return "<undefined>";
			}
		}
		return "";
	}

};

// forward declaration
class ExpressionEvaluator;

/* function callback type */
typedef void (*EvaluationCallbackType)(ExpValue& value_in, ExpValue& value_out);

// Token types
enum TokenType {
	TOKEN_NUMBER,
	TOKEN_VARIABLE,
	TOKEN_OPERATOR,
	TOKEN_FUNCTION,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_COMMA,
	TOKEN_TRUE,
	TOKEN_FALSE
};

struct Token {
	TokenType type;
	std::string representation;
	OperatorType op_type;
	int precedence;
	EvaluationCallbackType callback;
	
	// For numbers and variables
	ExpValue value;
	
	Token() : type(TOKEN_NUMBER), op_type(OP_INFIX), precedence(0), callback(nullptr) {}
	Token(TokenType t, const std::string& rep = "") : type(t), representation(rep), op_type(OP_INFIX), precedence(0), callback(nullptr) {}
};

class ExpressionEvaluator {
public:
	// Makes an empty expression; use the set() function to populate
	ExpressionEvaluator() {
		initializeBuiltIns();
	}

	// Makes an expression out of the passed string
	ExpressionEvaluator(const std::string& expression_str) {
		initializeBuiltIns();
		set(expression_str);
	}

	~ExpressionEvaluator() {}

	// Set the expression string
	void set(const std::string& expression_str) {
		expression = expression_str;
		tokens.clear();
		rpn.clear();
		variables.clear();
		tokenize();
		buildRPN();
		extractVariables();
	}

	// Return the number of different named variables in this expression
	size_t count_variables() const {
		return variables.size();
	}

	// Return the name of the i'th named variable in this expression
	std::string variable_name(size_t i) const {
		if (i >= variables.size()) return "";
		auto it = variables.begin();
		std::advance(it, i);
		return it->first;
	}

	// Get the value of a variable by index or name
	ExpValue& get_variable(size_t i) {
		static ExpValue err(ERR_OUT_OF_BOUNDS);
		if (i >= variables.size()) {
			return err;
		}
		auto it = variables.begin();
		std::advance(it, i);
		return it->second;
	}

	ExpValue& get_variable(const std::string& name) {
		auto it = variables.find(name);
		static ExpValue err(ERR_OUT_OF_BOUNDS);
		if (it != variables.end()) {
			return it->second;
		} else {
			return err;
		}
	}

	// Set the value of a variable
	int set_variable(const std::string& variable_name, const ExpValue& new_value) {
		auto it = variables.find(variable_name);
		if (it != variables.end()) {
			it->second = new_value;
			return 0;
		}
		return 1; // Variable not found
	}

	int set_variable(size_t i, const ExpValue& new_value) {
		if (i >= variables.size()) return 1;
		auto it = variables.begin();
		std::advance(it, i);
		it->second = new_value;
		return 0;
	}

	// Assign one variable's value to another
	int set_variable(const std::string& variable_dest, const std::string& variable_src) {
		auto src_it = variables.find(variable_src);
		auto dst_it = variables.find(variable_dest);
		if (src_it != variables.end() && dst_it != variables.end()) {
			dst_it->second = src_it->second;
			return 0;
		}
		return 1; // One or both variables not found
	}

	int set_variable(int i_dest, int i_src) {
		if (i_dest >= (int)variables.size() || i_src >= (int)variables.size()) return 1;
		auto dest_it = variables.begin();
		auto src_it = variables.begin();
		std::advance(dest_it, i_dest);
		std::advance(src_it, i_src);
		dest_it->second = src_it->second;
		return 0;
	}

	// Define a new user-specified operator
	int define_new_op(const std::string& str_representation, OperatorType op_type, int op_priority, EvaluationCallbackType eval_callback) {
		if (op_priority < 0) return 1; // Invalid priority
		
		// Check if this operator already exists
		for (auto& op : operators) {
			if (op.representation == str_representation && op.op_type == op_type) {
				return 1; // Already exists
			}
		}
		
		Token op(TOKEN_OPERATOR, str_representation);
		op.op_type = op_type;
		op.precedence = op_priority;
		op.callback = eval_callback;
		operators.push_back(op);
		return 0;
	}

	// Get or set operator priority
	int get_operator_priority(const std::string& representation, OperatorType op_type) const {
		for (const auto& op : operators) {
			if (op.representation == representation && op.op_type == op_type) {
				return op.precedence;
			}
		}
		return -1; // Not found
	}

	int set_operator_priority(const std::string& representation, OperatorType op_type, int new_priority) {
		if (new_priority < 0) return -1;
		
		for (auto& op : operators) {
			if (op.representation == representation && op.op_type == op_type) {
				op.precedence = new_priority;
				return new_priority;
			}
		}
		return -1; // Not found
	}

	// Define a new user-specified function
	int define_new_fn(const std::string& fn_name, size_t c_args, EvaluationCallbackType eval_callback) {
		// Check if function already exists
		auto it = functions.find(fn_name);
		if (it != functions.end()) {
			return 1; // Already exists
		}
		
		Function func;
		func.name = fn_name;
		func.arg_count = c_args;
		func.callback = eval_callback;
		functions[fn_name] = func;
		return 0;
	}

	// Evaluate the expression
	ExpValue evaluate() {
		std::stack<ExpValue> stack;
		static ExpValue err_undef(ERR_UNDEFINED);
		static ExpValue err_oob(ERR_OUT_OF_BOUNDS);
		
		for (const Token& token : rpn) {
			switch (token.type) {
				case TOKEN_NUMBER:
				case TOKEN_TRUE:
				case TOKEN_FALSE:
					stack.push(token.value);
					break;
					
				case TOKEN_VARIABLE: {
					auto it = variables.find(token.representation);
					if (it != variables.end()) {
						stack.push(it->second);
					} else {
						return err_oob;
					}
					break;
				}
				
				case TOKEN_FUNCTION: {
					// Pop arguments from stack
					ExpValue args;
					args.reserve_array(token.value.i_val); // Number of args stored in i_val
					
					// Pop arguments in reverse order
					for (int i = token.value.i_val - 1; i >= 0; --i) {
						if (stack.empty()) return err_oob;
						args[i] = stack.top();
						stack.pop();
					}
					
					// Evaluate function
					ExpValue result;
					auto func_it = functions.find(token.representation);
					if (func_it != functions.end() && func_it->second.callback) {
						func_it->second.callback(args, result);
					} else {
						result = err_undef;
					}
					
					// Check for error
					if (result.vt == VALUE_ERROR) {
						return result;
					}
					
					stack.push(result);
					break;
				}
				
				case TOKEN_OPERATOR: {
					ExpValue result;
					
					// Handle different operator types
					if (token.op_type == OP_PREFIX) {
						// Unary prefix: pop one operand
						if (stack.empty()) return err_oob;
						ExpValue operand = stack.top();
						stack.pop();
						
						// Special handling for logical operators (short-circuiting)
						if (token.representation == "!" && operand.vt == VALUE_INT) {
							// Logical NOT with short-circuit
							result = ExpValue(operand.i_val == 0 ? (int64_t) 1 : (int64_t) 0);
						} else if (token.callback) {
							token.callback(operand, result);
						}
						
						// Check for error
						if (result.vt == VALUE_ERROR) {
							return result;
						}
						
						stack.push(result);
						
					} else if (token.op_type == OP_POSTFIX) {
						// Unary postfix: pop one operand
						if (stack.empty()) return err_oob;
						ExpValue operand = stack.top();
						stack.pop();
						
						if (token.callback) {
							token.callback(operand, result);
						}
						
						// Check for error
						if (result.vt == VALUE_ERROR) {
							return result;
						}
						
						stack.push(result);
						
					} else if (token.op_type == OP_INFIX) {
						// Binary: pop two operands (right then left)
						if (stack.size() < 2) return err_oob;
						ExpValue right = stack.top();
						stack.pop();
						ExpValue left = stack.top();
						stack.pop();
						
						// Special handling for logical operators (short-circuiting)
						if (token.representation == "&&" && left.vt == VALUE_INT) {
							if (left.i_val == 0) {
								// Short-circuit: don't evaluate right operand
								result = ExpValue((int64_t) 0);
							} else {
								// Need to evaluate right operand
								if (right.vt == VALUE_ERROR) {
									return right;
								}
								// Convert to boolean
								int64_t right_bool = (right.vt == VALUE_INT) ? 
									(right.i_val != 0 ? 1 : 0) : 
									(right.d_val != 0.0 ? 1 : 0);
								result = ExpValue(right_bool);
							}
						} else if (token.representation == "||" && left.vt == VALUE_INT) {
							if (left.i_val != 0) {
								// Short-circuit: don't evaluate right operand
								result = ExpValue((int64_t) 1);
							} else {
								// Need to evaluate right operand
								if (right.vt == VALUE_ERROR) {
									return right;
								}
								// Convert to boolean
								int64_t right_bool = (right.vt == VALUE_INT) ? 
									(right.i_val != 0 ? 1 : 0) : 
									(right.d_val != 0.0 ? 1 : 0);
								result = ExpValue(right_bool);
							}
						} else {
							// Standard infix operator
							ExpValue operands;
							operands.reserve_array(2);
							operands[0] = left;
							operands[1] = right;
							
							if (token.callback) {
								token.callback(operands, result);
							}
						}
						
						// Check for error
						if (result.vt == VALUE_ERROR) {
							return result;
						}
						
						stack.push(result);
					}
					break;
				}
				
				default:
					break;
			}
		}
		
		// Final result should be the only thing on the stack
		if (stack.empty()) {
			return err_undef;
		}
		
		return stack.top();
	}

private:
	struct Function {
		std::string name;
		size_t arg_count;
		EvaluationCallbackType callback;
	};
	
	std::string expression;
	std::vector<Token> tokens;
	std::vector<Token> rpn;
	std::map<std::string, ExpValue> variables;
	std::vector<Token> operators;
	std::map<std::string, Function> functions;
	
	void initializeBuiltIns() {
		// Clear existing operators and functions
		operators.clear();
		functions.clear();
		
		// Define built-in operators
		
		// Arithmetic operators
		define_new_op("+", OP_INFIX, 10, callback_add);
		define_new_op("-", OP_INFIX, 10, callback_subtract);
		define_new_op("+", OP_PREFIX, 15, callback_unary_plus);  // Unary plus
		define_new_op("-", OP_PREFIX, 15, callback_unary_minus); // Unary minus
		define_new_op("*", OP_INFIX, 20, callback_multiply);
		define_new_op("/", OP_INFIX, 20, callback_divide);
		define_new_op("\\", OP_INFIX, 20, callback_int_divide); // Integer division
		define_new_op("%", OP_INFIX, 20, callback_modulo);
		define_new_op("^", OP_INFIX, 30, callback_power);
		define_new_op("!", OP_POSTFIX, 15, callback_factorial); // Factorial
		
		// Comparison operators
		define_new_op("==", OP_INFIX, 5, callback_equal);
		define_new_op("!=", OP_INFIX, 5, callback_not_equal);
		define_new_op("<", OP_INFIX, 5, callback_less_than);
		define_new_op("<=", OP_INFIX, 5, callback_less_equal);
		define_new_op(">", OP_INFIX, 5, callback_greater_than);
		define_new_op(">=", OP_INFIX, 5, callback_greater_equal);
		
		// Logical operators
		define_new_op("!", OP_PREFIX, 15, callback_logical_not);
		define_new_op("&&", OP_INFIX, 3, callback_logical_and);
		define_new_op("||", OP_INFIX, 2, callback_logical_or);
		
		// Define built-in functions
		define_new_fn("abs", 1, callback_abs);
		define_new_fn("sgn", 1, callback_sgn);
		define_new_fn("floor", 1, callback_floor);
		define_new_fn("ceil", 1, callback_ceil);
		define_new_fn("exp", 1, callback_exp);
		define_new_fn("ln", 1, callback_ln);
		define_new_fn("pow", 2, callback_pow);
		define_new_fn("sqrt", 1, callback_sqrt);
		define_new_fn("gamma", 1, callback_gamma);
		define_new_fn("sin", 1, callback_sin);
		define_new_fn("cos", 1, callback_cos);
		define_new_fn("tan", 1, callback_tan);
		define_new_fn("asin", 1, callback_asin);
		define_new_fn("acos", 1, callback_acos);
		define_new_fn("atan", 1, callback_atan);
		define_new_fn("atan2", 2, callback_atan2);
	}
	
	void tokenize() {
		tokens.clear();
		std::string cleaned;
		
		// Remove whitespace
		for (char c : expression) {
			if (!std::isspace(c)) {
				cleaned += c;
			}
		}
		
		size_t i = 0;
		while (i < cleaned.size()) {
			char c = cleaned[i];
			
			// Handle numbers
			if (std::isdigit(c) || (c == '.' && i + 1 < cleaned.size() && std::isdigit(cleaned[i+1]))) {
				std::string num_str;
				bool has_decimal = false;
				
				while (i < cleaned.size() && (std::isdigit(cleaned[i]) || (!has_decimal && cleaned[i] == '.'))) {
					if (cleaned[i] == '.') has_decimal = true;
					num_str += cleaned[i++];
				}
				
				Token token(TOKEN_NUMBER);
				if (has_decimal) {
					token.value = ExpValue(std::stod(num_str));
				} else {
					token.value = ExpValue((int64_t) std::stoll(num_str));
				}
				tokens.push_back(token);
				continue;
			}
			
			// Handle identifiers (variables, functions, true/false)
			if (std::isalpha(c) || c == '_') {
				std::string ident;
				while (i < cleaned.size() && (std::isalnum(cleaned[i]) || cleaned[i] == '_')) {
					ident += cleaned[i++];
				}
				
				// Check for boolean literals
				if (ident == "true") {
					tokens.push_back(Token(TOKEN_TRUE));
					tokens.back().value = ExpValue((int64_t) 1);
				} else if (ident == "false") {
					tokens.push_back(Token(TOKEN_FALSE));
					tokens.back().value = ExpValue((int64_t) 0);
				} else {
					// Check if it's a function
					bool is_function = false;
					if (i < cleaned.size() && cleaned[i] == '(') {
						is_function = true;
						Token token(TOKEN_FUNCTION, ident);
						tokens.push_back(token);
					} else {
						// It's a variable
						Token token(TOKEN_VARIABLE, ident);
						tokens.push_back(token);
					}
				}
				continue;
			}
			
			// Handle multi-character operators
			if (i + 1 < cleaned.size()) {
				std::string two_chars = cleaned.substr(i, 2);
				if (two_chars == "==" || two_chars == "!=" || two_chars == "<=" || 
					two_chars == ">=" || two_chars == "&&" || two_chars == "||") {
					addOperatorToken(two_chars, i);
					i += 2;
					continue;
				}
			}
			
			// Handle single-character operators and delimiters
			std::string one_char(1, c);
			if (c == '(') {
				Token tok(TOKEN_LPAREN, "(");
				tok.precedence = 999;
				tok.callback = nullptr;
				tokens.push_back(tok);
			} else if (c == ')') {
				Token tok(TOKEN_RPAREN, ")");
				tok.precedence = 999;
				tok.callback = nullptr;
				tokens.push_back(tok);
			} else if (c == ',') {
				Token tok(TOKEN_COMMA, ",");
				tok.precedence = 1;
				tok.callback = nullptr;
				tokens.push_back(tok);
			} else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || 
				c == '!' || c == '<' || c == '>' || c == '\\') {
				addOperatorToken(one_char, i);
				i++;
				continue;
			}
			
			i++; // Skip unrecognized character
		}
	}
	
	void addOperatorToken(const std::string& op_str, size_t& i) {
		// Special handling for operators that could be prefix or infix
		bool is_prefix = false;
		
		if ((op_str == "+" || op_str == "-") && 
			(tokens.empty() || tokens.back().type == TOKEN_OPERATOR || 
			 tokens.back().representation == "(" || tokens.back().type == TOKEN_FUNCTION)) {
			// This is a unary prefix operator
			for (const auto& op : operators) {
				if (op.representation == op_str && op.op_type == OP_PREFIX) {
					Token token(TOKEN_OPERATOR, op_str);
					token.op_type = OP_PREFIX;
					token.precedence = op.precedence;
					token.callback = op.callback;
					tokens.push_back(token);
					return;
				}
			}
		}
		
		// Look for matching operator
		for (const auto& op : operators) {
			if (op.representation == op_str) {
				Token token(TOKEN_OPERATOR, op_str);
				token.op_type = op.op_type;
				token.precedence = op.precedence;
				token.callback = op.callback;
				tokens.push_back(token);
				return;
			}
		}
		
		// If not found, just add as is (will be handled during RPN conversion)
		Token token(TOKEN_OPERATOR, op_str);
		tokens.push_back(token);
	}
	
	void buildRPN() {
		rpn.clear();
		std::stack<Token> op_stack;
		
		for (size_t i = 0; i < tokens.size(); i++) {
			Token token = tokens[i];
			
			switch (token.type) {
				case TOKEN_NUMBER:
				case TOKEN_VARIABLE:
				case TOKEN_TRUE:
				case TOKEN_FALSE:
					rpn.push_back(token);
					break;
					
				case TOKEN_FUNCTION:
					op_stack.push(token);
					break;
					
				case TOKEN_OPERATOR:
					// Handle operator precedence
					while (!op_stack.empty()) {
						Token top = op_stack.top();
						
						if (top.type == TOKEN_LPAREN) {
							break;
						}
						
						if (top.type == TOKEN_FUNCTION) {
							// Functions have high precedence
							break;
						}
						
						if (top.type == TOKEN_OPERATOR) {
							// Precedence comparison
							bool should_pop = false;
							
							if (token.op_type == OP_PREFIX) {
								// Prefix operators: only pop higher precedence operators
								should_pop = (top.precedence > token.precedence);
							} else if (token.op_type == OP_POSTFIX) {
								// Postfix operators: pop equal or higher precedence
								should_pop = (top.precedence >= token.precedence);
							} else {
								// Infix operators: left-associative, pop higher or equal precedence
								should_pop = (top.precedence >= token.precedence);
							}
							
							if (should_pop) {
								rpn.push_back(top);
								op_stack.pop();
							} else {
								break;
							}
						} else {
							break;
						}
					}
					
					op_stack.push(token);
					break;
					
				case TOKEN_LPAREN:
					op_stack.push(token);
					break;
					
				case TOKEN_RPAREN:
					// Pop until left parenthesis
					while (!op_stack.empty() && op_stack.top().type != TOKEN_LPAREN) {
						rpn.push_back(op_stack.top());
						op_stack.pop();
					}
					
					if (!op_stack.empty() && op_stack.top().type == TOKEN_LPAREN) {
						op_stack.pop(); // Remove the left parenthesis
						
						// If the token before the left parenthesis was a function, pop it
						if (!op_stack.empty() && op_stack.top().type == TOKEN_FUNCTION) {
							Token func = op_stack.top();
							op_stack.pop();
							
							// Count arguments (for function evaluation)
							ExpValue arg_count;
							// We need to determine this during parsing - simplified approach:
							// For now, we'll assume the function knows its arity
							arg_count = ExpValue((int64_t) functions[func.representation].arg_count);
							func.value = arg_count;
							rpn.push_back(func);
						}
					}
					break;
					
				case TOKEN_COMMA:
					// Pop until left parenthesis (but don't remove it)
					while (!op_stack.empty() && op_stack.top().type != TOKEN_LPAREN) {
						rpn.push_back(op_stack.top());
						op_stack.pop();
					}
					break;
					
				default:
					break;
			}
		}
		
		// Pop remaining operators
		while (!op_stack.empty()) {
			rpn.push_back(op_stack.top());
			op_stack.pop();
		}
	}
	
	void extractVariables() {
		variables.clear();
		
		for (const Token& token : tokens) {
			if (token.type == TOKEN_VARIABLE) {
				// Initialize variable with undefined value
				if (variables.find(token.representation) == variables.end()) {
					variables[token.representation] = ExpValue(ERR_UNDEFINED);
				}
			}
		}
	}
	
	// Built-in function callbacks
	
	static void callback_add(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		// Check for double operands
		bool has_double = (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE);
		
		if (has_double) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			output = ExpValue(left + right);
		} else {
			// Both are integers
			int64_t result = input[0].i_val + input[1].i_val;
			// Check for overflow
			if ((input[1].i_val > 0 && result < input[0].i_val) || 
				(input[1].i_val < 0 && result > input[0].i_val)) {
				output = ExpValue(ERR_OVERFLOW);
				return;
			}
			output = ExpValue(result);
		}
	}
	
	static void callback_subtract(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool has_double = (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE);
		
		if (has_double) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			output = ExpValue(left - right);
		} else {
			int64_t result = input[0].i_val - input[1].i_val;
			// Check for overflow
			if ((input[1].i_val < 0 && result < input[0].i_val) || 
				(input[1].i_val > 0 && result > input[0].i_val)) {
				output = ExpValue(ERR_OVERFLOW);
				return;
			}
			output = ExpValue(result);
		}
	}
	
	static void callback_unary_plus(ExpValue& input, ExpValue& output) {
		output = input;
	}
	
	static void callback_unary_minus(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		if (input.vt == VALUE_DOUBLE) {
			output = ExpValue(-input.d_val);
		} else if (input.vt == VALUE_INT) {
			if (input.i_val == INT64_MIN) {
				output = ExpValue(ERR_OVERFLOW);
			} else {
				output = ExpValue(-input.i_val);
			}
		} else {
			output = ExpValue(ERR_UNDEFINED);
		}
	}
	
	static void callback_multiply(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool has_double = (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE);
		
		if (has_double) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			output = ExpValue(left * right);
		} else {
			// Integer multiplication
			int64_t result = input[0].i_val * input[1].i_val;
			// This is a simplified overflow check
			if (input[0].i_val != 0 && result / input[0].i_val != input[1].i_val) {
				output = ExpValue(ERR_OVERFLOW);
				return;
			}
			output = ExpValue(result);
		}
	}
	
	static void callback_divide(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		// Always return double for division
		double left, right;
		
		if (input[0].vt == VALUE_DOUBLE) {
			left = input[0].d_val;
		} else {
			left = (double)input[0].i_val;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			right = input[1].d_val;
		} else {
			right = (double)input[1].i_val;
		}
		
		if (right == 0.0) {
			output = ExpValue(ERR_DIV_BY_ZERO);
			return;
		}
		
		output = ExpValue(left / right);
	}
	
	static void callback_int_divide(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		int64_t left, right;
		
		if (input[0].vt == VALUE_DOUBLE) {
			left = (int64_t)input[0].d_val;
		} else {
			left = input[0].i_val;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			right = (int64_t)input[1].d_val;
		} else {
			right = input[1].i_val;
		}
		
		if (right == 0) {
			output = ExpValue(ERR_DIV_BY_ZERO);
			return;
		}
		
		output = ExpValue(left / right);
	}
	
	static void callback_modulo(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		int64_t left, right;
		
		if (input[0].vt == VALUE_DOUBLE) {
			left = (int64_t)input[0].d_val;
		} else {
			left = input[0].i_val;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			right = (int64_t)input[1].d_val;
		} else {
			right = input[1].i_val;
		}
		
		if (right == 0) {
			output = ExpValue(ERR_DIV_BY_ZERO);
			return;
		}
		
		output = ExpValue(left % right);
	}
	
	static void callback_power(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		double base, exp;
		
		if (input[0].vt == VALUE_DOUBLE) {
			base = input[0].d_val;
		} else {
			base = (double)input[0].i_val;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			exp = input[1].d_val;
		} else {
			exp = (double)input[1].i_val;
		}
		
		// Check for domain errors
		if (base == 0.0 && exp < 0) {
			output = ExpValue(ERR_DIV_BY_ZERO);
			return;
		}
		
		if (base < 0 && floor(exp) != exp) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		double result = pow(base, exp);
		if (std::isinf(result)) {
			output = ExpValue(ERR_OVERFLOW);
			return;
		}
		
		if (std::isnan(result)) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		output = ExpValue(result);
	}
	
	static void callback_factorial(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		int64_t n;
		if (input.vt == VALUE_DOUBLE) {
			double d = input.d_val;
			if (d < 0 || d != floor(d)) {
				output = ExpValue(ERR_FN_DOMAIN);
				return;
			}
			n = (int64_t)d;
		} else {
			if (input.i_val < 0) {
				output = ExpValue(ERR_FN_DOMAIN);
				return;
			}
			n = input.i_val;
		}
		
		if (n > 20) { // Prevent overflow
			output = ExpValue(ERR_OVERFLOW);
			return;
		}
		
		int64_t result = 1;
		for (int64_t i = 2; i <= n; i++) {
			result *= i;
		}
		
		output = ExpValue(result);
	}
	
	// Comparison operators
	static void callback_equal(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool equal = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			equal = (fabs(left - right) < 1e-10);
		} else {
			equal = (input[0].i_val == input[1].i_val);
		}
		
		output = ExpValue(equal ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_not_equal(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool not_equal = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			not_equal = (fabs(left - right) >= 1e-10);
		} else {
			not_equal = (input[0].i_val != input[1].i_val);
		}
		
		output = ExpValue(not_equal ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_less_than(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool less = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			less = (left < right);
		} else {
			less = (input[0].i_val < input[1].i_val);
		}
		
		output = ExpValue(less ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_less_equal(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool less_equal = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			less_equal = (left <= right);
		} else {
			less_equal = (input[0].i_val <= input[1].i_val);
		}
		
		output = ExpValue(less_equal ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_greater_than(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool greater = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			greater = (left > right);
		} else {
			greater = (input[0].i_val > input[1].i_val);
		}
		
		output = ExpValue(greater ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_greater_equal(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		bool greater_equal = false;
		
		if (input[0].vt == VALUE_DOUBLE || input[1].vt == VALUE_DOUBLE) {
			double left = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
			double right = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
			greater_equal = (left >= right);
		} else {
			greater_equal = (input[0].i_val >= input[1].i_val);
		}
		
		output = ExpValue(greater_equal ? (int64_t) 1 : (int64_t) 0);
	}
	
	// Logical operators
	static void callback_logical_not(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		int64_t value;
		if (input.vt == VALUE_DOUBLE) {
			value = (input.d_val != 0.0) ? 1 : 0;
		} else {
			value = (input.i_val != 0) ? 1 : 0;
		}
		
		output = ExpValue((value == 0) ? (int64_t) 1 : (int64_t) 0);
	}
	
	static void callback_logical_and(ExpValue& input, ExpValue& output) {
		// We don't actually use this because we handle && with short-circuiting in evaluate()
		// But we need it for the operator table
#if 0		
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		int64_t left = 0, right = 0;
		
		if (input[0].vt == VALUE_DOUBLE) {
			left = (input[0].d_val != 0.0) ? 1 : 0;
		} else {
			left = (input[0].i_val != 0) ? 1 : 0;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			right = (input[1].d_val != 0.0) ? 1 : 0;
		} else {
			right = (input[1].i_val != 0) ? 1 : 0;
		}
		
		output = ExpValue((left && right) ? 1 : 0);
#endif		
	}
	
	static void callback_logical_or(ExpValue& input, ExpValue& output) {
		// We don't actually use this because we handle || with short-circuiting in evaluate()
#if 0		
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		int64_t left = 0, right = 0;
		
		if (input[0].vt == VALUE_DOUBLE) {
			left = (input[0].d_val != 0.0) ? 1 : 0;
		} else {
			left = (input[0].i_val != 0) ? 1 : 0;
		}
		
		if (input[1].vt == VALUE_DOUBLE) {
			right = (input[1].d_val != 0.0) ? 1 : 0;
		} else {
			right = (input[1].i_val != 0) ? 1 : 0;
		}
		
		output = Value((left || right) ? 1 : 0);
#endif		
	}
	
	// Math functions
	static void callback_abs(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		if (input.vt == VALUE_DOUBLE) {
			output = ExpValue(fabs(input.d_val));
		} else {
			if (input.i_val == INT64_MIN) {
				output = ExpValue(ERR_OVERFLOW);
			} else {
				output = ExpValue(input.i_val >= 0 ? input.i_val : -input.i_val);
			}
		}
	}
	
	static void callback_sgn(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		int64_t result;
		if (input.vt == VALUE_DOUBLE) {
			if (input.d_val > 0) result = 1;
			else if (input.d_val < 0) result = -1;
			else result = 0;
		} else {
			if (input.i_val > 0) result = 1;
			else if (input.i_val < 0) result = -1;
			else result = 0;
		}
		
		output = ExpValue(result);
	}
	
	static void callback_floor(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		if (input.vt != VALUE_DOUBLE) {
			output = input;
		} else {
			output = ExpValue(floor(input.d_val));
		}
	}
	
	static void callback_ceil(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		if (input.vt != VALUE_DOUBLE) {
			output = input;
		} else {
			output = ExpValue(ceil(input.d_val));
		}
	}
	
	static void callback_exp(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		double result = exp(x);
		
		if (std::isinf(result)) {
			output = ExpValue(ERR_OVERFLOW);
		} else {
			output = ExpValue(result);
		}
	}
	
	static void callback_ln(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		if (x <= 0) {
			output = ExpValue(ERR_FN_DOMAIN);
		} else {
			output = ExpValue(log(x));
		}
	}
	
	static void callback_pow(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		double base = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
		double exp = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
		
		if (base == 0.0 && exp < 0) {
			output = ExpValue(ERR_DIV_BY_ZERO);
			return;
		}
		
		if (base < 0 && floor(exp) != exp) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		double result = pow(base, exp);
		if (std::isinf(result)) {
			output = ExpValue(ERR_OVERFLOW);
			return;
		}
		
		if (std::isnan(result)) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		output = ExpValue(result);
	}
	
	static void callback_sqrt(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		if (x < 0) {
			output = ExpValue(ERR_FN_DOMAIN);
		} else {
			output = ExpValue(sqrt(x));
		}
	}
	
	static void callback_gamma(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		if (x <= 0 && fabs(x - round(x)) < 1e-10) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		double result = tgamma(x);
		if (std::isinf(result)) {
			output = ExpValue(ERR_OVERFLOW);
		} else if (std::isnan(result)) {
			output = ExpValue(ERR_FN_DOMAIN);
		} else {
			output = ExpValue(result);
		}
	}
	
	static void callback_sin(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		output = ExpValue(sin(x));
	}
	
	static void callback_cos(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		output = ExpValue(cos(x));
	}
	
	static void callback_tan(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		// Check for asymptotes
		double abs_x = fabs(x);
		double remainder = fmod(abs_x + M_PI_2, M_PI);
		if (fabs(remainder - M_PI_2) < 1e-10) {
			output = ExpValue(ERR_FN_DOMAIN);
			return;
		}
		
		double result = tan(x);
		if (std::isinf(result)) {
			output = ExpValue(ERR_OVERFLOW);
		} else {
			output = ExpValue(result);
		}
	}
	
	static void callback_asin(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		if (x < -1 || x > 1) {
			output = ExpValue(ERR_FN_DOMAIN);
		} else {
			output = ExpValue(asin(x));
		}
	}
	
	static void callback_acos(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		
		if (x < -1 || x > 1) {
			output = ExpValue(ERR_FN_DOMAIN);
		} else {
			output = ExpValue(acos(x));
		}
	}
	
	static void callback_atan(ExpValue& input, ExpValue& output) {
		if (input.vt == VALUE_ERROR) {
			output = input;
			return;
		}
		
		double x = (input.vt == VALUE_DOUBLE) ? input.d_val : (double)input.i_val;
		output = ExpValue(atan(x));
	}
	
	static void callback_atan2(ExpValue& input, ExpValue& output) {
		if (input[0].vt == VALUE_ERROR || input[1].vt == VALUE_ERROR) {
			output = (input[0].vt == VALUE_ERROR) ? input[0] : input[1];
			return;
		}
		
		double y = (input[0].vt == VALUE_DOUBLE) ? input[0].d_val : (double)input[0].i_val;
		double x = (input[1].vt == VALUE_DOUBLE) ? input[1].d_val : (double)input[1].i_val;
		output = ExpValue(atan2(y, x));
	}
};

#endif  // __EXPEVAL_H__
/* end expeval.h */
