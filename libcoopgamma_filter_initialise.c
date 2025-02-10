/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_filter_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int
libcoopgamma_filter_initialise(libcoopgamma_filter_t *restrict this)
{
	memset(this, 0, sizeof(*this));
	this->crtc = NULL;
	this->class = NULL;
	this->ramps.u8.red = NULL;
	this->ramps.u8.green = NULL;
	this->ramps.u8.blue = NULL;
	return 0;
}
