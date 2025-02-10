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
	return 0;
}
