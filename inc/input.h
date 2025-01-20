#ifndef INPUT_H
#define INPUT_H

#include <string>

/*** Asks a yes/no prompt of the user, returns true for 'yes'. Rejects invalid input. ***/
extern bool yes_no(const std::string& prompt);

/*** Return a non-negative integer from the user. Rejects non-numeric input. 
	You can specify a minimum and maximum permissible value (inclusive); if max_val is 0, the maximum is ignored. ***/
extern u32 user_u32(const std::string& prompt, u32 min_val, u32 max_val);

/*** as above, but signed integer ***/
extern i32 user_i32(const std::string& prompt, i32 min_val, i32 max_val);

/*** C string versions of the above. ***/
extern bool yes_no(const char* prompt);
extern u32 user_u32(const char* prompt, u32 min_val, u32 max_val);

#endif  // INPUT_H
