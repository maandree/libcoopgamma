/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_async_context_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_async_context_initialise`
 * or failed call to `libcoopgamma_async_context_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_async_context_destroy(libcoopgamma_async_context_t *restrict this)
{
	(void) this;
}
