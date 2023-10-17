/***

	tribool.h

	A three-state Boolean logic type -- true, false, and indeterminate. Also includes
	fuzzy bools, representing probabilities of truth.

	Copyright (c) 2014-2022 C. M. Street

***/

enum TriboolTruth {
	TRIBOOL_FALSE = -1,
	TRIBOOL_INDETERMINATE = 0,
	TRIBOOL_TRUE = 1
};

class Tribool {
public:
	Tribool() {
		state = TRIBOOL_INDETERMINATE;
	}

	Tribool(TriboolTruth st) {
		state = st;
	}

	Tribool(bool st) {
		state = st ? TRIBOOL_TRUE : TRIBOOL_FALSE;
	}

	const Tribool operator&&(const Tribool& rhs) const {
		if (state == TRIBOOL_INDETERMINATE || rhs.state == TRIBOOL_INDETERMINATE)
			return Tribool();
		if (state == TRIBOOL_TRUE && rhs.state == TRIBOOL_TRUE)
			return Tribool(true);
		return Tribool(false);
	}

	const Tribool operator||(const Tribool& rhs) const {
		if (state == TRIBOOL_TRUE || rhs.state == TRIBOOL_TRUE)
			return Tribool(TRIBOOL_TRUE);
		if (state == TRIBOOL_INDETERMINATE || rhs.state == TRIBOOL_INDETERMINATE)
			return Tribool();
		return Tribool(TRIBOOL_FALSE);
	}

	const Tribool operator!() const {
		if (state == TRIBOOL_TRUE)
			return Tribool(false);
		if (state == TRIBOOL_FALSE)
			return Tribool(true);
		return Tribool();
	}

	const Tribool operator^(const Tribool& rhs) const {
		return (*this || rhs) && !(*this && rhs);
	}

	const Tribool operator&(const Tribool& rhs) const {
		return *this && rhs;
	}

	const Tribool operator|(const Tribool& rhs) const {
		return *this || rhs;
	}

	const Tribool operator&&(bool rhs) const {
		if (rhs && state == TRIBOOL_TRUE)
			return Tribool(true);
		if (state == TRIBOOL_INDETERMINATE)
			return Tribool();
		return Tribool(false);
	}

	const Tribool operator||(bool rhs) const {
		if (rhs)
			return Tribool(true);
		return Tribool(state);
	}

	const Tribool operator=(bool rhs) {
		*this = Tribool(rhs);
		return *this;
	}

	bool operator==(const Tribool& rhs) const {
		return state == rhs.state;
	}

	bool operator==(bool rhs) const {
		if (rhs)
			return state == TRIBOOL_TRUE;
		return state == TRIBOOL_FALSE;
	}

	bool operator!=(const Tribool& rhs) const {
		return !(*this == rhs);
	}

	bool operator!=(bool rhs) const {
		return !(*this == rhs);
	}

	enum TriboolTruth tru(void) const {
		return state;
	}

	/* Allow casting to integeral or boolean types, so we can do "if (tribool)" */
	operator bool() const         { return (state == TRIBOOL_TRUE); }
	operator int() const          { return (int)(state == TRIBOOL_TRUE); }
	operator unsigned int() const { return (unsigned int)(state == TRIBOOL_TRUE); }

private:
	enum TriboolTruth state;
};

/*** FuzzyBool -- represents a logical probability. Operations that evaluate truth respond
	true or false based on a random probability. ***/

const double FUZZY_TRUE = 1.0;
const double FUZZY_FALSE = 0.0;
const double FUZZY_INDETERMINATE = 0.5;

class FuzzyBool {
public:
	FuzzyBool() { p = FUZZY_INDETERMINATE; }

	FuzzyBool(bool tr) {
		p = tr ? FUZZY_TRUE : FUZZY_FALSE;
	}

	FuzzyBool(double d) {
		p = CLAMP(d, FUZZY_FALSE, FUZZY_TRUE);
	}

	FuzzyBool(const Tribool& trib) {
		if (trib.tru() == TRIBOOL_INDETERMINATE)
			p = FUZZY_INDETERMINATE;
		else if (trib.tru() == TRIBOOL_TRUE)
			p = FUZZY_TRUE;
		else
			p = FUZZY_FALSE;
	}

	const FuzzyBool operator&&(const FuzzyBool& rhs) const {
		return FuzzyBool(p * rhs.p);
	}

	const FuzzyBool operator||(const FuzzyBool& rhs) const {
		return FuzzyBool(p + rhs.p - (p * rhs.p));
	}

	const FuzzyBool operator!() const {
		return FuzzyBool(FUZZY_TRUE - p);
	}

	const FuzzyBool operator^(const FuzzyBool& rhs) const {
		return (*this || rhs) && !(*this && rhs);
	}

	const FuzzyBool operator&(const FuzzyBool& rhs) const {
		return *this && rhs;
	}

	const FuzzyBool operator|(const FuzzyBool& rhs) const {
		return *this || rhs;
	}

	const FuzzyBool operator=(bool rhs) {
		*this = FuzzyBool(rhs);
		return *this;
	}

	const FuzzyBool operator=(const Tribool& trib) {
		*this = FuzzyBool(trib);
		return *this;
	}

	bool operator==(const FuzzyBool& rhs) {
		return p == rhs.p;
	}

	bool operator==(const Tribool& rhs) const {
		return *this == FuzzyBool(rhs);
	}

	bool operator==(bool rhs) const {
		if (rhs)
			return p == FUZZY_TRUE;
		return p == FUZZY_FALSE;
	}

	bool operator!=(const FuzzyBool& rhs) const {
		return !(*this == rhs);
	}

	bool operator!=(const Tribool& rhs) const {
		return !(*this == rhs);
	}

	bool operator!=(bool rhs) const {
		return !(*this == rhs);
	}

	bool tru(void) const {
		return RandDouble(FUZZY_FALSE, FUZZY_TRUE) <= p;
	}

	operator bool() const         { return tru(); }
	operator int() const          { return (int)(tru()); }
	operator unsigned int() const { return (unsigned int)(tru()); }

private:
	double p;
};

/* end tribool.h */