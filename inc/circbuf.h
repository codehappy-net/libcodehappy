/***

	circbuf.h

	Maintains a circular FIFO buffer of (numeric) objects. You can perform such
	operations as average, median, min/max, newest, oldest, etc. and have 
	random access to the elements in the buffer. The buffer size can be
	changed (if the size is decreased below the number of populated elements, 
	the oldest members are discarded.)

	Operations cache their results, and recalculate only as required. min, max, mean, and
	sum cache results are maintained as objects are inserted or removed, so are normally 
	O(1) rather than O(n).

	Insertions and removals always hold a mutex for thread safety. The default versions of 
	each operation also hold the mutex. There are also lockfree and non-caching versions
	of each operation.

	Useful for moving averages, rate estimators, convolution operations, fast estimation of 
	median or percentiles, or generally keeping running tabs on a stream of quantities.

	See also the 'tally' library in src/external.cpp; this is less general but provides a
	subset of this functionality in a non-template C99-compatible way.

	Use with any built-in arithmetic type, or any type that has these easy requirements: 
		a copy constructor (implicit or explicit) that ideally doesn't do anything crazy,
		the greater-than, less-than, & the addition and subtraction operators (including += and -=), 
		an additive identity (i.e. a zero). You must be able to cast/assign int(0) to your additive identity,
		can be divided by an integer, for mean(),
		a defined hash function, for mode(),
		a sort predicate, for median() and percentile(); your operator<() should do.

	Copyright (c) 2022 Chris Street.

***/
#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

#include <functional>

template<typename _T_, typename SortPredicate = std::less<_T_>, typename HashFunction = std::hash<_T_> >
class CircBuffer {
public:
	/* Create an empty circular buffer of the specified capacity. */
	CircBuffer(u32 size) {
		assert(size > 0);
		buf = new _T_ [size];
		idx_first = 0;
		nel = 0;
		sz = size;
		csum = 0;
		valid_flags = FLAG_SUM;
	}

	/* Create a circular buffer of the given size and fill it identically with "fill". */
	CircBuffer(u32 size, _T_ fill) {
		assert(size > 0);
		buf = new _T_ [size];
		idx_first = 0;
		sz = size;
		nel = sz;
		cmod = fill;
		cmed = fill;
		cmin = fill;
		cmax = fill;
		/* Do it this way, so we don't have to assume we can multiply _T_ by an int. */
		csum = fill;
		buf[0] = fill;
		for (u32 e = 1; e < sz; ++e) {
			buf[e] = fill;
			csum += fill;
		}
		valid_flags = FLAG_MODE | FLAG_MEDIAN | FLAG_MIN | FLAG_MAX | FLAG_SUM;
	}

	~CircBuffer() {
		delete [] buf;
	}

	/* Insertions and removals. These all grab our mutex for thread safety. */
	void insert(_T_ val) {
		lock();
		if (nel == sz) {
			cache_remove(buf[idx_first]);
			buf[idx_first] = val;
			idx_first = idx(idx_first + 1);
		} else {
			buf[idx(idx_first + nel)] = val;
			++nel;
		}
		if (valid(FLAG_SUM)) {
			csum += val;
		}
		if (valid(FLAG_MAX)) {
			if (val > cmax)
				cmax = val;
		}
		if (valid(FLAG_MIN)) {
			if (val < cmin)
				cmin = val;
		}
		if (1 == nel) {
			/* We get to fill our cache! */
			cmax = val;
			cmin = val;
			csum = val;
			cmed = val;
			cmod = val;
			is_valid(FLAG_MAX | FLAG_MIN | FLAG_SUM | FLAG_MEDIAN | FLAG_MODE);
		} else {
			/* Median no longer known to be valid, mode possibly not valid either. */
			/* (We could check that the inserted value isn't equal to known mode, but that's a pretty edge case.) */
			invalidate_sort();
		}
		unlock();
	}

	void remove_newest() {
		lock();
		if (valid(FLAG_MIN | FLAG_MAX | FLAG_SUM)) {
			cache_remove(newest());
		}
		if (nel > 0)
			--nel;
		invalidate_sort();
		unlock();
	}

	void remove_oldest() {
		if (0 == nel)
			return;
		lock();
		if (valid(FLAG_MIN | FLAG_MAX | FLAG_SUM)) {
			cache_remove(oldest());
		}
		idx_first = idx(idx_first + 1);
		--nel;
		invalidate_sort();
		unlock();
	}

	void remove_index(u32 i) {
		if (0 == nel) {
			return;
		}
		if (i == (nel - 1)) {
			remove_newest();
			return;
		}
		i += idx_first;
		if (idx_first == i) {
			remove_oldest();
			return;
		}
		lock();
		if (valid(FLAG_MIN | FLAG_MAX | FLAG_SUM)) {
			cache_remove(buf[i]);
		}
		u32 j;
		j = idx(i + 1);
		while (j != idx_first) {
			buf[i] = buf[j];
			i = j;
			j = idx(i + 1);
		}
		--nel;
		invalidate_sort();
		unlock();
	}

	void remove_min() {
		if (0 == nel)
			return;
		_T_ ex = buf[idx_first];
		u32 i = 0;
		u32 j = idx(idx_first + 1);
		for (u32 e = 1; e < nel; ++e) {
			if (buf[j] < ex) {
				ex = buf[j];
				i = e;
			}
			j = idx(j + 1);
		}
		invalidate(FLAG_MIN);
		remove_index(i);
	}

	void remove_max() {
		if (0 == nel)
			return;
		_T_ ex = buf[idx_first];
		u32 i = 0;
		u32 j = idx(idx_first + 1);
		for (u32 e = 1; e < nel; ++e) {
			if (buf[j] > ex) {
				ex = buf[j];
				i = e;
			}
			j = idx(j + 1);
		}
		invalidate(FLAG_MAX);
		remove_index(i);
	}

	void resize(u32 new_size) {
		_T_* newbuf;
		assert(new_size > 0);
		newbuf = new _T_ [new_size];
		while (nel > new_size) {
			remove_oldest();
		}
		for (u32 e = 0; e < nel; ++e)
			newbuf[e] = (*this)[e];
		lock();
		delete [] buf;
		buf = newbuf;
		sz = new_size;
		idx_first = 0;
		unlock();
	}

	/* Addressing elements. */
	_T_ newest() const {
		if (0 == nel)
			return (*this)[0];
		return (*this)[nel - 1];
	}

	_T_ oldest() const {
		return buf[idx_first];
	}

	_T_ operator[](u32 ix) const {
		if (nel == 0)
			return buf[0];
		ix += idx_first;
		if (ix >= nel)
			ix %= nel;
		return buf[ix];
	}

	/* Operations: const lockfree versions that _do not_ use or update cache results. The 
	   more useful versions are built from these primitives. */
	_T_ min_nocache() const {
		_T_ ret = buf[idx_first];
		for (u32 e = 1; e < nel; ++e) {
			_T_ v = (*this)[e];
			if (v < ret)
				ret = v;
		}
		return ret;
	}

	_T_ max_nocache() const {
		_T_ ret = buf[idx_first];
		for (u32 e = 1; e < nel; ++e) {
			_T_ v = (*this)[e];
			if (v > ret)
				ret = v;
		}
		return ret;
	}

	_T_ median_nocache() const {
		_T_ ret = (*this)[0];
		if (0 == nel)
			return ret;
		std::vector<_T_> els(nel);
		for (u32 e = 0; e < nel; ++e) {
			els[e] = (*this)[e];
		}
		std::sort(els.begin(), els.end(), SortPredicate{});
		if ((nel & 1) == 0)
			ret = (els[nel / 2] + els[(nel / 2) - 1]) / 2;
		else
			ret = els[nel / 2];
		return ret;
	}

	_T_ mode_nocache() const {
		_T_ ret = (*this)[0];
		std::unordered_map<_T_, u32, HashFunction> map;
		u32 ct = 0;

		for (u32 e = 0; e < nel; ++e)
			map[(*this)[e]]++;

		for (const auto& ent : map) {
			if (ent.second > ct) {
				ret = ent.first;
				ct = ent.second;
			}
		}

		return ret;
	}

	_T_ mean_nocache() const {
		_T_ ret = 0;
		if (0 == nel)
			return ret;

		for (u32 e = 0; e < nel; ++e)
			ret += (*this)[e];

		return ret / nel;
	}

	_T_ sum_nocache() const {
		_T_ ret = 0;
		for (u32 e = 0; e < nel; ++e) {
			ret += (*this)[e];
		}
		return ret;
	}

	/* Operations. These are lockfree but non-const since they cache results. */
	_T_ min_lockfree() {
		if (valid(FLAG_MIN))
			return cmin;
		_T_ ret = min_nocache();
		if (nel > 0) {
			cmin = ret;
			is_valid(FLAG_MIN);
		}
		return ret;
	}

	_T_ max_lockfree() {
		if (valid(FLAG_MAX))
			return cmax;
		_T_ ret = max_nocache();
		if (nel > 0) {
			cmax = ret;
			is_valid(FLAG_MAX);
		}
		return ret;
	}

	_T_ median_lockfree() {
		if (valid(FLAG_MEDIAN))
			return cmed;
		_T_ ret = median_nocache();
		if (nel > 0) {
			cmed = ret;
			is_valid(FLAG_MEDIAN);
		}
		return ret;
	}

	_T_ mode_lockfree() {
		if (valid(FLAG_MODE))
			return cmod;
		_T_ ret = mode_nocache();
		if (nel > 0) {
			cmod = ret;
			is_valid(FLAG_MODE);
		}
		return ret;
	}

	_T_ mean_lockfree() {
		if (valid(FLAG_SUM))
			return csum / nel;
		_T_ ret = sum_nocache();
		if (0 == nel)
			return ret;
		csum = ret;
		is_valid(FLAG_SUM);

		return ret / nel;
	}

	_T_ percentile_lockfree(u32 pct) {
		if (nel <= 1)
			return (*this)[0];
		if (50 == pct)
			return median_lockfree();
		if (pct >= 100)
			return max_lockfree();
		if (0 == pct)
			return min_lockfree();
		std::vector<_T_> els(nel);
		for (u32 e = 0; e < nel; ++e) {
			els[e] = (*this)[e];
		}
		std::sort(els.begin(), els.end(), SortPredicate{});
		u32 idx = ((nel - 1) * pct) / 100;
		return (*this)[idx];
	}

	_T_ sum_lockfree() {
		if (valid(FLAG_SUM))
			return csum;
		_T_ ret = sum_nocache();
		if (nel > 0) {
			csum = ret;
			is_valid(FLAG_SUM);
		}
		return ret;
	}

	/* Operations: thread-safe versions that hold our mutex and cache results. These are the 
	   ones you should normally call. */
	_T_ min() {
		ScopeMutex sm(m);
		return min_lockfree();
	}

	_T_ max() {
		ScopeMutex sm(m);
		return max_lockfree();
	}

	_T_ median() {
		ScopeMutex sm(m);
		return median_lockfree();
	}

	_T_ mode() {
		ScopeMutex sm(m);
		return mode_lockfree();
	}

	_T_ mean() {
		ScopeMutex sm(m);
		return mean_lockfree();
	}

	_T_ percentile(u32 pct) {
		ScopeMutex sm(m);
		return percentile_lockfree(pct);
	}

	_T_ sum() {
		ScopeMutex sm(m);
		return sum_lockfree();
	}

	u32 elements() const { return nel; }
	u32 size() const     { return sz;  }

private:
	u32 idx(u32 i) const {
		if (0 == nel)
			return 0;
		if (i >= nel)
			i %= nel;
		return i;
	}

	void lock()   { m.lock(); }
	void unlock() { m.unlock(); }

	bool valid(u32 flag) const { return (valid_flags & flag) != 0; }
	void invalidate(u32 flag)  { valid_flags &= (~flag); }
	void invalidate_sort()     { invalidate(FLAG_MEDIAN | FLAG_MODE); }
	void is_valid(u32 flag)    { valid_flags |= flag; }

	void cache_remove(const _T_& val) {
		if (val == cmin) {
			invalidate(FLAG_MIN);
		}
		if (val == cmax) {
			invalidate(FLAG_MAX);
		}
		if (valid(FLAG_SUM)) {
			csum -= val;
		}
	}

	_T_* buf;
	u32 idx_first;
	u32 nel;
	u32 sz;
	std::mutex m;

	/* Cached results. */
	u32 valid_flags;
	_T_ cmax;
	_T_ cmin;
	_T_ csum;
	_T_ cmed;
	_T_ cmod;
	const u32 FLAG_MAX = 1, FLAG_MIN = 2, FLAG_SUM = 4, FLAG_MEDIAN = 8, FLAG_MODE = 16;
};

#endif  // _CIRCBUF_H_
/* end circbuf.h */