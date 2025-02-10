/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Initialise a `libcoopgamma_error_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int
libcoopgamma_error_initialise(libcoopgamma_error_t *restrict this)
{
	this->number = 0;
	this->custom = 0;
	this->description = NULL;
	return 0;
}
