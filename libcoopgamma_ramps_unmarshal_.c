/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` from a buffer
 * 
 * @param   this   The output parameter for unmarshalled record
 * @param   vbuf   The buffer with the marshalled record
 * @param   np     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @param   width  The `sizeof(*(this->red))`
 * @return         `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                 `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_ramps_unmarshal_(void *restrict this, const void *restrict vbuf,
                              size_t *restrict np, size_t width)
{
	libcoopgamma_ramps8_t *restrict this8 = (libcoopgamma_ramps8_t *restrict)this;
	UNMARSHAL_PROLOGUE;
	unmarshal_version(LIBCOOPGAMMA_RAMPS_VERSION);
	unmarshal_prim(this8->red_size, size_t);
	unmarshal_prim(this8->green_size, size_t);
	unmarshal_prim(this8->blue_size, size_t);
	unmarshal_buffer(this8->red, (this8->red_size + this8->green_size + this8->blue_size) * width);
	this8->green = this8->red + this8->red_size * width;
	this8->blue = this8->green + this8->green_size * width;
	UNMARSHAL_EPILOGUE;
}
