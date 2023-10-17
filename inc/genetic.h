/***

	genetic.h

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
	the minimum accuracy required (e.g., 5% would be 0.05). New generations will be 
	created until the best solution fails to improve by this much and mutation rate
	is at maximum.

	The return value is a double*, owned by the GeneticOptimizer, representing the
	optimum input. This memory will be reused on subsequent calls, so consume it right 
	away or copy the solution if you need it later.

	You may enforce a constraint on the weights that they equal a given sum.

	There is no guarantee we reach a global optimum, though mutation rate is managed
	as the algorithm converges to make getting caught in shallow local optima unlikely.
	Being continuous, differentiable, and otherwise well-behaved increases the chances 
	of convergence to a global optimum.

	You can re-run optimize*() to create more generations, and sometimes achieve a
	better solution, or reset() to free the generated solutions and start over.

	Copyright (c) 2022 C. M. Street.

***/
#ifndef __GENETIC_H__
#define __GENETIC_H__

/*** Callback for the target functions ***/
typedef double (*OptimizeCallback)(double *, void *);

struct GeneticOrganism {
public:
	GeneticOrganism(u32 n_inputs);
	~GeneticOrganism();

	double* in;
	double fitness_raw;
	double fitness_norm;
	u32 age;
};

class GeneticOptimizer {
public:
	GeneticOptimizer(u32 n_inputs);
	GeneticOptimizer(u32 n_inputs, OptimizeCallback fn);
	GeneticOptimizer(u32 n_inputs, OptimizeCallback fn, void* ud);
	~GeneticOptimizer();

	void set_lobound(double min_val);
	void set_lobound(u32 input_idx, double min_val);
	void set_hibound(double max_val);
	void set_hibound(u32 input_idx, double max_val);

	bool is_verbose() const { return vs.is_verbose(); }
	void verbose(bool v)    { if (v) vs.verbose(); else vs.quiet(); }

	void set_target_fn(OptimizeCallback fn) { target_fn = fn; }
	void set_user_data(void* ud) { user_data = ud; }
	void set_constraint_sum_weights(double sum) { constraint_sum = sum; constraint = true; }
	void set_optimum_is_zero(bool isz) { optimum_is_zero = isz; }

	double* optimize_min(double accuracy);
	double* optimize_max(double accuracy);

	void reset();

private:
	double* optimize(double accuracy);
	void new_generation(void);
	double random_input_in_range(u32 input_idx) const;
	void fill_random_inputs(double* v) const;
	double calc_fitness();
	u32 weight_from_normed_fitness(double fit) const;
	GeneticOrganism* couple(GeneticOrganism* p1, GeneticOrganism* p2) const;
	void enforce_sum_constraint(double* v) const;

	const u32 NUM_ORGANISMS = 8192;
	const u32 GENERATION_TURNOVER = 2048;
	const u32 INITIAL_MUTATION = 64;
	const u32 LAST_MUTATION = 8;

	std::vector<GeneticOrganism *> orgs;
	double* best_in;
	double* lobound_in;
	double* hibound_in;
	u32 mutation_chance;
	u32 ni;
	double best_fitness;
	double worst_fit;
	OptimizeCallback target_fn;
	VerboseStream vs;
	bool want_max;
	void* user_data;
	bool constraint;
	double constraint_sum;
	bool optimum_is_zero;
};

#endif  // __GENETIC_H__
/* end genetic.h */