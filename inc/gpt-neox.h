/***

	gpt-neox.h

	C++ API to GPT-NeoX-20B, a large language model (LLM) trained by EluthierAI.
	Can be used for other EleutherAI LLMs with the same DeepSpeed-based run harness.
	
	Copyright (c) 2022, Chris Street.

***/
#ifndef _GPT_NEOX_
#define _GPT_NEOX_

#ifdef CODEHAPPY_NATIVE

/* A response from the model. */
struct GPTResponse {
	/* The prompt (context) used to condition the LLM */
	std::string context;
	/* The LLM response */
	std::string completion;
	/* Length in tokens */
	int length;
	/* Is the response finished/complete? */
	bool finished;
	/* Message (error/warning) from the LLM evalution. */
	std::string message;
	/* Duration for inference. */
	double duration_sec;
};

/* String replacement: placeholders (@1, @2, @3, etc.) are filled from the vector of strings. */
extern void placeholder_string(std::string& format_str, std::vector<std::string>& replacements, char replace_char = '@');
/* String replacement: in string 's', replace all instances of string 'f' with 'r' */
extern void string_replace(std::string& s, const std::string& f, const std::string& r);
extern void string_replace(std::string& s, const char* f, const char* r);
/* Convert escaped C-style characters (\n for newline, etc.) in a string. */
extern void string_convert_c_escaped(std::string& s);
/* Convert http entities in a string. */
extern void string_convert_http_entities(std::string& s);


/* The main API. */
class GPTNeoX {
public:
	GPTNeoX();

	/* Prompt with the passed-in string; return the response. */
	GPTResponse prompt(const std::string& p) const;

	/* Prompt with the passed-in string n times; fill the vector r with the responses. */
	void prompt(const std::string& p, int np, std::vector<GPTResponse>& r) const;

	/* Prompt with a vector of strings; fill the vector r with the responses. */
	void prompt(std::vector<std::string> ps, std::vector<GPTResponse>& r) const;

	/* Set the maximum number of output tokens. */
	void set_max_output_token_length(int mot);

private:
	/* read GPT-NeoX-20B responses from the output file */
	void parse_responses(std::vector<GPTResponse>& r) const;
	/* create an API .yml file with the desired maximum number of output tokens. */
	void generate_api_yml() const;
	/* execute the script */
	void execute_script() const;

	/* Maximum output token settings. */
	int max_out_tokens;
};

#endif  // CODEHAPPY_NATIVE

#endif  // _GPT_NEOX_
/* end gpt-neox.h */
