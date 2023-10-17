/***

	space.cpp

        RGB-to-HSV and HSV-to-RGB conversions,
        generic color space stuff

        Copyright (c) Chris Street, 2002-2022

***/

#include "libcodehappy.h"
#include <math.h>

void RGB_HSV(int r, int g, int b, int *h, int *s, int *v) {

        /*
                red-green-blue to hue-saturation-value
                scale: all from 0 to 255
        */

        double rr, gg, bb;
        double min, max, delta;
        double hh, ss, vv;

        rr = r / 255.;
        gg = g / 255.;
        bb = b / 255.;

        min = (rr < gg ? rr : gg);
        min = (bb < min ? bb : min);
        max = (rr > gg ? rr : gg);
        max = (bb > max ? bb : max);

        vv = max;

        delta = (max - min);

        if (delta == 0.) {      /* straight equality is OK, delta is an integer */
                *s = 0;
                *h = r;
                *v = r; /* zero chromality */
                return;
        }

        if (max != 0.)
                ss = delta / max;
        else {
                *h = 0;
                *s = 0;
                *v = 0;
                return;
        }

        if (rr == max)
                hh = (gg - bb) / delta;
        else if (g == max)
                hh = 2 + (bb - rr) / delta; 
        else
                hh = 4 + (rr - gg) / delta;

        hh *= 60.;

        if (hh < 0.)
                hh += 360.;

       *h = (int)(hh * 255. / 360.) & 0xff;
       *s = (int)(ss * 255.) & 0xff;
       *v = (int)(vv * 255.) & 0xff;

}

void HSV_RGB(int h, int s, int v, int *r, int *g, int *b) {

        int i;
        double f, p, q, t;
        double rr, gg, bb, hh, ss, vv;

        hh = (double)(h) * 360. / 255.;
        ss = (double)s / 255.;
        vv = (double)v / 255.;

        hh /= 60.;
        i = floor(hh);
        f = hh - i;
        p = vv * (1. - ss);
        q = vv * (1. - ss * f);
        t = vv * (1. - ss * (1 - f));

        switch (i) {

                case 0:
                        rr = vv;
                        gg = t;
                        bb = p;
                        break;
                case 1:
                        rr = q;
                        gg = vv;
                        bb = p;
                        break;
                case 2:
                        rr = p;
                        gg = vv;
                        bb = t;
                        break;
                case 3:
                        rr = p;
                        gg = q;
                        bb = vv;
                        break;
                case 4:
                        rr = t;
                        gg = p;
                        bb = vv;
                        break;
                default:
                        rr = vv;
                        gg = p;
                        bb = q;
                        break;
        }

        *r = (int)floor(rr * 255. + 0.5);
        *g = (int)floor(gg * 255. + 0.5);
        *b = (int)floor(bb * 255. + 0.5);
	if (*r < 0)	*r = 0;
	if (*r > 255)	*r = 255;
	if (*g < 0)	*g = 0;
	if (*g > 255)	*g = 255;
	if (*b < 0)	*b = 0;
	if (*b > 255)	*b = 255;
}

void RGB_YIQ(int r, int g, int b, int *y, int *i, int *q) {
        double yy, ii, qq;

        yy = 0.299 * r + 0.587 * g + 0.114 * b;
        ii = 0.596 * r - 0.275 * g - 0.321 * b;
        qq = 0.212 * r - 0.523 * g + 0.311 * b;

        /* normalize */

        ii += 151.98;
        ii *= (255. / 303.96);
        qq += 133.365;
        qq *= (255. / 266.73);

        *y = (int)(yy) & 0xff;
        *i = (int)(ii) & 0xff;
        *q = (int)(qq) & 0xff;


}

void YIQ_RGB(int y, int i, int q, int *r, int *g, int *b) {

        double rr, gg, bb;
        double ii, qq;

        /* normalize */

        ii = i*(303.96 / 255.);
        ii -= 151.98;
        qq = q*(266.73 / 255.);
        qq -= 133.365;

        rr = y + 0.956 * ii + 0.621 * qq;
        gg = y - 0.272 * ii - 0.647 * qq;
        bb = y - 1.105 * ii + 1.702 * qq;

        rr = (rr < 0. ? 0. : rr);
        rr = (rr > 255. ? 255. : rr);
        gg = (gg < 0. ? 0. : gg);
        gg = (gg > 255. ? 255. : gg);
        bb = (bb < 0. ? 0. : bb);
        bb = (bb > 255. ? 255. : bb);


        *r = (int)(rr) & 0xff;
        *g = (int)(gg) & 0xff;
        *b = (int)(bb) & 0xff;

}

void RGB_YCbCr_601(int r, int g, int b, int *y, int *cb, int *cr) {

        double yy, ccb, ccr;

        yy = 0.2989 * r + 0.5866 * g + 0.1145 * b;
        ccb = -0.1687 * r - 0.3312 * g + 0.5 * b;
        ccr = 0.5 * r - 0.4183 * g - 0.0816 * b;
        ccb += 128.;
        ccr += 128.;

        ccb = (ccb > 255. ? 255. : ccb);
        ccb = (ccb < 0 ? 0 : ccb);
        ccr = (ccr > 255. ? 255. : ccr);
        ccr = (ccr < 0 ? 0 : ccr);

        *y = (int)(yy) & 0xff;
        *cb = (int)(ccb) & 0xff;
        *cr = (int)(ccr) & 0xff;

}


void YCbCr_601_RGB(int y, int cb, int cr, int *r, int *g, int *b, int brightness_adjust) {

        double yy, ccb, ccr;
        double rr, gg, bb;

        yy = (y+brightness_adjust) / 255.;
        ccb = (cb / 255.) - 0.5;
        ccr = (cr / 255.) - 0.5;

        rr = yy                + 1.4022 * ccr;
        gg = yy - 0.3456 * ccb - 0.7145 * ccr;
        bb = yy + 1.7710 * ccb;

        rr = (rr < 0. ? 0. : rr);
        rr = (rr > 1. ? 1. : rr);
        gg = (gg < 0. ? 0. : gg);
        gg = (gg > 1. ? 1. : gg);
        bb = (bb < 0. ? 0. : bb);
        bb = (bb > 1. ? 1. : bb);

        *r = (int)(rr * 255.) & 0xff;
        *g = (int)(gg * 255.) & 0xff;
        *b = (int)(bb * 255.) & 0xff;

}

void interpolate_color(int r1, int g1, int b1,
			int r2, int g2, int b2,
			int *ri, int *gi, int *bi,
			int interp) {
	*ri = (r1 * interp) + (r2 * (1000 - interp));
	*ri /= 1000;
	*gi = (g1 * interp) + (g2 * (1000 - interp));
	*gi /= 1000;
	*bi = (b1 * interp) + (b2 * (1000 - interp));
	*bi /= 1000;
}

void interpolate_rgbcolor(RGBColor rgb1, RGBColor rgb2, RGBColor* rgb_out, int interp) {
	int r1, g1, b1, r2, g2, b2, ri, gi, bi;

	r1 = RGB_RED(rgb1);
	r2 = RGB_RED(rgb2);
	g1 = RGB_GREEN(rgb1);
	g2 = RGB_GREEN(rgb2);
	b1 = RGB_BLUE(rgb1);
	b2 = RGB_BLUE(rgb2);

	r1 = COMPONENT_RANGE(r1);
	r2 = COMPONENT_RANGE(r2);
	g1 = COMPONENT_RANGE(g1);
	g2 = COMPONENT_RANGE(g2);
	b1 = COMPONENT_RANGE(b1);
	b2 = COMPONENT_RANGE(b2);

	interpolate_color(r1, g1, b1, r2, g2, b2, &ri, &gi, &bi, interp);

	*rgb_out = RGB_NO_CHECK(ri, gi, bi);

	return;
}

/* end space.cpp */
