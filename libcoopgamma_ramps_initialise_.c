/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t`
 * 
 * `this->red_size`, `this->green_size`, and `this->blue_size` must already be set
 * 
 * @param   this   The record to initialise
 * @param   width  The `sizeof(*(this->red))`
 * @return         Zero on success, -1 on error
 */
int
libcoopgamma_ramps_initialise_(void *restrict this, size_t width)
{
	libcoopgamma_ramps8_t *restrict this8 = (libcoopgamma_ramps8_t *restrict)this;
	this8->red = this8->green = this8->blue = NULL;
	this8->red = malloc((this8->red_size + this8->green_size + this8->blue_size) * width);
	if (!this8->red)
		return -1;
	this8->green = this8->red   + this8->red_size   * width;
	this8->blue  = this8->green + this8->green_size * width;
	return 0;
}
