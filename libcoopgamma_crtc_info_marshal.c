/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_crtc_info_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_crtc_info_marshal(const libcoopgamma_crtc_info_t *restrict this, void *restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_CRTC_INFO_VERSION);
	marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	marshal_version(LIBCOOPGAMMA_SUPPORT_VERSION);
	marshal_version(LIBCOOPGAMMA_COLOURSPACE_VERSION);
	marshal_prim(this->cooperative);
	marshal_prim(this->depth);
	marshal_prim(this->red_size);
	marshal_prim(this->green_size);
	marshal_prim(this->blue_size);
	marshal_prim(this->supported);
	marshal_prim(this->colourspace);
	marshal_prim(this->have_gamut);
	marshal_prim(this->red_x);
	marshal_prim(this->red_y);
	marshal_prim(this->green_x);
	marshal_prim(this->green_y);
	marshal_prim(this->blue_x);
	marshal_prim(this->blue_y);
	marshal_prim(this->white_x);
	marshal_prim(this->white_y);
	MARSHAL_EPILOGUE;
}
