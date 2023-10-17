/***

	require.h

	Specify logical requirements (equalities/inequalities) that a numeric value 
	must satisfy, and evaluate those requirements. Template class; use with any
	numeric type. (Anything with a copy constructor, the arithmetic operators
	defined, and a cast from int to type should work.)

	Copyright (c) 2022, C. M. Street.

***/
#ifndef REQUIRE_H
#define REQUIRE_H

enum ReqOper {
	REQ_EQ = 0,
	REQ_NEQ,
	REQ_GTE,
	REQ_LTE,
	REQ_GT,
	REQ_LT,
	REQ_INT,
};

/* Individual requirement struct. */
template<typename _T_>
struct ReqIndiv {
	ReqIndiv(_T_ v)            { op = REQ_EQ; val = v; }
	ReqIndiv(ReqOper o, _T_ v) { op = o; val = v; }

	bool eval(_T_ v) const {
		switch (op) {
		case REQ_EQ:
			return v == val;
		case REQ_NEQ:
			return v != val;
		case REQ_GTE:
			return v >= val;
		case REQ_LTE:
			return v <= val;
		case REQ_GT:
			return v > val;
		case REQ_LT:
			return v < val;
		case REQ_INT:
			return v == floor(v);
		}
		return false;
	}

	ReqOper op;
	_T_ val;
};

/* Grouped requirements -- these are essentially independent clauses that are ORed together in the
   parent Requirement, and can be individually negated. */
template<typename _T_>
struct ReqGroup {
	ReqGroup() { is_negated = false; }

	ReqGroup(const ReqIndiv<_T_>& req) {
		is_negated = false;
		add(req);
	}

	void add(const ReqIndiv<_T_>& req) {
		reqs.push_back(req);
	}

	bool eval(_T_ v) const {
		for (const auto& req : reqs) {
			if (!req.eval(v))
				return is_negated;
		}
		return !is_negated;
	}

	bool vacuous() const {
		return reqs.size() == 0;
	}

	std::vector< ReqIndiv<_T_> > reqs;
	bool is_negated;
};

/* Requirements class. */
template<typename _T_>
class Requirement {
public:
	/* Note that an empty Requirement will return false evaluated on any point in its domain. */
	Requirement() {
		new_clause();
	}

	/* Begin a new requirements clause. All requirements within a clause must be true in order
	   for the clause to be true (they're ANDed together.) Multiple clauses are ORed together
	   (i.e. the Requirement is true if any one is true.) Each clause can be negated (so a
	   requirement in the clause must be false for the clause to be true.) By combining
	   requirement clauses, complex logical requirements can be built. */
	void new_clause() {
		ReqGroup<_T_> rg;
		reqs.push_back(rg);
	}

	/* Negate the current requirement clause. */
	void negate_clause() {
		auto& req = reqs[reqs.size() - 1];
		req.is_negated = !req.is_negated;
	}

	/* Add an equal condition to the current requirement clause. */
	void equal(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(v));
	}

	/* Add a not-equal condition to the current requirement clause. */
	void not_equal(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(REQ_NEQ, v));
	}

	/* Add a less than-or-equal condition to the current requirement clause. */
	void lte(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(REQ_LTE, v));
	}

	/* Add a greater than-or-equal condition to the current requirement clause. */
	void gte(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(REQ_GTE, v));
	}

	/* Add a less than condition to the current requirement clause. */
	void lt(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(REQ_LT, v));
	}

	/* Add a greater than condition to the current requirement clause. */
	void gt(_T_ v) {
		auto& req = reqs[reqs.size() - 1];
		req.add(ReqIndiv<_T_>(REQ_GT, v));
	}

	/* Add an integeral condition to the current requirement clause. */
	void integeral() {
		auto& req = reqs[reqs.size() - 1];
		_T_ dummy;
		req.add(ReqIndiv<_T_>(REQ_INT, dummy));
	}

	/* Add an in range (inclusive) condition to the current requirement clause. */
	void in_range_incl(_T_ val1, _T_ val2) {
		SORT2(val1, val2, _T_);
		gte(val1);
		lte(val2);
	}

	/* Add an in range (exclusive) condition to the current requirement clause. */
	void in_range_excl(_T_ val1, _T_ val2) {
		SORT2(val1, val2, _T_);
		gt(val1);
		lt(val2);
	}

	/* Add an out of range condition to the current requirement clause. (Endpoints included in range.) */
	void out_range_incl(_T_ val1, _T_ val2) {
		auto& req = reqs[reqs.size() - 1];
		if (!req.vacuous()) {
			new_clause();
		}
		in_range_incl(val1, val2);
		negate_clause();
		new_clause();
	}

	/* Add an out of range condition to the current requirement clause. (Endpoints excluded from range.) */
	void out_range_excl(_T_ val1, _T_ val2) {
		auto& req = reqs[reqs.size() - 1];
		if (!req.vacuous()) {
			new_clause();
		}
		in_range_excl(val1, val2);
		negate_clause();
		new_clause();
	}

	/* Add a negative condition to the current requirement clause. */
	void negative() {
#if 0
		if (std::is_arithmetic_v<_T_>) {
			lt(<_T_>(0));
		}
#else
		lt((_T_)0);
#endif
	}

	/* Add a positive condition to the current requirement clause. */
	void positive() {
#if 0
		if (std::is_arithmetic_v<_T_>) {
			gt(<_T_>(0));
		}
#else
		gt((_T_)0);
#endif
	}

	/* Add a nonnegative condition to the current requirement class. */
	void nonnegative() {
#if 0
		if (std::is_arithmetic_v<_T_>) {
			gte(<_T_>(0));
		}
#else
		gte((_T_)0);
#endif
	}

	/* Add a nonpositive condition to the current requirement class. */
	void nonpositive() {
#if 0
		if (std::is_arithmetic_v<_T_>) {
			lte(<_T_>(0));
		}
#else
		lte((_T_)0);
#endif
	}

	bool eval(_T_ v) const {
		for (const auto& req : reqs) {
			if (req.vacuous())
				continue;
			if (req.eval(v))
				return true;
		}
		return false;
	}

	bool vacuous() const {
		for (const auto& req : reqs) {
			if (!req.vacuous())
				return false;
		}
		return true;
	}

private:
	std::vector< ReqGroup<_T_> > reqs;
};

#endif  // REQUIRE_H
/* end require.h */