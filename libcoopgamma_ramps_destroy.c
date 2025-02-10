/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`,
 * `libcoopgamma_ramps32_t`, `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`,
 * `libcoopgamma_rampsd_t`, or `libcoopgamma_ramps_t`, the allocation of the record
 * itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_ramps_initialise`
 * or failed call to `libcoopgamma_ramps_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_ramps_destroy(void *restrict this)
{
	libcoopgamma_ramps8_t *restrict this8 = (libcoopgamma_ramps8_t *restrict)this;
	free(this8->red);
	this8->red = this8->green = this8->blue = NULL;
}
