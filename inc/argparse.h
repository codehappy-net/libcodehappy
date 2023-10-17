/***

	argparse.h

	Command line argument parsing and verification for libcodehappy.

	The argument parser can save options/flags and their values, as well as any command
	line arguments that are not known flags. (Flags may be indicated by a prefix of "-",
	"--", or "/" identically.) ArgParse comes with a built-in help flag (named "help" or "?"),
	which displays all of the known options and their descriptions. Requirements may
	be added to arguments to ensure that values are in the correct range.

	Copyright (c) 2022 C. M. Street.

***/
#ifndef _ARGPARSE_H_
#define _ARGPARSE_H_

enum ArgType {
	type_int = 0,
	type_uint = 1,
	type_int64 = 2,
	type_uint64 = 3,
	type_double = 4,
	type_string = 5,
	type_bool = 6,
	/* This is a flag that is either present or not present; it has no value per se. */
	type_none = 7,
};

/* Struct representing a command line argument flag. */
struct ArgumentCmd {
	/* The name of the flag. */
	std::string name;
	std::string helpstr;
#ifdef GENERIC_REQUIREMENTS
	std::string reqfail_str;
	Requirement req;
#endif
	ArgType ty;
	bool present;
	union {
		int v_i;
		uint v_u;
		i64 v_i64;
		u64 v_u64;
		double v_g;
		char* v_str;
		bool v_b;
	} val;
	void* ptr;
};

class ArgParse {
public:
	ArgParse();
	~ArgParse();

	/* Specify an argument. */
	void add_argument(const std::string& argname, ArgType typ, const std::string& help_string, void* data_ptr = nullptr);
#ifdef GENERIC_REQUIREMENTS
	void enforce_requirement(const std::string& argname, const Requirement& rqt, const std::string& reqfail_str);
#endif

	/* Process the arguments. */
	void ensure_args(const char* full_argstr);
	void ensure_args(int argc, const char* argv[]);

	/* Is a flag present or known? */
	bool flag_present(const std::string& name) const;
	bool flag_known(const std::string& name) const;

	/* Any argument that isn't an identifiable flag is simply stored in the order in which
           they appear; get the number of these arguments and retrieve a specified one. */
	u32 nonflag_args() const { return unenum_args.size(); }
	void nonflag_arg(u32 idx, std::string& str_out) const;
	void all_nonflag_args(std::string& str_out) const;

	/* Get the value of the specified flag as a type. */
	int value_int(const std::string& option_name) const;
	uint value_uint(const std::string& option_name) const;
	i64 value_i64(const std::string& option_name) const;
	u64 value_u64(const std::string& option_name) const;
	double value_double(const std::string& option_name) const;
	std::string value_str(const std::string& option_name) const;
	void value_str(const std::string& option_name, std::string& str_out) const;
	bool value_bool(const std::string& option_name) const;

	/* Show the help messages. */
	void show_help() const;

private:
	int ensure_arg_val(int argc, const char* argv[], int idx, ArgumentCmd& a);
	void say_bad_type(const std::string& option_name) const;
	void add_help_flags();

	std::unordered_map<std::string, ArgumentCmd> args;
	std::vector<std::string> unenum_args;
};

/* Macro to parse the command line arguments. Use in app_main() after instantiating ArgParse and specifying the valid options. */
#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_SDL) && defined(CODEHAPPY_WINDOWS)
#define parse_cmd_args(ap)	ap.ensure_args((const char*)lpszArgument)
#else
#define parse_cmd_args(ap)	ap.ensure_args(argc, (const char**)argv)
#endif

#endif  // _ARGPARSE_H_
/* end argparse.h */
