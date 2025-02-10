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
	unmarshal_prim(this->cooperative);
	unmarshal_prim(this->depth);
	unmarshal_prim(this->red_size);
	unmarshal_prim(this->green_size);
	unmarshal_prim(this->blue_size);
	unmarshal_prim(this->supported);
	unmarshal_prim(this->colourspace);
	unmarshal_prim(this->have_gamut);
	unmarshal_prim(this->red_x);
	unmarshal_prim(this->red_y);
	unmarshal_prim(this->green_x);
	unmarshal_prim(this->green_y);
	unmarshal_prim(this->blue_x);
	unmarshal_prim(this->blue_y);
	unmarshal_prim(this->white_x);
	unmarshal_prim(this->white_y);
	UNMARSHAL_EPILOGUE;
}
