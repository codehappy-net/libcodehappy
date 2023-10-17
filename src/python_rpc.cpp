/***

	python_rpc.cpp

	Functions for calling Python scripts, etc.  Includes client
	interface for LWGAN models.

	Copyright (c) 2022 Chris Street.

***/
#include <libcodehappy.h>

std::string python_cmd = "python";

/* Make a system command call, but suppress any child window (under Windows.) */
void system_call(const char* cmd) {
#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_WINDOWS)
	/* Native Windows build. We can use WinExec(). */
	// WinExec() is non-blocking, which may be a problem if we have to wait for output.
	WinExec(cmd, SW_HIDE);
#else
#ifdef CODEHAPPY_NATIVE
	/* Not native Windows build. */
	system(cmd);
#else
	/* Not a native build at all. */
	assert(false);
#endif
#endif
}

void PythonScript::add_option(const char* opt, const char* val) {
	std::string o = opt;
	std::string v = val;
	add_option(o, v);
}

void PythonScript::add_option(const std::string& opt, const std::string& val) {
	args[opt] = val;
}

void PythonScript::remove_option(const char* opt) {
	std::string o = opt;
	remove_option(o);
}

void PythonScript::remove_option(const std::string& opt) {
	const auto& i = args.find(opt);
	if (i == args.end())
		return;
	args[opt] = "";
}

void PythonScript::clear_options() {
	args.clear();
}

void PythonScript::run_script() const {
	/* Python's sys.exit() terminates parent processes recursively, apparently. So let's
           use a server/client architecture, passing job paramaters in JSON. Saves us the trouble
           of loading the checkpoint each run anyway. */
#ifdef OLDWAY
	std::string cmd;
	cmd = python_cmd + " " + lwgan_eval_loc + "/eval.py ";
	for (const auto& e : args) {
		if (e.second.empty())
			continue;
		cmd += " ";
		cmd += "--";
		cmd += e.first;
		cmd += " ";
		cmd += e.second;
	}
	codehappy_cout << "Running Python script with call:\n\t" << cmd << std::endl;
	system_call(cmd.c_str());
#endif
	std::string out_path, out_file = "command.json";
	make_pathname(lwgan_eval_loc, out_file, out_path);
	regularize_slashes((char *)out_path.c_str());
	std::ofstream o;
	bool comma = false;
	o.open(out_path.c_str());
	o << "{";
	for (const auto& e : args) {
		if (e.second.empty())
			continue;
		if (comma)
			o << ", ";
		o << "\"" << e.first << "\": \"" << e.second << "\"";
		comma = true;
	}
	o << "}\n";
	o.close();
	o.clear();
}

std::string lwgan_root_loc = "c:/ml/lwgan";
std::string lwgan_eval_loc = "c:/ml/lwgen_cpu";

void lv_zero(LatentVector& lv, u32 N) {
	lv.clear();
	for (u32 i = 0; i < N; ++i)
		lv.push_back(0.0);
}

void lv_rand(LatentVector& lv, u32 N) {
	lv.clear();
	for (u32 i = 0; i < N; ++i)
		lv.push_back(rand_gaussian(0.0, 1.0));
}

void lv_mul(LatentVector& lv, double m) {
	for (u32 i = 0; i < lv.size(); ++i)
		lv[i] *= m;
}

void lv_add(const LatentVector& lv1, LatentVector& lv2_dest) {
	assert(lv1.size() == lv2_dest.size());
	for (u32 i = 0; i < lv2_dest.size(); ++i)
		lv2_dest[i] += lv1[i];
}

void lv_sub(LatentVector& lv1_dest, const LatentVector& lv2) {
	assert(lv1_dest.size() == lv2.size());
	for (u32 i = 0; i < lv1_dest.size(); ++i)
		lv1_dest[i] += lv2[i];
}

void lv_basis_vector(LatentVector& lv_out, u32 N, u32 axis, double magnitude) {
	assert(axis < N);
	lv_zero(lv_out, N);
	lv_out[axis] = magnitude;
}

void lv_norm(const LatentVector& lv, LatentVector& lv_out) {
	double m = lv_mag(lv);
	if (0. == m) {
		lv_zero(lv_out, lv.size());
		if (lv.size() > 0)
			lv_out[0] = 1.0;
		return;
	}
	lv_out = lv;
	lv_mul(lv_out, 1.0 / m);
}

double lv_mag(const LatentVector& lv) {
	double m = 0.0;
	for (int e = 0; e < lv.size(); ++e)
		m += lv[e] * lv[e];
	m = sqrt(m);
	return m;
}

void lv_slerp(const LatentVector& v1, const LatentVector& v2, LatentVector& lv_out, double t) {
	spherical_interpolate(v1, v2, t, lv_out);
}

void spherical_interpolate(const LatentVector& v1, const LatentVector& v2, double t, LatentVector& lv) {
	LatentVector n1, n2;
	double dotp = 0.0, omega, den, w1, w2;

	lv_norm(v1, n1);
	lv_norm(v2, n2);
	for (int e = 0; e < n1.size(); ++e)
		dotp += (n1[e] * n2[e]);
	omega = acos(dotp);
	den = sin(omega);
	w1 = sin((1 - t) * omega) / den;
	w2 = sin(t * omega) / den;
	lv = v1;
	lv_mul(lv, w1);
	n2 = v2;
	lv_mul(n2, w2);
	lv_add(n2, lv);
}

LWGAN::LWGAN(const char* model_name, u32 latent_dim) {
	mname = model_name;
	ldim = latent_dim;
	ps = nullptr;
	chkpt = -1;
	oaat = false;
	use_ema = false;
	ah = 1;
	aw = 1;
	nacc = 0;
	sync_checkpoint_model();
	set_seed(123456789);
	accumulator_clear();
}

LWGAN::LWGAN(const std::string& model_name, u32 latent_dim) {
	mname = model_name;
	ldim = latent_dim;
	ps = nullptr;
	chkpt = -1;
	oaat = false;
	use_ema = false;
	ah = 1;
	aw = 1;
	nacc = 0;
	sync_checkpoint_model();
	set_seed(123456789);
	accumulator_clear();
}

LWGAN::LWGAN() {
	ps = nullptr;
	chkpt = -1;
	oaat = false;
	use_ema = false;
	ah = 1;
	aw = 1;
	nacc = 0;
	sync_checkpoint_model();
	set_seed(123456789);
	accumulator_clear();
}

LWGAN::~LWGAN() {
	if (ps != nullptr)
		delete ps;
	ps = nullptr;
}

void LWGAN::random_latent(LatentVector& z) const {
	z.clear();
	for (u32 i = 0; i < ldim; ++i)
		z.push_back(rand_gaussian(0.0, 1.0));
}

void LWGAN::random_latent(std::vector<LatentVector>& lvs, u32 N) const {
	lvs.clear();
	for (u32 i = 0; i < N; ++i) {
		LatentVector z;
		random_latent(z);
		lvs.push_back(z);
	}
}

static bool sort_by_sort(const LatentVecGANImgOrder& lvg1, const LatentVecGANImgOrder& lvg2) {
	return lvg1.idx_sort < lvg2.idx_sort;
}

static bool sort_by_idx(const LatentVecGANImgOrder& lvg1, const LatentVecGANImgOrder& lvg2) {
	return lvg1.idx_orig < lvg2.idx_orig;
}

u32 LWGAN::guess_img_size() const {
	if (mname == "bbmodel")
		return 256;
	if (mname == "cartoon256")
		return 256;
	return 512;
}

/* Generate images corresponding to the passed latent vectors. The value returned is the size (each side; the LWGAN output is square) of the images. */
u32 LWGAN::generate(const std::vector<LatentVector>& lvs, std::vector<GANImg>& data, bool use_buffer_lv) {
	u32 idx = 0, togen = lvs.size(), retval = guess_img_size(), genper = MAX_BATCH_GEN;
	if (0 == togen) {
		if (data.size() > 0)
			return data[0].bmp->width();
		return retval;
	}
	std::vector<LatentVecGANImgOrder> imgs(lvs.size());
	for (u32 e = 0; e < lvs.size(); ++e) {
		imgs[e].idx_orig = e;
		imgs[e].idx_sort = RandU32();
		imgs[e].lv = lvs[e];
	}
	/* Sort in random order: this way, long interpolations (e.g.) don't result in batches of
	   100 images all close to each other in latent space; there will be 100 points
	   from throughout the interpolation in each batch. */
	std::sort(imgs.begin(), imgs.end(), sort_by_sort);

	/* Keep the individual batches roughly in equal size. */
	if (togen > MAX_BATCH_GEN) {
		u32 nbatch = togen / MAX_BATCH_GEN;
		if (nbatch * MAX_BATCH_GEN < togen)
			nbatch++;
		genper = (togen / nbatch);
		if (genper * nbatch < togen)
			++genper;
		genper = std::min(genper, u32(MAX_BATCH_GEN));
	}

	while (togen > genper) {
		retval = generate_batch(imgs, idx, genper, use_buffer_lv);
		idx += genper;
		togen -= genper;
	}
	if (togen > 0)
		retval = generate_batch(imgs, idx, togen, use_buffer_lv);
	std::sort(imgs.begin(), imgs.end(), sort_by_idx);
	assert(imgs.size() == lvs.size());
	for (u32 e = 0; e < lvs.size(); ++e) {
		data.push_back(imgs[e].gi);
	}
	return retval;
}

/* Generate N images based on random latent space vectors. The value returned is the size of each images' sides. */
u32 LWGAN::generate(u32 N, std::vector<GANImg>& data, bool use_buffer_lv) {
	std::vector<LatentVector> lvs;
	random_latent(lvs, N);
	return generate(lvs, data, use_buffer_lv);
}

/* The main generator function, creates a batch of up to MAX_BATCH_GEN generated LWGAN images. */
u32 LWGAN::generate_batch(std::vector<LatentVecGANImgOrder>& lvs, u32 idx, u32 ngen, bool use_buffer_lv) {
	u32 e;
	u32 no;
	std::string out_path, output_name = "output.png", input_name, tl = "tlist.txt";
	make_pathname(lwgan_eval_loc, output_name, out_path);
	regularize_slashes((char *)out_path.c_str());
	if (FileExists(out_path.c_str()))
		remove(out_path.c_str());

	/* Set up the input tensor list. */
	std::ofstream o;
	make_pathname(lwgan_eval_loc, tl, input_name);
	regularize_slashes((char *)input_name.c_str());
	o.open(input_name);
	o << "[";
	no = 0;
	for (e = idx; e < lvs.size() && e < idx + ngen; ++e) {
		bool comma = false;
		assert(lvs[e].lv.size() == ldim);
		o << "[";
		for (auto& f : lvs[e].lv) {
			if (comma)
				o << ",";
			o << f;
			comma = true;
		}
		o << "]";
		if (use_buffer_lv || e + 1 < idx + ngen)
			o << ",";
		o << "\n";
		++no;
	}

	if (use_buffer_lv) {
		/* we request the buffer latent vectors, as well. */
		for (e = 0; e < NUM_BUFFER_LATENTS; ++e) {
			bool comma = false;
			assert(buf_lat[e].size() == ldim);
			o << "[";
			for (auto& f : buf_lat[e]) {
				if (comma)
					o << ",";
				o << f;
				comma = true;
			}
			o << "]";
			if (e + 1 < NUM_BUFFER_LATENTS)
				o << ",";
			o << "\n";
			++no;
		}
	}
	o << "]\n";
	o.close();
	o.clear();

	/* set up script args */
	validate_script();
	ps->add_option("tensor_list", tl);

	/* have the script create the output image */
	ps->run_script();

	/* divide the image into each individual generated image. */
	SBitmap* bigbmp;
	Stopwatch sw;
	u32 grid_size, ret;
	sw.start(200, UNIT_MILLISECOND);
	while (!FileExists(out_path.c_str())) {
		sw.sleep();
		sw.restart();
	}
	int minsize = std::min(50000 * int(no), 1000000);
        while (flength(out_path.c_str()) < minsize) {
		sw.sleep();
		sw.restart();
	}
	sw.start(300, UNIT_MILLISECOND);
	sw.sleep();
	bigbmp = SBitmap::load_bmp(out_path.c_str());
	bigbmp->fill_alpha_channel(ALPHA_OPAQUE);
	// bigbmp->save_bmp("bigbmp.png");
	grid_size = isqrt(no);
	if (grid_size * grid_size < no)
		++grid_size;
	ret = bigbmp->width() / grid_size;	/* zero padding between images in the grid */
	codehappy_cout << ret << " image size, " << no << " total outputs including buffer latent vectors.\n";
	assert(ispow2(ret));
	for (e = 0; e < grid_size; ++e) {
		for (u32 f = 0; f < grid_size; ++f) {
			SBitmap* bmp;
			u32 ix = (e * grid_size) + f + idx;
			if (ix >= lvs.size())
				break;	// we don't return the buffer latent results.
			bmp = new SBitmap(ret, ret);
			bmp->clear();	/* for opaque alpha layer */
			bigbmp->blit(ret * f, ret * e, ret * (f + 1) - 1, ret * (e + 1) - 1, bmp, 0, 0);
			if (ah != aw) {
				bmp->aspect_ratio_replace(aw, ah);
			}
			lvs[ix].gi.bmp = bmp;
			lvs[ix].gi.lv = lvs[ix].lv;
			lvs[ix].gi.seed = seed;
			lvs[ix].gi.checkpoint = chkpt;
			lvs[ix].gi.model = mname;
		}
	}
	delete bigbmp;

	return ret;
}

void LWGAN::set_checkpoint(int checkpoint) {
	char str[16];
	if (chkpt == checkpoint)
		return;	// no op
	chkpt = checkpoint;
	sprintf(str, "%d", chkpt);
	validate_script();
	ps->add_option("checkpoint", str);
	ps->run_script();
	Stopwatch sw(5, StopwatchUnits::UNIT_SECOND);
	sw.sleep();
	// resync to verify the checkpoint number.
	sync_checkpoint_model();
}

void LWGAN::set_seed(u32 new_seed) {
	/* Turns out we don't need to reset the server seed to anything (unless we use a feature that
           calls torch.randn()), but we do need to generate a bunch of latent vectors to pad out generation 
           batches and ensure that features are used. So we must record the seed used for generating these 
           latent vectors. */
	seed = new_seed;
	generate_buffer_latents();
}

void LWGAN::validate_script() {
	/* Verifies that the PythonScript is created and has the correct basic options set. */
	if (is_null(ps)) {
		ps = new PythonScript(lwgan_eval_loc);
	}

	ps->clear_options();
	ps->add_option("name", mname.c_str());
	ps->add_option("rootdir", lwgan_root_loc.c_str());
	if (oaat)
		ps->add_option("oaat", "1");
	if (use_ema)
		ps->add_option("use_ma", "1");
}

void LWGAN::set_oaat(bool v) {
	if (oaat != v) {
		oaat = v;
		codehappy_cout << "Generating images " << (oaat ? "in batches.\n" : "one at a time.\n");
	}
}

void LWGAN::set_ema(bool v) {
	if (use_ema != v) {
		use_ema = v;
		codehappy_cout << "Using " << (use_ema ? "exponentially smoothed" : "original") << " weights in model.\n";
	}
}

u32 LWGAN::interpolate(const LatentVector& v1, const LatentVector& v2, u32 N, std::vector<GANImg>& data, bool use_buffer_lv) {
	std::vector<LatentVector> v;
	interpolate(v1, v2, N, v);
	return generate(v, data, use_buffer_lv);
}

void LWGAN::interpolate(const LatentVector& v1, const LatentVector& v2, u32 N, std::vector<LatentVector>& lv_out) {
	assert(N > 0);
	if (1 == N) {
		LatentVector half = v1;
		lv_add(v2, half);
		lv_mul(half, 0.5);
		lv_out.push_back(half);
	} else {
		for (u32 e = 0; e < N; ++e) {
			LatentVector c1, c2;
			double wt;
			wt = double(e) / double(N - 1);
			c1 = v1;
			c2 = v2;
			lv_mul(c1, (1.0 - wt));
			lv_mul(c2, wt);
			lv_add(c1, c2);
			lv_out.push_back(c2);
		}
	}
}

SBitmap* save_image_grid(const char* fname, std::vector<GANImg>& imgs, bool number_grid) {
	SBitmap* bmp;
	u32 gs = isqrt(imgs.size());
	if (gs * gs < imgs.size())
		gs++;
	if (0 == gs)
		return nullptr;
	bmp = new SBitmap(imgs[0].bmp->width() * gs, imgs[0].bmp->height() * gs);
	bmp->clear();	/* need to set the alpha channel for PNG save. */
	u32 x = 0, y = 0;
	u32 e;
	for (e = 0; e < imgs.size(); ++e) {
		imgs[e].bmp->blit(bmp, x * imgs[e].bmp->width(), y * imgs[e].bmp->height());
		if (number_grid) {
			char str[8];
			SCoord pos(x * imgs[e].bmp->width(), y * imgs[e].bmp->height(),
			           (x + 1) * imgs[e].bmp->width() - 1, (y + 1) * imgs[e].bmp->height() - 1);
			sprintf(str, "%d", e);
			bmp->render_text(str, pos, &font_swansea_bold, C_WHITE, 64, ALIGN_LOWER_LEFT);
		}
		++x;
		if (gs == x) {
			x = 0;
			++y;
		}
	}
	if (fname != nullptr) {
		bmp->save_bmp(fname);
		delete bmp;
		bmp = nullptr;
	}
	return bmp;
}

/* Free the data in a vector of GANImgs. */
void free_ganimgs(std::vector<GANImg>& imgs) {
	for (auto& gi : imgs) {
		delete gi.bmp;
		gi.bmp = nullptr;
	}
	imgs.clear();
}

/* Interpolate N images along the specified latent basis vector, from min_magnitude to max_magnitude. */
u32 LWGAN::interpolate_basis(u32 axis, u32 N, double min_mag, double max_mag, std::vector<GANImg>& data) {
	LatentVector lv1, lv2;
	lv_basis_vector(lv1, ldim, axis, min_mag);
	lv_basis_vector(lv2, ldim, axis, max_mag);
	return interpolate(lv1, lv2, N, data, false);
}

void LWGAN::interpolate_basis(u32 axis, u32 N, double min_mag, double max_mag, std::vector<LatentVector>& lv_out) {
	LatentVector lv1, lv2;
	lv_basis_vector(lv1, ldim, axis, min_mag);
	lv_basis_vector(lv2, ldim, axis, max_mag);
	interpolate(lv1, lv2, N, lv_out);
}

void LWGAN::accumulator_clear() {
	lv_zero(acc, ldim);
	nacc = 0;
}

void LWGAN::accumulator_add(const LatentVector& lv) {
	lv_add(lv, acc);
	nacc++;
}

void LWGAN::accumulator_sub(const LatentVector& lv) {
	lv_sub(acc, lv);
}

void LWGAN::accumulator_neg() {
	for (u32 e = 0; e < acc.size(); ++e)
		acc[e] = -acc[e];
}

void LWGAN::accumulator_mul(double mlplcand) {
	for (u32 e = 0; e < acc.size(); ++e)
		acc[e] *= mlplcand;
}

void LWGAN::accumulator_ret(LatentVector& lv_out) {
	lv_out = acc;
}

void LWGAN::accumulator_avg(LatentVector& lv_out) {
	if (0 == nacc) {
		lv_zero(lv_out, ldim);
		return;
	}
	for (u32 e = 0; e < acc.size(); ++e)
		lv_out[e] = acc[e] / double(nacc);
}

u32 LWGAN::number_lvs_accumulated() const {
	return nacc;
}

void LWGAN::generate_buffer_latents() {
	/* Create a new ISAAC context with our seed each time for deterministic results. */
	DetRand rng(seed);

	buf_lat.clear();
	for (u32 e = 0; e < NUM_BUFFER_LATENTS; ++e) {
		LatentVector lv;
		for (u32 f = 0; f < ldim; ++f) {
			lv.push_back(rng.normald());
		}
		buf_lat.push_back(lv);
	}
}

void LWGAN::sync_checkpoint_model() {
	/* Request current model, checkpoint, and latent dimensionality from the server. */
	validate_script();
	ps->add_option("sync", "1");
	ps->run_script();
	Stopwatch sw(1, StopwatchUnits::UNIT_SECOND);
	sw.sleep();
	std::ifstream i;
	std::string out_path, out_file = "heartbeat.txt";
	make_pathname(lwgan_eval_loc, out_file, out_path);
	if (!FileExists(out_path.c_str())) {
		codehappy_cerr << "server not running?\n";
		exit(1);
	}
	i.open(out_path);
	i >> chkpt;
	i >> ldim;
	i >> mname;
	i.close();
	i.clear();
	codehappy_cout << "Heartbeat received from server running model '" << mname << "' on checkpoint " << chkpt << " with latent dimension " << ldim << ".\n";
}

/*** end rpc.cpp ***/