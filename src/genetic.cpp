/***

	genetic.cpp

	Non-linear optimization via genetic algorithms.

	The object of the GeneticOptimizer is to discover the vector of floating-point
	parameters that, as input, minimize (or maximize) a given function.

	To use it, construct a GeneticOptimizer with the right input size, and
	give it the function to optimize. The target function must have the basic prototype

	double func_name(double* in_vector, void* user_data);

	in_vector is the vector of floating point inputs. user_data is a provided void
	pointer; this can be a structure or class that provides any other parameters or
	context for evaluating the function, or left null if unneeded. The return value 
	must be the target function's output. For singularities or input vectors otherwise
	out of range, return nan(). (You can set bounds on input parameters to eliminate 
	or limit out of range cases.) 

	You then call optimize_min() or optimize_max() to estimate the minimum or 
	maximum. The optimize*() functions take one parameter: a double that represents
	the minimum accuracy required. New generations will be created until the best 
	solution fails to improve by this much.

	The return value is a double*, owned by the GeneticOptimizer, representing the
	optimum input. This memory will be reused on subsequent calls, so consume it right 
	away or copy the solution if you need it later.

	There is no guarantee we reach a global optimum, though mutation rate is managed
	as the algorithm converges to make getting caught in shallow local optima unlikely.
	Being continuous, differentiable, and otherwise well-behaved increases the chances 
	of convergence to a global optimum.

	You can re-run optimize*() to create more simulations, and sometimes achieve a
	better solution, or reset() to free the generated solutions.

	Copyright (c) 2022 C. M. Street.

***/
#include <libcodehappy.h>

GeneticOrganism::GeneticOrganism(u32 n_inputs) {
	in = new double [n_inputs];
	fitness_raw = nan("");
	fitness_norm = 0.0;
	age = 0;
}

GeneticOrganism::~GeneticOrganism() {
	delete [] in;
}

GeneticOptimizer::GeneticOptimizer(u32 n_inputs) {
	best_in = new double [n_inputs];
	lobound_in = nullptr;
	hibound_in = nullptr;
	mutation_chance = INITIAL_MUTATION;	// 1 in 256 mutation chance, to start
	best_fitness = nan("");
	target_fn = nullptr;
	vs.quiet();
	want_max = true;
	ni = n_inputs;
	user_data = nullptr;
	constraint = false;
	optimum_is_zero = false;
}

GeneticOptimizer::GeneticOptimizer(u32 n_inputs, OptimizeCallback fn) {
	best_in = new double [n_inputs];
	lobound_in = nullptr;
	hibound_in = nullptr;
	mutation_chance = INITIAL_MUTATION;
	best_fitness = nan("");
	target_fn = fn;
	vs.quiet();
	want_max = true;
	ni = n_inputs;
	user_data = nullptr;
	constraint = false;
	optimum_is_zero = false;
}

GeneticOptimizer::GeneticOptimizer(u32 n_inputs, OptimizeCallback fn, void* ud) {
	best_in = new double [n_inputs];
	lobound_in = nullptr;
	hibound_in = nullptr;
	mutation_chance = INITIAL_MUTATION;
	best_fitness = nan("");
	target_fn = fn;
	vs.quiet();
	want_max = true;
	ni = n_inputs;
	user_data = ud;
	constraint = false;
	optimum_is_zero = false;
}

GeneticOptimizer::~GeneticOptimizer() {
	delete [] best_in;
	for (auto go : orgs)
		delete go;
	orgs.empty();
	if (!is_null(lobound_in))
		delete [] lobound_in;
	if (!is_null(hibound_in))
		delete [] hibound_in;
}

void GeneticOptimizer::set_lobound(double min_val) {
	for (u32 e = 0; e < ni; ++e)
		set_lobound(e, min_val);
}

void GeneticOptimizer::set_lobound(u32 input_idx, double min_val) {
	if (input_idx >= ni)
		return;
	if (is_null(lobound_in)) {
		lobound_in = new double [ni];
		for (u32 e = 0; e < ni; ++e)
			lobound_in[e] = nan("");
	}
	lobound_in[input_idx] = min_val;
}

void GeneticOptimizer::set_hibound(double max_val) {
	for (u32 e = 0; e < ni; ++e)
		set_hibound(e, max_val);
}

void GeneticOptimizer::set_hibound(u32 input_idx, double max_val) {
	if (input_idx >= ni)
		return;
	if (is_null(hibound_in)) {
		hibound_in = new double [ni];
		for (u32 e = 0; e < ni; ++e)
			hibound_in[e] = nan("");
	}
	hibound_in[input_idx] = max_val;
}

double* GeneticOptimizer::optimize_min(double accuracy) {
	want_max = false;
	return optimize(accuracy);
}

double* GeneticOptimizer::optimize_max(double accuracy) {
	want_max = true;
	return optimize(accuracy);
}

double* GeneticOptimizer::optimize(double accuracy) {
	u32 e;
	u32 g = 0, gl = 0;
	if (target_fn == nullptr)
		return nullptr;
	if (accuracy > 0.9)
		accuracy = 0.9;
	if (orgs.empty()) {
		for (e = 0; e < NUM_ORGANISMS; ++e) {
			GeneticOrganism* go = new GeneticOrganism(ni);
			fill_random_inputs(go->in);
			enforce_sum_constraint(go->in);
			orgs.push_back(go);
		}
	}
	if (isnan(best_fitness)) {
		/* Calculate initial fitness. */
		best_fitness = calc_fitness();
	}
	if (isnan(best_fitness)) {
		return best_in;
	}

	/* Generational loop. */
	forever {
		double new_fit, fit_ratio;
		/* Generational turnover. */
		vs << "Creating new generation... ";
		new_generation();
		++g;

		/* Calculate new fitness. */
		new_fit = calc_fitness();
		vs << "fitness: " << new_fit << ".\n";
		if (isnan(new_fit)) {
			vs << "Warning: new fitness is not a number, bailing.\n";
			break;
		}
		if (want_max) {
			fit_ratio = new_fit / best_fitness;
			if (fit_ratio < 0.) {
				fit_ratio = 2.;
			}
		} else {
			fit_ratio = best_fitness / new_fit;
			if (fit_ratio < 0.) {
				fit_ratio = 2.;
			}
		}
		vs << "Fitness ratio " << fit_ratio << ".\n";
		if (optimum_is_zero && std::abs(new_fit) < accuracy) {
			vs << "Fitness is within accuracy specification.\n";
			break;
		}
		if (!optimum_is_zero && /* g >= 200 && */ fit_ratio < 1.0 + accuracy) {
			// not a good enough improvement -- increase mutation rate if we can, else we're done.
			++gl;
			if (gl >= 2 /* 20 */) {
				if (mutation_chance <= LAST_MUTATION) {
					vs << "Mutation rate at maximum, bailing.\n";
					break;
				}
				mutation_chance >>= 1;
				vs << "Mutation rate doubled to 1/" << mutation_chance << ".\n";
				gl = 0;
			}
		}
		if ((want_max && new_fit > best_fitness) || (!want_max && new_fit < best_fitness)) {
			best_fitness = new_fit;
			vs << "New best fitness: " << best_fitness << ".\n";
		}
	}

	return best_in;
}

void GeneticOptimizer::new_generation(void) {
	GrabBag<u32> repro, dieoff;
	GeneticOrganism* newgen[GENERATION_TURNOVER];
	u32 e, sz;
	sz = orgs.size();
	for (e = 0; e < sz; ++e) {
		GeneticOrganism* go = orgs[e];
		++go->age;
		if (isnan(go->fitness_raw)) {
			// if the fitness is NaN, this organism should die but not reproduce.
			dieoff.Insert(e, 1024);
			continue;
		}
		/* organisms are chosen to reproduce based on fitness (normed into [0, 1]). */
		repro.Insert(e, weight_from_normed_fitness(go->fitness_norm));
		/* organisms are chosen to perish based on the inverse of fitness. */
		dieoff.Insert(e, weight_from_normed_fitness(1.0 - go->fitness_norm));
	}

	/* First, create the new organisms. */
	for (e = 0; e < GENERATION_TURNOVER; ++e) {
		GeneticOrganism* parent_1, * parent_2;
		parent_1 = orgs[repro.Select()];
		do {
			parent_2 = orgs[repro.Select()];
		} while (parent_2 == parent_1);
		newgen[e] = couple(parent_1, parent_2);
	}

	/* Now, replace the organisms that are chosen to die off this generation. */
	dieoff.SetReplace(false);
	dieoff.SetRemoveSelectedEntirely(true);
	for (e = 0; e < GENERATION_TURNOVER; ++e) {
		u32 unlucky_idx = dieoff.Select();
		// Destroy the old and replace with the new.
		delete orgs[unlucky_idx];
		orgs[unlucky_idx] = newgen[e];
	}
}

double GeneticOptimizer::random_input_in_range(u32 input_idx) const {
	double lo, hi;
	const double GENERIC_LO = -1000., GENERIC_HI = 1000.;
	if (input_idx >= ni) {
		return nan("");
	}
	if (lobound_in == nullptr && hibound_in == nullptr) {
		return RandDouble(GENERIC_LO, GENERIC_HI);
	}
	if (lobound_in == nullptr) {
		lo = GENERIC_LO;
		hi = hibound_in[input_idx];
	} else if (hibound_in == nullptr) {
		lo = lobound_in[input_idx];
		hi = GENERIC_HI;
	} else {
		lo = lobound_in[input_idx];
		hi = hibound_in[input_idx];
	}
	if (isnan(lo))
		lo = GENERIC_LO;
	if (isnan(hi))
		hi = GENERIC_HI;
	SORT2(lo, hi, double);
	return RandDouble(lo, hi);
}

void GeneticOptimizer::fill_random_inputs(double* v) const {
	for (u32 e = 0; e < ni; ++e) {
		v[e] = random_input_in_range(e);
	}
}

void GeneticOptimizer::reset() {
	for (auto go : orgs)
		delete go;
	orgs.clear();
	mutation_chance = INITIAL_MUTATION;
	best_fitness = nan("");
}

static bool __genorgsort(const GeneticOrganism* go1, const GeneticOrganism* go2) {
	return go1->fitness_raw < go2->fitness_raw;
}

double GeneticOptimizer::calc_fitness() {
	/* Calculate fitness for every new organism, and return the best fitness we see. */
	double ret = best_fitness;
	bool first = true;

	worst_fit = nan("");
	for (auto go : orgs) {
		if (0 == go->age) {
			// A newborn, fill in fitness.
			go->fitness_raw = target_fn(go->in, user_data);
		}
		if (isnan(go->fitness_raw))
			continue;
		if (first) {
			if (isnan(ret) || (want_max && go->fitness_raw > ret) || (!want_max && go->fitness_raw < ret)) {
				ret = go->fitness_raw;
			}
			worst_fit = go->fitness_raw;
			first = false;
		} else {
			if ((want_max && go->fitness_raw > ret) || (!want_max && go->fitness_raw < ret)) {
				ret = go->fitness_raw;
				memcpy(best_in, go->in, sizeof(double) * ni);
				// we only set best_fitness in the generational loop, so we can compare improvement.
			}
			if ((want_max && go->fitness_raw < worst_fit) || (!want_max && go->fitness_raw > worst_fit))
				worst_fit = go->fitness_raw;
		}
	}

	if (first) {
		// There is no non-nan() fitness...
		vs << "No numeric fitness found?\n";
		return ret;
	}

	/* We can now normalize fitness to [0, 1], by closeness to our objective. */
	std::sort(orgs.begin(), orgs.end(), __genorgsort);
	for (u32 e = 0; e < orgs.size(); ++e) {
		GeneticOrganism* go = (GeneticOrganism *)orgs[e];
		if (want_max) {
			// from 0 to 1
			go->fitness_norm = double(e) / double(orgs.size() - 1);
		} else {
			// from 1 to 0
			go->fitness_norm = 1.0 - (double(e) / double(orgs.size() - 1));
		}
	}
#if 0
	for (auto go : orgs) {
		if (isnan(go->fitness_raw))
			continue;
		if (worst_fit == ret) {
			// Just one valid fitness, or all are equally fit?
			go->fitness_norm = 0.5;
			continue;
		}
		if (want_max) {
			// Translates the range [worst_fit, ret] to [0, 1]
			go->fitness_norm = (go->fitness_norm - worst_fit) / (ret - worst_fit);
		} else {
			// Translates the range [ret, worst_fit] to [1, 0]
			go->fitness_norm = 1.0 - (go->fitness_norm - ret) / (worst_fit - ret);
		}
	}
#endif

	return ret;
}

u32 GeneticOptimizer::weight_from_normed_fitness(double fit) const {
	/***
		1.0 = 1024
		0.9 = 512
		0.8 = 256
		...
		0.2 = 4
		0.1 = 2
		0.0 = 1

		i.e. 2 ^ (fit * 10)
	***/
	double v = pow(2.0, fit * 10.0);
	return (u32)(floor(v + 0.5));
}

GeneticOrganism* GeneticOptimizer::couple(GeneticOrganism* p1, GeneticOrganism* p2) const {
	GeneticOrganism* ret = new GeneticOrganism(ni);

	/* The options for each 'gene' (weight in our vector):
		First check for mutation -- if so, random weight, else select an outcome in [0, 3]:
		0 = copy parent 1's gene unchanged.
		1 = copy parent 2's gene unchanged.
		2 = a random weight between the two parent's genes.
		3 = the average of the parent's genes. */
	for (u32 e = 0; e < ni; ++e) {
		if (OneIn(mutation_chance)) {
			ret->in[e] = random_input_in_range(e);
			continue;
		}
		switch (RandU32Range(0, 3)) {
		case 0:	
			ret->in[e] = p1->in[e];
			break;
		case 1:
			ret->in[e] = p2->in[e];
			break;
		case 2:
			if (p1->in[e] < p2->in[e]) {
				ret->in[e] = RandDouble(p1->in[e], p2->in[e]);
			} else {
				ret->in[e] = RandDouble(p2->in[e], p1->in[e]);
			}
			break;
		case 3:
			ret->in[e] = (p1->in[e] + p2->in[e]) / 2.0;
			break;
		}
	}

	enforce_sum_constraint(ret->in);

	return ret;
}

void GeneticOptimizer::enforce_sum_constraint(double* v) const {
	if (!constraint)
		return;
	double sum = 0.;
	u32 e;
	for (e = 0; e < ni; ++e) {
		sum += v[e];
	}
	if (sum == 0.0) {
		v[0] = constraint_sum;
		return;
	}
	sum = (constraint_sum / sum);
	for (e = 0; e < ni; ++e) {
		v[e] *= sum;
	}
}

/* end genetic.cpp */