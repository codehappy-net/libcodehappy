/***

	elo.cpp

	Calculate Elo performance ratings for a series of paired game results.

	Can be used to rank things: let the "game" be 'which item is better?',
	get the results for each possible pair, and calculate the Elo.

	Copyright (c) 2022 Chris Street.

***/

EloResult::EloResult(int winner, int loser) {
	w = winner;
	l = loser;
}

EloCalc::EloCalc() {
	base_rating = 1000;
	algorithm400 = 400;
	min_i = INT32_MAX;
	max_i = INT32_MIN;
	ratings = nullptr;
	cratings = 0;
}

EloCalc::EloCalc(int base_rating_, u32 algorithm400_) {
	base_rating = base_rating_;
	algorithm400 = algorithm400_;
	min_i = INT32_MAX;
	max_i = INT32_MIN;
	ratings = nullptr;
	cratings = 0;
}

EloCalc::~EloCalc() {
	if (ratings != nullptr) {
		delete ratings;
		ratings = nullptr;
		cratings = 0;
	}
}

void EloCalc::add_result(const EloResult& er) {
	if (er.w < min_i)
		min_i = er.w;
	if (er.l < min_i)
		min_i = er.l;
	if (er.l > max_i)
		max_i = er.l;
	if (er.w > max_i)
		max_i = er.w;
	results.push_back(er);
}

void EloCalc::clear() {
	results.clear();
	min_i = INT32_MAX;
	max_i = INT32_MIN;
	if (ratings != nullptr) {
		delete ratings;
		ratings = nullptr;
		cratings = 0;
	}
}

void EloCalc::calc_ratings(u32 max_iter) {
	int ct = max_i - min_i + 1, e;
	u32 i = 0;

	if (cratings < ct) {
		if (ratings != nullptr) {
			delete ratings;
		}
		ratings = new int [ct];
		cratings = ct;
	}
	for (i = 0; i < cratings; ++i)
		ratings[i] = base_rating;

	i = 0;
	while (i < max_iter) {
		int j;
		for (j = min_i; j <= max_i; ++j) {
			int w, l;
			ct = 0;	// total rating of j's opponents
			w = 0;	// j's wins
			l = 0;	// j's losses
			for (const auto& r : results) {
				if (r.w == j) {
					++w;
					ct += ratings[r.l - min_i];
				} else if (r.l == j) {
					++l;
					ct += ratings[r.w - min_i];
				}
			}
			if (0 == w + l)
				continue;
			/* j's new rating. */
			ratings[j - min_i] = (ct + int(algorithm400) * (w - l)) / (w + l);
		}
		// perform multiple iterations to converge on the correct Elo ratings.
		++i;
	}
}

int EloCalc::elo_rating(int idx) const {
	if (idx < min_i)
		return base_rating;
	if (idx > max_i)
		return base_rating;
	idx -= min_i;
	if (idx >= cratings)
		return base_rating;
	return ratings[idx];
}
