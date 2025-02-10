/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_async_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int
libcoopgamma_async_context_initialise(libcoopgamma_async_context_t *restrict this)
{
	this->message_id = 0;
	this->coalesce = 0;
	return 0;
}
