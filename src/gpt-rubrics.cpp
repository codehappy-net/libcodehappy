
/* A rubric: a template used to (hopefully!) produce a desired generation from an LLM. */
struct Rubric {
	std::string desc;
	std::string fmt;
	std::string answer_delimiter;
	std::vector<std::string> placeholder_expl;
};

/* GPT-NeoX-20B is pretty amazingly good at low-shot completions! All of these builtin rubrics have a high probability
   of working (when they do fail it's often in interesting ways.) Using consensus answers from multiple generations can improve
   accuracy at the cost of generation speed. */
static Rubric __builtin_rubrics[] = {
/* Translate English text to German. */
{
"Natural language translation (English -> German)",
"ENGLISH PHRASE A: \"Which color is the book?\" GERMAN PHRASE A: \"Welche Farbe hat das Buch?\" ENGLISH PHRASE B: \"The man and his wife\" GERMAN PHRASE "
"B: \"Der Mann und seine Frau\" ENGLISH PHRASE C: \"@1\" GERMAN PHRASE C:",
"ENGLISH PHRASE D",
{ "English text to translate into German:" }
},
/* Answer a general knowledge question. */
{
"General knowledge question",
"Q1: Who discovered radium? A1: Madame Curie. Q2: Whose expedition was the first to circumnavigate the globe? A2: Ferdinand Magellan's. Q3: Who was the "
"19th president of the USA? A3: Rutherford B. Hayes. Q4: List the lightest four elements of the periodic table by atomic mass. A4: Hydrogen, helium, "
"lithium, beryllium. Q5: @1 A5:",
"Q6:",
{ "Ask a question:" }
},
/* Summarize the plot of a movie. */
{
"Summarize the plot of a movie",
"[QUERY:] Summarize the plot of the film \"Citizen Kane.\". [SUMMARY:] Charles Foster Kane is a media mogul that has broken thousands of men on his climb "
"to unimaginable wealth and immense political power. One mysterious word muttered on his deathbed sends a journalist searching Kane's past for the real "
"story. [QUERY:] Summarize the plot of the film \"Back to the Future\". [SUMMARY:] A teenager named Marty McFly accidentally travels 30 years into the "
"past using the Delorean time machine built by his eccentric inventor friend, Doc Brown. He inadvertently interferes with his mother meeting his father, "
"and must set the past right while finding a way to go back to the future. [QUERY:] Summarize the plot of the film \"Star Wars\". [SUMMARY:] The galaxy is "
"under the tyrannical rule of the Empire, which is constructing a planet-killing superweapon, the Death Star. Luke Skywalker, an ancient Jedi knight "
"named Obi-Wan, a rogue named Han Solo, and two droids go on a quest to destroy the Death Star and defeat the dark lord Darth Vader. [QUERY:] Summarize "
"the plot of the film \"@1\". [SUMMARY:]",
"[QUERY:]",
{ "Provide the title of the film to summarize:" }
},
/* Write C code. */
{
"Write C code",
"EXERCISE: Write a Hello World program in C. ANSWER: #include <stdio.h> int main() { printf(\"Hello, world!\\n\"); return 0; } EXERCISE: Write a function "
"in C that calculates the factorial recursively. ANSWER: int factorial(int n) { if (n <= 1) return 1; return n * factorial(n - 1); } EXERCISE: Write a "
"function in C to reverse the order of characters in a string in place. ANSWER: void reverse_string(char* str) { char* w = str + strlen(str) - 1; while "
"(str < w) { char tmp = *str; *str = *w; *w = tmp; ++str; --w; } } EXERCISE: Write a function in C @1 ANSWER:",
"EXERCISE:",
{ "Describe the desired output by completing 'Write a function in C...':" }
},
};

