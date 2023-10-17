/***

	entropy.cpp

	An entropy encoder/compressor.

	Copyright (c) 2014-2022 C. M. Street

***/
static treenode* new_treenode(void) {
	treenode* node;

	node = NEW(treenode);
	node->lbound = 0UL;
	node->hbound = 0UL;
	node->symbol_idx = 0UL;
	node->left = NULL;
	node->right = NULL;
	
	return(node);
}

static treenode* build_freq_tree(freqsymbol* sym, u32 i1, u32 i2, u32 f1, u32 f2) {
	/***
		If there is no symbol that starts in [f1, f2], return NULL.

		If there is only one symbol that starts in [f1, f2], give a leaf node for that.

		If there are multiple symbols starting in [f1, f2], generate the child nodes recursively.
	***/
	u32 sym_min, sym_max;
	u32 i;
	treenode* tn;

	if (f2 < f1)
		return(NULL);

	sym_min = i2 + 1;
	sym_max = i1;

	// TODO: this could be better implemented as a binary search
	for (i = i1; i <= i2; ++i)
		{
		if (sym[i].start >= f1 && sym[i].start <= f2)
			{
			if (i < sym_min)
				sym_min = i;
			if (i > sym_max)
				sym_max = i;
			}
		}
	if (sym_min > sym_max)
		return(NULL);

	tn = new_treenode();

	tn->lbound = sym[sym_min].start;
	tn->hbound = sym[sym_max].start;

	if (sym_min == sym_max)
		{
		tn->symbol_idx = sym[sym_min].symbol_idx;
		/* leave children NULL */
		return(tn);
		}

	i = (tn->lbound + tn->hbound) >> 1;

	tn->left = build_freq_tree(sym, sym_min, sym_max, tn->lbound, i);
	tn->right = build_freq_tree(sym, sym_min, sym_max, i + 1, tn->hbound);

	return(tn);
}

static void print_tree_coding(const treenode* tree, int start) {
	forever
		{
		if (tree->left == NULL && tree->right == NULL)
			return;
		if (tree->left && start <= tree->left->hbound)
			{
			tree = tree->left;
			printf("0");
			}
		else
			{
			tree = tree->right;
			printf("1");
			if (is_null(tree))
				exit(1);
			}
		}
}

static void print_all_tree_codings(const treenode* tree, freqsymbol* symbs, int nsym) {
	int i;

	for (i = 0; i < nsym; ++i)
		{
		if (symbs[i].freq == 0UL)
			break;
		printf("%03d: ", symbs[i].symbol_idx);
		print_tree_coding(tree, symbs[i].start);
		printf("\n");
		}
}

static void print_tree(const treenode* tn, int lvl) {
	int i;

	if (is_null(tn))
		return;

	for (i = 0; i <= lvl - 1; ++i)
		printf(" ");
	printf("%lu %lu %lu %d\n", (unsigned long)tn->lbound, (unsigned long)tn->hbound, (unsigned long)tn->symbol_idx, lvl);

	print_tree(tn->left, lvl + 1);
	print_tree(tn->right, lvl + 1);
}

static u32 start_from_c(int c, freqsymbol* symbs, u32 ns) {
	u32 i;
	// TODO: should be a lookup table built for this
	for (i = 0; i < ns; ++i) {
		if (symbs[i].symbol_idx == c)
			return(symbs[i].start);
		}
//	check_or_die_msg(0, "error\n");
	exit(1);
	return(0xfffffffful);
}

static void output_entropy_code(int c, freqsymbol* symbs, u32 ns, treenode* tree, bitfile* bf) {
	u32 start = start_from_c(c, symbs, ns);

	forever
		{
		if (is_null(tree->left) && is_null(tree->right))
			return;
		if (not_null(tree->left) && start <= tree->left->hbound)
			{
			tree = tree->left;
			bitfile_writebit(bf, false);
			}
		else
			{
			tree = tree->right;
			bitfile_writebit(bf, true);
//			check_or_die_msg(not_null(tree), "unexpected node\n");
			}
		}
}

static void tree_to_disk(const treenode* node, bitfile* bf) {
	if (is_null(node->left) && is_null(node->right))
		{
		bitfile_writebit(bf, false);
		bitfile_writebyte(bf, node->symbol_idx);
		return;
		}

	bitfile_writebit(bf, true);
	tree_to_disk(node->left, bf);
	tree_to_disk(node->right, bf);
}

static void tree_from_disk(treenode* node, bitfile* bf) {
	int b;
	
	b = bitfile_readbit(bf);
	if (b)
		{
		node->left = new_treenode();
		node->right = new_treenode();
		tree_from_disk(node->left, bf);
		tree_from_disk(node->right, bf);
		}
	else
		{
		node->left = NULL;
		node->right = NULL;
		node->symbol_idx = bitfile_readbyte(bf);
		}
}

static void free_tree(treenode* node) {
	if (not_null(node->left))
		free_tree(node->left);
	if (not_null(node->right))
		free_tree(node->right);
	delete node;
}

static void entropy_encode_1(RamFile* file_in, bitfile* file_out, freqsymbol* symbs, treenode* tree)
{
	bitfile_write32(file_out, file_in->length());
	tree_to_disk(tree, file_out);

	forever {
		int c;

		c = file_in->getc();
		if (c < 0)			/* EOF */
			break;

		output_entropy_code(c, symbs, 256, tree, file_out);
	}
}

static int comp_freqsymbol(const void* v1, const void* v2) {
	const freqsymbol* fs1 = (const freqsymbol*)v1;
	const freqsymbol* fs2 = (const freqsymbol*)v2;

	return (fs2->freq - fs1->freq);
}

static void init_symbols(freqsymbol* sym, u32 ns) {
	int b;
	for (b = 0; b < ns; ++b) {
		sym[b].symbol_idx = b;
		sym[b].freq = 0UL;
		sym[b].start = 0UL;
		sym[b].total = 0UL;
		}
}

static void total_symbols(freqsymbol* sym, u32 ns) {
	int b;
	for (b = 0; b < ns; ++b)
		sym[0].total += sym[b].freq;
	for (b = 1; b < ns; ++b)
		sym[b].total = sym[0].total;
}

static void start_symbols(freqsymbol* sym, u32 ns) {
	int b;
	for (b = 1; b < ns; ++b)
		sym[b].start = sym[b - 1].start + sym[b - 1].freq;
}

freqsymbol* compile_symbol_freq_1(RamFile* rf) {
	int b;
	freqsymbol* sym;
	const u32 ns = 256;

	sym = NEW_ARRAY(freqsymbol, ns);

	init_symbols(sym, ns);

	forever {
		b = rf->getc();
		if (b < 0)
			break;
		sym[b & 0xff].freq++;
	}

	total_symbols(sym, ns);

	qsort(sym, ns, sizeof(freqsymbol), comp_freqsymbol);

	start_symbols(sym, ns);

	rf->rewind();

	return(sym);
}

static void output_symbol_freq(const char* fname, freqsymbol* sym, u32 ns) {
	FILE* f;
	int e;

	f = fopen(fname, "w");
	for (e = 0; e < ns; ++e) {
		fprintf(f, "%d\t%d\t%d\t%d\t%g\n",
			sym[e].symbol_idx,
			sym[e].freq,
			sym[e].start,
			sym[e].total,
			(double)(sym[e].freq) / sym[e].total);
	}
	fclose(f);
}

static void do_entropy_encoding(RamFile* file_in, bitfile* file_out, freqsymbol* sym, u32 ns) {
	treenode* treetop;
	u32 last_symbol;

	/*** eliminate symbols that have 0 frequency ***/
	for (last_symbol = ns - 1; ; --last_symbol) {
		if (sym[last_symbol].freq > 0)
			break;
		if (last_symbol == 0)
			return;
	}

	treetop = build_freq_tree(sym, 0, last_symbol, 0, sym[last_symbol].start);

#if 0
	print_tree(treetop, 0);
	printf("\n");
	print_all_tree_codings(treetop, sym, ns);
	printf("\n");
#endif

	entropy_encode_1(file_in, file_out, sym, treetop);

	free_tree(treetop);
}

static void do_entropy_decoding(bitfile* file_in, RamFile* file_out, u32 file_len) {
	treenode* treetop = new_treenode();
	
	tree_from_disk(treetop, file_in);

	do {
		treenode* node;

		node = treetop;
		until (is_null(node->left) && is_null(node->right))
			{
			int b;

			b = bitfile_readbit(file_in);
//			check_or_die(b >= 0);
			if (b)
				node = node->right;
			else
				node = node->left;
			}

		file_out->putc(node->symbol_idx);
		--file_len;
	} while (file_len > 0);

	free_tree(treetop);
}

void entropy_compress_file(const char* fname_in, const char* fname_out) {
	RamFile rf;
	bitfile bf;
	freqsymbol* sym;

	rf.open(fname_in, RAMFILE_DEFAULT);
	bitfile_open(&bf, fname_out, true);

	sym = compile_symbol_freq_1(&rf);
	do_entropy_encoding(&rf, &bf, sym, 256);

	delete sym;

	rf.close();
	bitfile_close(&bf);
}

/*** Output buffer must be allocated and passed in. Returns TRUE if compression was successful (fits inside
	the output buffer), FALSE otherwise. ***/
bool entropy_compress_membuf(char* buf_in, u32 input_len, char* buf_out, u32 output_len, u32* compress_len) {
	RamFile rf;
	bitfile bf;
	freqsymbol* sym;
	bool ret;

	rf.open_static(buf_in, input_len, RAMFILE_DEFAULT);
	bitfile_open_mem(&bf, buf_out, output_len, true);

	sym = compile_symbol_freq_1(&rf);
	do_entropy_encoding(&rf, &bf, sym, 256);

	delete sym;

	ret = (bf.buf < bf.bufe);

	rf.close();
	bitfile_close(&bf);

	if (not_null(compress_len))
		*compress_len = bf.buf - bf.bufs;

	return(ret);
}

void entropy_decompress_file(const char* fname_in, const char* fname_out) {
	RamFile rf;
	bitfile bf;
	freqsymbol* sym;
	u32 file_len;

	bitfile_open(&bf, fname_in, false);	
	rf.open(fname_out, RAMFILE_DEFAULT);

	file_len = bitfile_read32(&bf, NULL);
	do_entropy_decoding(&bf, &rf, file_len);

	rf.close();
	bitfile_close(&bf);
}

void entropy_decompress_membuf(char* buf_in, u32 input_len, char** buf_out, u32* output_len) {
	RamFile rf;
	bitfile bf;
	u32 file_len;

//	check_or_die(not_null(buf_out) && not_null(output_len));

	bitfile_open_mem(&bf, buf_in, input_len, false);
	file_len = bitfile_read32(&bf, NULL);
	*buf_out = NEW_ARRAY(char, file_len + 1);
	*output_len = file_len;
	
	rf.open_static(*buf_out, file_len, RAMFILE_DEFAULT);

	do_entropy_decoding(&bf, &rf, file_len);

	rf.close();
	bitfile_close(&bf);
}

/*** Slower but much better compression for low-entropy data ***/
bool entropy_compress_file_full(const char* fname_in, const char* fname_out) {
	RamFile rf;
	char* buf[16];
	int i;
	int e;
	u32 threshold;
	u32 new_len;
	char* lastbuf;
	bitfile bf;

	rf.open(fname_in, RAMFILE_DEFAULT);

	threshold = rf.length();
	lastbuf = (char *)rf.buffer();
	for (i = 0; i < 16; ++i)
		{
		buf[i] = NEW_ARRAY(char, threshold + 1);
		if (entropy_compress_membuf(lastbuf, threshold, buf[i], threshold, &new_len))
			threshold = new_len;
		else
			break;
		lastbuf = buf[i];
		}

	if (i == 0)
		{
		delete [] buf[0];
		return false;
		}

	bitfile_open(&bf, fname_out, true);

	bitfile_writebyte(&bf, i - 1);
	for (e = 0; e < threshold; ++e)
		bitfile_writebyte(&bf, buf[i - 1][e]);

	bitfile_close(&bf);

	for (e = 0; e < i; ++e)
		delete [] buf[e];

	return true;
}

bool entropy_decompress_file_full(const char* fname_in, const char* fname_out) {
	u32 icompress;
	u32 file_len;
	RamFile rf;
	bitfile bf;
	int e;
	char *buf;
	u32 out_len;
	FILE* fout;

	rf.open(fname_in, RAMFILE_READONLY);

	icompress = rf.buffer()[0] + 1;
	buf = (char*)rf.buffer() + 1;
	out_len = rf.length() - 1;

	for (e = 0; e < icompress; ++e)
		{
		char *buf_out;
		
		entropy_decompress_membuf(buf, out_len, &buf_out, &out_len);
		
		if (e > 0)
			delete [] buf;

		buf = buf_out;
		}

	rf.close();

	fout = fopen(fname_out, "wb");
	for (e = 0; e < out_len; ++e)
		fputc(buf[e], fout);
	fclose(fout);
	
	return true;
}

/* end entropy.cpp */
