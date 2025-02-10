/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_filter_query_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int
libcoopgamma_filter_query_initialise(libcoopgamma_filter_query_t *restrict this)
{
	this->crtc = NULL;
	this->coalesce = 0;
	this->high_priority = INT64_MAX;
	this->low_priority = INT64_MIN;
	return 0;
}
