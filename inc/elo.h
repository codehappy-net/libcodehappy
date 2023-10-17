/***

	elo.h

	Calculate Elo performance ratings for a series of paired game results.

	Can be used to rank things: let the "game" be 'which item is better?',
	get the results for each possible pair, and calculate the Elo.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __ELO_H__
#define __ELO_H__

struct EloResult {
	EloResult(int winner, int loser);

	int w;
	int l;
};

class EloCalc {
public:
	/* default constructor with default settings for base rating/algorithm of 400 */
	EloCalc();
	/* Create an Elo calculator with the specified base/default/average rating,
           and the "algorithm of 400" number (a weight that determines the amount the
           rating changes per win.) */
	EloCalc(int base_rating_, u32 algorithm400_ = 400);
	~EloCalc();

	/* Add a result to the calculator. */
	void add_result(const EloResult& er);

	/* Remove all results from the calculator. */
	void clear();

	/* Calculate the Elo ratings. */
	void calc_ratings(u32 max_iter = 16);

	/* Return Elo rating for the specified index. Returns base_rating if ratings
	   haven't been calculated or if no results for the given index have been seen. */
	int elo_rating(int idx) const;

private:
	int base_rating;
	u32 algorithm400;
	std::vector<EloResult> results;
	int min_i, max_i;
	int* ratings;
	u32 cratings;
};

#endif  // __ELO_H__
/* end elo.h */