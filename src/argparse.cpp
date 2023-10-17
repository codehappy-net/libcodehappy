/***

	argparse.cpp

	Command line argument parsing and verification for libcodehappy.

	The argument parser can save options/flags and their values, as well as any command
	line arguments that are not known flags. (Flags may be indicated by a prefix of "-",
	"--", or "/" identically.) ArgParse comes with a built-in help flag (named "help" or "?"),
	which displays all of the known options and their descriptions. Requirements may
	be added to arguments to ensure that values are in the correct range.

	Copyright (c) 2022 C. M. Street.

***/
#include <libcodehappy.h>

const u32 MAX_CMD_LINE_ARGS = 256;

ArgParse::ArgParse() {
}

ArgParse::~ArgParse() {
	for (auto& e : args) {
		auto& cmd = e.second;
		if (cmd.ty == type_string && cmd.present) {
			delete cmd.val.v_str;
			cmd.present = false;
		}
	}
}

void ArgParse::add_argument(const std::string& argname, ArgType typ, const std::string& help_string, void* data_ptr) {
	ArgumentCmd cmd;
	cmd.name = argname;
	cmd.ty = typ;
	cmd.present = false;
	cmd.helpstr = help_string;
#ifdef GENERIC_REQUIREMENTS
	cmd.reqfail_str = "";
#endif
	cmd.ptr = data_ptr;
	args[argname] = cmd;
}

#ifdef GENERIC_REQUIREMENTS
void ArgParse::enforce_requirement(const std::string& argname, const Requirement& rqt, const std::string& reqfail_str) {
	if (args.find(argname) == args.end()) {
		codehappy_cerr << "Unknown flag '" << argname << "' when setting requirement\n";
		return;
	}
	args[argname].req = rqt;
	args[argname].reqfail_str = reqfail_str;
}
#endif

void ArgParse::add_help_flags() {
	args["help"].name = "help";
	args["help"].helpstr = "Show this message";
	args["help"].ty = type_none;
	args["help"].present = false;
	args["help"].ptr = nullptr;
	args["?"] = args["help"];
}

void ArgParse::ensure_args(const char* full_argstr) {
	std::vector<char*> args;
	char* cpy;
	char* w, * we;

	cpy = new char [strlen(full_argstr) + 1];
	strcpy(cpy, full_argstr);

	w = cpy;
	we = w + strlen(w);

	while (w < we) {
		if (isspace(*w)) {
			++w;
			continue;
		}
		// An argument enclosed in double quotes.
		if (*w == '\"') {
			++w;
			args.push_back(w);
			while (w < we && *w != '\"') {
				++w;
			}
			*w = '\000';
			if (w < we)
				++w;
			continue;
		}
		// A regular argument.
		args.push_back(w);
		while (w < we && !isspace(*w))
			++w;
		*w = '\000';
		++w;
	}

	int argc = args.size();
	char* argv[MAX_CMD_LINE_ARGS];
	for (int e = 0; e < argc && e < 256; ++e) {
		argv[e] = args[e];
	}
	ensure_args(argc, (const char**)argv);

	delete cpy;
}

void ArgParse::ensure_args(int argc, const char* argv[]) {
	int e;
	/* Flags "help" and "?" are always defined. */
	add_help_flags();
	for (e = 1; e < argc; ++e) {
		if (argv[e][0] == '-' || argv[e][0] == '/') {
			/* A flag/option. */
			const char* w = argv[e] + 1;
			while (*w == '-')
				++w;
			if (flag_known(w)) {
				e = ensure_arg_val(argc, argv, e, args[w]);
			} else {
				codehappy_cerr << "Unknown flag '" << w << "'\n";
				show_help();
			}
		} else {
			/* An unenumerated/unknown option. Store it. */
			unenum_args.push_back(argv[e]);
		}
	}
}

int ArgParse::ensure_arg_val(int argc, const char* argv[], int idx, ArgumentCmd& a) {
	if (a.ty == type_none) {
		a.present = true;
		if (a.name == "help" || a.name == "?")
			show_help();
		if (a.ptr != nullptr)
			*((bool *)(a.ptr)) = true;
		return idx;
	}
	if (idx + 1 >= argc) {
		codehappy_cerr << "Argument '" << argv[idx] << "' requires a value\n";
		a.present = false;
		return idx;
	}

	++idx;

	switch (a.ty) {
	case type_int:
		a.val.v_i = atoi(argv[idx]);
		a.present = true;
		if (a.ptr != nullptr)
			*((int *)(a.ptr)) = a.val.v_i;
		break;

	case type_uint:
		a.val.v_u = strtoul(argv[idx], nullptr, 10);
		a.present = true;
		if (a.ptr != nullptr)
			*((uint *)(a.ptr)) = a.val.v_u;
		break;

	case type_int64:
		a.val.v_i64 = strtoll(argv[idx], nullptr, 10);
		a.present = true;
		if (a.ptr != nullptr)
			*((i64 *)(a.ptr)) = a.val.v_i64;
		break;

	case type_uint64:
		a.val.v_u64 = strtoull(argv[idx], nullptr, 10);
		a.present = true;
		if (a.ptr != nullptr)
			*((u64 *)(a.ptr)) = a.val.v_u64;
		break;

	case type_double:
		a.val.v_g = atof(argv[idx]);
		a.present = true;
		if (a.ptr != nullptr)
			*((double *)(a.ptr)) = a.val.v_g;
		break;

	case type_string:
		a.val.v_str = new char [strlen(argv[idx]) + 1];
		strcpy(a.val.v_str, argv[idx]);
		a.present = true;
		// TODO: allow specifying the length of the passed buffer?
		if (a.ptr != nullptr)
			strcpy((char*)a.ptr, a.val.v_str);
		break;

	case type_bool:
		/* Bools can be represented the following ways:
			(Numeric)	0 = false, anything else = true
			t/f/true/false	Capitalization ignored.
			y/n/yes/no	Capitalization ignored. */
		a.present = true;
		if (isdigit(argv[idx][0])) {
			int v = atoi(argv[idx]);
			a.val.v_b = (v != 0);
		} else if (strlen(argv[idx]) == 1) {
			switch(tolower(argv[idx][0])) {
			case 't':
			case 'y':
				a.val.v_b = true;
				break;
			case 'n':
			case 'f':
				a.val.v_b = false;
				break;
			default:
				codehappy_cerr << "unknown boolean " << argv[idx] << " -- does this mean 'true' or 'false'?\n";
				a.present = false;
				break;
			}
		} else {
			if (!__strnicmp(argv[idx], "true", 4)) {
				a.val.v_b = true;
			} else if (!__strnicmp(argv[idx], "false", 5)) {
				a.val.v_b = false;
			} else if (!__strnicmp(argv[idx], "no", 2)) {
				a.val.v_b = false;
			} else if (!__strnicmp(argv[idx], "yes", 3)) {
				a.val.v_b = true;
			} else {
				codehappy_cerr << "unknown boolean option " << argv[idx] << "\n";
				a.present = false;
			}
		}
		if (a.ptr != nullptr)
			*((bool *)(a.ptr)) = a.val.v_b;
		break;
	}

	return idx;
}

bool ArgParse::flag_present(const std::string& name) const {
	const auto it = args.find(name);
	if (it == args.end())
		return false;
	return it->second.present;
}

bool ArgParse::flag_known(const std::string& name) const {
	const auto it = args.find(name);
	return it != args.end();
}

void ArgParse::nonflag_arg(u32 idx, std::string& str_out) const {
	if (idx >= unenum_args.size()) {
		str_out.clear();
		return;
	}
	str_out = unenum_args[idx];
}

void ArgParse::all_nonflag_args(std::string& str_out) const {
	if (0 == nonflag_args()) {
		str_out.clear();
		return;
	}
	nonflag_arg(0, str_out);
	// if the text is separated by spaces w/o quotes, it's present as multiple command line arguments; let's assemble the complete text.
	for (int e = 1; e < nonflag_args(); ++e) {
		std::string text_cont;
		str_out += " ";
		nonflag_arg(e, text_cont);
		str_out += text_cont;
	}
}

void ArgParse::say_bad_type(const std::string& option_name) const {
	codehappy_cerr << "Incompatible type for option '" << option_name << "'\n";
}

int ArgParse::value_int(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return 0;
	switch (it->second.ty) {
	case type_double:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_i;
}

uint ArgParse::value_uint(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return 0;
	switch (it->second.ty) {
	case type_double:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_u;
}

i64 ArgParse::value_i64(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return 0;
	switch (it->second.ty) {
	case type_double:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_i64;
}

u64 ArgParse::value_u64(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return 0;
	switch (it->second.ty) {
	case type_double:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_u64;
}

double ArgParse::value_double(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return 0;
	switch (it->second.ty) {
	case type_int:
	case type_uint:
	case type_int64:
	case type_uint64:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_g;
}

std::string ArgParse::value_str(const std::string& option_name) const {
	std::string ret;
	const auto it = args.find(option_name);
	if (it == args.end())
		return "";
	if (it->second.ty != type_string) {
		say_bad_type(option_name);
		ret = "";
	} else {
		ret = it->second.val.v_str;
	}
	return ret;
}

void ArgParse::value_str(const std::string& option_name, std::string& str_out) const {
	if (flag_present(option_name))
		str_out = value_str(option_name);
}

bool ArgParse::value_bool(const std::string& option_name) const {
	const auto it = args.find(option_name);
	if (it == args.end())
		return false;
	switch (it->second.ty) {
	case type_double:
	case type_string:
	case type_none:
		say_bad_type(option_name);
		break;
	}
	return it->second.val.v_b;
}

struct HelpStr {
	std::string name;
	std::string helpstr;
};

static bool helpstr_sort_predicate(const HelpStr& hs1, const HelpStr& hs2) {
	return hs1.name < hs2.name;
}

void ArgParse::show_help() const {
	std::vector<HelpStr> helpstrs;
	int maxlen = 0;

	for (const auto& e : args) {
		const auto& ac = e.second;
		HelpStr hs;
		if (e.first == "?")
			continue;
		hs.name = ac.name;
		hs.helpstr = ac.helpstr;
		helpstrs.push_back(hs);
		maxlen = std::max(maxlen, (int) hs.name.length());
	}
	std::sort(helpstrs.begin(), helpstrs.end(), helpstr_sort_predicate);
	for (const auto& e : helpstrs) {
		std::cout << e.name;
		for (int f = e.name.length(); f <= maxlen; ++f)
			std::cout << " ";
		std::cout << e.helpstr << "\n";
	}
	exit(1);
}

/* end argparse.cpp */
