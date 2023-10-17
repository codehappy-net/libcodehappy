/***

	gpt-neox.cpp

	C++ API to GPT-NeoX-20B, a large language model (LLM) trained by EluthierAI.
	Can be used for other EleutherAI LLMs with the same run harness.

	Copyright (c) 2022, Chris Street.

***/
#ifdef CODEHAPPY_NATIVE

// NB: This is largely obsolete now; the ggml library is perfectly capable of running
// GPT-NeoX inference natively.

/* path should have the trailing slash */
#define API_SCRIPT_PATH	"/home/exx/ml/gpt-neox/"
/* API.yml is a config created by set_max_output_token_length() below. 20B.yml is the GPT-NeoX-20B model config. */
#define API_SCRIPT_CMD		"python ./deepy.py generate.py -d configs 20B.yml API.yml" DEV_NULL
#define CONDITIONING_IN	"sample_input.txt"
#define RESPONSE_OUT		"sample_output.txt"

/* String replacement: placeholders (@1, @2, @3, etc.) are filled from the vector of strings. */
void placeholder_string(std::string& format_str, std::vector<std::string>& replacements, char replace_char) {
	char str[16];
	std::string s;
	for (int e = 0; e < replacements.size(); ++e) {
		sprintf(str, "%c%d", replace_char, e + 1);
		s = str;
		string_replace(format_str, s, replacements[e]);
	}
	/* Allow simply '@' to stand in for the first replacement string */
	str[0] = replace_char;
	str[1] = '\000';
	s = str;
	string_replace(format_str, s, replacements[0]);
}

/* String replacement: in string 's', replace all instances of string 'f' with 'r' */
void string_replace(std::string& s, const std::string& f, const std::string& r) {
	size_t pos = 0;
	forever {
		pos = s.find(f, pos);
		if (pos == std::string::npos) {
			break;
		}
		s.erase(pos, f.length());
		s.insert(pos, r);
		pos += r.length();
	}
}

void string_replace(std::string& s, const char* f, const char* r) {
	std::string ff, rr;
	ff = f;
	rr = r;
	string_replace(s, ff, rr);
}

/* Convert escaped C-style characters (\n for newline, etc.) in a string. */
void string_convert_c_escaped(std::string& s) {
	string_replace(s, "\\n", "\n");
	string_replace(s, "\\t", "\t");
	string_replace(s, "\\r", "\r");
	string_replace(s, "\\a", "\a");
	string_replace(s, "\\b", "\b");
	string_replace(s, "\\f", "\f");
	string_replace(s, "\\v", "\v");
	string_replace(s, "\\\\", "\\");
	string_replace(s, "\\'", "'");
	string_replace(s, "\\\"", "\"");
	string_replace(s, "\\?", "?");
	/* let's replace some Unicode characters: umlauts, diareses, es-stett, etc. */
	string_replace(s, "\\u00a1", "¡");
	string_replace(s, "\\u00a2", "¢");
	string_replace(s, "\\u00a3", "£");
	string_replace(s, "\\u00a4", "¤");
	string_replace(s, "\\u00a5", "¥");
	string_replace(s, "\\u00a6", "|");
	string_replace(s, "\\u00a7", "§");
	string_replace(s, "\\u00a8", "¨");
	string_replace(s, "\\u00a9", "©");
	string_replace(s, "\\u00aa", "ª");
	string_replace(s, "\\u00ab", "«");
	string_replace(s, "\\u00ac", "¬");
	string_replace(s, "\\u00ad", "--");
	string_replace(s, "\\u00ae", "®");
	string_replace(s, "\\u00af", "¯");
	string_replace(s, "\\u00b0", "°");
	string_replace(s, "\\u00b1", "±");
	string_replace(s, "\\u00b2", "²");
	string_replace(s, "\\u00b3", "³");
	string_replace(s, "\\u00b4", "´");
	string_replace(s, "\\u00b5", "µ");
	string_replace(s, "\\u00b6", "¶");
	string_replace(s, "\\u00b7", "·");
	string_replace(s, "\\u00b8", "¸");
	string_replace(s, "\\u00b9", "¹");
	string_replace(s, "\\u00ba", "º");
	string_replace(s, "\\u00bb", "»");
	string_replace(s, "\\u00bd", "½");
	string_replace(s, "\\u00bc", "¼");
	string_replace(s, "\\u00be", "¾");
	string_replace(s, "\\u00bf", "¿");
	string_replace(s, "\\u00c0", "À");
	string_replace(s, "\\u00c1", "Á");
	string_replace(s, "\\u00c2", "Â");
	string_replace(s, "\\u00c3", "Ã");
	string_replace(s, "\\u00c4", "Ä");
	string_replace(s, "\\u00c5", "Å");
	string_replace(s, "\\u00c6", "Æ");
	string_replace(s, "\\u00c7", "Ç");
	string_replace(s, "\\u00c8", "È");
	string_replace(s, "\\u00c9", "É");
	string_replace(s, "\\u00ca", "Ê");
	string_replace(s, "\\u00cb", "Ë");
	string_replace(s, "\\u00cc", "Ì");
	string_replace(s, "\\u00cd", "Í");
	string_replace(s, "\\u00ce", "Î");
	string_replace(s, "\\u00cf", "Ï");
	string_replace(s, "\\u00d0", "Ð");
	string_replace(s, "\\u00d1", "Ñ");
	string_replace(s, "\\u00d2", "Ò");
	string_replace(s, "\\u00d3", "Ó");
	string_replace(s, "\\u00d4", "Ô");
	string_replace(s, "\\u00d5", "Õ");
	string_replace(s, "\\u00d6", "Ö");
	string_replace(s, "\\u00d7", "×");
	string_replace(s, "\\u00d8", "Ø");
	string_replace(s, "\\u00d9", "Ù");
	string_replace(s, "\\u00da", "Ú");
	string_replace(s, "\\u00db", "Û");
	string_replace(s, "\\u00dc", "Ü");
	string_replace(s, "\\u00dd", "Ý");
	string_replace(s, "\\u00de", "Þ");
	string_replace(s, "\\u00df", "ß");
	string_replace(s, "\\u00e0", "à");
	string_replace(s, "\\u00e1", "á");
	string_replace(s, "\\u00e2", "â");
	string_replace(s, "\\u00e3", "ã");
	string_replace(s, "\\u00e4", "ä");
	string_replace(s, "\\u00e5", "å");
	string_replace(s, "\\u00e6", "æ");
	string_replace(s, "\\u00e7", "ç");
	string_replace(s, "\\u00e8", "è");
	string_replace(s, "\\u00e9", "é");
	string_replace(s, "\\u00ea", "ê");
	string_replace(s, "\\u00eb", "ë");
	string_replace(s, "\\u00ec", "ì");
	string_replace(s, "\\u00ed", "í");
	string_replace(s, "\\u00ee", "î");
	string_replace(s, "\\u00ef", "ï");
	string_replace(s, "\\u00f0", "ð");
	string_replace(s, "\\u00f1", "ñ");
	string_replace(s, "\\u00f2", "ò");
	string_replace(s, "\\u00f3", "ó");
	string_replace(s, "\\u00f4", "ô");
	string_replace(s, "\\u00f5", "õ");
	string_replace(s, "\\u00f6", "ö");
	string_replace(s, "\\u00f7", "÷");
	string_replace(s, "\\u00f8", "ø");
	string_replace(s, "\\u00f9", "ù");
	string_replace(s, "\\u00fa", "ú");
	string_replace(s, "\\u00fb", "û");
	string_replace(s, "\\u00fc", "ü");
	string_replace(s, "\\u00fd", "ý");
	string_replace(s, "\\u00fe", "þ");
	string_replace(s, "\\u00ff", "ÿ");
	string_replace(s, "\\u2013", "--");
	string_replace(s, "\\u2014", "---");
	string_replace(s, "\\u2018", "'");
	string_replace(s, "\\u2019", "'");
	string_replace(s, "\\u201c", "\"");
	string_replace(s, "\\u201d", "\"");
	string_replace(s, "\\u2026", "...");
	// TODO: Octal, hexadecimal, Unicode characters?
}

/* Convert http entities in a string. */
void string_convert_http_entities(std::string& s) {
	std::unordered_map<std::string, std::string> conversions = {
		{ "&nbsp;", " " }, { "&lt;", "<" }, { "&gt;", ">", }, { "&amp;", "&" },
		{ "&quot;", "\"" }, { "&apos;", "'" }, { "&cent;", "\242" }, { "&pound;", "\243" },
		{ "&curren;", "\244" }, { "&yen;", "\245" }, { "&brvbar;", "|" }, { "&sect;", "\247" },
		{ "&uml;", "\250" }, { "&copy;", "\251" }, { "&ordf;", "\252" }, { "&laquo;", "\253" },
		{ "&not;", "\254" }, { "&shy;", "\255" }, { "&reg;", "\256" }, { "&macr;", "\257" }, { "&deg;", "\260" },
		{ "&plusmn;", "\261" }, { "&sup2;", "\262" }, { "&sup3;", "\263" }, { "&acute;", "\264" }, { "&micro;", "\265" },
		{ "&para;", "\266" }, { "&cedil;", "\267" }, { "&sup1;", "\270" }, { "&ordm;", "\271" }, { "&raquo;", "\272" },
		{ "&frac14;", "\273" }, { "&frac12;", "\274" }, { "&frac34;", "\275" }, { "&iquest;", "\276" }, { "&times;", "\327" },
		{ "&divide;", "\367" },
	};
	for (const auto& e : conversions) {
		string_replace(s, e.first, e.second);
	}
}

GPTNeoX::GPTNeoX() {
	max_out_tokens = 300;
	generate_api_yml();
}

void GPTNeoX::execute_script() const {
	if (FileExists(RESPONSE_OUT)) {
		remove(RESPONSE_OUT);
	}
	system(API_SCRIPT_CMD);
}

/* Prompt with the passed-in string; return the response. */
GPTResponse GPTNeoX::prompt(const std::string& p) const {
	std::ofstream o;
	std::vector<GPTResponse> rs;

	SaveCurDir();
	ChangeDir(API_SCRIPT_PATH);
	
	o.open(CONDITIONING_IN);
	o << p << "\n";
	o.close();
	o.clear();

	execute_script();

	parse_responses(rs);
	assert(rs.size() >= 1);

	RestoreCurDir();
	return rs[0];
}

/* Prompt with the passed-in string n times; fill the vector r with the responses. */
void GPTNeoX::prompt(const std::string& p, int np, std::vector<GPTResponse>& r) const {
	std::ofstream o;

	if (0 == np)
		return;
	SaveCurDir();
	ChangeDir(API_SCRIPT_PATH);
	
	o.open(CONDITIONING_IN);
	for (int e = 0; e < np; ++e)
		o << p << "\n";
	o.close();
	o.clear();

	execute_script();

	parse_responses(r);
	assert(r.size() >= np);

	RestoreCurDir();
}

/* Prompt with a vector of strings; fill the vector r with the responses. */
void GPTNeoX::prompt(std::vector<std::string> ps, std::vector<GPTResponse>& r) const {
	std::ofstream o;

	if (ps.empty())
		return;
	SaveCurDir();
	ChangeDir(API_SCRIPT_PATH);
	
	o.open(CONDITIONING_IN);
	for (const auto& s : ps)
		o << s << "\n";
	o.close();
	o.clear();

	execute_script();

	parse_responses(r);
	assert(r.size() >= ps.size());

	RestoreCurDir();
}

void GPTNeoX::parse_responses(std::vector<GPTResponse>& r) const {
	SaveCurDir();
	ChangeDir(API_SCRIPT_PATH);
	if (!FileExists(RESPONSE_OUT)) {
		RestoreCurDir();
		return;
	}
	RamFile rf(RESPONSE_OUT, RAMFILE_READONLY);
	char* w = (char *)rf.buffer(), *w2;
	GPTResponse rr;

	forever {
		w = strstr(w, "{\"context\"");
		BREAK_NULL(w);
		w += 12;
		if (*w == '\"')
			++w;
		w2 = strstr(w, "\", \"text");
		BREAK_NULL(w2);
		*w2 = '\000';
		rr.context = w;
		w = w2 + 12;
		w2 = strstr(w, "\", \"length\": ");
		BREAK_NULL(w2);
		*w2 = '\000';
		rr.completion = w;
		w = w2 + 13;
		rr.length = atoi(w);
		w2 = strstr(w, "finished\": false,");
		rr.finished = is_null(w2);
		rr.message.clear();
		w2 = strstr(w, "\"message\": null");
		if (is_null(w2)) {
			w2 = strstr(w, "\"message\": \"");
			if (!is_null(w2)) {
				w = w2 + 12;
				w2 = strstr(w, "\", \"duration_");
				BREAK_NULL(w2);
				*w2 = '\000';
				rr.message = w;
				w = w2 + 1;
			}
		}
		w = strstr(w, ", \"duration_");
		BREAK_NULL(w);
		w += 22;
		rr.duration_sec = atof(w);
		r.push_back(rr);
	}

	RestoreCurDir();
}

void GPTNeoX::generate_api_yml() const {
	std::ofstream o;
	SaveCurDir();
	ChangeDir(API_SCRIPT_PATH "configs/");
	o.open("API.yml");
	o << R"foo(
{
  "text-gen-type": "input-file",

  "maximum_tokens": )foo";
  	o << max_out_tokens << ",\n";
	o << R"foo(
  "temperature": 1.0,
  "top_p": 0.0,
  "top_k": 0,
  "recompute": false,

  "num-samples": 10,

  "sample-input-file": "sample_input.txt",
  "sample-output-file": "sample_output.txt",
}
)foo";
	o.close();
	o.clear();
	RestoreCurDir();
}

void GPTNeoX::set_max_output_token_length(int mot) {
	if (mot != max_out_tokens) {
		max_out_tokens = mot;
		generate_api_yml();
	}
}

#endif  // CODEHAPPY_NATIVE

/* end gpt-neox.cpp */
