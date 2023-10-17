/***

	input.cpp

	Retrieving user input. Handles prompts, menus, etc.

	Copyright (c) 2020-2022 Chris Street

***/

#include <iostream>
#include "libcodehappy.h"

/*** Asks a yes/no prompt of the user, returns true for 'yes'. Rejects invalid input. ***/
bool yes_no(std::string& prompt)
{
	std::string inp;
	bool y;
	forever
		{
		bool v = false;
		std::cout << prompt << " [yn] ";
		getline(std::cin, inp);
		if (inp.length() > 0)
			{
			switch (inp[0])
				{
			case 'y':
			case 'Y':
				v = true;
				y = true;
				break;
			case 'n':
			case 'N':
				v = true;
				y = false;
				break;
				}
			}
		if (v)
			break;
		std::cout << "Please answer 'y' or 'n'." << std::endl;
		}
	return y;
}

/*** Return a non-negative integer from the user. Rejects non-numeric input. 
	You can specify a minimum and maximum permissible value (inclusive); if max_val is 0, the maximum is ignored. ***/
u32 user_u32(std::string& prompt, u32 min_val, u32 max_val)
{
	std::string inp;
	u32 val;
	forever
		{
		bool v = false, dg = false;
		std::cout << prompt << " ";
		if (max_val > 0)
			{
			std::cout << "(" << min_val << "-" << max_val << ") ";
			}

		getline(std::cin, inp);
		if (inp.length() == 0) {
			v = false;
			break;
		}
		v = true;
		dg = false;
		for (size_t i = 0; i < inp.length(); ++i)
			{
			if (!isdigit(inp[i]) && !isspace(inp[i]))
				{
				std::cout << "Please enter a number." << std::endl;
				v = false;
				break;
				}
			if (isdigit(inp[i]))
				dg = true;
			}
		if (!v || !dg)
			continue;
		val = stoi(inp);

		if (val < min_val)
			{
			std::cout << "Please enter an integer at least " << min_val;
			if (max_val > 0)
				std::cout << "and at most " << max_val;
			std::cout << "." << std::endl;
			v = false;
			}
		if (max_val > 0 && val > max_val)
			{
			std::cout << "Please enter an integer ";
			if (min_val > 0)
				std::cout << "at least " << min_val << " and ";
			std::cout << "at most " << max_val << "." << std::endl;
			v = false;
			}
		if (v)
			break;
		}
	return val;
}

bool yes_no(const char* prompt)
{
	std::string s = prompt;
	return yes_no(s);
}

u32 user_u32(const char* prompt, u32 min_val, u32 max_val)
{
	std::string s = prompt;
	return user_u32(s, min_val, max_val);
}

/*** end input.cpp ***/