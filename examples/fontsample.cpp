#include "libcodehappy.h"

struct FontSample {
	const char* text;
	ttfont* ptr;
	SBitmap* render;
	u32 font_size;
};

int main(void) {
	FontSample fs[] = {

	{ "20th Century 32pt", &font_20th_century, nullptr, 32 },
	{ "256 Bytes 32pt", &font_256_bytes, nullptr, 32 },
	{ "3-D Gold Rush 32pt", &font_3D_gold_rush, nullptr, 32 },
	{ "3-D Hotdog 32pt", &font_3D_hotdog, nullptr, 32 },
	{ "Abduction 32pt", &font_abduction, nullptr, 32 },
	{ "Aboriginal Sans 32pt", &font_aboriginal_sans, nullptr, 32 },
	{ "Aboriginal Sans Bold 32pt", &font_aboriginal_sans_bold, nullptr, 32 },
	{ "Aboriginal Sans Bold Italic 32pt", &font_aboriginal_sans_bold_italic, nullptr, 32 },
	{ "Aboriginal Sans Italic 32pt", &font_aboriginal_sans_italic, nullptr, 32 },
	{ "Aboriginal Serif 32pt", &font_aboriginal_serif, nullptr, 32 },
	{ "Aboriginal Serif Bold 32pt", &font_aboriginal_serif_bold, nullptr, 32 },
	{ "Aboriginal Serif Bold Italic 32pt", &font_aboriginal_serif_bold_italic, nullptr, 32 },
	{ "Aboriginal Serif Italic 32pt", &font_aboriginal_serif_italic, nullptr, 32 },
	{ "African 32pt", &font_african, nullptr, 32 },
	{ "Aftershock 32pt", &font_aftershock, nullptr, 32 },
	{ "Agate 32pt", &font_agate, nullptr, 32 },
	{ "Agate Bold 32pt", &font_agate_bold, nullptr, 32 },
	{ "Agent Orange 32pt", &font_agent_orange, nullptr, 32 },
	{ "Alba 32pt", &font_alba, nullptr, 32 },
	{ "Alba Super 32pt", &font_alba_super, nullptr, 32 },
	{ "Albatross 32pt", &font_albatross, nullptr, 32 },
	{ "Algerian 32pt", &font_algerian, nullptr, 32 },
	{ "Aliquam 32pt", &font_aliquam, nullptr, 32 },
	{ "Aliquam Italic 32pt", &font_aliquam_italic, nullptr, 32 },
	{ "Aliquam Ultralight 32pt", &font_aliquam_ultralight, nullptr, 32 },
	{ "Aliquam Ultralight Italic 32pt", &font_aliquam_ultralight_italic, nullptr, 32 },
	{ "Allura 32pt", &font_allura, nullptr, 32 },
	{ "Amelia 32pt", &font_amelia, nullptr, 32 },
	{ "American Uncial 32pt", &font_american_uncial, nullptr, 32 },
	{ "Apple ][ 40 Columns 32pt", &font_apple_ii_40, nullptr, 32 },
	{ "Apple ][ 80 Columns 32pt", &font_apple_ii_80, nullptr, 32 },
	{ "Atlas 32pt", &font_atlas, nullptr, 32 },
	{ "Aunt Judy 32pt", &font_aunt_judy, nullptr, 32 },
	{ "Automania 32pt", &font_automania, nullptr, 32 },
	{ "Avalon 32pt", &font_avalon, nullptr, 32 },
	{ "Avalon Bold 32pt", &font_avalon_bold, nullptr, 32 },
	{ "Avalon Bold Italic 32pt", &font_avalon_bold_italic, nullptr, 32 },
	{ "Avalon Italic 32pt", &font_avalon_italic, nullptr, 32 },
	{ "Back to 1982 32pt", &font_backto1982, nullptr, 32 },
	{ "Bahamas 32pt", &font_bahamas, nullptr, 32 },
	{ "Bahamas Bold 32pt", &font_bahamas_bold, nullptr, 32 },
	{ "Bahamas Light 32pt", &font_bahamas_light, nullptr, 32 },
	{ "Bahamas Heavy 32pt", &font_bahamas_heavy, nullptr, 32 },
	{ "Bas Relief 32pt", &font_bas_relief, nullptr, 32 },
	{ "Bazooka 32pt", &font_bazooka, nullptr, 32 },
	{ "Brela 32pt", &font_brela, nullptr, 32 },
	{ "Buckingham 32pt", &font_buckingham, nullptr, 32 },
	{ "Caesar 32pt", &font_caesar, nullptr, 32 },
	{ "Cairo 32pt", &font_cairo, nullptr, 32 },
	{ "CheddarCake Factory 32pt", &font_cheddarcake, nullptr, 32}, 
	{ "Chopin Script 48pt", &font_chopin_script, nullptr, 48 },
	{ "Cloister Black 48pt", &font_cloister_black, nullptr, 48 },
	{ "Comic Sans MS 32pt", &font_comic_sans_ms, nullptr, 32 },
	{ "Comic Sans MS Bold Italic 32pt", &font_comic_sans_ms_bold_italic, nullptr, 32 },
	{ "Comic Sans MS Italic 32pt", &font_comic_sans_ms_italic, nullptr, 32 },
	{ "Commodore PET 32pt", &font_commodore_pet, nullptr, 32 },
	{ "Commodore VIC=20 32pt", &font_vic20, nullptr, 32 },
	{ "Constructium 32pt", &font_constructium, nullptr, 32 },
	{ "Double Feature 32pt", &font_double_feature, nullptr, 32 },
	{ "Edit Undo Line BRK 32pt", &font_edit_undo_line_brk, nullptr, 32 },
	{ "Emulogic 32pt", &font_emulogic, nullptr, 32 },
	{ "Eurostile 32pt", &font_eurostile, nullptr, 32 },
	{ "Fairfax 32pt", &font_fairfax, nullptr, 32 },
	{ "Fairfax Bold 32pt", &font_fairfax_bold, nullptr, 32 },
	{ "Fairfax Italic 32pt", &font_fairfax_italic, nullptr, 32 },
	{ "Fairfax Serif 32pt", &font_fairfax_serif, nullptr, 32 },
	{ "FHWA (Highway Gothic) Series B 32pt", &font_fhwa_series_b, nullptr, 32 },
	{ "FHWA (Highway Gothic) Series C 32pt", &font_fhwa_series_c, nullptr, 32 },
	{ "FHWA (Highway Gothic) Series D 32pt", &font_fhwa_series_d, nullptr, 32 },
	{ "FHWA (Highway Gothic) Series E 32pt", &font_fhwa_series_e, nullptr, 32 },
	{ "FHWA (Highway Gothic) Series F 32pt", &font_fhwa_series_f, nullptr, 32 },
	{ "GB Boot 32pt", &font_gb_boot, nullptr, 32 },
	{ "Handwriting 32pt", &font_handwriting, nullptr, 32 },
	{ "Heuristica 32pt", &font_heuristica, nullptr, 32 },
	{ "Heuristica Bold 32pt", &font_heuristica_bold, nullptr, 32 },
	{ "Heuristica Italic 32pt", &font_heuristica_italic, nullptr, 32 },
	{ "Heuristica Bold Italic 32pt", &font_heuristica_bold_italic, nullptr, 32 },
	{ "Informa 32pt", &font_informa, nullptr, 32 },
	{ "IPA Gothic 32pt", &font_ipa_gothic, nullptr, 32 },
	{ "Junicode 32pt", &font_junicode, nullptr, 32 },
	{ "Junicode Bold 32pt", &font_junicode_bold, nullptr, 32 },
	{ "Junicode Bold Italic 32pt", &font_junicode_bold_italic, nullptr, 32 },
	{ "Junicode Italic 32pt", &font_junicode_italic, nullptr, 32 },
	{ "Let's Go Digital 32pt", &font_lets_go_digital, nullptr, 32 },
	{ "Mecha 32pt", &font_mecha, nullptr, 32 },
	{ "ModeSeven 20pt", &font_modeseven, nullptr, 20 },
	{ "Montserrat 32pt", &font_montserrat, nullptr, 32 },
	{ "Montserrat Bold 32pt", &font_montserrat_bold, nullptr, 32 },
	{ "My Underwood 32pt", &font_my_underwood, nullptr, 32 },
	{ "Oregon 32pt", &font_oregon, nullptr, 32 },
	{ "Oregon Bold 32pt", &font_oregon_bold, nullptr, 32 },
	{ "Oregon Italic 32pt", &font_oregon_italic, nullptr, 32 },
	{ "Oregon Italic Bold 32pt", &font_oregon_italic_bold, nullptr, 32 },
	{ "Oregon Wide 32pt", &font_oregon_wide, nullptr, 32 },
	{ "Oregon Wide Bold 32pt", &font_oregon_wide_bold, nullptr, 32 },
	{ "Oregon Wide Italic 32pt", &font_oregon_wide_italic, nullptr, 32 },
	{ "Oregon Wide Bold Italic 32pt", &font_oregon_wide_bold_italic, nullptr, 32 },
	{ "Oregon Condensed 32pt", &font_oregon_condensed, nullptr, 32 },
	{ "Oregon Condensed Bold 32pt", &font_oregon_condensed_bold, nullptr, 32 },
	{ "Oregon Condensed Italic 32pt", &font_oregon_condensed_italic, nullptr, 32 },
	{ "Oregon Condensed Bold Italic 32pt", &font_oregon_condensed_bold_italic, nullptr, 32 },
	{ "Papyrus 32pt", &font_papyrus, nullptr, 32 },
	{ "Qaungo 32pt", &font_quango, nullptr, 32 },
	{ "Quivira 32pt", &font_quivira, nullptr, 32 },
	{ "Raconteur NF 32pt", &font_raconteur_nf, nullptr, 32 },
	{ "Railway 32pt", &font_railway, nullptr, 32 },
	{ "Reislust 32pt", &font_reislust, nullptr, 32 },
	{ "Rochester 48pt", &font_rochester, nullptr, 48 },
	{ "Romantiques 32pt", &font_romantiques, nullptr, 32 },
	{ "Pacifico 32pt", &font_pacifico, nullptr, 32 },
	{ "Rapscallion 32pt", &font_rapscallion, nullptr, 32 },
	{ "Sacramento 48pt", &font_sacramento, nullptr, 48 },
	{ "Schoolbell 32pt", &font_schoolbell, nullptr, 32 }, 
	{ "Shylock 32pt", &font_shylock, nullptr, 32 },
	{ "Spicy Rice 32pt", &font_spicy_rice, nullptr, 32 },
	{ "Star Jedi 32pt", &font_star_jedi, nullptr, 32 },
	{ "Star Jedi Contour 32pt", &font_star_jedi_contour, nullptr, 32 },
	{ "Swansea 16pt", &font_swansea, nullptr, 16 },
	{ "Swansea Bold 16pt", &font_swansea_bold, nullptr, 16 },
	{ "Swansea Italic 16pt", &font_swansea_italic, nullptr, 16 },
	{ "Swansea Bold Italic 16pt", &font_swansea_bold_italic, nullptr, 16 },
	{ "Symbola 32pt", &font_symbola, nullptr, 32 },
	{ "Theano Didot 32pt", &font_theano_didot, nullptr, 32 },
	{ "Theano Modern 32pt", &font_theano_modern, nullptr, 32 },
	{ "Theano Old Style 32pt", &font_theano_old_style, nullptr, 32 },
	{ "Timeburner 32pt", &font_timeburner, nullptr, 32 },
	{ "Times Square 48pt", &font_times_square, nullptr, 48 },
	{ "Tinos 32pt", &font_tinos, nullptr, 32 },
	{ "Tinos Bold 32pt", &font_tinos_bold, nullptr, 32 },
	{ "Tinos Italic 32pt", &font_tinos_italic, nullptr, 32 },
	{ "Tinos Bold Italic 32pt", &font_tinos_bold_italic, nullptr, 32 },
	{ "TNG Monitors 32pt", &font_tng_monitors, nullptr, 32 },
	{ "TNG Title 32pt", &font_tng_title, nullptr, 32 },
	{ "TOS Title 32pt", &font_tos_title, nullptr, 32 },
	{ "TruTypewriter PolyglOTT 32pt", &font_truetypewriter_polyglott, nullptr, 32 },

	};
	u32 w = 0, h = 0;

	for (auto& f : fs) {
		Font font(f.ptr);
		f.render = font.render_cstr(f.text, f.font_size, true, nullptr);
		h += f.render->height() + 20;
		w = std::max(w, f.render->width() + 20);
	}

	SBitmap* out = new SBitmap(w, h);
	out->clear(C_WHITE);
	h = 10;
	for (auto& f : fs) {
		Font::blit(f.render, out, 10, h, C_BLACK);
		h += f.render->height() + 20;
	}
	out->save_bmp("font_samples.png");
	return 0;
}
