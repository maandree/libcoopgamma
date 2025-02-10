/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_initialise`
 * or failed call to `libcoopgamma_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_filter_destroy(libcoopgamma_filter_t *restrict this)
{
	free(this->crtc);
	free(this->class);
	free(this->ramps.u8.red);
	memset(this, 0, sizeof(*this));
	this->crtc = NULL;
	this->class = NULL;
	this->ramps.u8.red = NULL;
	this->ramps.u8.green = NULL;
	this->ramps.u8.blue = NULL;
}
