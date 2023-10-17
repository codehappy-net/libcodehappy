/***

	sql.cpp

	A SQLite console. Open databases and execute queries on them.

	Copyright (c) 2022 C. M. Street.

***/
#include "libcodehappy.h"

static bool _first = true;
static int col_callback(void *v, int ncols, char **cols, char **colname) {
	if (_first) {
		std::cout << "------------------------------------------------------------\n";
		for (u32 e = 0; e < ncols; ++e) {
			if (!is_null(colname[e]))
				std::cout << colname[e];
			std::cout << "\t";
		}
		std::cout << "\n------------------------------------------------------------\n";
		_first = false;
	}
	for (u32 e = 0; e < ncols; ++e) {
		std::cout << cols[e] << "\t";
	}
	std::cout << std::endl;
	return(0);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: sql [DB name]\n";
		return 1;
	}

	sqlite3 *db;
	sqlite3_open(argv[1], &db);
	std::cout << "Database " << argv[1] << " opened. Enter queries to execute, or 'exit' to quit.\n";
	forever {
		std::string prompt;
		int err;
		std::getline(std::cin, prompt);
		if (prompt == "exit" || prompt == "quit")
			break;
		if (prompt == "schema" || prompt == "format")
			prompt = "SELECT * FROM sqlite_master;";
		err = sqlite3_exec(db, prompt.c_str(), col_callback, NULL, NULL);
		if (0 != err) {
			std::cout << "SQLite reports error " << err << ".\n";
		}
		_first = true;
	}
	sqlite3_close(db);
	db = NULL;

	return 0;
}


