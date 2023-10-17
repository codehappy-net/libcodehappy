/***

	invfn.cpp

	Invert a function, using an ANN trained on the original function.

	Copyright (c) 2022 Chris Street.

***/

InvFn::InvFn(u32 I, u32 O, InvFnType fn) {
	ni = I;
	no = O;
	func = fn;
	in = new float [ni];
	out = new float [no];
	radius = 1.;
	ntrain = 0;
	neurons = 800;
	layers = 2;
	user_data = nullptr;
	vs.quiet();
	init_nnet();
}

InvFn::InvFn(u32 I, u32 O, InvFnType fn, void* ud) {
	ni = I;
	no = O;
	func = fn;
	in = new float [ni];
	out = new float [no];
	radius = 1.;
	ntrain = 0;
	neurons = 800;
	layers = 2;
	user_data = ud;
	vs.quiet();
	init_nnet();
}

InvFn::InvFn(u32 I, u32 O, InvFnType fn, void* ud, u32 neurons_) {
	ni = I;
	no = O;
	func = fn;
	in = new float [ni];
	out = new float [no];
	radius = 1.;
	ntrain = 0;
	neurons = neurons_;
	layers = 2;
	user_data = ud;
	vs.quiet();
	init_nnet();
}

InvFn::InvFn(u32 I, u32 O, InvFnType fn, void* ud, u32 neurons_, u32 hidden_layers) {
	ni = I;
	no = O;
	func = fn;
	in = new float [ni];
	out = new float [no];
	radius = 1.;
	ntrain = 0;
	neurons = neurons_;
	layers = hidden_layers;
	user_data = ud;
	vs.quiet();
	init_nnet();
}

InvFn::~InvFn() {
	delete [] in;
	delete [] out;
	kann_delete(nnet);
}

/* Helper: initialize the ANN. */
void InvFn::init_nnet() {
	kad_node_t *t;
	// The neural network's outputs are the original function's inputs, and vice versa.
	t = kann_layer_input(no);
	for (u32 e = 0; e < layers; ++e) {
		t = kann_layer_dropout(kad_relu(kann_layer_dense(t, neurons)), 0.0f);
	}
	t = kann_layer_cost(t, ni, KANN_C_CEB);
	kann_verbose = 0;
	nnet = kann_new(t, 0);
}

/* Train the neural network on the function over niter iterations. */
void InvFn::train(u32 niter) {
	float** batch_in, **batch_out;
	u32 c = 0;
	if (niter < 1)
		return;
	batch_in = new float * [niter];
	batch_out = new float * [niter];

	vs << "Training over " << niter << " iterations...\n";

	for (u32 e = 0; e < niter; ++e) {
		/* Generate a random input and calculate the function's output. */
		generate_random_pt(in);
		if (!func(in, out, user_data)) {
			continue;
		}
		batch_in[c] = new float [no];
		batch_out[c] = new float [ni];
		memcpy(batch_in[c], out, sizeof(double) * no);
		memcpy(batch_out[c], in, sizeof(double) * ni);
		++c;
	}

	kann_train_fnn1(nnet, 0.001 /* learning rate */, 64 /* mini_size? */, 10 /* max epoch */, 10 /* max_drop_streak? */, 0.1f /* validation fraction */, c, batch_in, batch_out);
	ntrain += niter;

	for (u32 e = 0; e < c; ++e) {
		delete [] batch_in[e];
		delete [] batch_out[e];
	}
	delete batch_in;
	delete batch_out;
}

/* Give an estimate for the average loss over our sphere. */
float InvFn::avg_loss() {
	float loss = 0.;
	u32 c = 0;
	for (u32 e = 0; e < 10000; ++e) {
		const float* fo;
		float li;
		generate_random_pt(in);
		if (!func(in, out, user_data)) {
			continue;
		}
		++c;
		fo = domain_from_range(out);
		assert(fo != nullptr);
		li = 0.;
		for (u32 f = 0; f < ni; ++f) {
			li += (in[f] - fo[f]) * (in[f] - fo[f]);
		}
		li = sqrt(li);
		loss += li;
	}
	if (0 == c)
		return 0.;
	vs << "Calculated estimated loss: " << loss / float(c) << "\n";
	return loss / float(c);
}

/* Return the model's inverse for the specified point in the function's range. */
const float* InvFn::domain_from_range(float* range_point) {
	if (0 == ntrain) {
		vs << "We must train the neural network before evaluating it.\n";
		train();
	}
	return kann_apply1(nnet, range_point);
}

/* Special case: for inverting an R->R function (i.e. 1 input, 1 output.) */
float InvFn::inv(float range_point) {
	ship_assert(ni == 1 && no == 1);
	return *domain_from_range(&range_point);
}

/* Special case: for inverting an R^n->R function (i.e. n inputs, 1 output.) */
void InvFn::inv(float range_point, float* domain_out) {
	const float* fo;
	ship_assert(no == 1);
	fo = domain_from_range(&range_point);
	for (u32 e = 0; e < ni; ++e)
		domain_out[e] = fo[e];
}

/* Helper: give a random point in our sphere. */
void InvFn::generate_random_pt(float* pt) const {
	u32 e;
	float l = 0., ll;
	for (e = 0; e < ni; ++e) {
		pt[e] = RandFloat(0., 1.);
		l += (pt[e] * pt[e]);
	}
	l = sqrt(l);
	if (0. == l)
		return;
	ll = RandFloat(0., radius);
	for (e = 0; e < ni; ++e) {
		pt[e] /= l;
		pt[e] *= ll;
	}
}

/* end invfn.cpp */