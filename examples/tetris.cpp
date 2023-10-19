/***

	tetris.cpp
	
	An example game: good ol' Tetris. Build as native + SDL or as WebAssembly.

	WebAssembly demo at:
	
	https://www.codehappy.net/tetris/

	2023, Chris Street
	
***/
#define CODEHAPPY_SDL
#include <libcodehappy.h>

#define APP_WIDTH	800
#define APP_HEIGHT	800

// Tetromino shapes and dimensions
int tetrominos[][4][4][4] = {
	// straight piece ('I')
	{
	 {
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 }
	 },
 	 { 
	  { 1, 1, 1, 1 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 }
	 },
	 {
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 }
	 },
	 {
	  { 1, 1, 1, 1 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 }
	 },
	},

	// square piece ('O')
	{
	 {
	  { 1, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	},
	// T piece
	{
	 {
	  { 1, 1, 1, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 1, 0, 0 },
	  { 1, 1, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 0, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	},
	// L piece
	{
	 {
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 1, 0 },
	  { 1, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 0, 1, 0 },
	  { 1, 1, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	},
	// J piece
	{
	 {
	  { 0, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 0, 0, 0 },
	  { 1, 1, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 1, 0 },
	  { 0, 0, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	},
	// S piece
	{
	 {
	  { 0, 1, 1, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 0, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 1, 1, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 0, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 0, 1, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	},
	// Z piece
	{
	 {
	  { 1, 1, 0, 0 },
	  { 0, 1, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 1, 1, 0, 0 },
	  { 0, 1, 1, 0 },
	  { 0, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	 {
	  { 0, 1, 0, 0 },
	  { 1, 1, 0, 0 },
	  { 1, 0, 0, 0 },
	  { 0, 0, 0, 0 },
	 },
	}
};

int tetromino_shapes[][2] = {
	// straight piece ('I')
	{ 1, 4 },
	// square piece ('O')
	{ 2, 2 },
	// T piece
	{ 3, 2 },
	// L piece
	{ 2, 3 },
	// J piece
	{ 2, 3 },
	// S piece
	{ 3, 2 },
	// Z piece
	{ 3, 2 },
};

RGBColor tetromino_colors[] = {
	C_BLUE, C_BRIGHT_GREEN, C_RED, C_WHITE, C_YELLOW, H_CHOCOLATE, H_PURPLE, H_ORANGE
};

const int num_tetrominos = 7;
const int num_colrs = 8;

// dimensions of the game board
const int board_x = 10;
const int board_y = 20;

struct Tetromino {
	Tetromino();
	void make_new();
	bool solid_at(int x, int y) const;
	int typ;
	int rot;
	RGBColor colr;
};

bool in_range(int x, int y) {
	return (x >= 0 && x < 4 && y >= 0 && y < 4);
}

Tetromino::Tetromino() {
	make_new();
}

bool Tetromino::solid_at(int x, int y) const {
	int yd;
	yd = tetromino_shapes[typ][(rot == 1 || rot == 3) ? 0 : 1];
	y -= (4 - yd);

	if (!in_range(x, y))
		return false;

	return tetrominos[typ][rot][y][x] != 0;
}

void Tetromino::make_new() {
	typ = RandU32Range(0, num_tetrominos - 1);
	rot = RandU32Range(0, 3);
	colr = tetromino_colors[RandU32Range(0, num_colrs - 1)];
}

const int min_speed = 12;

class Board {
public:
	Board();

	// Start a new game.
	void new_game();

	// Advance a single frame; returns true iff the game is over.
	bool advance();

	// (Attempt to) rotate the current Tetromino.
	void rotate(int dr);

	// (Attempt to) move the current Tetromino left or right.
	void move(int dx);

	// (Attempt to) hurry the piece downward at the user's behest.
	void hurry();

	// Color of the board at position x, y -- C_BLACK if no piece is there.
	RGBColor board_color(int x, int y, bool inc_cur = true) const;

	// Color of the next tetromino at x, y (for drawing the next piece)
	RGBColor next_color(int x, int y) const;

	int speed_index() const	{ return min_speed - speed; }

	// Game properties that we want accessible.
	int speed;
	int lines_completed;
	int score;

private:
	// Is user input OK? (i.e. we're not animating, the game is not over?)
	bool user_ok() const;

	// Drop the current Tetromino one row; returns true iff the game is over.
	bool advance_row();

	// Place the current piece at the top of the board.
	void place_piece_top();

	// Check for completed rows, and begin the row-clearing animation if they exist.
	void check_completed_rows();

	// Are we currently animating row clearing?
	bool animating_row_clear() const;

	// pieces placed on the board
	RGBColor board[board_x][board_y];
	// Tetrominos and current position
	Tetromino cur;
	Tetromino next;
	int cur_x, cur_y;
	bool game_over;
	// when a row is being cleared, track the row animation.
	int row_1st;
	bool row_state[board_y];
	int row_animation;
	int frame;
};

Board::Board() {
	new_game();
}

void Board::new_game() {
	game_over = false;
	cur.make_new();
	next.make_new();
	place_piece_top();
	row_1st = -1;
	row_animation = 0;
	frame = 0;
	speed = min_speed;
	lines_completed = 0;
	score = 0;
	for (int e = 0; e < board_x; ++e)
		for (int f = 0; f < board_y; ++f)
			board[e][f] = C_BLACK;
}

bool Board::advance() {
	++frame;
	if (animating_row_clear() && (frame & 4) == 0) {
		++row_animation;
		if (!animating_row_clear()) {
			// now actually clear the lines
			int row, row2, col;
			for (row = 0; row < board_y; ++row) {
				if (!row_state[row])
					continue;
				for (col = 0; col < board_x; ++col)
					board[col][row] = C_BLACK;
			}
			for (row = board_y - 1; row >= 0; --row) {
				int e, f;
				if (!row_state[row])
					continue;
				for (row2 = row; row2 > 0; --row2) {
					row_state[row2] = row_state[row2 - 1];
					for (col = 0; col < board_x; ++col)
						board[col][row2] = board[col][row2 - 1];
				}
				for (col = 0; col < board_x; ++col)
					board[col][0] = C_BLACK;
				row_state[0] = false;
				++row;
			}
		}
	}
	if (!animating_row_clear() && (frame % speed) == 0) {
		return advance_row();
	}
	return game_over;
}

bool Board::advance_row() {
	int x, y;
	if (game_over)
		return true;
	ship_assert(!animating_row_clear());

	bool ok = true;
	++cur_y;
	if (cur_y + 3 >= board_y)
		ok = false;
	for (x = 0; x < 4 && ok; ++x)
		for (y = 0; y < 4 && ok; ++y) {
			if (cur.solid_at(x, y) && board_color(cur_x + x, cur_y + y, false) != C_BLACK)
				ok = false;
		}
	if (!ok) {
		// place the piece, then check for completed row(s)
		--cur_y;
		for (x = 0; x < 4; ++x) {
			for (y = 0; y < 4; ++y) {
				if (cur.solid_at(x, y)) {
					if (cur_y + y < 0) {
						// the piece extends beyond the top of the board, this is game over.
						game_over = true;
					} else {
						if (cur_x + x >= 0 && cur_x + x < board_x && cur_y + y < board_y)
							board[cur_x + x][cur_y + y] = cur.colr;
					}
				}
			}
		}
		check_completed_rows();
		// place the next Tetronimo at the top of the board.
		cur = next;
		place_piece_top();
		next.make_new();
	}

	return game_over;
}

RGBColor Board::board_color(int x, int y, bool inc_cur) const {
	if (x < 0 || x >= board_x || y < 0 || y >= board_y)
		return C_BLACK;

	if (animating_row_clear() && row_state[y]) {
		if (x >= (board_x / 2) - row_animation && x <= (board_x / 2) + row_animation)
			return C_BLACK;
	}

	if (board[x][y] != C_BLACK)
		return board[x][y];

	if (inc_cur && cur.solid_at(x - cur_x, y - cur_y))
		return cur.colr;

	return C_BLACK;
}

RGBColor Board::next_color(int x, int y) const {
	if (next.solid_at(x, y))
		return next.colr;
	return C_BLACK;
}

void Board::check_completed_rows() {
	int e, f, lc;
	row_1st = -1;
	row_animation = 0;
	lc = 0;
	for (e = 0; e < board_y; ++e)
		row_state[e] = true;
	for (e = 0; e < board_y; ++e)
		for (f = 0; row_state[e] && f < board_x; ++f) {
			if (board[f][e] == C_BLACK)
				row_state[e] = false;
		}
	for (e = 0; e < board_y; ++e)
		if (row_state[e]) {
			lines_completed++;
			lc++;
		}
	for (e = 0; e < board_y; ++e)
		if (row_state[e]) {
			row_1st = e;
			if (speed > 2 && OneIn(36 / speed)) {
				// increase game speed
				--speed;
			}
			break;
		}
	// scoring, based on the number of rows cleared at a time and the current game speed
	switch (lc) {
	case 1:
		score += 100 * (speed_index() + 1);
		break;
	case 2:
		score += 300 * (speed_index() + 1);
		break;
	case 3:
		score += 500 * (speed_index() + 1);
		break;
	case 4:
		score += 800 * (speed_index() + 1);
		break;
	}
}

void Board::place_piece_top() {
	forever {
		int x, y;
		bool ok = true;
		cur_y = -3;
		cur_x = RandU32Range(0, board_x - 1);
		for (x = 0; ok && x < 4; ++x)
			for (y = 0; ok && y < 4; ++y) {
				if (cur.solid_at(x, y) && (cur_x + x >= board_x))
					ok = false;
			}
		if (ok)
			break;
	}	
}

void Board::rotate(int dr) {
	int x, y;
	if (!user_ok())
		return;
	cur.rot += dr;
	if (cur.rot > 3)
		cur.rot -= 4;
	if (cur.rot < 0)
		cur.rot += 4;
	bool ok = true;
	for (x = 0; ok && x < 4; ++x)
		for (y = 0; ok && y < 4; ++y) {
			if (cur.solid_at(x, y) && board_color(cur_x + x, cur_y + y, false) != C_BLACK) {
				// can't rotate, there's a piece in the way
				ok = false;
			}
			if (cur.solid_at(x, y) && (cur_x + x < 0 || cur_x + x >= board_x || cur_y + y >= board_y)) {
				// can't rotate, the piece goes off the board.
				ok = false;
			}
		}

	if (!ok) {
		cur.rot -= dr;
		if (cur.rot > 3)
			cur.rot -= 4;
		if (cur.rot < 0)
			cur.rot += 4;
	}
}

void Board::hurry() {
	if (!user_ok())
		return;
	advance_row();
}

void Board::move(int dx) {
	int x, y;
	bool ok = true;

	if (!user_ok())
		return;
	cur_x += dx;
	for (x = 0; ok && x < 4; ++x)
		for (y = 0; ok && y < 4; ++y) {
			if (cur.solid_at(x, y) && (cur_x + x < 0 || cur_x + x >= board_x))
				ok = false;
			if (cur.solid_at(x, y) && board_color(cur_x + x, cur_y + y, false) != C_BLACK)
				ok = false;
		}
	if (!ok)
		cur_x -= dx;
}

bool Board::animating_row_clear() const {
	if (row_1st >= 0 && row_animation + row_animation <= board_x)
		return true;
	return false;
}

bool Board::user_ok() const {
	if (game_over || animating_row_clear())
		return false;
	return true;
}

void draw_block(SBitmap* bmp, RGBColor clr, int x, int y) {
	if (C_BLACK == clr) {
		bmp->rect_fill(x, y, x + 35, y + 35, C_BLACK);
		return;
	}
	bmp->rect(x, y, x + 35, y + 35, C_GREY);
	bmp->rect_fill(x + 1, y + 1, x + 34, y + 34, clr);
	bmp->rect(x + 4, y + 4, x + 31, y + 31, C_BLACK);
}

void main_loop(Display* display, void* user_data) {
	static KeyLast kl(display);
	static const char* midis[] = {
		"assets-tetris/tetris-a.mid", "assets-tetris/tetris-b.mid", "assets-tetris/tetris-b.mid",
	};
	static int midi_idx = 0;
	static Board game_board;
	const bool quiet = *((bool *) user_data);
	int x, y;
	bool go;

	if (kl.first()) {
		codehappy_window_title("Tetris!");
		if (!quiet)
			play_midi(midis[0], "assets-tetris/sfnt.sf2");
	}
	
	if (!quiet && !midi_playing()) {
		midi_idx++;
		if (midi_idx >= 3)
			midi_idx = 0;
		play_midi(midis[midi_idx], (tsf *)nullptr);
	}

	// Advance the game one frame
	go = game_board.advance();

	/** Keyboard controls ***/
	// Left arrow (or 'F'): attempt to move current piece left
	if (display->key_down(SKEY_LEFT_ARROW) || display->key_down('f') || display->key_down('F')) {
		game_board.move(-1);
	}
	// Right arrow (or 'G'): attempt to move current piece right
	if (display->key_down(SKEY_RIGHT_ARROW) || display->key_down('g') || display->key_down('G')) {
		game_board.move(1);
	}
	// Up arrow (or 'T'): attempt to rotate current piece clockwise
	if (kl.now_down(SKEY_UP_ARROW) || display->key_down('T') || display->key_down('t')) {
		game_board.rotate(1);
	}
	// Down arrow (or 'V'): attempt to hurry the piece
	if (display->key_down(SKEY_DOWN_ARROW) || display->key_down('v') || display->key_down('V')) {
		game_board.hurry();
	}
	// Spacebar: attempt to rotate current piece counterclockwise (or begin a new game, if game over)
	if (kl.now_down(' ')) {
		if (go)
			game_board.new_game();
		else
			game_board.rotate(-1);
	}

	// Draw the game board
	display->bitmap()->clear(C_GREY);
	display->bitmap()->rect_fill(SPoint(38, 38), SPoint(402, 762), C_YELLOW);
	for (x = 0; x < board_x; ++x)
		for (y = 0; y < board_y; ++y)
			draw_block(display->bitmap(), (go ? C_GREY : game_board.board_color(x, y)), 40 + 36 * x, 40 + 36 * y);

	// Draw the score and the upcoming piece
	display->bitmap()->rect_fill(SPoint(402, 38), SPoint(762, 762), C_YELLOW);
	display->bitmap()->rect_fill(SPoint(404, 40), SPoint(760, 760), C_BLACK);

	char line[64];
	sprintf(line, "SCORE %07d", game_board.score);
	display->bitmap()->render_text(line, SCoord(440, 60, 700, 100), &font_emulogic, C_WHITE, 18, ALIGN_LEFT | CENTERED_VERT);
	sprintf(line, "LINES %04d", game_board.lines_completed);
	display->bitmap()->render_text(line, SCoord(440, 110, 700, 150), &font_emulogic, C_WHITE, 18, ALIGN_LEFT | CENTERED_VERT);
	sprintf(line, "SPEED %02d", game_board.speed_index());
	display->bitmap()->render_text(line, SCoord(440, 160, 700, 210), &font_emulogic, C_WHITE, 18, ALIGN_LEFT | CENTERED_VERT);

	if (go) {
		display->bitmap()->render_text("GAME OVER", SCoord(440, 400, 700, 440), &font_emulogic, C_YELLOW, 18, ALIGN_LEFT | CENTERED_VERT);
		display->bitmap()->render_text("SPACEBAR TO", SCoord(440, 460, 700, 500), &font_emulogic, C_WHITE, 18, ALIGN_LEFT | CENTERED_VERT);
		display->bitmap()->render_text("PLAY AGAIN", SCoord(440, 520, 700, 560), &font_emulogic, C_WHITE, 18, ALIGN_LEFT | CENTERED_VERT);
	} else {
		display->bitmap()->render_text("NEXT:", SCoord(440, 400, 700, 440), &font_emulogic, C_BLUE, 36, ALIGN_LEFT | CENTERED_VERT);
		for (x = 0; x < 4; ++x)
			for (y = 0; y < 4; ++y)
				draw_block(display->bitmap(), game_board.next_color(x, y), 480 + 36 * x, 480 + 36 * y);
	}

	kl.save(display);
}

int app_main() {
	ArgParse ap;
	bool quiet = false;

	ap.add_argument("quiet", type_none, "suppress music output during game", &quiet);
	ap.ensure_args(argc, argv);

	codehappy_main(main_loop, &quiet, APP_WIDTH, APP_HEIGHT, 24);

	return 0;
}

/*** end tetris.cpp ***/
