/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_filter_query_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_query_initialise`
 * or failed call to `libcoopgamma_filter_query_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_filter_query_destroy(libcoopgamma_filter_query_t *restrict this)
{
	free(this->crtc);
	this->crtc = NULL;
}
