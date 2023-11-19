/***

	thegame.cpp

	"The Game" -- human vs. LLM where the LLM is trying to get the human to say 
	a secret 'magic word'.

	2023, Chris Street

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

#define BOT_COLOR	CC_FG_CYAN
#define USER_COLOR	CC_FG_YELLOW
#define WORD_COLOR	CC_FG_RED

/* possible choices for the 'magic word' -- mostly (relatively) common English words */
static std::string magic_words[] = {
	"abandon", "ability", "abroad", "absorb", "academic", "account", "address", "agency", "ailment",
	"alternative", "appropriate", "architect", "artist", "attention", "authority", "avoid", "baseball",
	"behavior", "benefit", "blacken", "board", "bowling", "brood", "brother", "building", "butter", "camera", 
	"campaign", "candidate", "capital", "century", "chair", "choose", "church", "citizen", "claim", 
	"coach",  "collection", "colorful", "commercial", "community", "computer", "condition", "conference", 
	"construction", "consumer", "contact", "container", "continue", "control", "country", "cultural",
	"daytime", "debate", "decide", "defend", "definition", "degree", "democracy", "depiction", "design", 
	"development", "device", "devote", "direction", "director", "discussion", "disease", "doctor",
	"dream", "economic", "effect", "effort", "eggplant", "election", "energy", "enjoy", "episode",
	"equality", "evening", "everybody", "executive", "experience", "eyeball", "factor", "family", 
	"feather", "feeling", "finally", "financial", "finger", "flying", "follower", "foreign", "former",
	"forward", "friendly", "future", "garden", "generation", "glamor", "golden", "ground", "growth",
	"happening", "healthy", "hearty", "heavy", "helpful", "historic", "history", "horror", "hospital",
	"hotel", "house", "hovering", "husband", "identify", "imagine", "increase", "information",
	"inside", "interesting", "interview", "introduction", "joining", "jigsaw", "jolly", "judge", "jumble", 
	"kangaroo", "keeping", "kernel", "kettle", "keyboard", "khaki", "kinetic", "kitchen", "knapsack", 
	"language", "larger", "laughter", "lawyer", "least", "letter", "lighting", "likely", "local",
	"lollipop", "loving", "machine", "magazine", "maintain", "manage", "material", "matter", "measure",
	"memory", "mention", "military", "minute", "mission", "model", "morning", "mother", "movement",
	"movie", "musical", "myself", "national", "natural", "necessary", "network", "news", "north",
	"nothing", "number", "oblong", "oboe", "officer", "oily", "operation", "opportunity", "option",
	"order", "organization", "outside", "owner", "painting", "parent", "partial", "particular",
	"partner", "party", "patience", "peaceful", "performance", "period", "personal", "phone", "physical",
	"picture", "place", "player", "politics", "popular", "possible", "pressure", "pretty", "price", 
	"private", "probably", "process", "product", "professional", "program", "property", "public",
	"quack", "quail", "quake", "quality", "quarter", "quarry", "queen", "quibble", "quiet", "quite", 
	"quiver", "quote", "radio", "raise", "rather", "reach", "reality", "receive", "recently", "record",
	"recognize", "reflect", "relationship", "remember", "remove", "represent", "require", "research",
	"resource", "respond", "responsible", "risky", "rocky", "roomy", "safely", "safety", "saving",
	"science", "school", "season", "second", "security", "seeking", "selling", "senior", "sensing",
	"seriously", "service", "setting", "seven", "several", "shaking", "shoulder", "showing", "similar",
	"simple", "simply", "single", "sister", "situation", "sizing", "skill", "skin", "smile", "social", 
	"society", "soldier", "somebody", "sometimes", "sooner", "southern", "speech", "spend", "sport",
	"staff", "stage", "standing", "standard", "starry", "statement", "station", "staying", "stepping",
	"stock", "student", "stuff", "style", "strategy", "street", "strong", "structure", "subject",
	"success", "suddenly", "suffer", "suggest", "summer", "support", "surface", "system", "table",
	"talking", "taxing", "teacher", "team", "technology", "television", "telling", "tending", "today",
	"together", "tonight", "totally", "toward", "truly", "twenty", "twice", "twine", "type", "typical", 
	"underneath", "understand", "united", "universal", "unknown", "unless", "unlikely", "unusual",
	"usually", "umbrella", "valley", "valuable", "value", "variety", "various", "victim", "village",
	"visit", "voting", "waiting", "walking", "wanting", "watching", "water", "wealth", "weapon",
	"wearing", "weight", "welcome", "western", "wetter", "whatever", "whether", "white", "whose",
	"window", "without", "woman", "wondrous", "worry", "writing", "wrong", "yacht", "yard", "yawn", 
	"yearly", "yelling", "yellow", "yesterday", "young", "yourself", "yummy", "zero", "zipper", "zoo"
};

bool contains_magic_word(const std::string& s, const std::string& mw) {
	auto it = std::search(
		s.begin(), s.end(),
		mw.begin(), mw.end(),
		[](unsigned char c1, unsigned char c2) { return std::tolower(c1) == std::tolower(c2); }
		);
	return (it != s.end());
}

int app_main() {
	ArgParse ap;
	bool spoiler = false;

	llama_args(ap);
	ap.ensure_args(argc, argv);
	ap.add_argument("spoiler", type_none, "reveal the magic word at the start of the game (for debugging)", &spoiler);
	Llama llama(ap);

	std::string prompt, magic_word;
	std::string bot_greeting = "Hello! Let's play The Game together. I will try to get you to say the secret magic word.";
	GrabBag<std::string> mws;
	for (auto& mw : magic_words)
		mws.Insert(mw, 1);
	magic_word = mws.Select();

	prompt =
		"You are Martin, an enthusiast of word games. You are going to play a game in which a magic word is secretly chosen, then "
		"you and Human converse back and forth. Your goal is to make Human say the magic word in conversation; if Human says the "
		"magic word, you have won the game.\n\nUse your cunning to try and get Human to use the word naturally in conversation; "
		"if you are too direct and Human is able to guess from your statements what the magic word is, they will avoid saying it.\n\n"
		"There is one more rule: you, Martin, are not allowed to say the magic word yourself.\n\n"
		"The magic word for this game is: \"";
	prompt += magic_word;
	prompt += "\".\n\nRemember, you, Martin, may not say the magic word \"";
	prompt += magic_word;
	prompt += "\" yourself, but your goal is to get Human to say the magic word \"";
	prompt += magic_word;
	prompt += "\". Take a deep breath, relax, and begin.";

	llama.chat_session(prompt, "Martin", "Human", bot_greeting);

	std::cout << "The Game begins. A secret magic word has been chosen. Type QUIT when done.\n\n";
	if (spoiler) {
		cc_fprintf(WORD_COLOR, stdout, "Spoiler: the magic word is '%s'\n", magic_word.c_str());
	}
	cc_fprintf(BOT_COLOR, stdout, "Martin: %s\n", bot_greeting.c_str());

	forever {
		std::string response;

		cc_fprintf(USER_COLOR, stdout, "Human: ");
		response = multiline_input();
		if (response == "QUIT")
			break;

		if (contains_magic_word(response, magic_word)) {
			/* human says the word: computer wins */
			cc_fprintf(WORD_COLOR, stdout, "*** you have used the magic word!!!: it was '%s'\n", magic_word.c_str());
			break;
		}

		llama.chat_user_response(response);

		forever {
			response = llama.chat_response();
			if (contains_magic_word(response, magic_word)) {
				/* computer says the word: you can call this a human win if you like */
				cc_fprintf(WORD_COLOR, stdout, "*** bleh, bot response used the magic word against the rules -- rewinding\n");
				if (spoiler) {
					cc_fprintf(WORD_COLOR, stdout, "Bad response: %s\n", response.c_str());
				}
				llama.chat_rewind();
				continue;
			}
			break;
		}
		cc_fprintf(BOT_COLOR, stdout, "Martin:%s\n", response.c_str());
	}

	return 0;
}

