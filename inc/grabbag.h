/***

	grabbag.h

	Template for creating a 'grab bag': a collection that can return objects 
	according to weighted probabilties (with or without replacement).

	Grab bag objects should be copy-constructable. Operations on the bag
	are guarded by mutexes for thread safety.

	Copyright (c) 2022 C. M. Street.

***/
#ifndef GRABBAG_H
#define GRABBAG_H

#include <vector>

template<typename _T_>
struct GrabBagObj {
	_T_ obj;
	u32 weight;
};

template<typename _T_>
class GrabBag {
public:
	GrabBag() {
		weight = 0U;
		replace = true;
		remove_entirely = false;
        }

	/* Insert a copy of obj into the grab bag with weight item_weight. */
	void Insert(const _T_& obj, u32 item_weight) {
		GrabBagObj<_T_> ins;
		ScopeMutex sm(m);
		ins.obj = obj;
		ins.weight = item_weight;
		bag.push_back(ins);
		weight += item_weight;
        }

	/* Return whether the grab bag selects with replacement. */
	bool Replace(void) const { return replace; }

	/* Set whether the grab bag selects with replacement. */
	void SetReplace(bool rep) { replace = rep; }

	/* Return whether non-replaced objects are removed from the grab bag,
		or have their weight decremented. Only matters iff !replace. */
	bool RemoveSelectedTotally(void) const { return remove_entirely; }

	/* Set whether selected but not-replaced objects are removed entirely, or
		have their weight decremented. */
	void SetRemoveSelectedEntirely(bool remove) { remove_entirely = remove; }

	/* Select an object at random, according to weights. */
	_T_ Select(void) {
		if (!Empty()) {
			u32 sel = RandU32Range(0, weight - 1);
			ScopeMutex sm(m);

			for (auto& obj : bag) {
				if (obj.weight > sel) {
					return DoSelect(obj);
				}
				sel -= obj.weight;
			}
		}
		// return the default _T_ on a selection from an empty grab bag.
		_T_ ret;
		return ret;
	}

	/* Count of selectable (non-zero weight) objects currently in
		the grab bag. */
	u32 Count(void) {
		ScopeMutex sm(m);
		u32 ret = 0U;
		for (const auto& obj : bag) {
			if (!iszero(obj.weight))
				++ret;
		}
		return ret;
	}

	/* Empty the grab bag. */
	void Clear() {
		ScopeMutex sm(m);
		bag.clear();
		weight = 0;
	}

	/* Is the grab bag empty? */
	bool Empty() const {
		return (0 == weight);
	}

	/* Swap the contents of this grab bag with another grab bag. */
	void Swap(GrabBag<_T_>& gb) {
		ScopeMutex sm1(m), sm2(gb.m);
		bag.swap(gb.bag);
		SWAP(weight, gb.weight, u32);
		SWAP(replace, gb.replace, bool);
		SWAP(remove_entirely, gb.remove_entirely, bool);
	}

private:
	_T_ DoSelect(GrabBagObj<_T_>& obj) {
		if (!replace) {
			if (remove_entirely) {
				weight -= obj.weight;
				obj.weight = 0U;
			} else {
				--obj.weight;
				--weight;
			}
		}
		return obj.obj;
	}

	std::vector<GrabBagObj<_T_> > bag;
	u32 weight;
	bool replace;
	bool remove_entirely;
	std::mutex m;
};

#endif  // GRABBAG_H
