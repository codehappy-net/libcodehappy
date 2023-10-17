/***

	Exported functions from this directory.

***/
#ifndef	FERGUSON_CODECS
#define FERGUSON_CODECS

/*********************************/
/*** Microsoft Video 1 encoder ***/
/*********************************/

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
extern void *msvc_encode_init(int width, int height, int key_rate);

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
extern void msvc_encode(void *context, unsigned char *frame, int width,
	int height, int quality, unsigned char *buf, int *size, unsigned int *flags);

/* ------------------------------------------------------------------------
 * Free memory allocated when encoding a sequence.
 * Pass this function the context created by msvc_encode_init() and used
 * by msvc_encode().
 */
extern void msvc_encode_free(void *context);

/******************************/
/*** Radius Cinepak decoder ***/
/******************************/

/* ------------------------------------------------------------------------
 * Call this function once at the start of the sequence and save the
 * returned context for calls to decode_cinepak().
 */
extern void *decode_cinepak_init(void);

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
extern void decode_cinepak(void *context, unsigned char *buf, int size, unsigned char *frame,
								int width, int height, int bit_per_pixel);

/**************************************/
/*** Creative YUV encoder & decoder ***/
/**************************************/

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
extern void cyuv_decode(unsigned char *buf, int size, unsigned char *frame, int width, int height, int bit_per_pixel);

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
extern void cyuv_encode(unsigned char *frame, int width, int height, unsigned char *buf, int *size, unsigned int *flags);

#endif  // FERGUSON_CODECS
