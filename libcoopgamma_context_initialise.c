/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int
libcoopgamma_context_initialise(libcoopgamma_context_t *restrict this)
{
	memset(this, 0, sizeof(*this));
	this->fd = -1;
	this->blocking = 1;
	return 0;
}
