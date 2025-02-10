/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   vbuf    The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   width  The `sizeof(*(this->red))`
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_ramps_marshal_(const void *restrict this, void *restrict vbuf, size_t width)
{
	const libcoopgamma_ramps8_t *restrict this8 = (const libcoopgamma_ramps8_t *restrict)this;
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_RAMPS_VERSION);
	marshal_prim(this8->red_size, size_t);
	marshal_prim(this8->green_size, size_t);
	marshal_prim(this8->blue_size, size_t);
	marshal_buffer(this8->red, (this8->red_size + this8->green_size + this8->blue_size) * width);
	MARSHAL_EPILOGUE;
}
