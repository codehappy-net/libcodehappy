/***

	invfn.h

	Invert a function, using an ANN trained on the original function.

	The number of neurons required will vary according to the complexity of the
	function -- test to find the accuracy/performance frontier for your application. You 
	can use more than 2 hidden layers as well, if there are singularities this may
	increase accuracy further.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __CODEHAPPY_INVFN__
#define __CODEHAPPY_INVFN__

/*** Function prototype. The first argument is the input vector. The second argument is a pointer
	to a vector where we put the outputs. The third parameter is user data. Return value
	is false if there is any error, true if and only if the function was evaluated successfully. ***/
typedef bool (*InvFnType)(float *, float *, void *);

class InvFn {
public:
	/* Create the inverse function object for a function with I real-valued inputs and 
	   O real-valued outputs. */
	InvFn(u32 I, u32 O, InvFnType fn);

	/* Constructors that also allow control over user data or the computational graph. */
	InvFn(u32 I, u32 O, InvFnType fn, void* ud);
	InvFn(u32 I, u32 O, InvFnType fn, void* ud, u32 neurons_);
	InvFn(u32 I, u32 O, InvFnType fn, void* ud, u32 neurons_, u32 hidden_layers);
	~InvFn();

	/* Train the neural network on the function over niter iterations. */
	void train(u32 niter = 2000);

	/* Give an estimate for the average loss over our sphere. */
	float avg_loss();

	/* Return the model's inverse for the specified point in the function's range. */
	const float* domain_from_range(float* range_point);

	/* Special case: for inverting an R->R function (i.e. 1 input, 1 output.) */
	float inv(float range_point);

	/* Special case: for inverting an R^n->R function (i.e. n inputs, 1 output.) */
	void inv(float range_point, float* domain_out);

	/* Accessors. */
	u32 num_inputs() const  { return ni; }
	u32 num_outputs() const { return no; }
	bool is_verbose() const { return vs.is_verbose(); }
	void verbose(bool v)    { if (v) vs.verbose(); else vs.quiet(); }
	float get_radius() const { return radius; }
	void set_radius(float r) { radius = r; }

private:
	/* Helper: give a random point in our sphere. */
	void generate_random_pt(float* pt) const;

	/* Helper: initialize the ANN. */
	void init_nnet();

	/* Our neural network. */
	kann_t* nnet;

	/* The number of inputs (to the original function) == the number of outputs to the neural net.
	   The number of outputs (from the original function) == the number of inputs to the neural net. */
	u32 ni, no;

	/* Some allocated space for inputs or outputs. */
	float* in;
	float* out;

	/* The radius of the central sphere that we use for training and loss calculation. */
	float radius;

	/* The total number of training iterations done. */
	u32 ntrain;

	/* The number of hidden layers and neurons per hidden layer in the computational graph structure. */
	u32 neurons, layers;

	/* Any user data that was provided. */
	void* user_data;

	/* The function we're inverting. */
	InvFnType func;

	/* Our VerboseStream. */
	VerboseStream vs;
};

#endif  // __CODEHAPPY_INVFN__
/* end invfn.h */