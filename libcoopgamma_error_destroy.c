/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_error_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_error_initialise`
 * or failed call to `libcoopgamma_error_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_error_destroy(libcoopgamma_error_t *restrict this)
{
	free(this->description);
	this->description = NULL;
}
