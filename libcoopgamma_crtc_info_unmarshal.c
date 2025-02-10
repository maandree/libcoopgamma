/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_crtc_info_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_crtc_info_unmarshal(libcoopgamma_crtc_info_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	UNMARSHAL_PROLOGUE;
	unmarshal_version(LIBCOOPGAMMA_CRTC_INFO_VERSION);
	unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	unmarshal_version(LIBCOOPGAMMA_SUPPORT_VERSION);
	unmarshal_version(LIBCOOPGAMMA_COLOURSPACE_VERSION);
	unmarshal_prim(this->cooperative, int);
	unmarshal_prim(this->depth, libcoopgamma_depth_t);
	unmarshal_prim(this->red_size, size_t);
	unmarshal_prim(this->green_size, size_t);
	unmarshal_prim(this->blue_size, size_t);
	unmarshal_prim(this->supported, libcoopgamma_support_t);
	unmarshal_prim(this->colourspace, libcoopgamma_colourspace_t);
	unmarshal_prim(this->have_gamut, int);
	unmarshal_prim(this->red_x, unsigned);
	unmarshal_prim(this->red_y, unsigned);
	unmarshal_prim(this->green_x, unsigned);
	unmarshal_prim(this->green_y, unsigned);
	unmarshal_prim(this->blue_x, unsigned);
	unmarshal_prim(this->blue_y, unsigned);
	unmarshal_prim(this->white_x, unsigned);
	unmarshal_prim(this->white_y, unsigned);
	UNMARSHAL_EPILOGUE;
}
