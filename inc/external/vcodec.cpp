/***

	vcodec.cpp

	Dr. Tim Ferguson's video codec encoding/decoding functions
	for Cinepak, CYUV, and MSVC support. 

***/

/* ------------------------------------------------------------------------
 * Radius Cinepak Video Decoder
 *
 * Dr. Tim Ferguson, 2001.
 * For more details on the algorithm:
 *         http://www.csse.monash.edu.au/~timf/videocodec.html
 *
 * This is basically a vector quantiser with adaptive vector density.  The
 * frame is segmented into 4x4 pixel blocks, and each block is coded using
 * either 1 or 4 vectors.
 *
 * There are still some issues with this code yet to be resolved.  In
 * particular with decoding in the strip boundaries.  However, I have not
 * yet found a sequence it doesn't work on.  Ill keep trying :)
 *
 * You may freely use this source code.  I only ask that you reference its
 * source in your projects documentation:
 *       Tim Ferguson: http://www.csse.monash.edu.au/~timf/
 * ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#define DBUG	0
#define MAX_STRIPS 32

/* ------------------------------------------------------------------------ */
typedef struct
{
	unsigned char y0, y1, y2, y3;
	char u, v;
	unsigned long rgb0, rgb1, rgb2, rgb3;		/* should be a union */
	unsigned char r[4], g[4], b[4];
} cvid_codebook;

typedef struct {
	cvid_codebook *v4_codebook[MAX_STRIPS];
	cvid_codebook *v1_codebook[MAX_STRIPS];
	int strip_num;
} cinepak_info;


/* ------------------------------------------------------------------------ */
static unsigned char *in_buffer, uiclip[1024], *uiclp = NULL;

#define get_byte() *(in_buffer++)
#define skip_byte() in_buffer++
#define get_word() ((unsigned short)(in_buffer += 2, \
	(in_buffer[-2] << 8 | in_buffer[-1])))
#define get_long() ((unsigned long)(in_buffer += 4, \
	(in_buffer[-4] << 24 | in_buffer[-3] << 16 | in_buffer[-2] << 8 | in_buffer[-1])))


/* ---------------------------------------------------------------------- */
static inline void read_codebook_32(cvid_codebook *c, int mode)
{
int uvr, uvg, uvb;

	if(mode)		/* black and white */
		{
		c->y0 = get_byte();
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = c->v = 0;

		c->rgb0 = (c->y0 << 16) | (c->y0 << 8) | c->y0;
		c->rgb1 = (c->y1 << 16) | (c->y1 << 8) | c->y1;
		c->rgb2 = (c->y2 << 16) | (c->y2 << 8) | c->y2;
		c->rgb3 = (c->y3 << 16) | (c->y3 << 8) | c->y3;
		}
	else			/* colour */
		{
		c->y0 = get_byte();  /* luma */
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = get_byte(); /* chroma */
		c->v = get_byte();

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->rgb0 = (uiclp[c->y0 + uvr] << 16) | (uiclp[c->y0 + uvg] << 8) | uiclp[c->y0 + uvb];
		c->rgb1 = (uiclp[c->y1 + uvr] << 16) | (uiclp[c->y1 + uvg] << 8) | uiclp[c->y1 + uvb];
		c->rgb2 = (uiclp[c->y2 + uvr] << 16) | (uiclp[c->y2 + uvg] << 8) | uiclp[c->y2 + uvb];
		c->rgb3 = (uiclp[c->y3 + uvr] << 16) | (uiclp[c->y3 + uvg] << 8) | uiclp[c->y3 + uvb];
		}
}


/* ------------------------------------------------------------------------ */
inline void cvid_v1_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb)
{
unsigned long *vptr = (unsigned long *)frm, rgb;
int row_inc = stride/4;

	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
}


/* ------------------------------------------------------------------------ */
inline void cvid_v4_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
unsigned long *vptr = (unsigned long *)frm;
int row_inc = stride/4;

	vptr[0] = cb0->rgb0;
	vptr[1] = cb0->rgb1;
	vptr[2] = cb1->rgb0;
	vptr[3] = cb1->rgb1;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = cb0->rgb2;
	vptr[1] = cb0->rgb3;
	vptr[2] = cb1->rgb2;
	vptr[3] = cb1->rgb3;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = cb2->rgb0;
	vptr[1] = cb2->rgb1;
	vptr[2] = cb3->rgb0;
	vptr[3] = cb3->rgb1;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = cb2->rgb2;
	vptr[1] = cb2->rgb3;
	vptr[2] = cb3->rgb2;
	vptr[3] = cb3->rgb3;
}


/* ---------------------------------------------------------------------- */
static inline void read_codebook_24(cvid_codebook *c, int mode)
{
int uvr, uvg, uvb;

	if(mode)		/* black and white */
		{
		c->y0 = get_byte();
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = c->v = 0;

		c->r[0] = c->g[0] = c->b[0] = c->y0;
		c->r[1] = c->g[1] = c->b[1] = c->y1;
		c->r[2] = c->g[2] = c->b[2] = c->y2;
		c->r[3] = c->g[3] = c->b[3] = c->y3;
		}
	else			/* colour */
		{
		c->y0 = get_byte();  /* luma */
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = get_byte(); /* chroma */
		c->v = get_byte();

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->r[0] = uiclp[c->y0 + uvr]; c->g[0] = uiclp[c->y0 + uvg]; c->b[0] = uiclp[c->y0 + uvb];
		c->r[1] = uiclp[c->y1 + uvr]; c->g[1] = uiclp[c->y1 + uvg]; c->b[1] = uiclp[c->y1 + uvb];
		c->r[2] = uiclp[c->y2 + uvr]; c->g[2] = uiclp[c->y2 + uvg]; c->b[2] = uiclp[c->y2 + uvb];
		c->r[3] = uiclp[c->y3 + uvr]; c->g[3] = uiclp[c->y3 + uvg]; c->b[3] = uiclp[c->y3 + uvb];
		}
}


/* ------------------------------------------------------------------------ */
void cvid_v1_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb)
{
unsigned char r, g, b;
int row_inc = stride-4*3;

	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
}


/* ------------------------------------------------------------------------ */
void cvid_v4_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
int row_inc = stride-4*3;

	*vptr++ = cb0->b[0]; *vptr++ = cb0->g[0]; *vptr++ = cb0->r[0];
	*vptr++ = cb0->b[1]; *vptr++ = cb0->g[1]; *vptr++ = cb0->r[1];
	*vptr++ = cb1->b[0]; *vptr++ = cb1->g[0]; *vptr++ = cb1->r[0];
	*vptr++ = cb1->b[1]; *vptr++ = cb1->g[1]; *vptr++ = cb1->r[1];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb0->b[2]; *vptr++ = cb0->g[2]; *vptr++ = cb0->r[2];
	*vptr++ = cb0->b[3]; *vptr++ = cb0->g[3]; *vptr++ = cb0->r[3];
	*vptr++ = cb1->b[2]; *vptr++ = cb1->g[2]; *vptr++ = cb1->r[2];
	*vptr++ = cb1->b[3]; *vptr++ = cb1->g[3]; *vptr++ = cb1->r[3];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb2->b[0]; *vptr++ = cb2->g[0]; *vptr++ = cb2->r[0];
	*vptr++ = cb2->b[1]; *vptr++ = cb2->g[1]; *vptr++ = cb2->r[1];
	*vptr++ = cb3->b[0]; *vptr++ = cb3->g[0]; *vptr++ = cb3->r[0];
	*vptr++ = cb3->b[1]; *vptr++ = cb3->g[1]; *vptr++ = cb3->r[1];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb2->b[2]; *vptr++ = cb2->g[2]; *vptr++ = cb2->r[2];
	*vptr++ = cb2->b[3]; *vptr++ = cb2->g[3]; *vptr++ = cb2->r[3];
	*vptr++ = cb3->b[2]; *vptr++ = cb3->g[2]; *vptr++ = cb3->r[2];
	*vptr++ = cb3->b[3]; *vptr++ = cb3->g[3]; *vptr++ = cb3->r[3];
}


/* ------------------------------------------------------------------------
 * Call this function once at the start of the sequence and save the
 * returned context for calls to decode_cinepak().
 */
void *decode_cinepak_init(void)
{
cinepak_info *cvinfo;
int i;

	if((cvinfo = (cinepak_info *)calloc(sizeof(cinepak_info), 1)) == NULL) return NULL;
	cvinfo->strip_num = 0;

	if(uiclp == NULL)
		{
		uiclp = uiclip+512;
		for(i = -512; i < 512; i++)
			uiclp[i] = (i < 0 ? 0 : (i > 255 ? 255 : i));
		}

	return (void *)cvinfo;
}


/* ------------------------------------------------------------------------
 * This function decodes a buffer containing a Cinepak encoded frame.
 *
 * context - the context created by decode_cinepak_init().
 * buf - the input buffer to be decoded
 * size - the size of the input buffer
 * frame - the output frame buffer (24 or 32 bit per pixel)
 * width - the width of the output frame
 * height - the height of the output frame
 * bit_per_pixel - the number of bits per pixel allocated to the output
 *   frame (only 24 or 32 bpp are supported)
 */
void decode_cinepak(void *context, unsigned char *buf, int size, unsigned char *frame, int width, int height, int bit_per_pixel)
{
cinepak_info *cvinfo = (cinepak_info *)context;
cvid_codebook *v4_codebook, *v1_codebook, *codebook = NULL;
unsigned long x, y, y_bottom, frame_flags, strips, cv_width, cv_height, cnum,
	strip_id, chunk_id, x0, y0, x1, y1, ci, flag, mask;
long len, top_size, chunk_size;
unsigned char *frm_ptr, *frm_end;
int i, cur_strip, d0, d1, d2, d3, frm_stride, bpp = 3;
void (*read_codebook)(cvid_codebook *c, int mode) = read_codebook_24;
void (*cvid_v1)(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb) = cvid_v1_24;
void (*cvid_v4)(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3) = cvid_v4_24;

	x = y = 0;
	y_bottom = 0;
	in_buffer = buf;

	frame_flags = get_byte();
	len = get_byte() << 16;
	len |= get_byte() << 8;
	len |= get_byte();

	switch(bit_per_pixel)
		{
		case 24:
			bpp = 3;
			read_codebook = read_codebook_24;
			cvid_v1 = cvid_v1_24;
			cvid_v4 = cvid_v4_24;
			break;
		case 32:
			bpp = 4;
			read_codebook = read_codebook_32;
			cvid_v1 = cvid_v1_32;
			cvid_v4 = cvid_v4_32;
			break;
		}

	frm_stride = width * bpp;
	frm_ptr = frame;
	frm_end = frm_ptr + width * height * bpp;

	if(len != size)
		{
		if(len & 0x01) len++; /* AVIs tend to have a size mismatch */
		if(len != size)
			{
			printf("CVID: corruption %d (QT/AVI) != %ld (CV)\n", size, len);
			// return;
			}
		}

	cv_width = get_word();
	cv_height = get_word();
	strips = get_word();

	if(strips > cvinfo->strip_num)
		{
		if(strips >= MAX_STRIPS) 
			{
			printf("CVID: strip overflow (more than %d)\n", MAX_STRIPS);
			return;
			}

		for(i = cvinfo->strip_num; i < strips; i++)
			{
			if((cvinfo->v4_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 260)) == NULL)
				{
				printf("CVID: codebook v4 alloc err\n");
				return;
				}

			if((cvinfo->v1_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 260)) == NULL)
				{
				printf("CVID: codebook v1 alloc err\n");
				return;
				}
			}
		}
	cvinfo->strip_num = strips;

#if DBUG
	printf("CVID: <%ld,%ld> strips %ld\n", cv_width, cv_height, strips);
#endif

	for(cur_strip = 0; cur_strip < strips; cur_strip++)
		{
		v4_codebook = cvinfo->v4_codebook[cur_strip];
		v1_codebook = cvinfo->v1_codebook[cur_strip];

		if((cur_strip > 0) && (!(frame_flags & 0x01)))
			{
			memcpy(cvinfo->v4_codebook[cur_strip], cvinfo->v4_codebook[cur_strip-1], 260 * sizeof(cvid_codebook));
			memcpy(cvinfo->v1_codebook[cur_strip], cvinfo->v1_codebook[cur_strip-1], 260 * sizeof(cvid_codebook));
			}

		strip_id = get_word();		/* 1000 = key strip, 1100 = iter strip */
		top_size = get_word();
		y0 = get_word();		/* FIXME: most of these are ignored at the moment */
		x0 = get_word();
		y1 = get_word();
		x1 = get_word();

		y_bottom += y1;
		top_size -= 12;
		x = 0;
		if(x1 != width) 
			printf("CVID: Warning x1 (%ld) != width (%d)\n", x1, width);

#if DBUG
	printf("   %d) %04lx %04ld <%ld,%ld> <%ld,%ld> yt %ld\n",
		cur_strip, strip_id, top_size, x0, y0, x1, y1, y_bottom);
#endif

		while(top_size > 0)
			{
			chunk_id  = get_word();
			chunk_size = get_word();

#if DBUG
	printf("        %04lx %04ld\n", chunk_id, chunk_size);
#endif
			top_size -= chunk_size;
			chunk_size -= 4;

			switch(chunk_id)
				{
					/* -------------------- Codebook Entries -------------------- */
				case 0x2000:
				case 0x2200:
					codebook = (chunk_id == 0x2200 ? v1_codebook : v4_codebook);
					cnum = chunk_size/6;
					for(i = 0; i < cnum; i++) read_codebook(codebook+i, 0);
					break;

				case 0x2400:
				case 0x2600:		/* 8 bit per pixel */
					codebook = (chunk_id == 0x2600 ? v1_codebook : v4_codebook);
					cnum = chunk_size/4;  
					for(i = 0; i < cnum; i++) read_codebook(codebook+i, 1);
					break;

				case 0x2100:
				case 0x2300:
					codebook = (chunk_id == 0x2300 ? v1_codebook : v4_codebook);

					ci = 0;
					while(chunk_size > 0)
						{
						flag = get_long();
						chunk_size -= 4;

						for(i = 0; i < 32; i++)
							{
							if(flag & 0x80000000)
								{
								chunk_size -= 6;
								read_codebook(codebook+ci, 0);
								}

							ci++;
							flag <<= 1;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x2500:
				case 0x2700:		/* 8 bit per pixel */
					codebook = (chunk_id == 0x2700 ? v1_codebook : v4_codebook);

					ci = 0;
					while(chunk_size > 0)
						{
						flag = get_long();
						chunk_size -= 4;

						for(i = 0; i < 32; i++)
							{
							if(flag & 0x80000000)
								{
								chunk_size -= 4;
								read_codebook(codebook+ci, 1);
								}

							ci++;
							flag <<= 1;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

					/* -------------------- Frame -------------------- */
				case 0x3000: 
					while((chunk_size > 0) && (y < y_bottom))
						{
						flag = get_long();
						chunk_size -= 4;
						
						for(i = 0; i < 32; i++)
							{ 
							if(y >= y_bottom) break;
							if(flag & 0x80000000)	/* 4 bytes per block */
								{
								d0 = get_byte();
								d1 = get_byte();
								d2 = get_byte();
								d3 = get_byte();
								chunk_size -= 4;
								cvid_v4(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
								}
							else		/* 1 byte per block */
								{
								cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte());
								chunk_size--;
								}

							x += 4;
							if(x >= width)
								{
								x = 0;
								y += 4;
								}
							flag <<= 1;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x3100:
					while((chunk_size > 0) && (y < y_bottom))
						{
							/* ---- flag bits: 0 = SKIP, 10 = V1, 11 = V4 ---- */
						flag = (unsigned long)get_long();
						chunk_size -= 4;
						mask = 0x80000000;

						while((mask) && (y < y_bottom))
							{
							if(flag & mask)
								{
								if(mask == 1)
									{
									if(chunk_size < 0) break;
									flag = (unsigned long)get_long();
									chunk_size -= 4;
									mask = 0x80000000;
									}
								else mask >>= 1;
								
								if(flag & mask)		/* V4 */
									{
									d0 = get_byte();
									d1 = get_byte();
									d2 = get_byte();
									d3 = get_byte();
									chunk_size -= 4;
									cvid_v4(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
									}
								else		/* V1 */
									{
									chunk_size--;
									cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte());
									}
								}		/* else SKIP */

							mask >>= 1;
							x += 4;
							if(x >= width)
								{
								x = 0;
								y += 4;
								}
							}
						}

					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x3200:		/* each byte is a V1 codebook */
					while((chunk_size > 0) && (y < y_bottom))
						{
						cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte());
						chunk_size--;
						x += 4;
						if(x >= width)
							{
							x = 0;
							y += 4;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				default:
					printf("CVID: unknown chunk_id %08lx\n", chunk_id);
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;
				}
			}
		}

	if(len != size)
		{
		if(len & 0x01) len++; /* AVIs tend to have a size mismatch */
		if(len != size)
			{
			long xlen;
			skip_byte();
			xlen = get_byte() << 16;
			xlen |= get_byte() << 8;
			xlen |= get_byte(); /* Read Len */
			printf("CVID: END INFO chunk size %d cvid size1 %ld cvid size2 %ld\n", size, len, xlen);
			}
		}
}

/* ------------------------------------------------------------------------
 * Creative YUV Video Decoder
 *
 * Dr. Tim Ferguson, 2001.
 * For more details on the algorithm:
 *         http://www.csse.monash.edu.au/~timf/videocodec.html
 *
 * This is a very simple predictive coder.  A video frame is coded in YUV411
 * format.  The first pixel of each scanline is coded using the upper four
 * bits of its absolute value.  Subsequent pixels for the scanline are coded
 * using the difference between the last pixel and the current pixel (DPCM).
 * The DPCM values are coded using a 16 entry table found at the start of the
 * frame.  Thus four bits per component are used and are as follows:
 *     UY VY YY UY VY YY UY VY...
 * This code assumes the frame width will be a multiple of four pixels.  This
 * should probably be fixed.
 *
 * You may freely use this source code.  I only ask that you reference its
 * source in your projects documentation:
 *       Tim Ferguson: http://www.csse.monash.edu.au/~timf/
 * ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* ------------------------------------------------------------------------
 * This function decodes a buffer containing a CYUV encoded frame.
 *
 * buf - the input buffer to be decoded
 * size - the size of the input buffer
 * frame - the output frame buffer (UYVY format)
 * width - the width of the output frame
 * height - the height of the output frame
 * bit_per_pixel - ignored for now: may be used later for conversions.
 */
void cyuv_decode(unsigned char *buf, int size, unsigned char *frame, int width, int height, int bit_per_pixel)
{
int i, xpos, ypos, cur_Y = 0, cur_U = 0, cur_V = 0;
char *delta_y_tbl, *delta_c_tbl, *ptr;

	delta_y_tbl = (char *)buf + 16;
	delta_c_tbl = (char *)buf + 32;
	ptr = (char *)buf + (16 * 3);

	for(ypos = 0; ypos < height; ypos++)
		for(xpos = 0; xpos < width; xpos += 4)
			{
			if(xpos == 0)		/* first pixels in scanline */
				{
				cur_U = *(ptr++);
				cur_Y = (cur_U & 0x0f) << 4;
				cur_U = cur_U & 0xf0;
				*frame++ = cur_U;
				*frame++ = cur_Y;

				cur_V = *(ptr++);
				cur_Y = (cur_Y + delta_y_tbl[cur_V & 0x0f]) & 0xff;
				cur_V = cur_V & 0xf0;
				*frame++ = cur_V;
				*frame++ = cur_Y;
				}
			else			/* subsequent pixels in scanline */
				{
				i = *(ptr++);
				cur_U = (cur_U + delta_c_tbl[i >> 4]) & 0xff;
				cur_Y = (cur_Y + delta_y_tbl[i & 0x0f]) & 0xff;
				*frame++ = cur_U;
				*frame++ = cur_Y;

				i = *(ptr++);
				cur_V = (cur_V + delta_c_tbl[i >> 4]) & 0xff;
				cur_Y = (cur_Y + delta_y_tbl[i & 0x0f]) & 0xff;
				*frame++ = cur_V;
				*frame++ = cur_Y;
				}

			i = *(ptr++);
			cur_Y = (cur_Y + delta_y_tbl[i & 0x0f]) & 0xff;
			*frame++ = cur_U;
			*frame++ = cur_Y;

			cur_Y = (cur_Y + delta_y_tbl[i >> 4]) & 0xff;
			*frame++ = cur_V;
			*frame++ = cur_Y;
			}
}

/* ------------------------------------------------------------------------
 * Creative YUV Video Encoder
 *
 * Dr. Tim Ferguson, 2001.
 * For more details on the algorithm:
 *         http://www.csse.monash.edu.au/~timf/videocodec.html
 *
 * This is a very simple predictive coder.  A video frame is coded in YUV411
 * format.  The first pixel of each scanline is coded using the upper four
 * bits of its absolute value.  Subsequent pixels for the scanline are coded
 * using the difference between the last pixel and the current pixel (DPCM).
 * The DPCM values are coded using a 16 entry table found at the start of the
 * frame.  Thus four bits per component are used and are as follows:
 *     UY VY YY UY VY YY UY VY...
 * This code assumes the frame width will be a multiple of four pixels.  This
 * should probably be fixed.
 * The code implements a non-linear quantisation of the prediction errors.
 *
 * You may freely use this source code.  I only ask that you reference its
 * source in your projects documentation:
 *       Tim Ferguson: http://www.csse.monash.edu.au/~timf/
 * ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#define AVIIF_TWOCC         0x00000002L
#define AVIIF_KEYFRAME      0x00000010L

/* ------------------------------------------------------------------------ */
static inline int cyuv_gen_delta(int cur, int new_) {	
	int delta, v;

	delta = (new_ - cur);
	v = (int)rint(sqrt((double)abs(delta)));
	if(delta < 0) return(v >= 8 ? 15 : v + 8);
	else return(v > 8 ? 0 : 8 - v);
}


/* ------------------------------------------------------------------------ */
static inline void cyuv_rgb_to_411(unsigned char *frame, int *y0, int *y1, int *y2,
	int *y3, int *u, int *v)
{
float fu, fv;
#define Y_VAL(r, g, b) (int)rint(((float)r * 0.299) + ((float)g * 0.587) + ((float)b * 0.114))
#define U_VAL(r, g, b) (((float)r * 0.5) + ((float)g * -0.4187) + ((float)b * -0.0813))
#define V_VAL(r, g, b) (((float)r * -0.1687) + ((float)g * -0.3313) + ((float)b * 0.5))
#define CLIPV(a, l, h) ((a) < l ? l : ((a) > h ? h : (a)))

	*y0 = Y_VAL(frame[0], frame[1], frame[2]);
	fu = U_VAL(frame[0], frame[1], frame[2]);
	fv = V_VAL(frame[0], frame[1], frame[2]);

	*y1 = Y_VAL(frame[3], frame[4], frame[5]);
	fu += U_VAL(frame[3], frame[4], frame[5]);
	fv += V_VAL(frame[3], frame[4], frame[5]);

	*y2 = Y_VAL(frame[6], frame[7], frame[8]);
	fu += U_VAL(frame[6], frame[7], frame[8]);
	fv += V_VAL(frame[6], frame[7], frame[8]);

	*y3 = Y_VAL(frame[9], frame[10], frame[11]);
	fu += U_VAL(frame[9], frame[10], frame[11]);
	fv += V_VAL(frame[9], frame[10], frame[11]);

	*y0 = CLIPV(*y0, 0, 255);
	*y1 = CLIPV(*y1, 0, 255);
	*y2 = CLIPV(*y2, 0, 255);
	*y3 = CLIPV(*y3, 0, 255);
	*u = (int)rint(CLIPV((fu/4 + 128), 0, 255));
	*v = (int)rint(CLIPV((fv/4 + 128), 0, 255));
}


/* ------------------------------------------------------------------------
 * This function encodes an RGB frame into a buffer using CYUV.
 *
 * Input:
 *   frame - the input frame buffer (24-bit RGB format)
 *   width - the width of the input frame
 *   height - the height of the input frame
 * Output:
 *   buf - the encoded output buffer to be saved in an AVI file
 *   size - the number of bytes put in the output buffer after encoding
 *   flags - the AVI flags the frame should be coded with (for the index)
 */
void cyuv_encode(unsigned char *frame, int width, int height, unsigned char *buf, int *size, unsigned int *flags)
{
int xpos, ypos, i, cur_Y = 0, cur_U = 0, cur_V = 0, delta_Y1, delta_Y2,
	delta_U, delta_V, delta_y_tbl[16], delta_c_tbl[16], delta_arr[1024],
	bpos = 0, y0, y1, y2, y3, u, v;
#define get_delta(cur, new_) delta_arr[512 + (new_) - (cur)];
#define app_delta(tbl, c, d) { c += tbl[d]; c = (c < 0 ? 0 : (c > 255 ? 255 : c)); }

	for(i = 0; i < 1024; i++) delta_arr[i] = cyuv_gen_delta(512, i);

	for(i = 0; i < 16; i++)
		{
		delta_y_tbl[i] = (i - 8) * (i - 8) * (i > 8 ? -1 : 1);
		delta_c_tbl[i] = delta_y_tbl[i];
		}

	for(i = 0; i < 16; i++) buf[bpos++] = delta_y_tbl[i];		/* luma */
	for(i = 0; i < 16; i++) buf[bpos++] = delta_y_tbl[i];		/* luma */
	for(i = 0; i < 16; i++) buf[bpos++] = delta_c_tbl[i];		/* chroma */

	for(ypos = 0; ypos < height; ypos++)
		for(xpos = 0; xpos < width; xpos += 4)
			{
			cyuv_rgb_to_411(frame, &y0, &y1, &y2, &y3, &u, &v);
			frame += (3 * 4);

			if(xpos == 0)
				{
				cur_Y = y0 & 0xf0;
				cur_U = u & 0xf0;
				cur_V = v & 0xf0;

				buf[bpos++] = (cur_Y >> 4) | cur_U;
				delta_Y1 = get_delta(cur_Y, y1);
				buf[bpos++] = (delta_Y1 & 0x0f) | cur_V;
				app_delta(delta_y_tbl, cur_Y, delta_Y1);
				}
			else
				{
				delta_Y1 = get_delta(cur_Y, y0);
				delta_U = get_delta(cur_U, u);

				buf[bpos++] = (delta_Y1 & 0x0f) | (delta_U << 4);
				app_delta(delta_y_tbl, cur_Y, delta_Y1);
				app_delta(delta_c_tbl, cur_U, delta_U);

				delta_Y1 = get_delta(cur_Y, y1);
				delta_V = get_delta(cur_V, v);

				buf[bpos++] = (delta_Y1 & 0x0f) | (delta_V << 4);
				app_delta(delta_y_tbl, cur_Y, delta_Y1);
				app_delta(delta_c_tbl, cur_V, delta_V);
				}

			delta_Y1 = get_delta(cur_Y, y2);
			app_delta(delta_y_tbl, cur_Y, delta_Y1);

			delta_Y2 = get_delta(cur_Y, y3);
			app_delta(delta_y_tbl, cur_Y, delta_Y2);

			buf[bpos++] = (delta_Y1 & 0x0f) | (delta_Y2 << 4);
			}

	*size = bpos;
	*flags = AVIIF_TWOCC | AVIIF_KEYFRAME;
}

/* ------------------------------------------------------------------------
 * Microsoft Video 1 Encoder
 *
 * Dr. Tim Ferguson, 2001.
 * For more details on the algorithm:
 *         http://www.csse.monash.edu.au/~timf/videocodec.html
 *
 * You may freely use this source code.  I only ask that you reference its
 * source in your projects documentation:
 *       Tim Ferguson: http://www.csse.monash.edu.au/~timf/
 * ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
//#include "msvc.h"

#define AVIIF_TWOCC         0x00000002L
#define AVIIF_KEYFRAME      0x00000010L

typedef struct {
	int width, height, key_rate, current_frame, last_key, mode_cnt[4];
	unsigned char *prev_img;
} msvc_enc_info;

/* ------------------------------------------------------------------------ */
#define MSVC_C1(ip, clr, rdec) { \
	*ip++ = clr; *ip++ = clr; *ip++ = clr; *ip = clr; ip -= rdec; \
	*ip++ = clr; *ip++ = clr; *ip++ = clr; *ip = clr; ip -= rdec; \
	*ip++ = clr; *ip++ = clr; *ip++ = clr; *ip = clr; ip -= rdec; \
	*ip++ = clr; *ip++ = clr; *ip++ = clr; *ip = clr; }

#define MSVC_C2(ip,flag,cA,cB,rdec) { \
	*ip++ = (flag & 0x01) ? (cB) : (cA); *ip++ = (flag & 0x02) ? (cB) : (cA); \
	*ip++ = (flag & 0x04) ? (cB) : (cA); *ip   = (flag & 0x08) ? (cB) : (cA); \
	ip-=rdec; \
	*ip++ = (flag & 0x10) ? (cB) : (cA); *ip++ = (flag & 0x20) ? (cB) : (cA); \
	*ip++ = (flag & 0x40) ? (cB) : (cA); *ip   = (flag & 0x80) ? (cB) : (cA); }

#define MSVC_C4(ip,flag,cA0,cA1,cB0,cB1,rdec) { \
	*ip++ = (flag & 0x01) ? (cB0) : (cA0); *ip++ =(flag & 0x02) ? (cB0) : (cA0); \
	*ip++ = (flag & 0x04) ? (cB1) : (cA1); *ip   =(flag & 0x08) ? (cB1) : (cA1); \
	ip-=rdec; \
	*ip++ = (flag & 0x10) ? (cB0) : (cA0); *ip++ =(flag & 0x20) ? (cB0) : (cA0); \
	*ip++ = (flag & 0x40) ? (cB1) : (cA1); *ip   =(flag & 0x80) ? (cB1) : (cA1); }

	/* convert a 15 bit colour value to 24 bits */
#define COL_15_TO_24(color) \
	((((color) << 9) & (0x1f << 19)) | (((color) << 6) & (0x1f << 11)) | \
		(((color) & 0x1f) << 3))

/* ------------------------------------------------------------------------ */
static void msvc_apply_block(unsigned char *img, int stride, unsigned long col[], unsigned long index)
{
int row_dec;
unsigned long *i_ptr = (unsigned long *)(img + stride * 3);

	row_dec = stride/4 + 3;

	if(index & 0x8000)		/* 1 color encoding (80-83) && (>=88)*/
		{
		unsigned long clr = COL_15_TO_24(index);
		MSVC_C1(i_ptr,clr,row_dec);
		}
	else		/* 2 or 8 color encoding */
		{
		unsigned long cA0,cB0;
		cB0 = COL_15_TO_24(col[0]);
		cA0 = COL_15_TO_24(col[1]);
		if(col[0] & 0x8000)   /* Eight Color Encoding */
			{
			unsigned long cA1, cB1, code = index >> 8;
			cB1 = COL_15_TO_24(col[2]);
			cA1 = COL_15_TO_24(col[3]);
			MSVC_C4(i_ptr,index,cA0,cA1,cB0,cB1,row_dec);
			i_ptr -= row_dec;
			cB0 = COL_15_TO_24(col[4]);
			cA0 = COL_15_TO_24(col[5]);
			cB1 = COL_15_TO_24(col[6]);
			cA1 = COL_15_TO_24(col[7]);
			MSVC_C4(i_ptr,code,cA0,cA1,cB0,cB1,row_dec);
			}
		else /* Two Color Encoding */
			{
			unsigned long code = index >> 8;
			MSVC_C2(i_ptr,index,cA0,cB0,row_dec);
			i_ptr -= row_dec;
			MSVC_C2(i_ptr,code,cA0,cB0,row_dec);
			}
		} /* end of 2 or 8 */
}


/* ------------------------------------------------------------------------ */
static double msvc_lbg_array(unsigned char pixels[][3], int len, unsigned long *col1,
	unsigned long *col2, unsigned long *index)
{
unsigned char bin_index[16], num[2];
unsigned long best[2][3], old_index = 0, new_index;
int i, j, iter, diff;
double sd = 0;

		/* init best 2 colours with first and last block colour */
	for(j = 0; j < 2; j++)
		for(i = 0; i < 3; i++) best[j][i] = pixels[j*(len-1)][i];

		/* LBG style VQ algorithm for finding best 2 colours for block */
	for(iter = 0; iter < 20; iter++)
		{
			/* sort into bins */
		num[0] = num[1] = 0;
		for(i = 0; i < len; i++)
			{
			bin_index[i] = (
			(abs((int)pixels[i][0] - (int)best[0][0]) + abs((int)pixels[i][1] - (int)best[0][1]) +
				abs((int)pixels[i][2] - (int)best[0][2])) >
				(abs((int)pixels[i][0] - (int)best[1][0]) + abs((int)pixels[i][1] - (int)best[1][1]) +
				abs((int)pixels[i][2] - (int)best[1][2]))  ?  1 : 0);
			num[bin_index[i]]++;
			}
		if(num[0] == 0) bin_index[0] = 0;		/* must always have stuff in bins */
		if(num[1] == 0) bin_index[1] = 1;

			/* generate new vectors */
		best[0][0] = best[0][1] = best[0][2] = best[1][0] = best[1][1] = best[1][2] = 0;
		num[0] = num[1] = 0;
		new_index = 0;
		for(i = 0; i < len; i++)
			{
			num[bin_index[i]]++;
			best[bin_index[i]][0] += pixels[i][0];
			best[bin_index[i]][1] += pixels[i][1];
			best[bin_index[i]][2] += pixels[i][2];
			new_index |= ((unsigned long)bin_index[i] << i);
			}

		for(j = 0; j < 2; j++)
			for(i = 0; i < 3; i++)
				{
				best[j][i] = best[j][i]/num[j];
				if(best[j][i] > 255) best[j][i] = 255;
				}

		if(old_index == new_index) break;	/* index never == 0 */
		old_index = new_index;
		}

		/* quantise colours */
	for(j = 0; j < 2; j++)
		for(i = 0; i < 3; i++) best[j][i] &= 0xf8;

		/* calculate error */
	for(i = 0; i < len; i++)
		{
		diff = (int)best[bin_index[i]][0] - pixels[i][0];  sd += (double)(diff * diff);
		diff = (int)best[bin_index[i]][1] - pixels[i][1];  sd += (double)(diff * diff);
		diff = (int)best[bin_index[i]][2] - pixels[i][2];  sd += (double)(diff * diff);
		}

	*col1 = (best[0][0] << 7) | (best[0][1] << 2) | (best[0][2] >> 3);
	*col2 = (best[1][0] << 7) | (best[1][1] << 2) | (best[1][2] >> 3);
	*index = new_index;

	return sd;
}


/* ------------------------------------------------------------------------
 *  1 colour for 4x4 block
 */
static double msvc_colour1(unsigned char *img, int stride, unsigned long *col)
{
double sd = 0;
int diff;
unsigned long avg_r, avg_g, avg_b;
unsigned char *lp, *cp;

	avg_r = avg_g = avg_b = 0;
	for(lp = img; lp < img + 4 * stride; lp += stride)
		for(cp = lp; cp < lp+12; cp += 3)
			{
			avg_r += (int)cp[2];
			avg_g += (int)cp[1];
			avg_b += (int)cp[0];
			}
	avg_r = (avg_r >> 4) & 0xf8;
	avg_g = (avg_g >> 4) & 0xf8;
	avg_b = (avg_b >> 4) & 0xf8;

	for(lp = img; lp < img + 4 * stride; lp += stride)
		for(cp = lp; cp < lp+12; cp += 3)
			{
			diff = (int)cp[2] - avg_r;  sd += (double)(diff * diff);
			diff = (int)cp[1] - avg_g;  sd += (double)(diff * diff);
			diff = (int)cp[0] - avg_b;  sd += (double)(diff * diff);
			}

	*col = (avg_r << 7) | (avg_g << 2) | (avg_b >> 3);
	return sd;
}


/* ------------------------------------------------------------------------ */
static double msvc_colour2(unsigned char *img, int stride, unsigned long *col1,
	unsigned long *col2, unsigned long *index)
{
unsigned char pixels[16][3];
unsigned long tmp;
int j;
double sd = 0;
unsigned char *lp, *cp;

	for(j = 0, lp = img + 3 * stride; lp >= img; lp -= stride)
		for(cp = lp; cp < lp+12; cp += 3, j++)
			{
			pixels[j][0] = (int)cp[2];
			pixels[j][1] = (int)cp[1];
			pixels[j][2] = (int)cp[0];
			}

	sd = msvc_lbg_array(pixels, 16, col2, col1, index);
	if(*index & 0x8000)		/* high index bit must be zero */
		{
		tmp = *col1; *col1 = *col2; *col2 = tmp;
		*index ^= 0xffff;
		}

	return sd;
}


/* ------------------------------------------------------------------------ */
static double msvc_colour8(unsigned char *img, int stride, unsigned long cols[8], unsigned long *index)
{
unsigned char pixels[4][3];
unsigned long tmp, new_index;
int j;
double sd;
unsigned char *lp, *cp;

	for(j = 0, lp = img + 3 * stride; lp >= img + 2 * stride; lp -= stride)
		for(cp = lp; cp < lp+6; cp += 3, j++)
			{
			pixels[j][0] = (int)cp[2];
			pixels[j][1] = (int)cp[1];
			pixels[j][2] = (int)cp[0];
			}
	sd = msvc_lbg_array(pixels, 4, cols+1, cols, &new_index);
	*index = (new_index & 0x3) | ((new_index & 0xC) << 2);

	for(j = 0, lp = img + 3 * stride; lp >= img + 2 * stride; lp -= stride)
		for(cp = lp+6; cp < lp+12; cp += 3, j++)
			{
			pixels[j][0] = (int)cp[2];
			pixels[j][1] = (int)cp[1];
			pixels[j][2] = (int)cp[0];
			}
	sd += msvc_lbg_array(pixels, 4, cols+3, cols+2, &new_index);
	*index |= (((new_index & 0x3) | ((new_index & 0xC) << 2)) << 2);

	for(j = 0, lp = img + stride; lp >= img; lp -= stride)
		for(cp = lp; cp < lp+6; cp += 3, j++)
			{
			pixels[j][0] = (int)cp[2];
			pixels[j][1] = (int)cp[1];
			pixels[j][2] = (int)cp[0];
			}
	sd += msvc_lbg_array(pixels, 4, cols+5, cols+4, &new_index);
	*index |= (((new_index & 0x3) | ((new_index & 0xC) << 2)) << 8);

	for(j = 0, lp = img + stride; lp >= img; lp -= stride)
		for(cp = lp+6; cp < lp+12; cp += 3, j++)
			{
			pixels[j][0] = (int)cp[2];
			pixels[j][1] = (int)cp[1];
			pixels[j][2] = (int)cp[0];
			}
	sd += msvc_lbg_array(pixels, 4, cols+7, cols+6, &new_index);
	if(new_index & 0x08)		/* high index bit must be zero */
		{
		tmp = cols[7]; cols[7] = cols[6]; cols[6] = tmp;
		new_index ^= 0xf;
		}
	*index |= (((new_index & 0x3) | ((new_index & 0xC) << 2)) << 10);

	*cols = *cols | 0x8000;
	return sd;
}


/* ------------------------------------------------------------------------ */
static double msvc_block_diff(unsigned char *cur_img, int cur_stride, unsigned char *prev_img, int prev_stride)
{
double sd = 0;
int diff;
unsigned char *c_lp, *c_cp, *p_lp, *p_cp;

	for(c_lp = cur_img, p_lp = prev_img; c_lp < cur_img + 4 * cur_stride; c_lp += cur_stride, p_lp += prev_stride)
		for(c_cp = c_lp, p_cp = p_lp; c_cp < c_lp+12; c_cp += 3, p_cp += 4)
			{
			diff = (int)c_cp[2] - (int)p_cp[2];  sd += (double)(diff * diff);
			diff = (int)c_cp[1] - (int)p_cp[1];  sd += (double)(diff * diff);
			diff = (int)c_cp[0] - (int)p_cp[0];  sd += (double)(diff * diff);
			}
	return sd;
}


/* ------------------------------------------------------------------------
 * Initalise a new encoding of a set of frames using MSVC.
 *
 * Input:
 *   width - the width of the input frames
 *   height - the height of the input frames
 *   key_rate - the rate at which key frames are to be inserted
 * Output:
 *   returns a context which must be passed to the encoder
 */
void *msvc_encode_init(int width, int height, int key_rate)
{
msvc_enc_info *mi;
int i;

	if((mi = (msvc_enc_info *)calloc(sizeof(msvc_enc_info), 1)) == NULL) return NULL;
	if((mi->prev_img = (unsigned char *)calloc(width * height, 4)) == NULL)
		{
		free(mi);
		return NULL;
		}
	mi->width = width;
	mi->height = height;
	mi->key_rate = key_rate;
	mi->current_frame = mi->last_key = 0;
	for(i = 0; i < 4; i++) mi->mode_cnt[i] = 0;
	return (void *)mi;
}


/* ------------------------------------------------------------------------
 * This function encodes an RGB frame into a buffer using MSVC.
 *
 * Input:
 *   context - the context created using msvc_encode_init()
 *   frame - the input frame buffer (24-bit RGB format)
 *   width - the width of the input frame
 *   height - the height of the input frame
 *   quality - the quality this frame is to be encoded with (0=best to 100=bad)
 * Output:
 *   buf - the encoded output buffer to be saved in an AVI file
 *   size - the number of bytes put in the output buffer after encoding
 *   flags - the AVI flags the frame should be coded with (for the index)
 */
void msvc_encode(void *context, unsigned char *frame, int width,
	int height, int quality, unsigned char *buf, int *size, unsigned int *flags)
{
msvc_enc_info *mi = (msvc_enc_info *)context;
int i, x, y, block_cnt, done = 0, cmode, key_code = 0, num_skip;
unsigned long col1, col2a, col2b, col2_index, col8[8], col8_index;
double err[4], bits[4], lambda;
unsigned char *out_ptr, *img_ptr, *prev_img_ptr;
#define put_word(v) { *(out_ptr++) = (v) & 0xff; *(out_ptr++) = ((v) >> 8) & 0xff; }

	out_ptr = buf;

	lambda = (quality *  quality * quality)/10000.0;		/* a little more linear */

	if(width != mi->width || height != mi->height)
		{
		printf("MSVC ERROR: width and height have changed between init and encode!\n");
		return;
		}

	if(mi->current_frame == 0 || (mi->key_rate > 0 && mi->last_key >= mi->key_rate))
		{
		key_code = 1;
		mi->last_key = 0;
		}

	block_cnt = ((width * height) >> 4) + 1;
	done = 0;
	x = 0;
	y = height - 1;
	num_skip = 0;

	while(!done)
		{
		block_cnt--;
		if(!block_cnt || y < 0)
			{
			if(num_skip > 0) put_word(0x8400+num_skip);
			put_word(0);
			done = 1;
			continue;
			}

		img_ptr = frame + ((width * (y-3)) + x) * 3;
		prev_img_ptr = mi->prev_img + ((width * (y-3)) + x) * 4;

			/* Try coding using skip block */
		if(key_code) { err[0] = HUGE_VAL; bits[0] = 0; }
		else
			{
			err[0] = msvc_block_diff(img_ptr, width * 3, prev_img_ptr, width * 4);
			bits[0] = 16.0/(double)(num_skip + 1);
			}

			/* Try coding using 1 colour */
		err[1] = msvc_colour1(img_ptr, width * 3, &col1);
		bits[1] = 16;

			/* Try coding using 2 colours */
		err[2] = msvc_colour2(img_ptr, width * 3, &col2a, &col2b, &col2_index);
		bits[2] = 48;

			/* Try coding using 8 colours */
		err[3] = msvc_colour8(img_ptr, width * 3, col8, &col8_index);
		bits[3] = 144;

			/* select best coding type based on lambda = quality */
		cmode = 0;
		for(i = 0; i < 4; i++)
			{
			if(err[i] + lambda * bits[i] < err[cmode] + lambda * bits[cmode])
				cmode = i;
			}

			/* if not another skip block and skips pending: write skip */
		if((cmode != 0 && num_skip > 0) || num_skip >= 0x3FF)
			{
			put_word(0x8400+num_skip);
			num_skip = 0;
			}

			/* write encoded block and apply it to frame store */
		switch(cmode)
			{
			case 0: num_skip++; break;
			case 1:
				put_word(col1 | 0x8000);
				msvc_apply_block(prev_img_ptr, width * 4, col8, col1);
				break;
			case 2:
				put_word(col2_index);
				put_word(col2a);
				put_word(col2b);
				col8[0] = col2a; col8[1] = col2b;
				msvc_apply_block(prev_img_ptr, width * 4, col8, col2_index);
				break;
			case 3:
				put_word(col8_index);
				for(i = 0; i < 8; i++) put_word(col8[i]);
				msvc_apply_block(prev_img_ptr, width * 4, col8, col8_index);
				break;
			}

		mi->mode_cnt[cmode]++;
		x += 4;
		if(x >= width) { x=0; y -= 4; }
		}

	mi->current_frame++;
	mi->last_key++;

	*size = (out_ptr - buf);
	*flags = (key_code ? AVIIF_KEYFRAME : 0);
}


/* ------------------------------------------------------------------------
 * Free memory allocated when encoding a sequence.
 * Pass this function the context created by msvc_encode_init() and used
 * by msvc_encode().
 */
void msvc_encode_free(void *context)
{
msvc_enc_info *mi = (msvc_enc_info *)context;

	if(mi == NULL) return;

#if DBUG
	printf("MSVC modes - skip: %d  col1: %d  col2: %d  col8: %d\n", mi->mode_cnt[0], mi->mode_cnt[1],
		mi->mode_cnt[2], mi->mode_cnt[3]);
#endif

	if(mi->prev_img != NULL) free(mi->prev_img);
	free(mi);
}

/*** end vcodec.cpp ***/