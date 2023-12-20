/***

	chat.cpp

	A chat session using Llama language models.

	2023, Chris Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

#define BOT_COLOR	CC_FG_CYAN
#define USER_COLOR	CC_FG_YELLOW

struct character_card {
	std::string user_name;
	std::string bot_name;
	std::string bot_greeting;
	std::string char_card;
};

character_card load_from_file(const std::string& filepath) {
	std::ifstream i;
	character_card ret;

	i.open(filepath);
	std::getline(i, ret.bot_name);
	std::getline(i, ret.user_name);
	std::getline(i, ret.bot_greeting);
	std::getline(i, ret.char_card);
	forever {
		std::string next_line;
		ret.char_card += "\n";
		std::getline(i, next_line);
		if (i.eof())
			break;
		ret.char_card += next_line;
	}
	i.close();
	i.clear();

	return ret;
}

int app_main() {
	ArgParse ap;
	std::string char_file;
	character_card cc;

	ap.add_argument("char", type_string, "a text file containing the bot name, the user name, the bot greeting (empty line for user goes first), and character card (can be multiline)");
	llama_args(ap);
	ap.ensure_args(argc, argv);

	ap.value_str("char", char_file);
	if (char_file.empty()) {
		std::cerr << "*** Error: a character card file is required for a chat session.\n";
		ap.show_help();
		return 1;
	}

	cc = load_from_file(char_file);
	Llama llama(ap);

	llama.chat_session(cc.char_card, cc.bot_name, cc.user_name, cc.bot_greeting);
	std::cout << "Chat session begins. Enter QUIT (all caps) to quit, or REGEN to rewind the last bot response and regenerate, or\nUNDO to go back and redo your last response. End a line with a backslash '\\' for multi-line input.\n\n";
	if (!cc.bot_greeting.empty()) {
		cc_fprintf(BOT_COLOR, stdout, "%s: %s\n", cc.bot_name.c_str(), cc.bot_greeting.c_str());
	}

	forever {
		std::string response;

		cc_fprintf(USER_COLOR, stdout, "%s: ", cc.user_name.c_str());
		response = multiline_input();
		if (response == "QUIT")
			break;
		if (response == "REGEN") {
			llama.chat_rewind();
		} else if (response == "UNDO") {
			llama.chat_rewind();
			llama.chat_rewind();
			continue;
		} else {
			llama.chat_user_response(response);
		}

		response = llama.chat_response();
		cc_fprintf(BOT_COLOR, stdout, "%s:%s\n", cc.bot_name.c_str(), response.c_str());
	}

	return 0;
}

