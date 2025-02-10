/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_queried_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_queried_filter_initialise`
 * or failed call to `libcoopgamma_queried_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_queried_filter_destroy(libcoopgamma_queried_filter_t *restrict this)
{
	free(this->class);
	this->class = NULL;
	libcoopgamma_ramps_destroy(&this->ramps.u8);
}
